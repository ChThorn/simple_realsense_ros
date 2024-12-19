# cmake_minimum_required(VERSION 3.8)
# project(simple_realsenseROS)

# if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#   add_compile_options(-Wall -Wextra -Wpedantic)
# endif()

# # find dependencies
# find_package(ament_cmake REQUIRED)
# find_package(rclcpp REQUIRED)

# if(BUILD_TESTING)
#   find_package(ament_lint_auto REQUIRED)
#   # the following line skips the linter which checks for copyrights
#   # comment the line when a copyright and license is added to all source files
#   set(ament_cmake_copyright_FOUND TRUE)
#   # the following line skips cpplint (only works in a git repo)
#   # comment the line when this package is in a git repo and when
#   # a copyright and license is added to all source files
#   set(ament_cmake_cpplint_FOUND TRUE)
#   ament_lint_auto_find_test_dependencies()
# endif()

# ament_package()

cmake_minimum_required(VERSION 3.10)
project(simple_realsense_ros)

# Make sure these are at the top
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Qt5 COMPONENTS Widgets Core REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED) # add this
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(realsense2 REQUIRED)
find_package(OpenCV REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# List all source files
set(realsense_viewer_SRC
    src/main.cpp
    src/viewer.cpp
    include/simple_realsense_ros/viewer.h
)

# Create executable
add_executable(realsense_viewer ${realsense_viewer_SRC})


# Add dependencies before linking
ament_target_dependencies(realsense_viewer
    "rclcpp"
    "sensor_msgs"
    "geometry_msgs"
    "cv_bridge"
    "image_transport"
)

# Link libraries
target_link_libraries(realsense_viewer
    Qt5::Widgets
    Qt5::Core
    ${realsense2_LIBRARY}
    ${OpenCV_LIBS}
)

#---------------------
# Depth Subscriber Node
add_executable(depth_subscriber
    src/depth_subscriber.cpp
)


ament_target_dependencies(depth_subscriber
    "rclcpp"
    "sensor_msgs"
    "geometry_msgs"
)

target_link_libraries(depth_subscriber
    ${OpenCV_LIBS}
)
#------------------------

# Install targets
install(TARGETS realsense_viewer 
    depth_subscriber
    DESTINATION lib/${PROJECT_NAME}
)

# Install include directory
install(DIRECTORY include/
    DESTINATION include
)

ament_package()