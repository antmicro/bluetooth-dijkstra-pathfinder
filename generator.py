import json
import os 
from jinja2 import Template, PackageLoader, Environment, FileSystemLoader


def find_cfunction_scope(function_name, file_handle):
    left_bracket_cnt = 0
    right_bracket_cnt = 0
    function_found = False
    
    # maybe for sanity check function can compare result to some
    # header with its size that will be appended by a function in
    # previous iterations or by a user in initial cycle
    for index, line in enumerate(file_handle):
        if function_name in line:
            function_found = True
        if function_found:
            if "{" in line:
                if left_bracket_cnt == 0:
                    begin = index + 1 
                left_bracket_cnt += 1

            if "}" in line:
                right_bracket_cnt += 1

            if left_bracket_cnt == right_bracket_cnt:
                end = index - 1
                break

    if not function_found:
        print("Function not found!")
        return {}
    
    print("Scope of a " + function_name + " function is:")
    print("Begin: " + str(begin) + " End: " + str(end)) 
    return {"begin": begin, "end":end}


""" Script """
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

with open("zephyr-rtos/src/graph_experiments.c") as filehandle:
    print(find_cfunction_scope("graph_init", filehandle))



