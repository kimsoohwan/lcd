#!/bin/bash
# The following configuration file contains all the paths and variables used by
# the functions and the scripts in the package. Edit it before generating new
# data. The meaning of each variable is as follows:
#
# * SCENENET_DATASET_PATH: path where the SceneNetRGBD dataset is located. It
#   should be such that the dataset is in its subfolder 'data' (e.g. the
#   training set should be in $SCENENET_DATASET_PATH/data/train).
# * SCENENN_DATASET_PATH: path where the SceneNN dataset is located. It should
#   contain a subfolder XYZ for each scene (where XYZ is a three-digit ID
#   associated to the scene, e.g. 005, cf. https://github.com/scenenn/scenenn)
#   and a subfolder 'intrinsic'.
# * SCENENET_SCRIPTS_PATH: path where the pySceneNetRGBD scripts (in
#   particular scenenet_pb2.py) are located. It should in principle coincide
#   with $SCENENET_DATASET_PATH, but it might differ in case the dataset is
#   stored in a separate folder.
# * SCENENN_SCRIPTS_PATH: path where the SceneNN scripts (in
#   particular scenenet_pb2.py) are located. It should in principle coincide
#   with $SCENENET_DATASET_PATH, but it might differ in case the dataset is
#   stored in a separate folder.
# * BAGFOLDER_PATH: path where to store the ROS bags generated by the script
#   scenenet_to_rosbag.py (or where ROS bags are already stored).
# * LINESANDIMAGESFOLDER_PATH: path where to store the output data. In
#   particular, the lines files produced by the detector are stored under
#   LINESANDIMAGESFOLDER_PATH/$DATASET_NAME_lines, whereas the virtual camera
#   images are stored under LINESANDIMAGESFOLDER_PATH/$DATASET_NAME.
# * PYTHONSCRIPTS_PATH: path that contains the line_tools Python scripts.
# * TARFILES_PATH: path where to store the archive files containing the text
#   lines files and the virtual camera images.
# * PICKLEANDSPLIT_PATH: path where to store pickle files and text files
#   indicating the splitting of the dataset.
# * TRAJ_NUM: index (in the dataset identified by the protobuf file) of the
#   trajectory for which to generate the data. NOTE: it should be used only for
#   the purpose of generating the data rather than processing it (e.g., it
#   should not be used during training or visualization), because more than one
#   trajectory might need to be considered at the same time in the latter case.
# * DATASET_NAME: identifies the dataset from which the data being processed
#   (i.e. the images from which we are extracting lines and generating bags)
#   comes from. Valid values are 'val' and 'train_0', indicating respectively
#   the validation set and the index-0 training set in SceneNetRGBD, but all
#   other values that find a correspondence in the protobuf 'database' (in
#   config_protobuf_paths) can be used.

# *** The following paths are independent of the specific data generation task
#     being carried out (i.e., on the trajectory number and the dataset from
#     which to extract the images) ***
SCENENET_DATASET_PATH=~/line_tools_data/pySceneNetRGBD/
SCENENN_DATASET_PATH=~/line_tools_data/scenenn/download/scenenn_data
SCENENET_SCRIPTS_PATH=~/catkin_ws/src/pySceneNetRGBD/
BAGFOLDER_PATH=~/line_tools_data/bags/
LINESANDIMAGESFOLDER_PATH=~/catkin_extended_ws/src/line_tools/data/
PYTHONSCRIPTS_PATH=~/catkin_extended_ws/src/line_tools/python/
TARFILES_PATH=~/line_tools_data/tar_files/
PICKLEANDSPLIT_PATH=~/line_tools_data/pickle_files/

# *** The following two variables are specific of the data generation task being
#     carried out ***
TRAJ_NUM=1
DATASET_NAME=train_0