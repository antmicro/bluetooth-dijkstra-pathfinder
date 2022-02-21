#!/bin/bash

# fail script if one of the commands fails
set -e

USAGE="Script is automatic test runner for packet travel time measuring robot
test. It runs a test and logs result to a file, until desired number of lines is
reached. 

\$1 - number of nodes, must be integer in range 3 - 64
\$2 - path to Renode root directory 

Example:
./tests_runner.sh 9 ~/path/to/renode/renode
"

PROJECT_ROOT_DIR=${PWD}/..

MIN_NODES_NUM=3
MAX_NODES_NUM=64


# input arguments 
if [ $# -lt 2 ]; then
    echo "ERROR: Necessary arguments not provided!"
    echo "$USAGE"
    exit 1
fi

NODES_NUM=$1
RENODE_ROOT_DIR=$2 #~/Repos/renode

# validate number of nodes
# any numeric value evaluation operations using non-numbers will result in 
# an error which will be implicitly considered as false in shell
# Redirection of standard error is there to hide the "integer expression 
# expected" message that bash prints out in case we do not have a number.

# check if in range and if int
if ! [ "$NODES_NUM" -eq "$NODES_NUM" ] 2>/dev/null; then
    echo "ERROR: Number of nodes must be an integer!" 
    echo "$USAGE"
    exit 1
else
    if ! ([ "$NODES_NUM" -ge "$MIN_NODES_NUM" ] && [ "$NODES_NUM" -le "$MAX_NODES_NUM" ]); then
        echo "ERROR: Number of nodes must be in range 3 - 64"
        echo "$USAGE"
        exit 1
    fi
fi


# check if renode-test exists under provided directory 
if ! [ -f "$RENODE_ROOT_DIR/renode-test" ]; then
    echo "ERROR: Provided directory does not contain renode-test script!"
    echo "$USAGE"
    exit 1
fi

# randomized topology config file path
TOPOLOGY_CONFIG=$PROJECT_ROOT_DIR/config-files/mesh-topology-desc/randomized_topology.json

# logging 
LOG_FILES_DIR=$PROJECT_ROOT_DIR/tests/out
LOG_FILE_NAME=${NODES_NUM}nodes.log
LOG_FILE_PATH=$LOG_FILES_DIR/$LOG_FILE_NAME

# scripts path 
SCRIPTS_PATH=$PROJECT_ROOT_DIR/scripts

# tests path
ROBOT_TESTS_PATH=$PROJECT_ROOT_DIR/tests

# build paths 
SOURCES_DIR=$PROJECT_ROOT_DIR/zephyr-rtos
BUILD_DIR=$PROJECT_ROOT_DIR/zephyr-rtos/build

# mobile broadcaster build paths
MB_SOURCES_DIR=$PROJECT_ROOT_DIR/mobile_broadcaster
MB_BUILD_DIR=$PROJECT_ROOT_DIR/mobile_broadcaster/build


# make sure that mobile broadcaster is built
west build -b nrf52840dk_nrf52840 $MB_SOURCES_DIR\
    --build-dir $MB_BUILD_DIR


# make sure that log file exists, if not create it 
mkdir -p $LOG_FILES_DIR && touch $LOG_FILES_DIR/$LOG_FILE_NAME


# application logic 
LINES=$(wc -l  $LOG_FILE_PATH | cut -f 1 -d " ")

while [ $LINES -le 10 ] 
do
    # randomize topology
    python3 $SCRIPTS_PATH/topology_randomizer.py $NODES_NUM

    # build
    west build -b nrf52840dk_nrf52840 $SOURCES_DIR \
        --build-dir $BUILD_DIR \
        -- -DMAX_MESH_SIZE=$NODES_NUM \
        -DTOPOLOGY_CONFIG_PATH:STRING=$TOPOLOGY_CONFIG
    
    # disable breaking the script when test fails, bcs some tests fail 
    # due to lack of connection between start and dst nodes (randomization is 
    # not perfect)
    set +e

    # perform tests and save to file
    $RENODE_ROOT_DIR/renode-test \
    $ROBOT_TESTS_PATH/test_packet_travel_time.robot \
        --variable NODES_NUM:$NODES_NUM
    set -e 
    
    # read how many lines were written to output file 
    LINES=$(wc -l $LOG_FILE_PATH | cut -f 1 -d " ")
    
    echo NUMBER OF READ LINES: $LINES
done

