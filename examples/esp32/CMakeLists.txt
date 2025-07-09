cmake_minimum_required(VERSION 3.16)

# Set project name
set(PROJECT_NAME "advanced_pwm_example")

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set project name
project(${PROJECT_NAME})

# Add source files
set(SOURCES
    advanced_pwm_example.cpp
)

# Create executable
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES driver esp_timer freertos
)

# Set C++ standard
target_compile_features(${PROJECT_NAME}.elf PRIVATE cxx_std_17)

# Add compiler flags for better debugging
target_compile_options(${PROJECT_NAME}.elf PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O2
    -g
)

# Enable verbose output for debugging
set(CMAKE_VERBOSE_MAKEFILE ON)
