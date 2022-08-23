import json
import os 
import sys
from jinja2 import Environment


template_to_load = """
/* THIS FILE IS AUTO GENERATED - DO NOT MODIFY DIRECTLY 
 * Check Your build options (arguments passed to build_as.sh script) and change
 * proper config-files/mesh-topology-desc/*.json file. 
 */

#include <stdint.h>
#include <stdlib.h>
#include <bluetooth/addr.h>

#include "../../include/graph.h"

uint8_t graph_init(struct node_t *graph){ 
    {% for node in nodes.values() %}
    // node 0x{{ node.addr }} 
    graph[{{ node.addr }}].addr = 0x{{ node.addr }};
    strncpy(graph[{{ node.addr }}].addr_bt_le, "{{ node.addr_bt_le }}", 18);
    graph[{{ node.addr }}].reserved = {% if node.reserved %}true{% else %}false{% endif %};
    graph[{{ node.addr }}].visited = false;
    graph[{{ node.addr }}].tentative_distance = INF;
    graph[{{ node.addr }}].missed_transmissions = 0;
    graph[{{ node.addr }}].paths_size = {{ node.paths_size }};
    {% if node.paths_size is greaterthan 0 %}
    graph[{{ node.addr }}].paths = k_malloc(sizeof(struct path_t) * graph[{{node.addr}}].paths_size);
    if(graph[{{ node.addr }}].paths == NULL) return 1;
    {% for path in node.paths %}
    (graph[{{ node.addr }}].paths + {{loop.index0}})->addr = 0x{{ path.addr }};
    (graph[{{ node.addr }}].paths + {{loop.index0}})->distance = {{ path.distance }};
    {% endfor %}
    {% endif %}
    {% endfor %}
    return 0;
}"""

# check if filepath was provided 
if len(sys.argv) < 2:
    sys.exit("Specify path to configuration file!")

# check if file exists 
config_file_path = os.path.realpath(sys.argv[1])
if not os.path.isfile(config_file_path):
    print("ERROR: Provided config file path ", config_file_path)
    sys.exit("ERROR: Incorrect filepath to .json file with topology config")

# create jinja enviroment and load a template 
env = Environment()
template = env.from_string(template_to_load)

print("Topology config file path:")
print(config_file_path)
with open(config_file_path, 'r') as config:
    nodes_config = json.loads(config.read())
    print(nodes_config)
    out = template.render(nodes=nodes_config)
    
project_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
target_file_path = os.path.join(project_dir, "zephyr-rtos/src/generated-src/graph_init.c")
print("Target file path: ")
print(target_file_path)
with open(target_file_path, 'w') as filehandle:
    for line in out:
        filehandle.write(line)

