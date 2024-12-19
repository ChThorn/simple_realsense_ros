#include <QApplication>
#include <rclcpp/rclcpp.hpp>
#include "simple_realsense_ros/viewer.h"

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    QApplication app(argc, argv);
    
    Viewer w;
    w.show();
    
    int result = app.exec();
    rclcpp::shutdown();
    return result;
}