cmake_minimum_required(VERSION 3.16)

# Flexible build system for different example types
# Usage: idf.py build -DEXAMPLE_TYPE=<type> -DBUILD_TYPE=<type>
# 
# Example types:
#   ascii_art        - ASCII art generator example (default)
#   gpio_test        - Comprehensive GPIO testing suite
#   adc_test         - Comprehensive ADC testing suite
#   uart_test        - Comprehensive UART testing suite
#   spi_test         - Comprehensive SPI testing suite
#   i2c_test         - Comprehensive I2C testing suite
#   pwm_test         - Comprehensive PWM testing suite
#   can_test         - Comprehensive CAN testing suite
#   temperature_test - Comprehensive Temperature testing suite
#   nvs_test         - Comprehensive NVS testing suite
#   timer_test       - Comprehensive Timer testing suite
#   wifi_test        - Comprehensive WiFi testing suite
#   bluetooth_test   - Comprehensive Bluetooth testing suite
#   utils_test       - Utilities testing suite
#
# Build types: Debug, Release (default: Release)

# Set example type from command line or default to ascii_art
set(EXAMPLE_TYPE "ascii_art" CACHE STRING "Example type to build")
set(BUILD_TYPE "Release" CACHE STRING "Build type (Debug/Release)")

# Validate example type
set(VALID_EXAMPLE_TYPES "ascii_art;gpio_test;adc_test;uart_test;spi_test;i2c_test;pwm_test;can_test;temperature_test;nvs_test;timer_test;wifi_test;bluetooth_test;utils_test")
if(NOT EXAMPLE_TYPE IN_LIST VALID_EXAMPLE_TYPES)
    message(FATAL_ERROR "Invalid EXAMPLE_TYPE: ${EXAMPLE_TYPE}. Valid types: ${VALID_EXAMPLE_TYPES}")
endif()

# Validate build type
set(VALID_BUILD_TYPES "Debug;Release")
if(NOT BUILD_TYPE IN_LIST VALID_BUILD_TYPES)
    message(FATAL_ERROR "Invalid BUILD_TYPE: ${BUILD_TYPE}. Valid types: ${VALID_BUILD_TYPES}")
endif()

message(STATUS "Building example type: ${EXAMPLE_TYPE}")
message(STATUS "Build type: ${BUILD_TYPE}")

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set project name based on example type
project(esp32_iid_${EXAMPLE_TYPE}_example)
