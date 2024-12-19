#include "simple_realsense_ros/viewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QMouseEvent>
#include <opencv2/opencv.hpp>

Viewer::Viewer(QWidget *parent)
    : QMainWindow(parent)
    , rclcpp::Node("realsense_viewer")
    , isRunning(false)
    , lastDepthValue(0.0f)
{
    setupUi();
    setupROS();
    setupCamera();
}

Viewer::~Viewer()
{
    if (isRunning) {
        pipe.stop();
    }
}

// void Viewer::setupUi()
// {
//     QWidget* central = new QWidget;
//     setCentralWidget(central);

//     QVBoxLayout* mainLayout = new QVBoxLayout(central);

//     // Top row - Color and Depth views
//     QHBoxLayout* topRowLayout = new QHBoxLayout;
    
//     // Color view
//     colorView = new QLabel("Color Feed");
//     colorView->setMinimumSize(640, 480);
//     colorView->setAlignment(Qt::AlignCenter);
    
//     // Depth view
//     depthView = new QLabel("Depth Feed");
//     depthView->setMinimumSize(640, 480);
//     depthView->setAlignment(Qt::AlignCenter);
    
//     topRowLayout->addWidget(colorView);
//     topRowLayout->addWidget(depthView);

//     // Bottom row - Subscribed depth view and info
//     QHBoxLayout* bottomRowLayout = new QHBoxLayout;
    
//     // Subscribed depth view
//     subscribedDepthView = new QLabel("Subscribed Depth Feed");
//     subscribedDepthView->setMinimumSize(640, 480);
//     subscribedDepthView->setAlignment(Qt::AlignCenter);
    
//     // Depth information panel
//     QVBoxLayout* infoLayout = new QVBoxLayout;
//     depthInfoLabel = new QLabel("Depth Information:\nClick on image to measure depth");
//     depthInfoLabel->setMinimumWidth(300);
//     depthInfoLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
//     depthInfoLabel->setStyleSheet("QLabel { background-color : white; padding: 10px; }");
    
//     startButton = new QPushButton("Start Camera");
    
//     infoLayout->addWidget(depthInfoLabel);
//     infoLayout->addWidget(startButton);
//     infoLayout->addStretch();

//     bottomRowLayout->addWidget(subscribedDepthView);
//     bottomRowLayout->addLayout(infoLayout);

//     // Add all layouts to main layout
//     mainLayout->addLayout(topRowLayout);
//     mainLayout->addLayout(bottomRowLayout);

//     connect(startButton, &QPushButton::clicked, this, &Viewer::handleStartButton);

//     setWindowTitle("RealSense ROS2 Viewer");
//     resize(1350, 1000);
// }

void Viewer::setupUi()
{
    QWidget* central = new QWidget;
    setCentralWidget(central);

    QHBoxLayout* mainLayout = new QHBoxLayout(central);

    // Left side - Color view
    QVBoxLayout* leftLayout = new QVBoxLayout;
    colorView = new QLabel("Color Feed");
    colorView->setMinimumSize(640, 480);
    colorView->setAlignment(Qt::AlignCenter);
    
    // Right side - Depth view
    QVBoxLayout* rightLayout = new QVBoxLayout;
    depthView = new QLabel("Depth Feed");
    depthView->setMinimumSize(640, 480);
    depthView->setAlignment(Qt::AlignCenter);

    // Controls
    startButton = new QPushButton("Start Camera");
    connect(startButton, &QPushButton::clicked, this, &Viewer::handleStartButton);
    
    leftLayout->addWidget(colorView);
    rightLayout->addWidget(depthView);
    
    QVBoxLayout* controlLayout = new QVBoxLayout;
    controlLayout->addWidget(startButton);

    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);
    mainLayout->addLayout(controlLayout);

    setWindowTitle("RealSense ROS2 Viewer");
    resize(1350, 500);
}


// void Viewer::mousePressEvent(QMouseEvent* event)
// {
//     QPoint pos = event->pos();
//     QWidget* widget = childAt(pos);

//     if (widget == subscribedDepthView) {
//         // Convert to widget-relative coordinates
//         QPoint localPos = subscribedDepthView->mapFrom(this, pos);
        
//         // Update the last clicked point
//         lastClickedPoint = localPos;
        
//         // Update depth information
//         updateDepthInfo(localPos.x(), localPos.y());
//     }
// }

void Viewer::mousePressEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();
    QWidget* widget = childAt(pos);

    if (widget == depthView) {
        // Convert to widget-relative coordinates
        QPoint localPos = depthView->mapFrom(this, pos);
        lastClickedPoint = localPos;
        
        // Create and publish the point message
        auto point_msg = std::make_unique<geometry_msgs::msg::Point>();
        point_msg->x = localPos.x();
        point_msg->y = localPos.y();
        point_msg->z = 0.0;  // Not used
        point_pub_->publish(std::move(point_msg));
        
        // Update the visual feedback
        QPixmap currentPixmap = depthView->pixmap(Qt::ReturnByValue);
        QImage image = currentPixmap.toImage();
        depthView->setPixmap(QPixmap::fromImage(drawPointOnImage(image, localPos, Qt::red)));
        
        updateDepthInfo(localPos.x(), localPos.y());
    }
}

