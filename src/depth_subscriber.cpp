#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <opencv2/opencv.hpp>

class DepthSubscriber : public rclcpp::Node
{
public:
    DepthSubscriber() : Node("depth_subscriber")
    {
        // Create subscription for depth images
        subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
            "camera/depth/image_raw", 10,
            std::bind(&DepthSubscriber::depth_callback, this, std::placeholders::_1));

        // Create subscription for clicked point coordinates
        point_sub_ = this->create_subscription<geometry_msgs::msg::Point>(
            "depth_point", 10,
            std::bind(&DepthSubscriber::point_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Depth Subscriber Node Started");
    }

private:
    void depth_callback(const sensor_msgs::msg::Image::SharedPtr msg)
    {
        if (last_clicked_point_.x >= 0 && last_clicked_point_.y >= 0) {
            // Get depth value at clicked point
            uint16_t* depth_data = (uint16_t*)msg->data.data();
            int index = last_clicked_point_.y * msg->width + last_clicked_point_.x;
            
            if (index < (msg->height * msg->width)) {
                float depth_value = depth_data[index] / 1000.0f; // Convert to meters
                
                RCLCPP_INFO(this->get_logger(), 
                    "Depth at point (%d, %d): %.3f meters", 
                    last_clicked_point_.x, 
                    last_clicked_point_.y, 
                    depth_value);
            }
        }
    }

    void point_callback(const geometry_msgs::msg::Point::SharedPtr msg)
    {
        last_clicked_point_.x = static_cast<int>(msg->x);
        last_clicked_point_.y = static_cast<int>(msg->y);
        RCLCPP_INFO(this->get_logger(), "Received new point: (%d, %d)", 
            last_clicked_point_.x, last_clicked_point_.y);
    }

    struct Point {
        int x = -1;
        int y = -1;
    } last_clicked_point_;

    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
    rclcpp::Subscription<geometry_msgs::msg::Point>::SharedPtr point_sub_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<DepthSubscriber>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}