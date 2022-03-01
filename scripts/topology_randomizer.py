import random as rnd
import sys 
import json
import os
from jinja2 import Template, PackageLoader, Environment, FileSystemLoader

# constants 
MAX_CONN_NUM = 3
MAX_DIST = 10

# validate input
if len(sys.argv) > 1:
    try:
        nodes = int(sys.argv[1]) # is integer 
        
        if not 3 <= nodes <=64: # is in range 
            sys.exit("Value not in range [1, 64]")
        

    except ValueError:
        sys.exit("Value should be integer in range [1, 64]")

else:
    sys.exit("Did not provide number of nodes!")

print("Input correct. Generating " + str(nodes) + " nodes...") 

# specified root directory of the project 
project_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")


index = 0
mesh = {}

while index < nodes:
    # randomize number of connections
    paths = [] 

    # pick paths_size not greater than number 
    # of nodes in case MAX_CONN_NUM > nodes
    paths_size = int(rnd.random() * 100) % min(MAX_CONN_NUM, nodes - 1) + 1 
     
    # randomize addrs and distances for this connections
    paths_index = 0
    while paths_index < paths_size:
        connection_addr = int(rnd.random() * 100 % nodes)
        connection_dist = int(rnd.random() * 100 % MAX_DIST) 
        paths.append(dict(addr=connection_addr, distance=connection_dist))
        paths_index += 1
    
    # create dict
    node_name = "node" + str(index)

    # create ble addr 
    if index > 9:
        ble_addr = "C0:00:00:00:00:" + str(index)
    else:
        ble_addr = "C0:00:00:00:00:0" + str(index)

    mesh[node_name] = {
            "addr":index,
            "addr_bt_le":ble_addr,
            "reserved":True,
            "paths_size":0,
            "paths":[]
            }
    index += 1


def randomize_index(nodes):
    return int(rnd.random() * 100) % nodes

# add connections
cnt = 0
MAX_CONN_NUM = (nodes * (nodes - 1))/5
existing_connections = []
while cnt < MAX_CONN_NUM:
    # pick random index
    first_node_index = randomize_index(nodes)
    second_node_index = randomize_index(nodes)

    # make sure they are different
    while second_node_index == first_node_index:
        second_node_index = randomize_index(nodes)

    # check if such connection already exists
    for conn in existing_connections:
        config1 = [first_node_index, second_node_index]
        config2 = [second_node_index, first_node_index]
        if conn == config1 or conn == config2:
            break
    else:
        # make connection bidirectional connection 
        mesh["node" + str(first_node_index)]["paths_size"] += 1
        mesh["node" + str(first_node_index)]["paths"].append(
                dict(
                    addr=mesh["node" + str(second_node_index)]["addr"],
                    distance=int(rnd.random() * 100 % MAX_DIST) + 1
                    )
                )
        
        mesh["node" + str(second_node_index)]["paths_size"] += 1
        mesh["node" + str(second_node_index)]["paths"].append(
                dict(
                    addr=mesh["node" + str(first_node_index)]["addr"],
                    distance=int(rnd.random() * 100 % MAX_DIST) + 1
                    )
                )
        existing_connections.append([first_node_index, second_node_index])
        # print([first_node_index, second_node_index])
        cnt += 1


# dict(addr=connection_addr, distance=connection_dist))
#print(json.dumps(mesh))

topology_config_file_path = os.path.join(project_dir,
        "config-files/mesh-topology-desc/randomized_topology.json")
with open(topology_config_file_path, "w") as f:
    f.write(json.dumps(mesh))

# .resc generation 
# ble_addr

constant_contents_prepend = """
############################################################
#THIS FILE IS AUTO GENERATED - DO NOT CHANGE DIRECTLY
############################################################

logLevel 0

emulation CreateWirelessMedium "wireless"
# emulation CreateWiresharkForWireless "wireless" #"bluetoothle"
# emulation LogToWireshark "wireless"
#emulation LogWirelessTraffic
logLevel -1 wireless


# mesh begin  
#############################################################
"""

constant_contents_mid = """
###########################################################
# mesh end 

mach add "mobile_broadcaster"
mach set "mobile_broadcaster"

machine LoadPlatformDescription @platforms/cpus/nrf52840.repl 

emulation SetGlobalQuantum "0.00001"
emulation SetQuantum "0.00001"

# analyzers 
showAnalyzer sysbus.uart0
connector Connect sysbus.radio wireless


macro reset
\"\"\"
    pause
    
    # mesh begin 

"""
constant_contents_append = """
    mach set "mobile_broadcaster"
    sysbus LoadELF @/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/zephyr/zephyr.elf

\"\"\"
runMacro $reset

# observe uarts for default start and dst nodes  
mach set "node0"
showAnalyzer sysbus.uart0
mach set "node2"
showAnalyzer sysbus.uart0

start

machine StartGdbServer 3333 true

"""


# templates
mach_create_template = """

{% for node in nodes_temp %} 
mach add "{{ node }}"
mach set "{{ node }}"
machine LoadPlatformDescription @platforms/cpus/nrf52840.repl 

# set unique identification address 
sysbus Tag <0x100000a4, 0x100000a7> "DEVICEADDR[0]" {{ 
["0x", ((nodes_temp[node]['addr_bt_le'])|replace(":", "") | replace("C0",""))]|join("") 
}} false

# time quantum setup 
emulation SetGlobalQuantum "0.00001"
emulation SetQuantum "0.00001"

# connect to medium 
connector Connect sysbus.radio wireless
{% endfor %}

"""

mach_load_desc_template = """
    {% for node in nodes_temp %}
    mach set "{{ node }}"
    sysbus LoadELF @/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/zephyr-rtos/build/zephyr/zephyr.elf
    {% endfor %}
"""
# later use this 
# $ORIGIN/zephyr-rtos/build/zephyr/zephyr.elf 

env = Environment()

# mach create 
template = env.from_string(mach_create_template)
out_mach_create = template.render(nodes_temp = mesh)

# desc load
template = env.from_string(mach_load_desc_template)
out_mach_load_desc = template.render(nodes_temp = mesh)

contents = (
        constant_contents_prepend 
        + out_mach_create 
        + constant_contents_mid 
        + out_mach_load_desc 
        + constant_contents_append)

#print(contents)

resc_file_path = os.path.join(project_dir,
        "config-files/renode-resc-files/randomized_topology.resc")
with open(resc_file_path, 'w') as rescfile:
    rescfile.write(contents)











