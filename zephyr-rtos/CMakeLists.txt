cmake_minimum_required(VERSION 3.20.0)

# add variable to store build option 
SET(MAX_MESH_SIZE "9" CACHE STRING "Define max mesh size")

# add variable to store max allowed ttl
SET(MAX_TTL "9" CACHE STRING "Define max time to live for a packet.")

# add variable to store topology config file path passed during west build
SET(TOPOLOGY_CONFIG_PATH "" CACHE STRING 
    "Pass path to topology .json file to c file generator")

# add a custom build target 
add_custom_target(generator ALL)

# attach command to it
add_custom_command(
  TARGET generator
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generator.py ${TOPOLOGY_CONFIG_PATH} 
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../${TOPOLOGY_CONFIG_PATH}
  BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/src/generated-src/graph_init.c
  VERBATIM)

set(BOARD nrf52840dk_nrf52840)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_zephyr_app)

target_include_directories(app PUBLIC ${include})

# build with max mesh size specified as preprocessor #define
target_compile_definitions(app PUBLIC MAX_MESH_SIZE=${MAX_MESH_SIZE})
target_compile_definitions(app PUBLIC MAX_TTL=${MAX_TTL})

# add dependency between app target and generator target 
add_dependencies(app generator)

target_sources(app PRIVATE 
    src/main.c
    src/graph.c 
    src/generated-src/graph_init.c
    src/dijkstra.c
    src/bluetooth_ble.c)

