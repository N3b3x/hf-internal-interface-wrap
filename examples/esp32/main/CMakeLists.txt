# Flexible main component for different example types
# Determine source file based on EXAMPLE_TYPE

# Get example type from parent
if(NOT DEFINED EXAMPLE_TYPE)
    set(EXAMPLE_TYPE "comprehensive")
endif()

# Map example types to source files
if(EXAMPLE_TYPE STREQUAL "comprehensive")
    set(MAIN_SOURCE "main.cpp")
elseif(EXAMPLE_TYPE STREQUAL "ascii_art")
    set(MAIN_SOURCE "AsciiArtExample.cpp")
else()
    message(FATAL_ERROR "Unknown EXAMPLE_TYPE: ${EXAMPLE_TYPE}")
endif()

# Check if source file exists
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_SOURCE}")
    message(FATAL_ERROR "Source file not found: ${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_SOURCE}")
endif()

message(STATUS "Using source file: ${MAIN_SOURCE}")

idf_component_register(
    SRCS "${MAIN_SOURCE}"
    INCLUDE_DIRS "." "../common"
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
