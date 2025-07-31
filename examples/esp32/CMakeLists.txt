cmake_minimum_required(VERSION 3.16)

# Flexible build system for different example types
# Usage: idf.py build -DEXAMPLE_TYPE=<type> -DBUILD_TYPE=<type>
# 
# Example types:
#   comprehensive  - Main integration test (default)
#   ascii_art      - ASCII art generator example
#
# Build types: Debug, Release (default: Release)

# Set example type from command line or default to comprehensive
set(EXAMPLE_TYPE "comprehensive" CACHE STRING "Example type to build")
set(BUILD_TYPE "Release" CACHE STRING "Build type (Debug/Release)")

# Validate example type
set(VALID_EXAMPLE_TYPES "comprehensive;ascii_art")
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