// void Viewer::updateDepthInfo(int x, int y)
// {
//     if (!currentDepthMsg || !isRunning) return;

//     // Calculate the position in the depth image
//     int width = currentDepthMsg->width;
//     int height = currentDepthMsg->height;
    
//     // Scale coordinates to image dimensions
//     int scaledX = (x * width) / subscribedDepthView->width();
//     int scaledY = (y * height) / subscribedDepthView->height();
    
//     // Ensure coordinates are within bounds
//     if (scaledX >= 0 && scaledX < width && scaledY >= 0 && scaledY < height) {
//         // Get depth value (assuming 16-bit depth data)
//         uint16_t* depth_data = (uint16_t*)currentDepthMsg->data.data();
//         int index = scaledY * width + scaledX;
//         float depth_value = depth_data[index] / 1000.0f; // Convert to meters
        
//         // Update depth information label
//         QString info = QString("Depth Information:\n\n"
//                              "Position: (%1, %2)\n"
//                              "Depth: %.3f meters\n"
//                              "Time: %3")
//                           .arg(scaledX)
//                           .arg(scaledY)
//                           .arg(depth_value)
//                           .arg(QString::fromStdString(
//                               std::to_string(currentDepthMsg->header.stamp.sec) + "." +
//                               std::to_string(currentDepthMsg->header.stamp.nanosec)));
        
//         depthInfoLabel->setText(info);
//         lastDepthValue = depth_value;
//     }
// }

void Viewer::updateDepthInfo(int x, int y)
{
    if (!isRunning) return;

    try {
        rs2::frameset frames = pipe.wait_for_frames();
        auto depth_frame = frames.get_depth_frame();

        if (depth_frame) {
            // Get the depth value at the clicked point
            float depth_value = depth_frame.get_distance(x, y);
            lastDepthValue = depth_value;
            
            RCLCPP_INFO(this->get_logger(), 
                "Depth at point (%d, %d): %.3f meters", x, y, depth_value);
        }
    } catch (const rs2::error& e) {
        RCLCPP_ERROR(this->get_logger(), "Error getting depth value: %s", e.what());
    }
}

QImage Viewer::drawPointOnImage(QImage image, QPoint point, QColor color)
{
    QImage imageCopy = image.copy();
    QPainter painter(&imageCopy);
    painter.setPen(QPen(color, 3));
    painter.drawEllipse(point, 5, 5);
    painter.drawLine(point.x() - 10, point.y(), point.x() + 10, point.y());
    painter.drawLine(point.x(), point.y() - 10, point.x(), point.y() + 10);
    return imageCopy;
}

// void Viewer::setupROS()
// {
//     // Create publishers
//     color_pub_ = this->create_publisher<sensor_msgs::msg::Image>("camera/color/image_raw", 10);
//     depth_pub_ = this->create_publisher<sensor_msgs::msg::Image>("camera/depth/image_raw", 10);

//     // Create subscriber
//     depth_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
//         "camera/depth/image_raw", 10,
//         std::bind(&Viewer::depthCallback, this, std::placeholders::_1)
//     );

//     // Setup ROS spinner timer
//     rosTimer = new QTimer(this);
//     connect(rosTimer, &QTimer::timeout, this, &Viewer::spinROS);
//     rosTimer->start(10); // 100Hz spin rate

//     // Setup frame processing timer
//     frameTimer = new QTimer(this);
//     connect(frameTimer, &QTimer::timeout, this, &Viewer::processFrames);
//     frameTimer->start(33); // ~30 fps
// }

void Viewer::setupROS()
{
    // Create publishers
    color_pub_ = this->create_publisher<sensor_msgs::msg::Image>("camera/color/image_raw", 10);
    depth_pub_ = this->create_publisher<sensor_msgs::msg::Image>("camera/depth/image_raw", 10);
    point_pub_ = this->create_publisher<geometry_msgs::msg::Point>("depth_point", 10);

    // Setup ROS spinner timer
    rosTimer = new QTimer(this);
    connect(rosTimer, &QTimer::timeout, this, &Viewer::spinROS);
    rosTimer->start(10); // 100Hz spin rate

    // Setup frame processing timer
    frameTimer = new QTimer(this);
    connect(frameTimer, &QTimer::timeout, this, &Viewer::processFrames);
    frameTimer->start(33); // ~30 fps
}

// void Viewer::depthCallback(const sensor_msgs::msg::Image::SharedPtr msg)
// {
//     try {
//         currentDepthMsg = msg;  // Store the current depth message
        
//         // Convert the received depth image to QImage
//         QImage depthQImage = convertDepthMsgToQImage(msg);
        
//         // If we have a clicked point, draw it on the image
//         if (!lastClickedPoint.isNull()) {
//             depthQImage = drawPointOnImage(depthQImage, lastClickedPoint, Qt::red);
//         }
        
//         subscribedDepthView->setPixmap(QPixmap::fromImage(depthQImage));
        
