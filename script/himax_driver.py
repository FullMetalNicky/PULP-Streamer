#!/usr/bin/env python

import fcntl
import os

import numpy as np

import rospy
from cv_bridge import CvBridge
from sensor_msgs.msg import Image
# import cv2

page_size = 655536  # 4092  # 8192  # 1024 # 4096
F_GETPIPE_SZ = 1032  # Linux 2.6.35+
F_SETPIPE_SZ = 1031  # Linux 2.6.35+


def read_from_pipe(pipein, size):
    # type: (int, int) -> np.array
    # rospy.loginfo("read_from_pipe %d %d", pipein, size)
    remaining_size = size
    data = []
    while(remaining_size > 0):
        # rospy.loginfo("Will read %d", min(remaining_size, page_size))
        output = os.read(pipein, min(remaining_size, page_size))
        remaining_size = remaining_size - len(output)
        # rospy.loginfo("RS %d", remaining_size)
        data.append(output)
    data_str = ''.join(data)
    if (len(data_str) < size):
        rospy.loginfo("Error, expecting {} bytes, received {}.".
                      format(size, len(data_str)))
        return None
    data_arr = np.frombuffer(data_str, dtype=np.uint8)
    return data_arr


def main():
    # type: () -> None
    rospy.init_node('himax_driver', anonymous=True)
    width = rospy.get_param('~width')
    height = rospy.get_param('~height')
    pipe = rospy.get_param('~pipe', '/tmp/image_pipe')
    rospy.loginfo("Ready to listen for %d x %d images on %s", width, height, pipe)
    image_pub = rospy.Publisher("image_raw", Image, queue_size=1)
    bridge = CvBridge()
    if not os.path.exists(pipe):
        os.mkfifo(pipe)
    pipein = os.open(pipe, os.O_RDONLY)
    fcntl.fcntl(pipein, F_SETPIPE_SZ, 1000000)
    seq = 0
    while not rospy.is_shutdown():
        data = read_from_pipe(pipein, width * height)
        # rospy.loginfo("D")
        if data is not None:
            cv_image = np.reshape(data, (height, width))

            # rospy.loginfo("Mean %d", np.mean(cv_image))

            # cv_image = cv_image.astype(np.float)
            # cv_image = cv_image * 1.5
            # cv_image = np.minimum(255, cv_image)
            # cv_image = cv_image.astype(np.uint8)
            # cv_image = cv2.equalizeHist(cv_image)
            msg = bridge.cv2_to_imgmsg(cv_image)
            msg.header.stamp = rospy.Time.now()
            msg.header.seq = seq
            seq += 1
            image_pub.publish(msg)
            rospy.sleep(0)
    os.close(pipein)


if __name__ == '__main__':
    main()
