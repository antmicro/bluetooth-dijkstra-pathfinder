#!/bin/bash

VERSION=$1

SRC_DIR=./zephyr-rtos
BUILD_DIR=./zephyr-rtos/build

build_randomized () {
    echo "Not implemented"
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
    # build also mobile broadcaster
    build_randomized
    ;;

  --basic)
    echo "Building basic 5 nodes version..." 
    # build also mobile broadcaster
    build_basic5node
    ;;

  *)
    echo "Specify correct application version to build:
    --randomized   - random topology
    --basic        - basic 5 nodes minimal topology  
    " 
    ;;
esac

