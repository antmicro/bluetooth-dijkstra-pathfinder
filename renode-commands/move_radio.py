import json
counter = 0

def mc_move(path):
    global counter
    with open(path) as f:
        mobile_broadcaster_positions = json.load(f)
     
    machine = monitor.Machine
    succ, wireless = emulationManager.Instance.CurrentEmulation.TryGetEmulationElementByName('wireless', machine)

    succ, radio= emulationManager.Instance.CurrentEmulation.TryGetEmulationElementByName('sysbus.radio', machine)
    
    x, y, z = mobile_broadcaster_positions[counter]

    print(machine)
    print("X: ", x, " Y: ", y, "Counter: ", counter)
    wireless.SetPosition(radio, x, y, z)
    counter += 1
    counter = counter % len(mobile_broadcaster_positions) 


