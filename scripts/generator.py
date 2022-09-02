import json
import os
import sys
from jinja2 import Environment

template_graph_api_h = """
/* THIS FILE IS AUTO GENERATED - DO NOT MODIFY DIRECTLY 
 * Check Your build options (arguments passed to build_as.sh script) and change
 * proper config-files/mesh-topology-desc/*.json file. 
 */

#ifndef GRAPH_API_GENERATED_H 
#define GRAPH_API_GENERATED_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel.h> 
#include <bluetooth/addr.h>

#define MAX_MESH_SIZE {{ nodes_n }}

{% for factor in factors -%}
#define {{ factor.name.upper() }} {{ factor.factor }}
{% endfor -%}

// addr and cost fields are always present, rest is defined in topology.json 
// file and loaded automatically
struct path_t {
    uint8_t addr;
    uint16_t cost;
    struct k_mutex path_mutex;
    {% for factor in factors -%}
    uint16_t {{factor.name}};
    {% endfor -%}
};

uint16_t calc_cost({% for factor in factors %}{% if loop.index0 != 0 %}, {% endif %}uint16_t {{ factor.name }}{% endfor %});

// Setters and getters respecting mutex access 
{% for factor in factors %}
int path_t_{{ factor.name }}_set(struct path_t *path, uint16_t new_val);
int path_t_{{ factor.name }}_get(struct path_t *path, uint16_t *ret_val);
{% endfor %}

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
#include "../../include/graph.h"

uint16_t calc_cost({% for factor in factors %}{% if loop.index0 != 0 %}, {% endif %}uint16_t {{ factor.name }}{% endfor %}) {
    return {% for factor in factors %}{% if loop.index0 != 0 %} + {% endif %}{{ factor.factor }} * {{ factor.name }}{% endfor %};
}

uint8_t graph_init(struct node_t *graph){ 
    {% for node in nodes.values() %}
    // node {{ node.addr }} 
    graph[{{ node.addr }}].addr = {{ node.addr }};
    strncpy(graph[{{ node.addr }}].addr_bt_le, "{{ node.addr_bt_le }}", 18);
    graph[{{ node.addr }}].reserved = {% if node.reserved %}true{% else %}false{% endif %};
    graph[{{ node.addr }}].visited = false;
    graph[{{ node.addr }}].tentative_distance = INF;
    graph[{{ node.addr }}].paths_size = {{ node.paths_size }};
    k_mutex_init(&graph[{{ node.addr }}].node_mutex);
    {% if node.paths_size is greaterthan 0 %}
    graph[{{ node.addr }}].paths = k_malloc(sizeof(struct path_t) * graph[{{node.addr}}].paths_size);
    if(graph[{{ node.addr }}].paths == NULL) return 1;
    {% for path in node.paths %}
    (graph[{{ node.addr }}].paths + {{loop.index0}})->addr = 0x{{ path.addr }};
    (graph[{{ node.addr }}].paths + {{loop.index0}})->cost = calc_cost({% for factor in path.factors %}{% if loop.index0 != 0 %}, {% endif %}{{ factor.value }}{% endfor %});
    {%- set outer_loop = loop -%}
    {%- for factor in path.factors %}
    (graph[{{ node.addr }}].paths + {{outer_loop.index0}})->{{ factor.name }} = {{ factor.value }};
    {%- endfor %}
    {%- endfor %}
    {% endif %}
    {% endfor %}
    return 0;
}

{% for factor in factors %}
int path_t_{{ factor.name }}_set(struct path_t *path, uint16_t new_val) {
    int err;
    err = k_mutex_lock(&path->path_mutex, K_FOREVER);
    if(err) return err;
    path->{{ factor.name }} = new_val;
    k_mutex_unlock(&path->path_mutex);
    if(err) return err;
    return 0;
}
int path_t_{{ factor.name }}_get(struct path_t *path, uint16_t *ret_val) {
    int err;
    err = k_mutex_lock(&path->path_mutex, K_FOREVER);
    if(err) return err;
    *ret_val = path->{{ factor.name }};
    k_mutex_unlock(&path->path_mutex);
    if(err) return err;
    return 0;
}
{% endfor %}

"""

# check if filepath was provided
if len(sys.argv) < 2:
    sys.exit("Specify path to configuration file!")

# check if file exists
config_file_path = sys.argv[1]
if not os.path.isfile(config_file_path):
    sys.exit("ERROR: Incorrect filepath to .json file with topology config: {}".format(config_file_path))

# create jinja enviroment and load a template
env = Environment()
template_c = env.from_string(template_graph_api_c)
template_h = env.from_string(template_graph_api_h)

print("Running generator script...")
print("Topology config file path:")
print(config_file_path)
with open(config_file_path, "r") as config:
    nodes_config = json.loads(config.read())
    nodes_n = len(nodes_config)

    # Get first element of paths of the first node, just as template for function
    cost_factors = list(nodes_config.values())[0]["paths"][0]["factors"]

    out_c = template_c.render(nodes=nodes_config, factors=cost_factors)
    out_h = template_h.render(factors=cost_factors, nodes_n=nodes_n)


target_file_c_path = "node/src/generated-src/graph_api_generated.c"
if not os.path.exists(target_file_c_path):
    os.makedirs(os.path.dirname(target_file_c_path), exist_ok=True) 
with open(target_file_c_path, "w") as filehandle:
    for line in out_c:
        filehandle.write(line)

target_file_h_path = "node/include/graph_api_generated.h"
if not os.path.exists(target_file_h_path):
    os.makedirs(os.path.dirname(target_file_h_path), exist_ok=True)
with open(target_file_h_path, "w") as filehandle:
    for line in out_h:
        filehandle.write(line)

print("Successfully generated:")
print("    node/src/generated-src/graph_api_generated.c")
print("    node/include/graph_api_generated.h")
