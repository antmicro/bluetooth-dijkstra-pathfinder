import json
import os 
from jinja2 import Template, PackageLoader, Environment, FileSystemLoader


constant_contents_prepend = [
        "    // graph mutex initialization \n",
        "    k_mutex_init(graph_mutex); \n",
        "\n",
        "    // nodes initialization \n",
        "    *graph = k_malloc(MAX_MESH_SIZE * sizeof(struct node_t)); \n",
        "    if((*graph) == NULL) return 1; \n",
        "\n"
        ]
constant_contents_append = [
        "return 0; \n"
        ]


def find_cfunction_scope(function_name, file_contents):
    left_bracket_cnt = 0
    right_bracket_cnt = 0
    function_found = False
    
    for index, line in enumerate(file_contents):
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
                end = index 
                break

    if not function_found:
        print("Function not found!")
        return {}
    print("this is scope" + str(begin) + " " + str(end))
    return {"begin": begin, "end":end}


def clear_in_scope(scope, file_contents):
    first_part = file_contents[:scope['begin']]
    second_part = file_contents[scope['end']:]
    return first_part + second_part


def insert_contents(start, function_contents, file_contents):
    first_part = file_contents[:start]
    second_part = file_contents[start:]
    global constant_contents_prepend
    global constant_contents_append
    output = (
            first_part 
            + constant_contents_prepend
            + function_contents 
            + constant_contents_append
            + second_part 
            )
    return output 


""" Script """
if __name__ == "__main__":
    # create jinja enviroment and load a template 
    execution_dir = os.path.dirname(os.path.abspath(__file__))
    loader = FileSystemLoader(execution_dir)
    env = Environment(loader=loader)
    template = env.get_template("node_init_template.c.j2")

    # fill the template with data from json config file
    config_file_path = os.path.join(execution_dir, "config.json") 
    with open(config_file_path, 'r') as config:
        function_new_contents = []
        nodes_config = json.loads(config.read())
        for node in nodes_config:
            out = template.render(
                    addr_t=node['addr'],
                    reserved_t=node['reserved'],
                    paths_size_t=node['paths_size'],
                    paths=node['paths']
                    )
            function_new_contents.append(out)

    # read contents of a file that will be impacted 
    target_file_path = os.path.join(execution_dir,  "zephyr-rtos/src/graph.c")
    with open(target_file_path, 'r') as filehandle:
        contents = filehandle.readlines()

        # get scope of a function to be filled with autogenerated code 
        scope = find_cfunction_scope("graph_init", contents)

    with open(target_file_path, 'w') as filehandle:
        # remove contents of a function in given scope  
        cleaned_file_contents = clear_in_scope(scope, contents)

        # insert new autogenerated content
        completed_file = insert_contents(scope['begin'], 
                function_new_contents, cleaned_file_contents)

        # write to file 
        for line in completed_file:
            filehandle.write(line)

