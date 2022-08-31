#PZIE: accross the whole project - formatting
# PZIE: a separate folder for a single command?
import json
import sys
counter = 0
mobile_broadcaster_positions = []


def mc_load_mb_path(path):
    global mobile_broadcaster_positions
    with open(path) as f:
        mobile_broadcaster_positions = json.load(f)


def mc_move():
    global counter
    global mobile_broadcaster_positions
     
    machine = monitor.Machine
    succ, wireless = emulationManager.Instance.CurrentEmulation.TryGetEmulationElementByName('wireless', machine)
    succ, radio = emulationManager.Instance.CurrentEmulation.TryGetEmulationElementByName('sysbus.radio', machine)
    
    print(mobile_broadcaster_positions)
    x, y, z = mobile_broadcaster_positions[counter]

    print(machine)
    print("X: ", x, " Y: ", y, "Counter: ", counter)
    wireless.SetPosition(radio, x, y, z)
    counter += 1
    counter = counter % len(mobile_broadcaster_positions) 


