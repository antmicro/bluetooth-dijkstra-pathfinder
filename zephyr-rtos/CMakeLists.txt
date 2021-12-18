cmake_minimum_required(VERSION 3.20.0)

set(BOARD nrf52840dongle_nrf52840)

find_package(Zephyr)
project(my_zephyr_app)

target_sources(app PRIVATE src/main.c)

