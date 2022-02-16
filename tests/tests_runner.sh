#!/bin/bash
# setup zephyr env
#source ~/Projects/zephyr-project/zephyr/zephyr-env.sh
#source ~/Projects/zephyr-project/.venv/bin/activate
#pip3 install robotframework==4.0.1

# TODO: add a timestamp for each logging session?
# TODO: add mobile broadcaster build 
# TODO: add $USAGE

USAGE="not implemented"

PROJECT_ROOT_DIR=../

MIN_NODES_NUM=3
MAX_NODES_NUM=64


# input arguments 
if [ $# -lt 2 ] 
then
    echo "Necessary arguments not provided!"
    echo $USAGE
    exit 1
fi

NODES_NUM=$1
RENODE_ROOT_DIR=$2 #~/Repos/renode

# validate number of nodes
# any numeric value evaluation operations using non-numbers will result in 
# an error which will be implicitly considered as false in shell
# Redirection of standard error is there to hide the "integer expression 
# expected" message that bash prints out in case we do not have a number.

# check if in range and int
if ! [[ "$NODES_NUM" -eq "$NODES_NUM" ]] 2>/dev/null; then
    echo noninteger
    exit 1
        exit 1
else # TODO: NOT WORKING !!!
    if [[ $NODES_NUM -ge $MIN_NODES_NUM ]] && [[ $NODES_NUM -le $MAX_NODES_NUM ]]
    then
        echo "Number of nodes must be integer between 3-64"
        exit 1
    fi
fi

# check if test.sh exists under provided directory 
if ! [ -f "$RENODE_ROOT_DIR/test.sh" ] 
then
    echo "Provided directory does not contain Renode's test.sh script!"
    echo "$USAGE"
    exit 1
fi


# debug 
echo Exiting...
exit 1


# logging 
LOG_FILES_DIR=$PROJECT_ROOT_DIR/tests/out
LOG_FILE_NAME=${NODES_NUM}nodes.log

TOPOLOGY_RANDOMIZER_PATH=$PROJECT_ROOT_DIR/scripts

SOURCES_PATH=$PROJECT_ROOT_DIR/zephyr-rtos

BUILD_DIR=$PROJECT_ROOT_DIR/zephyr-rtos/build


ROBOT_TESTS_PATH=$PROJECT_ROOT_DIR/tests

# make initial build
west build -b nrf52840dk_nrf52840 $PROJECT_ROOT_DIR/zephyr-rtos \
    --build-dir $PROJECT_ROOT_DIR/zephyr-rtos/build

# make sure that log file exists, if not create it 
mkdir $LOG_FILES_DIR && touch $LOG_FILES_DIR/$LOG_FILE_NAME

LINES=$(wc -l  $LOG_FILE_PATH | cut -f 1 -d " ")

while [ $LINES -le 50 ] 
do
    # randomize
    python3 $TOPOLOGY_RANDOMIZER_PATH/topology_randomizer.py $NODES_NUM

    # build
    west build -b nrf52840dk_nrf52840 $SOURCES_PATH --build-dir $BUILD_DIR 
    
    # perform tests and save to file
    $RENODE_ROOT_DIR/test.sh $ROBOT_TESTS_PATH/test_packet_travel_time.robot

    # read how many lines was written to output file 
    LINES=$(wc -l $LOG_FILE_PATH | cut -f 1 -d " ")
    
    echo NUMBER OF READ LINES: $LINES
done

