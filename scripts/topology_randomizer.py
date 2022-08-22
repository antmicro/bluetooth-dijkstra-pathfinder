import random as rnd
import sys 
import json
import os
from jinja2 import Template, PackageLoader, Environment, FileSystemLoader
from pyvis.network import Network
import math
import argparse

# constants 
AREA_X_DIM = 500
AREA_Y_DIM = 500

# validate input
parser = argparse.ArgumentParser(usage="""python3 topology_randomizer.py [NUMBER OF NODES]
Utility to randomly create specified amount of nodes and connect them depending on the distance between the nodes. It outputs .resc file for Renode simulation and .json file for further code generation. When --visualize option is specified, then also a network.html file is generated indicating nodes placement in the space and their connections.
""")
parser.add_argument("nodes_n", 
        help="number of nodes to generate, integer in range [1, 64]", 
        type=int)
parser.add_argument("--mbmove", 
        help="if set, mobile broadcaster will be moving with a use of provided move script extension for Renode",
        action="store_true")
parser.add_argument("--visualize",
        help="generate a .html file visualizing network topology",
        action="store_true")
parser.add_argument("--visualizemb", 
        help="include mobile_broadcaster positions in the visualization",
        action="store_true")
parser.add_argument("--verbose",
        help="explain what is being done",
        action="store_true")
args = parser.parse_args()
if args.nodes_n and args.nodes_n < 1 or args.nodes_n > 64:
    parser.error("ERROR: Value out of range. Specify amount in range [1, 64]")


# root directory of the project 
project_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")


# Visualize network 
if args.visualize:
    net = Network('500px', '500px')


# Create a dictionary that will be then converted to .json and used in jinja
mesh = {}
last_node_index = 0
for index in range(args.nodes_n):
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
    last_node_index = index
    if args.visualize: 
        net.add_node(index, "node" + str(index), x=x_pos, y=y_pos) # type: ignore

# Helper function to calculate distance between two points on a plane
def euclidean_distance(x1, y1, x2, y2):
    return math.sqrt(math.pow(x1 - x2, 2) + math.pow(y1 - y2, 2))


# add edges between nodes within RADIO_RANGE 
RADIO_RANGE = int(1.0 / float(args.nodes_n) * 500 * 4)
print("RADIO_RANGE: {}".format(RADIO_RANGE)) # TODO: change to verbose
for node in mesh.values():
    for neigh in mesh.values():
        # Do not connect to self
        if node["addr"] == neigh["addr"]:
            continue
        
        d = euclidean_distance(node['x'], node['y'], neigh['x'], neigh['y'])
        if args.verbose:
            print("distance between: {} and {} is {}".format(node["addr"], neigh["addr"], d))
        
        if d < RADIO_RANGE:
            node["paths_size"] += 1
            node["paths"].append(dict(addr=neigh["addr"], distance=int(d)))
            if args.visualize:
                net.add_edge(node["addr"], neigh["addr"], weight=int(d), title=int(d)) # type: ignore


# Visualize also positions of MB and to what nodes it will be connected
if args.visualize and args.visualizemb:
    positions = (
            (0, 0),
            (500, 0),
            (500, 500),
            (0, 500)
            )
    for i, position in enumerate(positions): 
        node_id = last_node_index + i + 1
        net.add_node(node_id, "MB_pos" + str(i), x=position[0], y=position[1], # type: ignore
                physics=False, color='#dd4b39') 
        for node in mesh.values():
            d = euclidean_distance(node["x"], node["y"], position[0], position[1])
            if d < RADIO_RANGE:
                net.add_edge(node_id, node["addr"]) # type: ignore


# Save and display visualization 
if args.visualize:
    network_html_path = "network_topology.html"
    net.show(network_html_path) # type: ignore
    if args.verbose:
        print("Network topology .html file written to: {}".format(network_html_path))

# Save topology to file
topology_config_file_path = os.path.join(project_dir,
        "config-files/mesh-topology-desc/randomized_topology.json")
with open(topology_config_file_path, "w") as f:
    f.write(json.dumps(mesh))
if args.verbose:
    print("Topology .json file written to {}".format(topology_config_file_path))


resc_file_template = """
############################################################
#THIS FILE IS AUTO GENERATED - DO NOT CHANGE DIRECTLY      #
############################################################

logLevel 0

emulation CreateBLEMedium "wireless"
emulation SetGlobalQuantum "0.00001"
emulation SetGlobalSerialExecution true
emulation SetSeed 42
emulation LogBLETraffic
logLevel -1 wireless

# mesh begin  
#############################################################
{% for node in nodes_temp %} 
mach add "{{ node }}"
mach set "{{ node }}"
machine LoadPlatformDescription @platforms/cpus/nrf52840.repl 

# set unique identification address 
sysbus Tag <0x100000a4, 0x100000a7> "DEVICEADDR[0]" {{ 
["0x", ((nodes_temp[node]['addr_bt_le'])|replace(":", "") | replace("C0",""))]|join("") 
}} false

# connect to medium 
connector Connect sysbus.radio wireless
wireless SetPosition sysbus.radio {{ nodes_temp[node]['x'] }} {{ nodes_temp[node]['y'] }} 0
wireless SetRangeWirelessFunction {{ radio_range }}

showAnalyzer sysbus.uart0
{% endfor %}

###########################################################
# mesh end 

mach add "mobile_broadcaster"
mach set "mobile_broadcaster"

machine LoadPlatformDescription @platforms/cpus/nrf52840.repl 

showAnalyzer sysbus.uart0
connector Connect sysbus.radio wireless
wireless SetPosition sysbus.radio 0 0 0
wireless SetRangeWirelessFunction {{ radio_range }}

macro reset
\"\"\"
    pause
    
    # mesh begin 
    ###########################################################
    {% for node in nodes_temp %}
    mach set "{{ node }}"
    sysbus LoadELF $ORIGIN/../../zephyr-rtos/build/zephyr/zephyr.elf 
    {% endfor %}

    ###########################################################
    # mesh end

    mach set "mobile_broadcaster"
    sysbus LoadELF $ORIGIN/../../mobile_broadcaster/build/zephyr/zephyr.elf

\"\"\"
runMacro $reset

{{ "i $ORIGIN/../../renode-commands/move_radio.py" if include_mbmove}}

start

{{ 'watch "move" 20000' if include_mbmove }}  

machine StartGdbServer 3333 true

"""

env = Environment()

# mach create 
template = env.from_string(resc_file_template)
output_resc = template.render(
        nodes_temp=mesh,
        radio_range=RADIO_RANGE, 
        include_mbmove=args.mbmove)

resc_file_path = os.path.join(project_dir,
        "config-files/renode-resc-files/randomized_topology.resc")
with open(resc_file_path, 'w') as rescfile:
    rescfile.write(output_resc)

if args.verbose:
    print("Renode's .resc file written to {}".format(resc_file_path))











