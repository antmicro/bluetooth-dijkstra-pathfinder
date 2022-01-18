import json
import os 
from jinja2 import Template, PackageLoader, Environment, FileSystemLoader

loader = FileSystemLoader(os.path.abspath("."))
env = Environment(loader=loader)
template = env.get_template("node_init_template.c.j2")

with open("config.json", 'r') as config:
    for line in config:
        print("Config content:")
        print(line)
        node_config = json.loads(line)
        out = template.render(
                addr_t=node_config['addr'],
                reserved_t=node_config['reserved'],
                paths_size_t=node_config['paths_size'],
                paths=node_config['paths']
                )
        print(out)
        
