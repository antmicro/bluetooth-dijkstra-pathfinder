# Jakub Szukała - Engineering Thesis
This repository contains project files of custom Bluetooth ad - hoc mesh implementation, with Dijkstra's as routing algorithm.

In folder dijkstra\_shortest\_path is my implementation of Dijkstra's shortest 
path algorithm. It does work, although there is still a lot of room for improvements. 
Algorithm was tested on examplary graph pictured on dijkstra\_graph.png

In folder zephyr\_project You can find Zephyr project files. Destination platform
of this project is nRF52840 Dongle with Zephyr RTOS. For now it is tested on 
Renode platform.

File init.resc contains script for Renode platform.

Project is still work in progress and I am also just learning Zephyr RTOS.

TODO: Develop this README
