import tensorflow as tf


def debug_metrics(tensor):
    def d_sum(y_true, y_pred):
        return tf.reduce_sum(tensor, axis=-1)

    def d_l1(y_true, y_pred):
        return tf.reduce_sum(tf.abs(tensor), axis=-1)

    def d_std(y_true, y_pred):
        return tf.math.reduce_std(tensor, axis=-1)

    def d_max(y_true, y_pred):
        return tf.reduce_max(tensor, axis=-1)

    def d_min(y_true, y_pred):
        return tf.reduce_min(tensor, axis=-1)

    return [d_sum, d_l1, d_std, d_max, d_min]


def iou_metric(labels, unique_labels, cluster_counts, bg_mask, valid_mask, max_clusters):
    def iou(y_true, y_pred):
        mask = tf.logical_and(tf.logical_not(bg_mask), valid_mask)
        mask = tf.expand_dims(tf.expand_dims(mask, axis=-1), axis=-1)

        gt_labels = tf.expand_dims(tf.expand_dims(labels, axis=-1), axis=-1)
        unique_gt_labels = tf.expand_dims(tf.expand_dims(unique_labels, axis=1), axis=-1)
        pred_labels = tf.expand_dims(tf.expand_dims(tf.argmax(y_pred, axis=-1, output_type=tf.dtypes.int32),
                                                    axis=-1), axis=-1)
        unique_pred_labels = tf.expand_dims(tf.expand_dims(tf.expand_dims(tf.range(0, 15, dtype='int32'),
                                                                          axis=0), axis=0), axis=0)

        gt_matrix = tf.equal(gt_labels, unique_gt_labels)
        pred_matrix = tf.equal(pred_labels, unique_pred_labels)

        intersections = tf.cast(tf.logical_and(tf.logical_and(gt_matrix, pred_matrix), mask), dtype='float32')

        unions = tf.cast(tf.logical_and(tf.logical_or(gt_matrix, pred_matrix), mask), dtype='float32')
        intersections = tf.reduce_sum(intersections, axis=1)
        unions = tf.reduce_sum(unions, axis=1)

        iou_out = tf.reduce_max(tf.math.divide_no_nan(intersections, unions), axis=-1)
        iou_out = tf.reduce_sum(iou_out, axis=-1, keepdims=True) / tf.cast(cluster_counts, dtype='float32')

        return tf.reduce_mean(iou_out)

    return iou


def get_kl_losses_and_metrics(instancing_tensor, labels_tensor, validity_mask, bg_mask, num_lines):
    h_labels = tf.expand_dims(labels_tensor, axis=-1)
    v_labels = tf.transpose(h_labels, perm=(0, 2, 1))

    mask_equal = tf.equal(h_labels, v_labels)
    mask_not_equal = tf.not_equal(h_labels, v_labels)

    h_bg = tf.expand_dims(tf.logical_not(bg_mask), axis=-1)
    v_bg = tf.transpose(h_bg, perm=(0, 2, 1))
    mask_not_bg = tf.logical_and(h_bg, v_bg)

    h_val = tf.expand_dims(validity_mask, axis=-1)
    v_val = tf.transpose(h_val, perm=(0, 2, 1))
    mask_val = tf.logical_and(h_val, v_val)
    mask_val = tf.linalg.set_diag(mask_val, tf.zeros(tf.shape(mask_val)[0:-1], dtype='bool'))

    loss_mask = tf.logical_and(mask_val, mask_not_bg)

    num_valid = tf.reduce_sum(tf.cast(loss_mask, dtype='float32'), axis=(1, 2), keepdims=True)

    def kl_cross_div_loss(y_true, y_pred):
        extended_pred = tf.expand_dims(instancing_tensor, axis=2)
        h_pred = extended_pred  # K.permute_dimensions(extended_pred, (0, 1, 2, 3))
        v_pred = tf.transpose(extended_pred, perm=(0, 2, 1, 3))
        d = h_pred * tf.math.log(tf.math.divide_no_nan(h_pred, v_pred + 1e-100) + 1e-100)
        d = tf.reduce_sum(d, axis=-1, keepdims=False)

        equal_loss = tf.where(tf.logical_and(mask_equal, loss_mask), d, 0.)
        not_equal_loss = tf.where(tf.logical_and(mask_not_equal, loss_mask),
                                  tf.maximum(0., 2.0 - d), 0.)
        return tf.math.divide_no_nan((equal_loss + not_equal_loss), num_valid) * 150. * 150.

    pred_labels = tf.argmax(instancing_tensor, axis=-1)
    h_pred_labels = tf.expand_dims(pred_labels, axis=-1)
    v_pred_labels = tf.transpose(h_pred_labels, perm=(0, 2, 1))

    pred_equals = tf.equal(h_pred_labels, v_pred_labels)
    pred_not_equals = tf.not_equal(h_pred_labels, v_pred_labels)

    true_p = tf.cast(tf.logical_and(pred_equals, tf.logical_and(loss_mask, mask_equal)), dtype='float32')
    true_p = tf.reduce_sum(true_p, axis=(1, 2))

    true_n = tf.cast(tf.logical_and(pred_not_equals, tf.logical_and(loss_mask, mask_not_equal)), dtype='float32')
    true_n = tf.reduce_sum(true_n, axis=(1, 2))

    gt_p = tf.cast(tf.logical_and(loss_mask, mask_equal), dtype='float32')
    gt_p = tf.reduce_sum(gt_p, axis=(1, 2))

    gt_n = tf.cast(tf.logical_and(loss_mask, mask_not_equal), dtype='float32')
    gt_n = tf.reduce_sum(gt_n, axis=(1, 2))

    pred_p = tf.cast(tf.logical_and(pred_equals, loss_mask), dtype='float32')
    pred_p = tf.reduce_sum(pred_p, axis=(1, 2))

    pred_n = tf.cast(tf.logical_and(pred_not_equals, loss_mask), dtype='float32')
    pred_n = tf.reduce_sum(pred_n, axis=(1, 2))

    def tp_gt_p(y_true, y_pred):
        return tf.reduce_mean(tf.math.divide_no_nan(true_p, gt_p))

    def tn_gt_n(y_true, y_pred):
        return tf.reduce_mean(tf.math.divide_no_nan(true_n, gt_n))

    def tp_pd_p(y_true, y_pred):
        return tf.reduce_mean(tf.math.divide_no_nan(true_p, pred_p))

    def tn_pd_n(y_true, y_pred):
        return tf.reduce_mean(tf.math.divide_no_nan(true_n, pred_n))

    return kl_cross_div_loss, [tp_gt_p, tn_gt_n, tp_pd_p, tn_pd_n]