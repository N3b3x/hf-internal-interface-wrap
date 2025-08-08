# Flexible main component for different example types
# Determine source file based on EXAMPLE_TYPE

# Get example type from parent
if(NOT DEFINED EXAMPLE_TYPE)
    set(EXAMPLE_TYPE "ascii_art")
endif()

# Debug information
message(STATUS "=== CMake Debug Information ===")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "EXAMPLE_TYPE: ${EXAMPLE_TYPE}")
message(STATUS "===============================")

# Map example types to source files
if(EXAMPLE_TYPE STREQUAL "ascii_art")
    set(MAIN_SOURCE "AsciiArtComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "gpio_test")
    set(MAIN_SOURCE "GpioComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "adc_test")
    set(MAIN_SOURCE "AdcComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "uart_test")
    set(MAIN_SOURCE "UartComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "spi_test")
    set(MAIN_SOURCE "SpiComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "i2c_test")
    set(MAIN_SOURCE "I2cComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "pwm_test")
    set(MAIN_SOURCE "PwmComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "can_test")
    set(MAIN_SOURCE "CanComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "temperature_test")
    set(MAIN_SOURCE "TemperatureComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "nvs_test")
    set(MAIN_SOURCE "NvsComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "timer_test")
    set(MAIN_SOURCE "TimerComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "logger_test")
    set(MAIN_SOURCE "LoggerComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "wifi_test")
    set(MAIN_SOURCE "WifiComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "bluetooth_test")
    set(MAIN_SOURCE "BluetoothComprehensiveTest.cpp")
elseif(EXAMPLE_TYPE STREQUAL "utils_test")
    set(MAIN_SOURCE "UtilsComprehensiveTest.cpp")
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
