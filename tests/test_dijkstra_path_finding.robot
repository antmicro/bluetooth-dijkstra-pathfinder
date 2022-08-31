*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Test Teardown   Test Teardown
Resource        ${RENODEKEYWORDS}


*** Variables ***
${PROJECT_ROOT_DIR}=     ${CURDIR}/..
${SCRIPT_PATH}=       ${PROJECT_ROOT_DIR}/config-files/renode-resc-files/basic_5_nodes.resc

*** Test Cases ***
Test Data Packet Transmission 
#PZIE: formatting
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
    Wait For Line On Uart     Path:           timeout=0.5
    Wait For Line On Uart     0               timeout=0.5
    Wait For Line On Uart     4               timeout=0.5
    Wait For Line On Uart     2               timeout=0.5
    Wait For Line On Uart     Next hop: 4     timeout=0.5

    # Check for new tentative_distance calculation
    Wait For Line On Uart     New calculated TD: 1    timeout=1
    Wait For Line On Uart     New calculated TD: 3    timeout=1

    # Check for replanned path
    Wait For Line On Uart     Path:           timeout=0.5
    Wait For Line On Uart     0               timeout=0.5
    Wait For Line On Uart     1               timeout=0.5
    Wait For Line On Uart     2               timeout=0.5
    Wait For Line On Uart     Next hop: 1     timeout=0.5


