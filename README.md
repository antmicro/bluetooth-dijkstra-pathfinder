# Jakub Szukała - Engineering Thesis
This repository contains project files of custom Bluetooth ad - hoc mesh implementation, with Dijkstra's as routing algorithm.

In folder dijkstra\_shortest\_path is my implementation of Dijkstra's shortest 
path algorithm. It does work, although there is still a lot of room for improvements. 
Algorithm was tested on examplary graph pictured on dijkstra\_graph.png

In folder zephyr\_project You can find Zephyr project files. Destination platform
of this project is nRF52840 Dongle with Zephyr RTOS. For now it is tested on 
Renode platform.

Default Dijkstra graph used in zephyr version is:
<pre>
(1)    2   (2)
  
   1       3
         
(1)    0   (2)
</pre>
In brackets are distances between nodes and without brackets are nodes addresses. 
This information is contained in graph.c file in initialization function.
This setup allows for simplest case where shortest path must be found. Solution to reach
node 2 from node 0 is path through 0, 1, 2 with total distance of 2.

File init.resc contains script for Renode platform.

Project is still work in progress and I am also just learning Zephyr RTOS.

TODO: Develop this README
