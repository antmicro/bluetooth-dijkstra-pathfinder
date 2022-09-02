import random as rnd
import json
import os
from jinja2 import Environment
from pyvis.network import Network
import math
import argparse
import warnings

# constants
AREA_X_DIM = 500
AREA_Y_DIM = 500

# validate input
parser = argparse.ArgumentParser(
    usage="""python3 topology_randomizer.py [NUMBER OF NODES]
Utility to randomly create specified amount of nodes and connect them depending on the distance between the nodes. It outputs .resc file for Renode simulation and .json file for further code generation. When --visualize option is specified, then also a network.html file is generated indicating nodes placement in the space and their connections.
"""
)
parser.add_argument(
    "nodes_n", help="Number of nodes to generate, integer in range [1, 64]", type=int
)
parser.add_argument(
    "--mbmove",
    help="If set, mobile broadcaster will be moving with a use of provided move script extension for Renode",
    action="store_true",
)
parser.add_argument(
    "--faulty_nodes",
    help="Specify number of faulty_nodes, that will be included in .json topology file but will not be initialized in Renode simualtion, leading to unresponsive member of the network. Use for testing rerouting capabilities of the network. Node 0 cannot be faulty, as by default it is sink",
    type=int,
    default=0,
)
parser.add_argument(
    "--visualize",
    help="Generate a .html file visualizing network topology",
    action="store_true",
)
parser.add_argument(
    "--visualizemb",
    help="Include mobile_broadcaster positions in the visualization",
    action="store_true",
)
parser.add_argument("--verbose", help="Explain what is being done", action="store_true")
parser.add_argument(
    "--mbpathfile",
    help="Provide name of the file containing a path for the mobile_broadcaster. Should be valid .json file contained in the directory config-files/mb-paths. Examplary default file in this dir is square_corners_path.json file and it should not be modified.",
    type=str,
    default="square_corners_path.json",
)

args = parser.parse_args()
if args.nodes_n and args.nodes_n < 1 or args.nodes_n > 64:
    parser.error("ERROR: Value out of range. Specify amount in integer range [1, 64]")
if args.faulty_nodes < 0 or args.faulty_nodes > args.nodes_n - 1:
    parser.error(
        "ERROR: Number of faulty nodes should be integer in range (0, NUMBER OF NODES)"
    )
if not args.mbmove:
    warnings.warn(
        "WARNING: switch --mbmove was not provided so the MB WILL NOT MOVE from it's default position, but depending on provided --mbpathfile and --visualizemb options multiple path points may appear in visualization. If this is intended, ignore this warning."
    )

# mobile broadcaster path config file
mb_path_file = os.path.join("config-files/mb-paths", args.mbpathfile)
if not os.path.exists(mb_path_file):
    parser.error("Provided config file does not exist: {}".format(mb_path_file))

# Output topology file path
topology_config_file_path = "config-files/mesh-topology-desc/randomized_topology.json"

if not os.path.exists(topology_config_file_path):
    topology_dir = os.path.dirname(topology_config_file_path)
    os.makedirs(topology_dir, exist_ok=True)
    with open(topology_config_file_path, "a") as f:
        pass

# Output Renode's resc file
resc_file_path = "config-files/renode-resc-files/randomized_topology.resc"

if not os.path.exists(resc_file_path):
    resc_dir = os.path.dirname(resc_file_path)
    os.makedirs(resc_dir, exist_ok=True)
    with open(resc_file_path, "a") as f:
        pass

# Load positions used in both visualization and generation of .resc file
with open(mb_path_file) as f:
    mb_positions = json.load(f)

# Visualize network
if args.visualize:
    net = Network("500px", "500px")


# Randomly pick faulty nodes and keep track of their ids. We will remove them
# from the mesh before initializing .resc file to simulate faulty_nodes
faulty_nodes_indexes = []
if args.faulty_nodes != 0:
    faulty_nodes_indexes = rnd.sample(range(1, args.nodes_n), args.faulty_nodes)


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
        "addr": index,
        "x": x_pos,
        "y": y_pos,
        "addr_bt_le": ble_addr,
        "reserved": True,
        "paths_size": 0,
        "paths": [],
    }
    last_node_index = index

    if args.visualize:
        if index in faulty_nodes_indexes:  # type: ignore
            net.add_node(index, "node" + str(index), x=x_pos, y=y_pos, color="#dd4b39")  # type: ignore
        else:
            net.add_node(index, "node" + str(index), x=x_pos, y=y_pos)  # type: ignore


# Helper function to calculate distance between two points on a plane
def euclidean_distance(x1, y1, x2, y2):
    return math.sqrt(math.pow(x1 - x2, 2) + math.pow(y1 - y2, 2))


