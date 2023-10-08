#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"

#include <px4_msgs/msg/vehicle_odometry.hpp>

#ifdef PX4_ROS_TIMESYNC
	#include <px4_msgs/msg/timesync.hpp>
#endif

#include <geometry_msgs/msg/pose_stamped.hpp>

using std::placeholders::_1;

using namespace std::chrono_literals;


class MocapPX4Bridge : public rclcpp::Node
{
public:
	MocapPX4Bridge() : Node("mocap_px4_bridge") {
		poseSub = this->create_subscription<geometry_msgs::msg::PoseStamped>("/Robot_1/pose", 10, std::bind(&MocapPX4Bridge::poseCallback, this, _1));
		odomPub = this->create_publisher<px4_msgs::msg::VehicleOdometry>("/fmu/in/vehicle_visual_odometry", 10);
	}

private:
	void poseCallback(const geometry_msgs::msg::PoseStamped::UniquePtr);

	rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr poseSub;
	rclcpp::Publisher<px4_msgs::msg::VehicleOdometry>::SharedPtr odomPub;
};

void MocapPX4Bridge::poseCallback(const geometry_msgs::msg::PoseStamped::UniquePtr poseMsg){
	RCLCPP_INFO_ONCE(get_logger(), "Recived first msg from optitrack.");
	RCLCPP_INFO_ONCE(get_logger(), "P: %f, %f, %f", poseMsg->pose.position.x, 
					poseMsg->pose.position.y, poseMsg->pose.position.z);
	RCLCPP_INFO_ONCE(get_logger(), "q: %f, %f, %f, %f", poseMsg->pose.orientation.w,
					poseMsg->pose.orientation.x, poseMsg->pose.orientation.y, poseMsg->pose.orientation.z);
	px4_msgs::msg::VehicleOdometry odomMsg;

	odomMsg.pose_frame = odomMsg.POSE_FRAME_NED;
	odomMsg.timestamp = uint64_t(poseMsg->header.stamp.sec)*1000000 + uint64_t(poseMsg->header.stamp.nanosec)/1000;
	odomMsg.timestamp_sample = odomMsg.timestamp;

	odomMsg.position[0] = poseMsg->pose.position.x;
	odomMsg.position[1] = -poseMsg->pose.position.y;
	odomMsg.position[2] = -poseMsg->pose.position.z;

	odomMsg.q[0] = poseMsg->pose.orientation.w;
	odomMsg.q[1] = poseMsg->pose.orientation.x;
	odomMsg.q[2] = - poseMsg->pose.orientation.y;
	odomMsg.q[3] = - poseMsg->pose.orientation.z;

	odomPub -> publish(odomMsg);
	RCLCPP_INFO_ONCE(get_logger(), "Sent to PX4 as:");
	RCLCPP_INFO_ONCE(get_logger(), "P: %f, %f, %f", odomMsg.position[0], 
					odomMsg.position[1], odomMsg.position[2]);
	RCLCPP_INFO_ONCE(get_logger(), "q: %f, %f, %f, %f", odomMsg.q[0],
					odomMsg.q[1], odomMsg.q[2], odomMsg.q[3]);
}


int main(int argc, char * argv[])
{
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<MocapPX4Bridge>());
	rclcpp::shutdown();
	return 0;
}
