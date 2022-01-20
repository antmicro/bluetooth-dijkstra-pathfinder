import json
import os 
from jinja2 import Template, PackageLoader, Environment, FileSystemLoader

constant_contents_prepend = """
/* THIS FILE IS AUTO GENERATED - DO NOT MODIFY DIRECTLY */
#include <stdint.h>
#include <stdlib.h>

#include "../../include/graph.h"

uint8_t graph_init(struct node_t ** graph, struct k_mutex * graph_mutex){ 
    // graph mutex initialization 
    k_mutex_init(graph_mutex); 

    // nodes initialization 
    *graph = k_malloc(MAX_MESH_SIZE * sizeof(struct node_t));
    if((*graph) == NULL) return 1;
    """

constant_contents_append = """return 0;
}"""
        

template_to_load = """// node 0x{{ addr_t }} 
    (*graph + {{ addr_t }})->reserved = {{ reserved_t }};
    (*graph + {{ addr_t }})->visited = false;
    (*graph + {{ addr_t }})->tentative_distance = INF;
    (*graph + {{ addr_t }})->paths_size = {{ paths_size_t }};
    {% if paths_size_t is greaterthan 0 %}
    (*graph + {{ addr_t }})->paths = k_malloc(sizeof(struct path_t) * (*graph)->paths_size);
    if((*graph + {{ addr_t }})->paths == NULL) return 1;
    {% for path in paths %}
    (*graph + {{ addr_t }})->paths->addr = 0x{{ path.addr }};
    (*graph + {{ addr_t }})->paths->distance = {{ path.distance }};
    {% endfor %}
    {% endif %}
    """
    
# create jinja enviroment and load a template 
env = Environment()
template = env.from_string(template_to_load)

# fill the template with data from json config file
project_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")
config_file_path = os.path.join(project_dir, "topology_config.json") 
print("cnf file path")
print(config_file_path)
with open(config_file_path, 'r') as config:
    function_contents = "" 
    nodes_config = json.loads(config.read())
    for node in nodes_config.values():
        out = template.render(
                addr_t=node['addr'],
                reserved_t=str(node['reserved']).lower(),
                paths_size_t=node['paths_size'],
                paths=node['paths']
                )
        function_contents += out

output = constant_contents_prepend + function_contents + constant_contents_append 

target_file_path = os.path.join(project_dir, "zephyr-rtos/src/generated-src/graph_init.c")
with open(target_file_path, 'w') as filehandle:
    for line in output:
        filehandle.write(line)

