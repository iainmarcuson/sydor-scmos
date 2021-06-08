#!/bin/bash

#
# v 1.0 YF Verify PV comms to dataViewer EPICs plugin
#
P="dp"
R="cam1"


wait_for_key () {
    echo "<Space> to continue, Press 'q' to exit"
    count=0
    while : ; do
        read -n 1 k <&1
        if [[ $k = q ]] ; then
            printf "\nQ\n"
            exit
        else
            return 0    
        fi
        done
}

## ***************************************************************************
## ***************************************************************************
## *************************M  A  I  N  **************************************
## ***************************************************************************
caput "$P":"$R":AcquireTime 0
caput "$P":"$R":InterframeTime 0
caput "$P":"$R":TriggerMode 0
caput "$P":"$R":NumImages 1    # This is 'NumTriggers'
caput "$P":"$R":NumFrames 1 
wait_for_key


caput "$P":"$R":AcquireTime 5.1
wait_for_key

caput "$P":"$R":InterframeTime 1.234
wait_for_key

caput "$P":"$R":TriggerMode 1
wait_for_key

caput "$P":"$R":TriggerMode 0  # Internal Trigger
wait_for_key

caput "$P":"$R":SensorPower 1
wait_for_key


caput "$P":"$R":NumImages 3    # This is 'NTriggers'
wait_for_key

caput "$P":"$R":NumFrames 12  
wait_for_key


caput "$P":"$R":SetName "set1"
wait_for_key
##                              123456789123456789123456789123456789123  // 53 CHARS max?
##                             "This describes the set of data we are t"
caput "$P":"$R":SetDescription "This describes the set of data."
wait_for_key


caput "$P":"$R":RunName ""
wait_for_key


caput "$P":"$R":StartRun 1   # value may not matter?
wait_for_key

# Should be ARMED and waiting for a trigger

caput "$P":"$R":ShutterControl 1
wait_for_key


# 3 triggers - matches 'NumImages = 3'
caput "$P":"$R":DoTrigger 1  # Software Trigger GO!
wait_for_key

caput "$P":"$R":DoTrigger 1  # Software Trigger GO!
wait_for_key

caput "$P":"$R":DoTrigger 1  # Software Trigger GO!
wait_for_key


caput "$P":"$R":ShutterControl 0
wait_for_key

 
caput "$P":"$R":SensorPower 0
wait_for_key

