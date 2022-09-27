*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Test Teardown   Test Teardown
Resource        ${RENODEKEYWORDS}

*** Variables ***
${PROJECT_ROOT_DIR}=     ${CURDIR}/..
${SCRIPT_PATH}=       ${PROJECT_ROOT_DIR}/config-files/renode-resc-files/basic_5_nodes.resc

# THIS TEST WILL ONLY WORK FOR BASIC 5 NODE BUILD!
*** Test Cases ***
Test Data Packet Transmission
    Execute Script    ${SCRIPT_PATH}
    Create Terminal Tester    sysbus.uart0    machine=node2
    Create Network Interface Tester    sysbus.radio    machine=node2
    Start Emulation

    # Check if advertisement of data packet started
    Wait For Line On Uart    Started advertising a data packet.    timeout=0.5
    Wait For Outgoing Packet    0.5

Test Dijkstra Path Planning
    Execute Script    ${SCRIPT_PATH}
    Create Terminal Tester     sysbus.uart0    machine=node2
    Start Emulation

    # Check path calculation
    Wait For Line On Uart     Path:           timeout=1
    Wait For Line On Uart     2->4->0         timeout=1

    # Check for new tentative_distance calculation
    
    Wait For Line On Uart     New calculated transition cost: 361    timeout=10

    # Check for replanned path
    Wait For Line On Uart     Path:           timeout=1
    Wait For Line On Uart     2->1->0        timeout=1  
    Wait For Line On Uart     Next hop: 1     timeout=1


