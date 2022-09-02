#!/usr/bin/env bash

# fail the entire script if any of commands fails or if unset variable will be expanded
set -eu

usage () {
    echo "Usage: ./build_as.sh [BUILD CONFIGURATION] [OPTIONS]
Build the application in one of supported configurations. Build process involves:
* Generating a graph data structure in node/src/generated-src/graph_api_generated.c
* API for handling this data structure
* Compiling resultant code into an application
Generation of graph data structure and in general mesh topology is based on the configuration files in config-files/mesh-topology-desc/*.json.
    Build configurations:
    --basic        - build basic 5 nodes configuration - config-files/mesh-topology-desc/basic_5_nodes.json
    --random       - build random topology - config-files/mesh-topology-desc/randomized_topology.json
    --input        - build from .json file with given name and located at config-files/mesh-topology-desc

    Options:
    --randomize    - reshuffles network for given number of nodes, only available with --random

    Example usage:
    $ ./build_as.sh --basic                   # Build basic 5 nodes static configuration
    $ ./build_as.sh --random                  # Build random configuration
    $ ./build_as.sh --random --randomize 10   # Generate new configuration of the network for 10 nodes and build
    $ ./build_as.sh --input example.json      # Build from user provided config file"
}

# At least configuration argument must be provided
if [ $# -lt 1 ]; then
    usage
    exit 0
fi

CONFIGURATION=$1

# case statement for picking configuration to run
case $CONFIGURATION in
  --random)
    TOPOLOGY_PATH="config-files/mesh-topology-desc/randomized_topology.json"
    if [ $# -eq 3 ]; then
        if [ "$2" == "--randomize" ]; then
            RANDOMIZE=1
            NODES_NUMBER=$3
        else
            usage
            echo $"ERROR: Unrecognized argument: $2. Did You mean --randomize?"
            exit 1
        fi
    else
        RANDOMIZE=0
    fi
    if [ $# -eq 2 ]; then
        usage
        echo "ERROR: Invalid syntax for --random build."
        exit 1
    fi
    ;;

  --basic)
    TOPOLOGY_PATH="config-files/mesh-topology-desc/basic_5_nodes.json"
    RANDOMIZE=0
    ;;

  --input)
    if [ $# -eq 2 ]; then
        TOPOLOGY_PATH=config-files/mesh-topology-desc/${2}
        if [ ! -f "$TOPOLOGY_PATH" ]; then
            usage
            echo $"ERROR: provided topology file: ${TOPOLOGY_PATH} does not exist."
            exit 1
        fi
        RANDOMIZE=0
    else
        usage
        echo $"ERROR: Provide name of the topology file."
        exit 1
    fi
    ;;

  *)
    usage
    echo "ERROR: Provide correct arguments."
    exit 1
    ;;
esac

# Randomize / reshuffle number of nodes specified by user and generate 
# corresponding .json topology and .resc file
if [ $RANDOMIZE -eq 1 ]; then
    python3 scripts/topology_randomizer.py $NODES_NUMBER \
    --mbmove \
    --verbose \
    --visualize \
    --visualizemb \
    --faulty_nodes 1
fi

# Generate graph and API 
python3 scripts/generator.py $TOPOLOGY_PATH

# Build network
west build -b nrf52840dk_nrf52840 \
        node/ \
        -d node/build \
        -- -DMAX_TTL=3 \

# Build mobile_broadcaster
west build -b nrf52840dk_nrf52840 \
        mobile_broadcaster \
        -d mobile_broadcaster/build



