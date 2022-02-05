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
        
        if not 1 <= nodes <=64: # is in range 
            raise ValueError("Value not in range [1, 64]")
        

    except ValueError:
        print("Value should be integer in range [1, 64]")
        exit()

else:
    exit()

print("Input correct. Generating " + str(nodes) + " nodes...") 

index = 0
mesh = {}

while index < nodes:
    # randomize number of connections
    paths = [] 

    # pick paths_size not greater than number 
    # of nodes in case MAX_CONN_NUM > nodes
    paths_size = int(rnd.random() * 100) % min(MAX_CONN_NUM, nodes) + 1 
     
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
            "paths_size":paths_size,
            "paths":paths
            }
    index += 1

print(json.dumps(mesh))

with open("tests.json", "w") as f:
    f.write(json.dumps(mesh))

# .resc generation 
# ble_addr

constant_contents_prepend = """
logLevel 0

emulation CreateWirelessMedium "wireless"
emulation CreateWiresharkForWireless "wireless" #"bluetoothle"
emulation LogToWireshark "wireless"
emulation LogWirelessTraffic
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
mach set "node1"
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

project_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")
# config_file_path = os.path.join(project_dir, "topology_config.json") 
config_file_path = os.path.join(project_dir, "scripts/tests.json")
with open(config_file_path, 'r') as config:
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

    print(contents)

for node in mesh:
    print(node)
    print(mesh[node]['addr_bt_le'])

with open("tests.resc", 'w') as rescfile:
    rescfile.write(contents)











