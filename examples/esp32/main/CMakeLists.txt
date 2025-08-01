# Flexible main component for different example types
# Determine source file based on EXAMPLE_TYPE

# Get example type from parent
if(NOT DEFINED EXAMPLE_TYPE)
    set(EXAMPLE_TYPE "comprehensive")
endif()

# Debug information
message(STATUS "=== CMake Debug Information ===")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "EXAMPLE_TYPE: ${EXAMPLE_TYPE}")
message(STATUS "===============================")

# Map example types to source files
if(EXAMPLE_TYPE STREQUAL "comprehensive")
    set(MAIN_SOURCE "main.cpp")
elseif(EXAMPLE_TYPE STREQUAL "ascii_art")
    set(MAIN_SOURCE "AsciiArtExample.cpp")
else()
    message(FATAL_ERROR "Unknown EXAMPLE_TYPE: ${EXAMPLE_TYPE}")
endif()

# Check if source file exists
set(SOURCE_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_SOURCE}")
message(STATUS "Looking for source file: ${SOURCE_FILE_PATH}")

# List files in current directory for debugging
file(GLOB DEBUG_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*")
message(STATUS "Files in source directory:")
foreach(DEBUG_FILE ${DEBUG_FILES})
    message(STATUS "  ${DEBUG_FILE}")
endforeach()

if(NOT EXISTS "${SOURCE_FILE_PATH}")
    # Try alternative paths for CI environment
    set(SOURCE_FILE_PATH "${CMAKE_CURRENT_LIST_DIR}/${MAIN_SOURCE}")
    message(STATUS "Trying alternative path: ${SOURCE_FILE_PATH}")
    if(NOT EXISTS "${SOURCE_FILE_PATH}")
        message(FATAL_ERROR "Source file not found: ${MAIN_SOURCE}\n"
                            "Tried paths:\n"
                            "  ${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_SOURCE}\n"
                            "  ${CMAKE_CURRENT_LIST_DIR}/${MAIN_SOURCE}\n"
                            "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}\n"
                            "CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")
    endif()
endif()

message(STATUS "Using source file: ${SOURCE_FILE_PATH}")

# Determine include paths based on environment
# In CI, inc is copied to ../inc, in development it's at ../../../inc
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../inc")
    # CI environment - inc is at project level
    set(INC_BASE_PATH "../inc")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../inc")
    # Development environment - inc is at workspace root
    set(INC_BASE_PATH "../../../inc")
else()
    message(FATAL_ERROR "Cannot find inc directory. Checked ../inc and ../../../inc")
endif()

message(STATUS "Using include base path: ${INC_BASE_PATH}")

idf_component_register(
    SRCS "${MAIN_SOURCE}"
    INCLUDE_DIRS "." "${INC_BASE_PATH}" "${INC_BASE_PATH}/base" "${INC_BASE_PATH}/mcu/esp32"
    REQUIRES driver esp_timer freertos iid-espidf
)

# Set C++ standard
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)

# Set compiler flags based on build type
if(BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -O0
        -g3
        -DDEBUG
    )
else()
    target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -O2
        -g
        -DNDEBUG
    )
endif()

# Add compile definitions for each example type
target_compile_definitions(${COMPONENT_LIB} PRIVATE 
    "EXAMPLE_TYPE_${EXAMPLE_TYPE}=1"
)
