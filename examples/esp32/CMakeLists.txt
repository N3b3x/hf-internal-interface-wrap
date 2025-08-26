cmake_minimum_required(VERSION 3.16)

# CRITICAL: Set variables BEFORE any other processing
# This ensures they are available during component configuration

# First, try to include the auto-generated app config file
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app_config.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/app_config.cmake")
    message(STATUS "Loaded app config from app_config.cmake")
endif()

# Then check command line arguments
if(DEFINED APP_TYPE)
    message(STATUS "APP_TYPE from command line: ${APP_TYPE}")
else()
    set(APP_TYPE "ascii_art")
    message(STATUS "APP_TYPE defaulting to: ${APP_TYPE}")
endif()

if(DEFINED BUILD_TYPE)
    message(STATUS "BUILD_TYPE from command line: ${BUILD_TYPE}")
else()
    set(BUILD_TYPE "Release")
    message(STATUS "BUILD_TYPE defaulting to: ${BUILD_TYPE}")
endif()

# Flexible build system for different app types
# Usage: idf.py build -DAPP_TYPE=<type> -DBUILD_TYPE=<type>
# 
# App types:
#   ascii_art        - ASCII art generator app (default)
#   gpio_test        - Comprehensive GPIO testing suite
#   adc_test         - Comprehensive ADC testing suite
#   uart_test        - Comprehensive UART testing suite
#   spi_test         - Comprehensive SPI testing suite
#   i2c_test         - Comprehensive I2C testing suite
#   pwm_test         - Comprehensive PWM testing suite
#   can_test         - Comprehensive CAN testing suite
#   pio_test         - Comprehensive PIO/RMT testing suite with WS2812 and logic analyzer
#   temperature_test - Comprehensive Temperature testing suite
#   nvs_test         - Comprehensive NVS testing suite
#   timer_test       - Comprehensive Timer testing suite
#   wifi_test        - Comprehensive WiFi testing suite
#   bluetooth_test   - Comprehensive Bluetooth testing suite
#   utils_test       - Utilities testing suite
#
# Build types: Debug, Release (default: Release)

# Validate app type
set(VALID_APP_TYPES "ascii_art;gpio_test;adc_test;uart_test;spi_test;i2c_test;pwm_test;can_test;pio_test;temperature_test;nvs_test;timer_test;wifi_test;bluetooth_test;utils_test;logger_test")
if(NOT APP_TYPE IN_LIST VALID_APP_TYPES)
    message(FATAL_ERROR "Invalid APP_TYPE: ${APP_TYPE}. Valid types: ${VALID_APP_TYPES}")
endif()

# Validate build type
set(VALID_BUILD_TYPES "Debug;Release")
if(NOT BUILD_TYPE IN_LIST VALID_BUILD_TYPES)
    message(FATAL_ERROR "Invalid BUILD_TYPE: ${BUILD_TYPE}. Valid types: ${VALID_BUILD_TYPES}")
endif()

message(STATUS "Building app type: ${APP_TYPE}")
message(STATUS "Build type: ${BUILD_TYPE}")
message(STATUS "CMake source dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMake current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")

# CRITICAL: Set these as global variables for all components
set(APP_TYPE "${APP_TYPE}" CACHE STRING "App type to build" FORCE)
set(BUILD_TYPE "${BUILD_TYPE}" CACHE STRING "Build type (Debug/Release)" FORCE)

# Also set as global properties to ensure components can access them
set_property(GLOBAL PROPERTY APP_TYPE "${APP_TYPE}")
set_property(GLOBAL PROPERTY BUILD_TYPE "${BUILD_TYPE}")

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set project name based on app type
project(esp32_iid_${APP_TYPE}_app)
