cmake_minimum_required(VERSION 3.16)

# Main test CMakeLists.txt for HardFOC IID unit tests
# This file sets up the Unity testing framework for ESP32 unit testing
# It creates a test project that can run both on hardware and in CI

project(hardfoc_iid_unit_tests
    VERSION 1.0.0
    LANGUAGES C CXX)

# Set C++ standard to match main project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Test configuration
set(TESTING_ENABLED TRUE)

# Add compiler definitions for testing
add_compile_definitions(
    UNIT_TESTING=1
    ESP32_TESTING=1
)

# Component dependencies for testing
set(EXTRA_COMPONENT_DIRS
    "../src"
    "../inc"
    "components/unity"
    "components/esp_idf_mocks"
)

# Test source files will be registered in main/CMakeLists.txt