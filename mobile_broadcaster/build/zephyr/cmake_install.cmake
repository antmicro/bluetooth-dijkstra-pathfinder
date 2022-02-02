# Install script for directory: /home/js/Projects/zephyr-project/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/home/js/zephyr-sdk-0.13.2/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/zephyr/soc/arm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/CANopenNode/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/civetweb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/altera/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/atmel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/cypress/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/espressif/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/hal_gigadevice/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/infineon/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/microchip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/nuvoton/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/hal_nxp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/openisa/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/quicklogic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/silabs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/stm32/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/telink/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/ti/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/xtensa/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/lz4/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/nanopb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/sof/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/tflite-micro/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/tinycbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/tinycrypt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/TraceRecorder/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/modules/zscilib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/js/Projects/antmicro/bluetooth-dijkstra-pathfinder/mobile_broadcaster/build/zephyr/cmake/reports/cmake_install.cmake")
endif()

