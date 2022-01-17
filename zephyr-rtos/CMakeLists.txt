cmake_minimum_required(VERSION 3.20.0)

set(BOARD nrf52840dk_nrf52840)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_zephyr_app)

target_include_directories(app PUBLIC ${include})

target_sources(app PRIVATE 
    src/main.c
    src/graph.c
    src/dijkstra.c)

