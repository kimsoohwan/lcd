#include <glog/logging.h>
#include <gtest/gtest.h>
#include <Eigen/Core>

#include "line_detection/common.h"
#include "line_detection/line_detection.h"
#include "line_detection/test/testing-entrypoint.h"

namespace line_detection {

class LineDetectionTest : public ::testing::Test {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

 protected:
  LineDetectionTest() { SetUp(); }

  virtual ~LineDetectionTest() {}

  cv::Mat test_image_;
  cv::Mat test_img_gray_;
  cv::Mat test_depth_load_;
  cv::Mat test_depth_;
  line_detection::LineDetector line_detector_;
  std::vector<cv::Vec4f> lines_;

  virtual void SetUp() {
    // Load the test image and compute a grayscale version of it.
    std::string testimage_path("test_data/hall.jpg");
    test_image_ = cv::imread(testimage_path, CV_LOAD_IMAGE_COLOR);
    cv::cvtColor(test_image_, test_img_gray_, CV_BGR2GRAY);
    // Load the depth data corresponding to the test image.
    std::string testdepth_path("test_data/hall_depth.png");
    test_depth_load_ = cv::imread(testdepth_path, CV_LOAD_IMAGE_UNCHANGED);
    if (test_depth_load_.type() != CV_16UC1)
      test_depth_load_.convertTo(test_depth_, CV_16UC1);
    else
      test_depth_ = test_depth_load_;
  }
};

TEST_F(LineDetectionTest, testLSDLineDetection) {
  size_t n_lines;
  // Calling the detector with LSD.
  line_detector_.detectLines(test_img_gray_, line_detection::Detector::LSD,
                             lines_);
  n_lines = lines_.size();
  EXPECT_EQ(n_lines, 716)
      << "LSD detection: Expected 84 lines to be found. Found " << n_lines;
}
TEST_F(LineDetectionTest, testEDLLineDetection) {
  size_t n_lines;
  // Calling the detector with EDL.
  line_detector_.detectLines(test_img_gray_, line_detection::Detector::EDL,
                             lines_);
  n_lines = lines_.size();
  EXPECT_EQ(n_lines, 172)
      << "EDL detection: Expected 18 lines to be found. Found " << n_lines;
}
TEST_F(LineDetectionTest, testFASTLineDetection) {
  size_t n_lines;
  // Calling the detector with FAST.
  line_detector_.detectLines(test_img_gray_, line_detection::Detector::FAST,
                             lines_);
  n_lines = lines_.size();
  EXPECT_EQ(n_lines, 598)
      << "Fast detection: Expected 70 lines to be found. Found " << n_lines;
}
TEST_F(LineDetectionTest, testHoughLineDetection) {
  size_t n_lines;
  // Calling the detector with HOUGH.
  line_detector_.detectLines(test_img_gray_, line_detection::Detector::HOUGH,
                             lines_);
  n_lines = lines_.size();
  EXPECT_EQ(n_lines, 165)
      << "HOUGH detection: Expected 16 lines to be found. Found " << n_lines;
}

TEST_F(LineDetectionTest, testComputePointCloud) {
  // Create calibration matrix and fill it (with non calibrated values!).
  cv::Mat K(3, 3, CV_32FC1);
  K.at<float>(0, 0) = 570.3f;
  K.at<float>(0, 1) = 0.0f;
  K.at<float>(0, 2) = 960.0f;
  K.at<float>(1, 0) = 0.0f;
  K.at<float>(1, 1) = 570.3f;
  K.at<float>(1, 2) = 540.0f;
  K.at<float>(2, 0) = 0.0f;
  K.at<float>(2, 1) = 0.0f;
  K.at<float>(2, 2) = 1.0f;
  // Point_cloud to be filled.
  pcl::PointCloud<pcl::PointXYZRGB> point_cloud;
  // Fill the point cloud (this is the functioned that is tested here)
  line_detector_.computePointCloud(test_image_, test_depth_, K, point_cloud);
  // Compue the mean of all entries of the point cloud
  double x_mean = 0;
  double y_mean = 0;
  double z_mean = 0;
  double r_mean = 0;
  double g_mean = 0;
  double b_mean = 0;
  for (int i = 0; i < point_cloud.size(); i++) {
    if (std::isnan(point_cloud.points[i].x)) continue;
    x_mean += point_cloud.points[i].x;
    y_mean += point_cloud.points[i].y;
    z_mean += point_cloud.points[i].z;
    r_mean += point_cloud.points[i].r;
    g_mean += point_cloud.points[i].g;
    b_mean += point_cloud.points[i].b;
  }
  x_mean = x_mean / point_cloud.size();
  y_mean = y_mean / point_cloud.size();
  z_mean = z_mean / point_cloud.size();
  r_mean = r_mean / point_cloud.size();
  g_mean = g_mean / point_cloud.size();
  b_mean = b_mean / point_cloud.size();

  // The function LineDetector::computePointCloud computes an ordered point
  // cloud. It does fill in points for which the depth image no information,
  // these are then just NaN values. But this means for every pixel there should
  // be a point in the cloud.
  EXPECT_EQ(point_cloud.size(), 1920 * 1080);
  // These are all values that were precomputed with the above calibration
  // matrix K. They are not the true values!
  EXPECT_NEAR(x_mean, 0.324596, 1e-5);
  EXPECT_NEAR(y_mean, -0.147148, 1e-5);
  EXPECT_NEAR(z_mean, 1.69212, 1e-5);
  EXPECT_NEAR(r_mean, 108.686, 1e-2);
  EXPECT_NEAR(g_mean, 117.155, 1e-2);
  EXPECT_NEAR(b_mean, 116.337, 1e-2);
}

TEST_F(LineDetectionTest, testAreLinesEqual2D) {
  EXPECT_TRUE(line_detection::areLinesEqual2D(cv::Vec4f(0, 0, 10, 10),
                                              cv::Vec4f(0, 0, 10, 10)));
  EXPECT_TRUE(line_detection::areLinesEqual2D(cv::Vec4f(0, 0, 10, 10),
                                              cv::Vec4f(10, 10, 30, 30)));
  EXPECT_FALSE(line_detection::areLinesEqual2D(cv::Vec4f(0, 0, 10, 10),
                                               cv::Vec4f(0, 0, 0, 10)));
}

TEST_F(LineDetectionTest, testCheckInBoundary) {
  EXPECT_EQ(line_detection::checkInBoundary(1, 0, 3), 1);
  EXPECT_EQ(line_detection::checkInBoundary(-1, 0, 3), 0);
  EXPECT_EQ(line_detection::checkInBoundary(10, 0, 3), 3);
}

TEST_F(LineDetectionTest, testCrossProdcut) {
  EXPECT_EQ(
      line_detection::crossProduct(cv::Vec3f(1, 0, 0), cv::Vec3f(0, 1, 0)),
      cv::Vec3f(0, 0, 1));
}

TEST_F(LineDetectionTest, testComputeDistPointToLine3D) {
  EXPECT_EQ(line_detector_.computeDistPointToLine3D(
                cv::Vec3f(0, 0, 0), cv::Vec3f(1, 0, 0), cv::Vec3f(0, 1, 0)),
            1);
}
}  // namespace line_detection

LINE_DETECTION_TESTING_ENTRYPOINT
