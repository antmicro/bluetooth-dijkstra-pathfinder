cmake_minimum_required(VERSION 3.20.0)

set(BOARD nrf52840dk_nrf52840)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_zephyr_app)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/graph.c 
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/../generator.py
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../config.json ${CMAKE_CURRENT_SOURCE_DIR}/../node_init_template.c.j2
  VERBATIM)

target_include_directories(app PUBLIC ${include})

target_sources(app PRIVATE 
    src/main.c
    src/graph.c
    src/dijkstra.c)