# add edges between nodes within RADIO_RANGE
RADIO_RANGE = int(1.0 / float(args.nodes_n) * 500 * 4)
print("RADIO_RANGE: {}".format(RADIO_RANGE))  # TODO: change to verbose
for node in mesh.values():
    for neigh in mesh.values():
        # Do not connect to self
        if node["addr"] == neigh["addr"]:
            continue

        d = euclidean_distance(node["x"], node["y"], neigh["x"], neigh["y"])
        if args.verbose:
            print(
                "distance between: {} and {} is {}".format(
                    node["addr"], neigh["addr"], d
                )
            )

        # Nodes are connected based on physical distance between them, but cost
        # used in Dijkstra calculation is different than pure physical distance,
        # it also may include other factors like signal strength etc.
        if d < RADIO_RANGE:
            node["paths_size"] += 1
            node["paths"].append(
                dict(
                    addr=neigh["addr"],
                    cost=0,  # used for dijkstra calc, evaluated in graph_init function in .c file
                    factors=[
                        {"name": "signal_str", "value": 5, "factor": 1},
                        {"name": "phy_distance", "value": int(d), "factor": 1},
                        {"name": "missed_transmissions", "value": 0, "factor": 1},
                    ],
                )
            )
            if args.visualize:
                if node["addr"] in faulty_nodes_indexes or neigh["addr"] in faulty_nodes_indexes:  # type: ignore
                    net.add_edge(node["addr"], neigh["addr"], weight=int(d * 1 + 5 * 1 + 0 * 1), title=int(d), color="#dd4b39")  # type: ignore
                else:
                    net.add_edge(node["addr"], neigh["addr"], weight=int(d), title=int(d))  # type: ignore


# Visualize also positions of MB and to what nodes it will be connected
if args.visualize and args.visualizemb:
    for i, position in enumerate(mb_positions):
        mb_node_id = last_node_index + i + 1
        net.add_node(
            mb_node_id,
            "MB_pos" + str(i),
            x=position[0],
            y=position[1],  # type: ignore
            physics=False,
            color="#00ff1e",
        )
        for node in mesh.values():
            d = euclidean_distance(node["x"], node["y"], position[0], position[1])
            if d < RADIO_RANGE:
                if node["addr"] in faulty_nodes_indexes:  # type: ignore
                    net.add_edge(mb_node_id, node["addr"], color="#dd4b39")  # type: ignore
                else:
                    net.add_edge(mb_node_id, node["addr"])  # type: ignore


# Save topology to .json file
with open(topology_config_file_path, "w") as f:
    f.write(json.dumps(mesh))
if args.verbose:
    print("Topology .json file written to {}".format(topology_config_file_path))


# If some faulty nodes were specified, we will knock them out at random from
# mesh dictionary that will be later used to initialize a .resc file, but they
# are still present in topology .json file. This leads to unresponsive nodes.
if args.faulty_nodes != 0:
    for i in faulty_nodes_indexes:  # type: ignore
        node_name = "node" + str(i)
        mesh.pop(node_name)


# Save and display visualization
if args.visualize:
    network_html_path = "network_topology.html"
    net.show(network_html_path)  # type: ignore
    if args.verbose:
        print("Network topology .html file written to: {}".format(network_html_path))


resc_file_template = """
############################################################
#THIS FILE IS AUTO GENERATED - DO NOT CHANGE DIRECTLY      #
############################################################

logLevel 0

emulation CreateBLEMedium "wireless"
emulation SetGlobalQuantum "0.00001"
emulation SetGlobalSerialExecution true
emulation SetSeed 42
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
    sysbus LoadELF $ORIGIN/../../node/build/zephyr/zephyr.elf
    {% endfor %}

    ###########################################################
    # mesh end

    mach set "mobile_broadcaster"
    sysbus LoadELF $ORIGIN/../../mobile_broadcaster/build/zephyr/zephyr.elf

\"\"\"
runMacro $reset

{{ "i $ORIGIN/../../scripts/renode_commands.py" if include_mbmove }}
{{ "# Included here ^ Renode command written in Python" if include_mbmove }}
{{ "load_mb_path $ORIGIN/../../" ~ mb_positions_path if include_mbmove }}

start

{{ 'watch "move" 30000' if include_mbmove }}
"""

env = Environment()

# mach create
template = env.from_string(resc_file_template)
output_resc = template.render(
    nodes_temp=mesh,
    radio_range=RADIO_RANGE,
    include_mbmove=args.mbmove,
    mb_positions_path=mb_path_file,
)

with open(resc_file_path, "w") as rescfile:
    rescfile.write(output_resc)

if args.verbose:
    print("Renode's .resc file written to {}".format(resc_file_path))
