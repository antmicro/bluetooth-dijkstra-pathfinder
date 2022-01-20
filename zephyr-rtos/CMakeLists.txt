cmake_minimum_required(VERSION 3.20.0)

# add a custom build target 
add_custom_target(generator ALL)

# attach command to it
add_custom_command(
  TARGET generator
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generator.py
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../topology_config.json
  VERBATIM)

set(BOARD nrf52840dk_nrf52840)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_zephyr_app)

target_include_directories(app PUBLIC ${include})

# add dependency between app target and generator target 
add_dependencies(app generator)

target_sources(app PRIVATE 
    src/main.c
    src/graph.c 
    src/generated-src/graph_init.c
    src/dijkstra.c)

