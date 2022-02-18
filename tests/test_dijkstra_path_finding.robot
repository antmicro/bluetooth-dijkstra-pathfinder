*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Test Teardown   Test Teardown
Resource        ${RENODEKEYWORDS}


*** Variables ***
${PROJECT_ROOT_DIR}=     ${CURDIR}/..
${SCRIPT_PATH}=       ${PROJECT_ROOT_DIR}/config-files/renode-resc-files/basic_5_nodes.resc

*** Test Cases ***
Should Print Route
    Execute Script    ${SCRIPT_PATH}
    Create Terminal Tester     sysbus.uart0    machine=node2
    Start Emulation 
    Wait For Line On Uart     Path:           timeout=0.5     
    Wait For Line On Uart     0               timeout=0.5     
    Wait For Line On Uart     1               timeout=0.5     
    Wait For Line On Uart     2               timeout=0.5     
    Wait For Line On Uart     Next hop: 1     timeout=0.5     
