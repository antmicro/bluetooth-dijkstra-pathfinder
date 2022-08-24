import json
import os 
import sys
from jinja2 import Environment

template_graph_api_h = """
#ifndef GRAPH_API_GENERATED_H 
#define GRAPH_API_GENERATED_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel.h> 
#include <bluetooth/addr.h>
#include "graph.h"

{% for factor in factors -%}
#define {{ factor.name.upper() }} {{ factor.factor }}
{% endfor %}
uint8_t graph_init(struct node_t *graph);
uint16_t calc_cost({% for factor in factors %}{% if loop.index0 != 0 %}, {% endif %}uint16_t {{ factor.name }}{% endfor %});
void node_update_missed_transmissions(struct node_t *node, 
        bool transmission_success);

#endif
"""


template_graph_api_c = """
/* THIS FILE IS AUTO GENERATED - DO NOT MODIFY DIRECTLY 
 * Check Your build options (arguments passed to build_as.sh script) and change
 * proper config-files/mesh-topology-desc/*.json file. 
 */

#include <stdint.h>
#include <stdlib.h>
#include <bluetooth/addr.h>

#include "../../include/graph_api_generated.h"

uint16_t calc_cost({% for factor in factors %}{% if loop.index0 != 0 %}, {% endif %}uint16_t {{ factor.name }}{% endfor %}) {
    return {% for factor in factors %}{% if loop.index0 != 0 %} * {% endif %}{{ factor.factor }} * {{ factor.name }}{% endfor %};
}

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
    (graph[{{ node.addr }}].paths + {{loop.index0}})->cost = {{ path.cost}};
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
template_c = env.from_string(template_graph_api_c)
template_h = env.from_string(template_graph_api_h)

print("Topology config file path:")
print(config_file_path)
with open(config_file_path, 'r') as config:
    nodes_config = json.loads(config.read())
    
    # Get first element of paths of the first node, just as template for function
    cost_factors = list(nodes_config.values())[0]['paths'][0]['factors']

    out_c = template_c.render(nodes=nodes_config, factors=cost_factors)
    out_h = template_h.render(factors=cost_factors)


project_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
target_file_c_path = os.path.join(project_dir, "zephyr-rtos/src/generated-src/graph_api_generated.c")
with open(target_file_c_path, 'w') as filehandle:
    for line in out_c:
        filehandle.write(line)

target_file_h_path = os.path.join(project_dir, "zephyr-rtos/include/graph_api_generated.h")
with open(target_file_h_path, 'w') as filehandle:
    for line in out_h:
        filehandle.write(line)


