#include "ros/ros.h"
#include "sensor_msgs/Image.h"
#include "sensor_msgs/LaserScan.h"
#include "core_msgs/ROIPointArray.h"

#define RAD2DEG(x) ((x)*180./M_PI)
#define Z_DEBUG true
ros::Publisher scan_publisher;
core_msgs::ROIPointArrayPtr obstacle_points;

void scanCallback(const sensor_msgs::LaserScan::ConstPtr& scan) {
  int count = scan->scan_time / scan->time_increment;
  ROS_INFO("angle_range, %f, %f", RAD2DEG(scan->angle_min), RAD2DEG(scan->angle_max));

  obstacle_points->Vector3DArray.clear();
  geometry_msgs::Vector3 point_;

  for(int i = 0; i< count; i++) {
    float degree = scan->angle_min + scan->angle_increment *i;
    point_.x = scan->ranges[i];
    point_.y = degree + M_PI/2;
    point_.z = 1;
    obstacle_points->Vector3DArray.push_back(point_);
  }
  obstacle_points->id.push_back(count);
  obstacle_points->extra.push_back(scan->angle_min+M_PI);
  obstacle_points->extra.push_back(scan->angle_increment);

  obstacle_points->header.stamp = ros::Time::now();
}

//the obstacle_points data is published only after the lane_map data is published
//this is for the synchronization for the lane_map and obstacle_points, also needed for map_generator
//TODO: Need more sophisticated synchronization
void callbackLane(const sensor_msgs::ImageConstPtr& msg_lane_map) {
  if(Z_DEBUG)
  {
    obstacle_points->Vector3DArray.clear();
    geometry_msgs::Vector3 point_;

    for(int i = 0; i< 100; i++) {
      point_.x = 0;
      point_.y = M_PI/2;
      point_.z = 1;
      obstacle_points->Vector3DArray.push_back(point_);
    }
    obstacle_points->id.push_back(100);
    obstacle_points->extra.push_back(M_PI);
    obstacle_points->extra.push_back(1);

    obstacle_points->header.stamp = ros::Time::now();
  }
  scan_publisher.publish(obstacle_points);
}

int main(int argc, char **argv) {
  ros::init(argc, argv, "lms_client");
  ros::NodeHandle nh;

  scan_publisher = nh.advertise<core_msgs::ROIPointArray>("/obstacle_points", 1);
  obstacle_points.reset(new core_msgs::ROIPointArray);

  ros::Subscriber sub = nh.subscribe<sensor_msgs::LaserScan>("/scan", 10, scanCallback);
  ros::Subscriber laneSub = nh.subscribe("/lane_map",1,callbackLane);

  ros::spin();
  return 0;
}
