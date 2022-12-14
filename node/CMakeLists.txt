cmake_minimum_required(VERSION 3.20.0)

# add variable to store max allowed ttl
SET(MAX_TTL "9" CACHE STRING "Define max time to live for a packet.")

# add variable to store topology config file path passed during west build
SET(TOPOLOGY_CONFIG_PATH "" CACHE STRING 
    "Pass path to topology .json file to c file generator")

set(BOARD nrf52840dk_nrf52840)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_zephyr_app)

target_include_directories(app PUBLIC ${include})

# build with max mesh size and max ttl specified as preprocessor #define
target_compile_definitions(app PUBLIC MAX_TTL=${MAX_TTL})

target_sources(app PRIVATE 
    src/main.c
    src/graph.c 
    src/generated-src/graph_api_generated.c
    src/dijkstra.c
    src/bluetooth_ble.c)

