import math

x = 0 
y = 0
degree_counter = 0.0

def mc_move():
    global x
    global y
    global degree_counter
    radius = 5
    
    machine = monitor.Machine
    succ, wireless = emulationManager.Instance.CurrentEmulation.TryGetEmulationElementByName('wireless', machine)

    succ, radio= emulationManager.Instance.CurrentEmulation.TryGetEmulationElementByName('sysbus.radio', machine)
    
    x = radius * math.cos(math.pi * degree_counter / 180.0) 
    y = radius * math.sin(math.pi * degree_counter / 180.0) + 3.0

    print(machine)
    print("X: ", x, " Y: ", y, " degrees: ", degree_counter)
    wireless.SetPosition(radio, x, y, 0)
    degree_counter += 5.0
    if degree_counter > 360.0:
        degree_counter = 0.0


