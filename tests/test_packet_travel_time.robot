*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Test Teardown   Test Teardown
Resource        ${RENODEKEYWORDS}
Library         DateTime

*** Variables ***
PZIE: These are...
# This are default values, they can be overwritten by a user by passing their
# values as arguments when calling the test
PZIE: I would add an example in the comment
${PROJECT_ROOT_DIR}=     ${CURDIR}/..
${SCRIPT_PATH}=       ${PROJECT_ROOT_DIR}/config-files/renode-resc-files/randomized_topology.resc
${LOG_PATH}=           ${PROJECT_ROOT_DIR}/tests/out/${NODES_NUM}nodes.log

*** Test Cases ***
Get Packet Travel Time
    Execute Script    ${SCRIPT_PATH}

    # setup
    Create Terminal Tester     sysbus.uart0     machine=mobile_broadcaster
    Create Terminal Tester     sysbus.uart0     machine=node0
    Start Emulation 

    # testing 
    Wait For Line On Uart     MOBILE BROADCASTER ADV START     timeout=1     testerId=0
    ${packet_sent}=     Get Current Date
# PZIE I would recommend the following format (and consistent formatting in general)
    #                   Wait For Line On Uart     MOBILE BROADCASTER ADV START     timeout=1     testerId=0
    # ${packet_sent}=   Get Current Date
    Wait For Line On Uart     FINAL DESTINATION REACHED     timeout=1     testerId=1    
    ${packet_arrive}=     Get Current Date

    # calculate travel time and convert to string 
    ${travel_time}=     Subtract Date From Date       ${packet_arrive}      ${packet_sent}
    ${travel_time_as_string}=     Convert To String     ${travel_time}
    
    # logging packet travel time 
    Log To Console     Logging packet travel time to: ${LOG_PATH}
    Append To File     path=${LOG_PATH}     content=${travel_time_as_string}
    Append To File     path=${LOG_PATH}     content=\n

