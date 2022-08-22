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
parser = argparse.ArgumentParser(usage="""python3 topology_randomizer.py [NUMBER OF NODES]
Utility to randomly create specified amount of nodes and connect them depending on the distance between the nodes. It outputs .resc file for Renode simulation and .json file for further code generation. When --visualize option is specified, then also a network.html file is generated indicating nodes placement in the space and their connections.
""")
parser.add_argument("nodes_n", 
        help="number of nodes to generate, integer in range [1, 64]", 
        type=int)
parser.add_argument("--visualize",
        help="generate a .html file visualizing network topology",
        action="store_true")
parser.add_argument("--verbose",
        help="explain what is being done",
        action="store_true")
args = parser.parse_args()
if args.nodes_n and args.nodes_n < 1 or args.nodes_n > 64:
    parser.error("ERROR: Value out of range. Specify amount in range [1, 64]")

if args.verbose:
    print("Input correct. Generating {} nodes...".format(args.nodes_n)) 

# specified root directory of the project 
project_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")

index = 0
mesh = {}

# Visualize network 
if args.visualize:
    net = Network('500px', '500px')

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
    if args.visualize: 
        # Add to visualization
        net.add_node(index, "node" + str(index), x=x_pos, y=y_pos) # type: ignore


# add edges 
RADIO_RANGE = int(1.0 / float(args.nodes_n) * 500 * 4)
print("RADIO_RANGE: {}".format(RADIO_RANGE))
for node in mesh.values():
    for neigh in mesh.values():
        # Do not connect to self
        if node["addr"] == neigh["addr"]:
            continue

        d = math.sqrt(math.pow(node["x"] - neigh["x"], 2) + math.pow(node["y"] - neigh["y"], 2))
        if args.verbose:
            print("distance between: {} and {} is {}".format(node["addr"], neigh["addr"], d))
            print("distance between: {},{} and {},{} is {}".format(
                node["x"], node["y"], 
                neigh["x"], neigh["y"], d))
        if d < RADIO_RANGE:
            node["paths_size"] += 1
            node["paths"].append(dict(addr=neigh["addr"], distance=int(d)))
            
            if args.verbose:
                print("adding edge between: {} and {}".format(node["addr"], neigh["addr"]))
            if args.visualize:
                net.add_edge(node["addr"], neigh["addr"], weight=int(d), title=int(d)) # type: ignore

if args.visualize:
    network_html_path = "network_topology.html"
    net.show(network_html_path) # type: ignore
    if args.verbose:
        print("Network topology .html file written to: {}".format(network_html_path))

topology_config_file_path = os.path.join(project_dir,
        "config-files/mesh-topology-desc/randomized_topology.json")
with open(topology_config_file_path, "w") as f:
    f.write(json.dumps(mesh))

if args.verbose:
    print("Topology .json file written to {}".format(topology_config_file_path))


# .resc generation 
# ble_addr

constant_contents_prepend = """
############################################################
#THIS FILE IS AUTO GENERATED - DO NOT CHANGE DIRECTLY
############################################################

logLevel 0

emulation CreateBLEMedium "wireless"
emulation SetGlobalQuantum "0.00001"
emulation SetGlobalSerialExecution true
emulation SetSeed 42
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

showAnalyzer sysbus.uart0
connector Connect sysbus.radio wireless
wireless SetPosition sysbus.radio 0 0 0
wireless SetRangeWirelessFunction 5

macro reset
\"\"\"
    pause
    
    # mesh begin 

"""
constant_contents_append = """
    mach set "mobile_broadcaster"
    sysbus LoadELF $ORIGIN/../../mobile_broadcaster/build/zephyr/zephyr.elf

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

# connect to medium 
connector Connect sysbus.radio wireless
wireless SetPosition sysbus.radio {{ nodes_temp[node]['x'] }} {{ nodes_temp[node]['y'] }} 0
wireless SetRangeWirelessFunction {{radio_range}}
{% endfor %}

"""

mach_load_desc_template = """
    {% for node in nodes_temp %}
    mach set "{{ node }}"
    sysbus LoadELF $ORIGIN/../../zephyr-rtos/build/zephyr/zephyr.elf 
    {% endfor %}
"""

env = Environment()

# mach create 
template = env.from_string(mach_create_template)
out_mach_create = template.render(nodes_temp = mesh, radio_range = RADIO_RANGE)

# desc load
template = env.from_string(mach_load_desc_template)
out_mach_load_desc = template.render(nodes_temp = mesh)

contents = (
        constant_contents_prepend 
        + out_mach_create 
        + constant_contents_mid 
        + out_mach_load_desc 
        + constant_contents_append)

resc_file_path = os.path.join(project_dir,
        "config-files/renode-resc-files/randomized_topology.resc")
with open(resc_file_path, 'w') as rescfile:
    rescfile.write(contents)

if args.verbose:
    print("Renode's .resc file written to {}".format(resc_file_path))











