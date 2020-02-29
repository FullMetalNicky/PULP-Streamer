# PULP Streamer

The repository is for all people who are working with PULP hardware and Himax cameras, and need an easy way to visualize and record their video streams. The basic c and python scripts started as a part of the [FrontNetPorting](https://github.com/FullMetalNicky/FrontNetPorting) project, but then they were transformed into a ROS masterpiece by [Jerome Guzzi](https://github.com/jeguzzi).

By using the ROS launch file, you activate a pipeline that includes streaming from the Himax, preprocessing (e.g cropping, downsampling), publishing the ROS messages and visualizing with rqt. 
Parameters of this pipeline can be adjusted through the Himax.launch file, and include:
* Cropping coordinates
* Downsampling factor 
* Format (QVGA, QQVGA, binned)
* Setting Pulp type - Gapuino or Shield 
* Himax target luminance 

### Project Structure
The project has the following directory structure. Change it at your own risk.

.
├── CMakeLists.txt
├── launch
│   └── himax.launch
├── package.xml
├── pulp
│   └── camera_to_fifo
|       ├── build
|       ├── Gap8.h
│       ├── ImgIO.c
│       ├── ImgIO.h
│       ├── Makefile
│       ├── test1.c
│       ├── test.c
│       └── test.txt
├── README.md
├── script
│   ├── himax.bash
│   ├── himax_driver.py
│   ├── preprocess.py
├── setup.py



### Installation
This code runs on Ubuntou 16.04 and 18.04. If it happens to run on any other OS, consider it a miracle.
The following dependencies are needed:

* ROS kinetic - http://wiki.ros.org/kinetic/Installation or newer versions (works with ROS Melodic too).
* PULP-sdk - https://github.com/pulp-platform/pulp-sdk


This can be installed as a ROS package. If you don't have a workspace, create an new folder - e.g. himax_ws. Inside it create another folder calles 'src'. when you are inside the himax_ws folder, run the command
```
catkin init
```
using [catkin tools](https://catkin-tools.readthedocs.io/en/latest/verbs/catkin_init.html). Clone this repository to the 'src' folder, and run  
```
catkin build
```
Don't forget to 
```
source devel/setup.bash
```

### Getting Started
First you need to create a pipe inside the pulp folder, like this
```
mkfifo /tmp/image_pipe
```
The name is important, because the scripts search for this names pipe.  

To compile the camera_to_fifo code, go to pulp/camera_to_fifo and execute
```
make clean conf all
```

To run it execute
```
plpbridge --chip=gap --cable=ftdi --binary=build/gap/test/test load ioloop reqloop start wait
```
To activate the who pipeline, launch the ROS publisher
```
roslaunch himax himax.roslaunch
```
To record the video, just run 
```
rosbag record /image_raw
```

### Development

Want to contribute? Great!
Send me six-packs of diet coke.

License
----

MIT
