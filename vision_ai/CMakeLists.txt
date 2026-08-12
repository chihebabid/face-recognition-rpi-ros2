cmake_minimum_required(VERSION 3.16)
project(vision_ai)


if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options( -Wall -Wextra -Wpedantic)
endif()

#SET(CMAKE_SYSTEM_NAME Linux)
#SET(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
#SET(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-gcc)

set(ORT_HEADER_DIR "${rootfs_dir}/opt/onnxruntime/include")
set(ORT_LIBRARY_DIR "${rootfs_dir}/opt/onnxruntime/lib")
#############################################################################
############################################
# Find the path to the pigpio includes.
find_path(pigpio_INCLUDE_DIR
        NAMES pigpio.h pigpiod_if.h pigpiod_if2.h
        HINTS ${rootfs_dir}/usr/local/include)

# Find the pigpio libraries.
find_library(pigpio_LIBRARY
        NAMES libpigpio.so
        HINTS ${rootfs_dir}/usr/local/lib)
find_library(pigpiod_if_LIBRARY
        NAMES libpigpiod_if.so
        HINTS ${rootfs_dir}/usr/local/lib)
find_library(pigpiod_if2_LIBRARY
        NAMES libpigpiod_if2.so
        HINTS ${rootfs_dir}/usr/local/lib)
########################################################################
#find_library(cv_bridge_LIBRARY
#        NAMES libcv_bridge.so
 #       HINTS ${rootfs_dir}/opt/ros/humble/lib)
########################################################################

# Set the pigpio variables to plural form to make them accessible for
# the paramount cmake modules.
set(pigpio_INCLUDE_DIRS ${pigpio_INCLUDE_DIR})
set(pigpio_INCLUDES     ${pigpio_INCLUDE_DIR})

# Handle REQUIRED, QUIET, and version arguments
# and set the <packagename>_FOUND variable.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pigpio
        DEFAULT_MSG
        pigpio_INCLUDE_DIR pigpio_LIBRARY pigpiod_if_LIBRARY pigpiod_if2_LIBRARY)
################# END PIGPIO INCLUSION ##################################################

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_target_dependencies REQUIRED) # starting from ROS2 lyrical
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(image_transport REQUIRED)

############## Find OpenCV
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(LIBCAMERA REQUIRED libcamera)

# libcamera
find_path(LIBCAMERA_INCLUDE_DIR
        NAMES libcamera/libcamera.h
        PATHS
        /usr/include
        /usr/include/libcamera
        /opt/ros2rootfs/usr/include
        /opt/ros2rootfs/usr/include/libcamera
)

find_library(LIBCAMERA_LIBRARY
        NAMES camera
        PATHS
        /opt/ros2rootfs/usr/lib/aarch64-linux-gnu
)

if(NOT LIBCAMERA_INCLUDE_DIR)
  message(FATAL_ERROR "libcamera headers not found")
endif()

if(NOT LIBCAMERA_LIBRARY)
  message(FATAL_ERROR "libcamera library not found")
else()
  message("libcamera library found ${LIBCAMERA_LIBRARY}")
endif()


include_directories(include include/vision_ai ${LIBCAMERA_INCLUDE_DIR})

add_executable(visionai src/camera.cpp src/cam_detect_sub.cpp src/controlservonode.cpp src/person_database.cpp
  src/arc_face_model.cpp src/yolo_detector.cpp src/core_detector.cpp)

target_include_directories(visionai PRIVATE ${ORT_HEADER_DIR}  ${OpenCV_INCLUDE_DIRS}
)
target_link_directories(visionai PRIVATE ${ORT_LIBRARY_DIR})
target_link_libraries(visionai PRIVATE pigpiod_if2 onnxruntime camera-base ${LIBCAMERA_LIBRARY})

target_link_libraries(visionai PUBLIC rclcpp::rclcpp std_msgs::std_msgs sensor_msgs::sensor_msgs  rmw_implementation::rmw_implementation ${OpenCV_LIBS}
        cv_bridge::cv_bridge  image_transport::image_transport pigpio)

# config for ROS2 cli tools
install(TARGETS visionai  DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY models face_db DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
