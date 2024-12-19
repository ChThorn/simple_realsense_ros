#ifndef VIEWER_H
#define VIEWER_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <librealsense2/rs.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.hpp>

class Viewer : public QMainWindow, public rclcpp::Node
{
    Q_OBJECT

public:
    explicit Viewer(QWidget *parent = nullptr);
    virtual ~Viewer();

protected:
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void processFrames();
    void handleStartButton();
    void spinROS();

private:
    void setupUi();
    void setupROS();
    void setupCamera();
    QImage convertColorToQImage(const rs2::frame& frame);
    void publishColorFrame(const rs2::frame& frame);
    void publishDepthFrame(const rs2::frame& frame);
    QImage drawPointOnImage(QImage image, QPoint point, QColor color);
    void updateDepthInfo(int x, int y);

    // Qt UI elements
    QLabel* colorView;
    QLabel* depthView;
    QPushButton* startButton;
    QTimer* frameTimer;
    QTimer* rosTimer;

    // RealSense objects
    rs2::pipeline pipe;
    rs2::config cfg;
    bool isRunning;
    
    // Depth measurement
    QPoint lastClickedPoint;
    float lastDepthValue;

    // ROS2 publishers
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr color_pub_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr depth_pub_;
    rclcpp::Publisher<geometry_msgs::msg::Point>::SharedPtr point_pub_;
};

#endif // VIEWER_H