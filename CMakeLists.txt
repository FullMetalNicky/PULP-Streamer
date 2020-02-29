cmake_minimum_required(VERSION 2.8.3)
project(himax)

find_package(catkin REQUIRED COMPONENTS
  rospy
  sensor_msgs
)

catkin_python_setup()
