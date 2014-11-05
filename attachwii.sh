#!/bin/bash
hcitool dev | grep hci >/dev/null
if test $? -eq 0 ; then
   sudo wminput -d -c  /home/pi/mywminput 00:1B:7A:DA:DD:46 &
   sudo wminput -d -c  /home/pi/mywminput 00:1C:BE:25:0B:FA &
else
   echo "Blue-tooth adapter not present!"
fi

