# =============================================================================
# HF Internal Interface Wrap - Root Component CMakeLists.txt
# =============================================================================
# This CMakeLists.txt configures the HardFOC Internal Interface Wrapper as an
# ESP-IDF component when consumed directly (outside examples/esp32/).
# It provides comprehensive hardware abstraction for HardFOC boards.
# =============================================================================

cmake_minimum_required(VERSION 3.16)

# =============================================================================
# Source File Discovery
# =============================================================================
# Discover all ESP32 implementation source files
file(GLOB_RECURSE ESP32_SOURCES "src/mcu/esp32/*.cpp")
file(GLOB_RECURSE UTIL_SOURCES "src/utils/*.cpp")

set(DRIVER_SRCS
    ${ESP32_SOURCES}
    ${UTIL_SOURCES}
)

# Check which source files actually exist and add them
set(EXISTING_SRCS "")
foreach(src_file ${DRIVER_SRCS})
    if(EXISTS "${src_file}")
        list(APPEND EXISTING_SRCS "${src_file}")
        message(STATUS "Found source: ${src_file}")
    else()
        message(WARNING "Source file not found: ${src_file}")
    endif()
endforeach()

# =============================================================================
# Include Directories Configuration
# =============================================================================
set(DRIVER_INCLUDE_DIRS
    "inc"
    "inc/base"
    "inc/mcu/esp32"
    "inc/mcu/esp32/utils"
    "inc/utils"
)

# =============================================================================
# Component Dependencies
# =============================================================================
set(DRIVER_REQUIRES
    freertos
    esp_common
    esp_hw_support
    esp_system
    log
    soc
    hal
    esp_wifi
    bt
    nvs_flash
    esp_driver_i2c
)

set(DRIVER_PRIV_REQUIRES
    esp_netif
    esp_event
    esp_timer
    lwip
)

# =============================================================================
# Component Registration
# =============================================================================
idf_component_register(
    SRCS ${EXISTING_SRCS}
    INCLUDE_DIRS ${DRIVER_INCLUDE_DIRS}
    REQUIRES ${DRIVER_REQUIRES}
    PRIV_REQUIRES ${DRIVER_PRIV_REQUIRES}
)

# =============================================================================
# Compiler Configuration
# =============================================================================
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)

target_compile_options(${COMPONENT_LIB} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-pedantic>
)
