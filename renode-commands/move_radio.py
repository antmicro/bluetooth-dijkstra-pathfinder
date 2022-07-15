import math

counter = 0

def mc_move():
    global counter
    
    mobile_broadcaster_positions = [
            (1.5, 4.5, 0.0),
            (1.5, 1.5, 0.0),
            (-1.5, 1.5, 0.0),
            (-1.5, 4.5, 0.0)
            ]

    machine = monitor.Machine
    succ, wireless = emulationManager.Instance.CurrentEmulation.TryGetEmulationElementByName('wireless', machine)

    succ, radio= emulationManager.Instance.CurrentEmulation.TryGetEmulationElementByName('sysbus.radio', machine)
    
    x, y, z = mobile_broadcaster_positions[counter]

    print(machine)
    print("X: ", x, " Y: ", y, "Counter: ", counter)
    wireless.SetPosition(radio, x, y, z)
    counter += 1
    counter = counter % len(mobile_broadcaster_positions) 


