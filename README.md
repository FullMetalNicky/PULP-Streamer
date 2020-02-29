# PULP Streamer

The repository is for all people who are working with PULP hardware and Himax cameras, and need an easy way to visualize and record their video streams. The basic c and python scripts started as a part of the [FrontNetPorting](https://github.com/FullMetalNicky/FrontNetPorting) project, but then they were transformed into a ROS masterpiece by [Jerome Guzzi](https://github.com/jeguzzi).

### Project Structure
The project has the following directory structure. Change it at your own risk.

### Installation
This code run on Ubuntou 16.04 and 18.04. If it happens to run on any other OS, consider it a miracle.
The following dependencies are needed:

* ROS kinetic - http://wiki.ros.org/kinetic/Installation or newer versions (works with ROS Melodic too).
* PULP-sdk - https://github.com/pulp-platform/pulp-sdk

### How-To Guide
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
and launch the ROS publisher
```
roslaunch himax himax.roslaunch
```

### Development

Want to contribute? Great!
Send me six-packs of diet coke.

License
----

MIT
