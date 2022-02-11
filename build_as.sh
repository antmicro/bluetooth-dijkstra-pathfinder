#!/bin/bash

VERSION=$1

# mesh app dirs
SRC_DIR=zephyr-rtos
BUILD_DIR=zephyr-rtos/build

# mobile broadcaster dirs
MB_SRC_DIR=mobile_broadcaster
MB_BUILD_DIR=mobile_broadcaster/build

build_mobile_broadcaster () {
    west build -b nrf52840dk_nrf52840 \
        $MB_SRC_DIR\
        -d $MB_BUILD_DIR
}


build_randomized () {
    west build -b nrf52840dk_nrf52840 \
        $SRC_DIR \
        -d $BUILD_DIR \
        -- -DMAX_MESH_SIZE=5 \
        -DTOPOLOGY_CONFIG_PATH:STRING=config-files/mesh-topology-desc/basic_5_nodes.json

}


build_basic5node () {
    west build -b nrf52840dk_nrf52840 \
        $SRC_DIR \
        -d $BUILD_DIR \
        -- -DMAX_MESH_SIZE=5 \
        -DTOPOLOGY_CONFIG_PATH:STRING=config-files/mesh-topology-desc/basic_5_nodes.json
}


# case statement for picking version to run
case $VERSION in

  --randomized)
    echo "Building randomized version..."

    # if randomized mesh specified, load also number of nodes to generate
    NODES_NUM=$2
    build_randomized
    build_mobile_broadcaster
    ;;

  --basic)
    echo "Building basic 5 nodes version..." 

    build_basic5node
    build_mobile_broadcaster
    ;;

  *)
    echo "Specify correct application version to build:
    --randomized   - random topology
    --basic        - basic 5 nodes minimal topology  
    " 
    ;;
esac

