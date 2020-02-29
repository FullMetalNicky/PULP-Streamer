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



