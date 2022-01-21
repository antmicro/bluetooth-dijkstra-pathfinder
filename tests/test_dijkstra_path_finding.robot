*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Test Teardown   Test Teardown
Resource        ${RENODEKEYWORDS}

*** Variables ***
${SCRIPT_PATH} =      ../init.resc

*** Test Cases ***
Should Print Route
    Execute Script    ${SCRIPT_PATH}
    Create Terminal Tester     sysbus.uart0
    Start Emulation 
    Wait For Line On Uart     Path: 2, 1, 0,     timeout=0.5     
