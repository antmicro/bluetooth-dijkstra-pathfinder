import json
import os 
from jinja2 import Template, PackageLoader, Environment, FileSystemLoader

loader = FileSystemLoader(os.path.abspath("."))
env = Environment(loader=loader)
template = env.get_template("node_init_template.c.j2")

print(template.render(name="Kuba"))

with open("config.json", 'r') as config:
    for line in config:
        print(line)
        print(json.loads(line))
        smth = json.loads(line)
        print(smth["paths"][0]["addr"])
        
        
