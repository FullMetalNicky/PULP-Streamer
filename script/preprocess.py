#!/usr/bin/env python

from typing import Callable

import cv2

import rospy
from cv_bridge import CvBridge
from sensor_msgs.msg import Image, CompressedImage
import numpy as np


def ApplyVignette(rows, cols, sigma=150):
    # type: (int, int, int) -> np.ndarray
    # generating vignette mask using Gaussian kernels
    kernel_x = cv2.getGaussianKernel(cols, sigma)
    kernel_y = cv2.getGaussianKernel(rows, sigma)
    kernel = kernel_y * kernel_x.T
    mask = kernel / kernel.max()
    return mask


def process(raw=True, image_width=108, image_height=60, sigma=50):
    # type: (bool, int, int, int) -> Callable[[Image], None]
    pub = rospy.Publisher("sink", Image, queue_size=1)
    bridge = CvBridge()

    vignette = ApplyVignette(image_width, image_width, sigma)[24:84, 0:108]

    def f(msg):
        # type: (Image) -> None
        if raw:
            img = bridge.imgmsg_to_cv2(msg)
        else:
            img = bridge.compressed_imgmsg_to_cv2(msg)
        img = cv2.resize(img, (image_width, image_height)) #, interpolation=cv2.INTER_AREA)
        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        img = img.astype(np.float)
        img *= vignette
        img = img.astype(np.uint8)
        msg = bridge.cv2_to_imgmsg(img)
        pub.publish(msg)

    return f


def main():
    # type: () -> None
    rospy.init_node('preprocess', anonymous=True)
    width = rospy.get_param('~width', 108)
    height = rospy.get_param('~height', 60)
    raw = rospy.get_param('~raw', True)
    if raw:
        topic_type = Image
    else:
        topic_type = CompressedImage
    rospy.Subscriber("source", topic_type, process(raw=raw, image_width=width, image_height=height),
                     queue_size=1)
    rospy.spin()


if __name__ == '__main__':
    main()