//         // Update depth information if we have a clicked point
//         if (!lastClickedPoint.isNull()) {
//             updateDepthInfo(lastClickedPoint.x(), lastClickedPoint.y());
//         }
//     } catch (const std::exception& e) {
//         RCLCPP_ERROR(this->get_logger(), "Error processing depth callback: %s", e.what());
//     }
// }

// QImage Viewer::convertDepthMsgToQImage(const sensor_msgs::msg::Image::SharedPtr msg)
// {
//     // Convert 16-bit depth data to 8-bit for visualization
//     cv::Mat depth_mat(msg->height, msg->width, CV_16UC1, 
//                      (void*)msg->data.data(), msg->step);
    
//     // Normalize the depth image for better visualization
//     cv::Mat depth_normalized;
//     cv::normalize(depth_mat, depth_normalized, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    
//     // Apply colormap
//     cv::Mat depth_colored;
//     cv::applyColorMap(depth_normalized, depth_colored, cv::COLORMAP_JET);

//     // Convert to QImage
//     return QImage(depth_colored.data, depth_colored.cols, depth_colored.rows,
//                  depth_colored.step, QImage::Format_RGB888).copy();
// }

void Viewer::setupCamera()
{
    // Declare and get ROS parameters
    this->declare_parameter("color_width", 640);
    this->declare_parameter("color_height", 480);
    this->declare_parameter("fps", 30);

    int width = this->get_parameter("color_width").as_int();
    int height = this->get_parameter("color_height").as_int();
    int fps = this->get_parameter("fps").as_int();

    // Configure RealSense pipeline
    cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_RGB8, fps);
    cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);
}

void Viewer::handleStartButton()
{
    if (!isRunning) {
        try {
            pipe.start(cfg);
            isRunning = true;
            startButton->setText("Stop Camera");
            RCLCPP_INFO(this->get_logger(), "Camera started successfully");
        } catch (const rs2::error& e) {
            RCLCPP_ERROR(this->get_logger(), "Failed to start camera: %s", e.what());
        }
    } else {
        pipe.stop();
        isRunning = false;
        startButton->setText("Start Camera");
        colorView->setText("Color Feed");
        depthView->setText("Depth Feed");
        RCLCPP_INFO(this->get_logger(), "Camera stopped");
    }
}

void Viewer::spinROS()
{
    if (rclcpp::ok()) {
        rclcpp::spin_some(this->get_node_base_interface());
    }
}

QImage Viewer::convertColorToQImage(const rs2::frame& frame)
{
    auto video_frame = frame.as<rs2::video_frame>();
    return QImage(
        (uchar*)frame.get_data(),
        video_frame.get_width(),
        video_frame.get_height(),
        video_frame.get_stride_in_bytes(),
        QImage::Format_RGB888
    ).copy();
}

void Viewer::publishColorFrame(const rs2::frame& frame)
{
    auto video_frame = frame.as<rs2::video_frame>();
    
    // Create and populate ROS Image message
    auto msg = std::make_unique<sensor_msgs::msg::Image>();
    msg->height = video_frame.get_height();
    msg->width = video_frame.get_width();
    msg->encoding = "rgb8";
    msg->step = video_frame.get_stride_in_bytes();
    msg->data.resize(msg->step * msg->height);
    memcpy(msg->data.data(), video_frame.get_data(), msg->step * msg->height);
    msg->header.frame_id = "camera_color_frame";
    msg->header.stamp = this->now();

    // Publish the message
    color_pub_->publish(std::move(msg));
}

void Viewer::publishDepthFrame(const rs2::frame& frame)
{
    auto depth_frame = frame.as<rs2::depth_frame>();
    
    // Create and populate ROS Image message
    auto msg = std::make_unique<sensor_msgs::msg::Image>();
    msg->height = depth_frame.get_height();
    msg->width = depth_frame.get_width();
    msg->encoding = "16UC1";  // 16-bit depth
    msg->step = depth_frame.get_stride_in_bytes();
    msg->data.resize(msg->step * msg->height);
    memcpy(msg->data.data(), depth_frame.get_data(), msg->step * msg->height);
    msg->header.frame_id = "camera_depth_frame";
    msg->header.stamp = this->now();

    // Publish the message
    depth_pub_->publish(std::move(msg));
}

void Viewer::processFrames()
{
    if (!isRunning) return;

    try {
        // Wait for new frames
        rs2::frameset frames = pipe.wait_for_frames();
        auto color_frame = frames.get_color_frame();
        auto depth_frame = frames.get_depth_frame();

        if (color_frame && depth_frame) {
            // Update UI with color frame
            colorView->setPixmap(QPixmap::fromImage(convertColorToQImage(color_frame)));

            // Create a colormap for depth visualization
            rs2::colorizer color_map;
            auto depth_colorized = color_map.colorize(depth_frame);
            
            // Update UI with colorized depth
            depthView->setPixmap(QPixmap::fromImage(convertColorToQImage(depth_colorized)));

            // Publish frames to ROS topics
            publishColorFrame(color_frame);
            publishDepthFrame(depth_frame);
        }
    } catch (const rs2::error& e) {
        RCLCPP_ERROR(this->get_logger(), "Frame capture error: %s", e.what());
    }
}