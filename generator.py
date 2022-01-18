import json

with open("config.json", 'r') as config:
    for line in config:
        print(line)
        print(json.loads(line))
