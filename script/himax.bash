#!/bin/bash

if [[ $6 -eq "1" ]]; then
  camera=0
  echo "using pulp-shield"
else
  camera=1
  echo "using gapuino"
fi

echo -e "$1\n$2\n$3\n$4\n$5\n$camera\n$7\n$8\nEND\n" > /tmp/config.txt
source ~/Documents/Drone/pulp-sdk/configs/gap.sh
source ~/Documents/Drone/pulp-sdk/configs/platform-board.sh

# plpbridge --cable=ftdi@digilent --boot-mode=jtag --binary=$6/build/gap/test/test --chip=gap reset stop load ioloop reqloop start wait

if [[ $6 -eq "1" ]]; then
  config="--config=gap --cable=ftdi"
  #--config=gapoc_a_revb
else
  config="--cable=ftdi@digilent"
fi

plpbridge $config --boot-mode=jtag --binary=$9/build/gap/test/test --chip=gap reset stop load ioloop reqloop start wait
