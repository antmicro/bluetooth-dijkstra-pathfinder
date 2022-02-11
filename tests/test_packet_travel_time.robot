*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Test Teardown   Test Teardown
Resource        ${RENODEKEYWORDS}
Library         DateTime

*** Variables ***
${SCRIPT_PATH} =       ${CURDIR}/../scripts/tests.resc
${LOG_PATH}=           /home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/tests/out/5nodes.log

*** Test Cases ***
Get Packet Travel Time
    Execute Script    ${SCRIPT_PATH}
    Create Terminal Tester     sysbus.uart0     machine=mobile_broadcaster
    Create Terminal Tester     sysbus.uart0     machine=node0
    Start Emulation 
    Wait For Line On Uart     MOBILE BROADCASTER ADV START     timeout=1     testerId=0
    ${packet_sent}=     Get Current Date
    Wait For Line On Uart     FINAL DESTINATION REACHED     timeout=1     testerId=1    
    ${packet_arrive}=     Get Current Date
    ${travel_time}=     Subtract Date From Date       ${packet_arrive}      ${packet_sent}
    ${travel_time_as_string}=     Convert To String     ${travel_time}
    Append To File     path=${LOG_PATH}     content=${travel_time_as_string}
    Append To File     path=${LOG_PATH}     content=\n

