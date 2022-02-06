#!/bin/bash
# setup zephyr env
source ~/Projects/zephyr-project/zephyr/zephyr-env.sh
source ~/Projects/zephyr-project/.venv/bin/activate

west build -b nrf52840dk_nrf52840 ~/Projects/antmicro/bluetooth-dijkstra-pathfinder/zephyr-rtos --build-dir ~/Projects/antmicro/bluetooth-dijkstra-pathfinder/zephyr-rtos/build

LINES=$(wc -l /home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/tests/out/4nodes.log | cut -f 1 -d " ")

while [ $LINES -le 10 ] 
do
    # randomize
    python3 ~/Projects/antmicro/bluetooth-dijkstra-pathfinder/scripts/topology_randomizer.py 10 

    # build
    west build -b nrf52840dk_nrf52840 ~/Projects/antmicro/bluetooth-dijkstra-pathfinder/zephyr-rtos --build-dir ~/Projects/antmicro/bluetooth-dijkstra-pathfinder/zephyr-rtos/build

    # perform tests and save to file
    ~/Repos/renode/test.sh  ~/Projects/antmicro/bluetooth-dijkstra-pathfinder/tests/test_packet_travel_time.robot
    
    # read how many lines was written to output file 
    LINES=$(wc -l /home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/tests/out/4nodes.log | cut -f 1 -d " ")
    
    echo NUMBER OF READ LINES: $LINES
done

