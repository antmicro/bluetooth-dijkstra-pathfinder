#!/bin/bash
# PZIE: use env instead

# fail the entire script if any of commands fails
set -e 
# PZIE: set -u is also a good idea

VERSION=$1

# project root directory
PROJECT_ROOT_DIR=$PWD

# mesh app dirs
SRC_DIR=zephyr-rtos
BUILD_DIR=zephyr-rtos/build

# mobile broadcaster dirs
MB_SRC_DIR=mobile_broadcaster
MB_BUILD_DIR=mobile_broadcaster/build

# config files dir
CONFIG_FILES_DIR=$PROJECT_ROOT_DIR/config-files/mesh-topology-desc

build_mobile_broadcaster () {
    west build -b nrf52840dk_nrf52840 \
        $MB_SRC_DIR\
        -d $MB_BUILD_DIR
}


build_randomized () {
    if [ "$2" = "--randomize" ]; then
        # randomize
        python3 scripts/topology_randomizer.py $1 \
            --mbmove \
            --verbose \
            --visualize \
            --visualizemb \
            --faulty_nodes 1
    fi

    # build 
    west build -b nrf52840dk_nrf52840 \
        $SRC_DIR \
        -d $BUILD_DIR \
        -- -DMAX_MESH_SIZE=$1 \
        -DMAX_TTL=3 \
        -DTOPOLOGY_CONFIG_PATH:STRING=$CONFIG_FILES_DIR/randomized_topology.json
}


build_basic5node () {
    west build -b nrf52840dk_nrf52840 \
        $SRC_DIR \
        -d $BUILD_DIR \
        -- -DMAX_MESH_SIZE=5 \
        -DMAX_TTL=3 \
        -DTOPOLOGY_CONFIG_PATH:STRING=$CONFIG_FILES_DIR/basic_5_nodes.json 
}


# case statement for picking version to run
case $VERSION in
  --random)
    echo "Building randomized version..."

    # if randomized mesh specified, load also number of nodes to generate   # PZIE: do not refrain from using verbs in comments. It's not really readable
    if [ $# -lt 2 ] 
    then
        echo "Specify number of nodes!"
        exit 1
    fi

    # input validation is made in topology_randomizer script

    build_randomized $2 $3
    build_mobile_broadcaster
    ;;

  --basic)
    echo "Building basic 5 nodes version..." 

    build_basic5node
    build_mobile_broadcaster
    ;;

  *)
    echo "Specify correct application version to build:
    --random       - build random topology, to reshuffle / randomize nodes provide also option --randomize   # PZIE: what does the end mean?
    --basic        - generate basic 5 nodes minimal topology and build 
    " 
    ;;
esac


