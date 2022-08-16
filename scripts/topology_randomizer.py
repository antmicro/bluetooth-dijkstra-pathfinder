import random as rnd
import sys 
import json
import os
from jinja2 import Template, PackageLoader, Environment, FileSystemLoader
from pyvis.network import Network
import math
import argparse

# constants 
MAX_CONN_NUM = 3
MAX_DIST = 10

AREA_X_DIM = 500
AREA_Y_DIM = 500

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

# Visualize network 
net = Network('500px', '500px')

for index in range(nodes):
    # create dict record
    node_name = "node" + str(index)
    if index > 9:
        ble_addr = "C0:00:00:00:00:" + str(index)
    else:
        ble_addr = "C0:00:00:00:00:0" + str(index)
    
    x_pos = rnd.randint(0, AREA_X_DIM)
    y_pos = rnd.randint(0, AREA_Y_DIM)

    mesh[node_name] = {
            "addr":index,
            "x":x_pos,
            "y":y_pos,
            "addr_bt_le":ble_addr,
            "reserved":True,
            "paths_size":0,
            "paths":[]
            }
    
    # Add to visualization
    net.add_node(index, "node" + str(index), x=x_pos, y=y_pos)


# add edges 
RADIO_RANGE = 200 # TODO: adjust that on the basis of the nodes provided
for node in mesh.values():
    for neigh in mesh.values():
        # Do not connect to self
        if node["addr"] == neigh["addr"]:
            continue

        d = math.sqrt(math.pow(node["x"] - neigh["x"], 2) + math.pow(node["y"] - neigh["y"], 2))
        print("distance between: {} and {} is {}".format(node["addr"], neigh["addr"], d))
        print("distance between: {},{} and {},{} is {}".format(
            node["x"], node["y"], 
            neigh["x"], neigh["y"], d))
        if d < RADIO_RANGE:
            node["paths_size"] += 1
            node["paths"].append(dict(addr=neigh["addr"], distance=int(d)))
            
            net.add_edge(node["addr"], neigh["addr"], weight=int(d), title=int(d))
            print("adding edge between: {} and {}".format(node["addr"], neigh["addr"]))

# dict(addr=connection_addr, distance=connection_dist))
#print(json.dumps(mesh))

topology_config_file_path = os.path.join(project_dir,
        "config-files/mesh-topology-desc/randomized_topology.json")
net.show('asd.html')

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











