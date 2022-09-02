#!/usr/bin/env bash

# fail the entire script if any of commands fails or if unset variable will be expanded
set -e
set -u

usage () {
    echo "Usage: ./build_as.sh [BUILD CONFIGURATION] [OPTIONS]
Build the application in one of supported configurations. Build process involves:
* Generating a graph data structure in node/src/generated-src/graph_api_generated.c
* API for handling this data structure
* Compiling resultant code into an application
Generation of graph data structure and in general mesh topology is based on the configuration files in config-files/mesh-topology-desc/*.json.
    Build configurations:
    -b        - build basic 5 nodes configuration - config-files/mesh-topology-desc/basic_5_nodes.json
    -r        - build random topology - config-files/mesh-topology-desc/randomized_topology.json
    -i        - build from provided .json file

    Options:
    -s        - shuffles network for given number of nodes, only available with -r
    -a        - enable or disable assertions in Zephyr build, available values: 0, 1, 2 with default 1

    Example usage:
    $ ./build_as.sh -b                   # Build basic 5 nodes static configuration
    $ ./build_as.sh -r                   # Build random configuration
    $ ./build_as.sh -r -s 10             # Generate new configuration of the network for 10 nodes and build
    $ ./build_as.sh -i example.json      # Build from user provided config file
    $ ./build_as.sh -b -a 1              # Build with asserts
    "
}

# At least configuration argument must be provided
if [ $# -lt 1 ]; then
    usage
    exit 0
fi

RANDOMIZE=0
ASSERT_LEVEL=1
NODES_NUMBER=0
CONFIGURATION= # May have values b, r, i
while getopts "bri:s:a:" opt
do
    case $opt in
        # Build configurations
        b)
            # If variable is already set
            if [ -v TOPOLOGY_PATH ]; then
                usage
                echo $"ERROR: Provide only one build configuration out of -b -r -i."
                exit 1
            fi
            TOPOLOGY_PATH="config-files/mesh-topology-desc/basic_5_nodes.json"
            CONFIGURATION=b
            ;;
        r)
            if [ -v TOPOLOGY_PATH ]; then
                usage
                echo $"ERROR: Provide only one build configuration out of -b -r -i."
                exit 1
            fi
            TOPOLOGY_PATH="config-files/mesh-topology-desc/randomized_topology.json"
            CONFIGURATION=r
            ;;
        i)
            if [ -v TOPOLOGY_PATH ]; then
                usage
                echo $"ERROR: Provide only one build configuration out of -b -r -i."
                exit 1
            fi
            TOPOLOGY_PATH=$OPTARG
            CONFIGURATION=i
            ;;
        # Options
        s)
            RANDOMIZE=1
            NODES_NUMBER=$OPTARG
            ;;
        a)
            ASSERT_LEVEL=$OPTARG
            ;;
        *)
            usage
            echo $"ERROR: Unrecognized option."
            exit 1
            ;;
    esac
done

if [ ! -v TOPOLOGY_PATH ]; then
    usage
    echo $"ERROR: invalid configuration options."
    exit 1
fi

if [ $RANDOMIZE -eq 1 ]; then
    if [ "$CONFIGURATION" == "r" ]; then
        python3 scripts/topology_randomizer.py $NODES_NUMBER \
            --mbmove \
            --verbose \
            --visualize \
            --visualizemb \
            --faulty_nodes 1
    else
        usage
        echo $"ERROR: Randomizing of nodes can only be performed with random build (option -r)"
        exit 1
    fi
fi

# Generate graph and API
python3 scripts/generator.py $TOPOLOGY_PATH

# Build network
west build -b nrf52840dk_nrf52840 \
        node/ \
        -d node/build \
        -- -DMAX_TTL=3 \
        -D__ASSERT_ON=$ASSERT_LEVEL

# Build mobile_broadcaster
west build -b nrf52840dk_nrf52840 \
        mobile_broadcaster \
        -d mobile_broadcaster/build \
        -D__ASSERT_ON=$ASSERT_LEVEL




