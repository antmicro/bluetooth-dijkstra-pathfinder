# Bluetooth Dijkstra Pathfinder 
This repository contains project files of custom Bluetooth ad - hoc mesh implementation, with Dijkstra's as routing algorithm.

In folder dijkstra\_shortest\_path is my implementation of Dijkstra's shortest 
path algorithm. It does work, although there is still a lot of room for improvements. 
Algorithm was tested on examplary graph: 
![Dijkstra graph example](./dijkstra_graph.png)


In folder zephyr\_project You can find Zephyr project files. Destination platform
of this project is nRF52840 Dongle with Zephyr RTOS. For now it is tested on 
Renode platform.

Default Dijkstra graph used in zephyr version is:
<pre>
(1)    2   (2)
  
   1       4
         
(1)    0   (2)
</pre>

In brackets are distances between nodes and without brackets are nodes addresses. 
This information is contained in graph.c file in initialization function.
This setup allows for simplest case where shortest path must be found. Solution to reach
node 2 from node 0 is path through 0, 1, 2 with total distance of 2.

### Short description of a demo 
Project in current state of development consists of 4 nodes in configuration 
described above that create a BLE mesh. There is also one independent broadcaster,
that is not really a part of a mesh, but it initiates a communication by sending
a **directed** (in general all messages are directed, received only by a node 
with address specified in advertisement configuration), advertisement to a node.

In current state, communication cycle looks as following:
1. Mobile broadcaster sends a message which is marked with destination - node 0.
Message is directly advertised to node 2. Payload of this message is broadcast 
start time (just zeros).
2. Node 2 after reception, should calculate dijkstra algorithm and decide to 
send the packet to node 1
3. Node 1 executes the same procedure as node 2
4. Message gets to node 0 that is destination of the packet so message is not 
passed anymore.
5. Here begins second cycle of communication, with exactly the same path but
different payload in the message.

### Short description of a packet transmission routine
#### Bluetooth setup description
Bluetooth advertising and scanning is set up in two functions: ble_scan_setup and 
ble_adv_sets_setup. Currently BLE is set to send directed messages, in extended
mode and continously and passively scan for advertisements.

#### Bluetooth packet "passing" routine 
* bt_direct_msg_received_cb - is a callback that fires when node succesfully 
scans a message. Its job is to print to the console that this event happened and 
put that message to a message queue for processing by thread (basically offloading
work from ISR)
* create_packet_thread_entry is a thread responsible for getting packets from 
message queue and processing them. Processing involves:
    * identifying what is the destination node of the packet (first byte of payload)
    * checking if the current node is dst node
    * calculate dijkstra algorithm
    * create a packet and add information what address next hop should be 
    * add this packet to FIFO for sending
* ble_send_packet_thread_entry is a thread that advertises the processed messages:
    * it gets the packet from FIFO (sleeps when fifo is empty)
    * on the basis of next node ID pick a proper advertising set
    * create a proper bluetooth packet
    * set it to advertising set 
    * wait for peripheral to be free and then advertise 

Threads are initialized in main and there also their priorities are set.

### Running the application
To build the project, go to zephyr-rtos directory and run build command:
> cd zephyr-rtos && west build -b nrf52840dk_nrf52840 

Then go back to the root of the project and build mobile broadcaster:
> cd ../mobile_broadcaster && west build -b nrf52840dk_nrf52840 

Now You should be ready to run a project. File named init.resc in project root
directory is for simulation with Renode and should be run with it (use Renode 
built from sources):
> ./renode ~/path/to/init.resc

