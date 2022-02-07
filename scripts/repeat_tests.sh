#!/bin/bash
# setup zephyr env
source ~/Projects/zephyr-project/zephyr/zephyr-env.sh
source ~/Projects/zephyr-project/.venv/bin/activate
pip3 install robotframework==4.0.1


west build -b nrf52840dk_nrf52840 ~/Projects/antmicro/bluetooth-dijkstra-pathfinder/zephyr-rtos --build-dir ~/Projects/antmicro/bluetooth-dijkstra-pathfinder/zephyr-rtos/build

NODES_NUM=10

LOG_FILE_PATH=/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/\
tests/out/${NODES_NUM}nodes.log

TOPOLOGY_RANDOMIZER_PATH=~/Projects/antmicro/bluetooth-dijkstra-pathfinder/\
scripts

SOURCES_PATH=~/Projects/antmicro/bluetooth-dijkstra-pathfinder/zephyr-rtos

BUILD_DIR=~/Projects/antmicro/bluetooth-dijkstra-pathfinder/zephyr-rtos/build

RENODE_ROOT_DIR=~/Repos/renode

ROBOT_TESTS_PATH=~/Projects/antmicro/bluetooth-dijkstra-pathfinder/tests



LINES=$(wc -l  $LOG_FILE_PATH | cut -f 1 -d " ")

while [ $LINES -le 24 ] 
do
    # randomize
    python3 $TOPOLOGY_RANDOMIZER_PATH/topology_randomizer.py $NODES_NUM

    # build
    west build -b nrf52840dk_nrf52840 $SOURCES_PATH --build-dir $BUILD_DIR 
    
    # perform tests and save to file
    $RENODE_ROOT_DIR/test.sh $ROBOT_TESTS_PATH/test_packet_travel_time.robot

    # read how many lines was written to output file 
    LINES=$(wc -l $LOG_FILE_PATH | cut -f 1 -d " ")
    
    echo NUMBER OF READ LINES: $LINES
done

