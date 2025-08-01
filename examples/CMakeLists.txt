# ESP32C6 NimBLE Bluetooth Test Example
# CMakeLists.txt for testing ESP32C6 BLE implementation

cmake_minimum_required(VERSION 3.16)

# Include ESP-IDF cmake functions
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set project name
project(esp32c6_nimble_test)

# ESP32C6 specific configuration
if(CONFIG_IDF_TARGET_ESP32C6)
    message(STATUS "Building for ESP32C6 with NimBLE support")
    
    # NimBLE components required for ESP32C6
    set(COMPONENT_REQUIRES 
        nvs_flash
        esp_system
        freertos
        bt
        nimble
    )
    
    # Add NimBLE specific compile definitions
    add_compile_definitions(
        CONFIG_BT_ENABLED=1
        CONFIG_BT_NIMBLE_ENABLED=1
        CONFIG_BT_NIMBLE_ROLE_PERIPHERAL=1
        CONFIG_BT_NIMBLE_ROLE_CENTRAL=1
        CONFIG_BT_NIMBLE_ROLE_BROADCASTER=1
        CONFIG_BT_NIMBLE_ROLE_OBSERVER=1
    )
    
    # Include NimBLE directories
    include_directories(
        $ENV{IDF_PATH}/components/nimble/nimble/porting/nimble/include
        $ENV{IDF_PATH}/components/nimble/nimble/nimble/include
        $ENV{IDF_PATH}/components/nimble/nimble/nimble/host/include
        $ENV{IDF_PATH}/components/nimble/port/include
    )
    
else()
    message(STATUS "Building for other ESP32 targets")
    
    # Standard Bluetooth components for other targets
    set(COMPONENT_REQUIRES 
        nvs_flash
        esp_system
        freertos
        bt
    )
endif()

# Common components for all targets
list(APPEND COMPONENT_REQUIRES
    app_update
    driver
    esp_timer
    log
)

# Create test executable
idf_component_register(
    SRCS "esp32c6_nimble_test.cpp"
    INCLUDE_DIRS 
        "../inc/base"
        "../inc/mcu/esp32"
    REQUIRES ${COMPONENT_REQUIRES}
)

# Link against HardFOC Bluetooth implementation
target_link_libraries(${COMPONENT_LIB} 
    PRIVATE 
    esp32_bluetooth
)

# ESP32C6 specific optimizations
if(CONFIG_IDF_TARGET_ESP32C6)
    # Enable BLE-specific optimizations
    target_compile_options(${COMPONENT_LIB} PRIVATE
        -DCONFIG_BT_NIMBLE_MAX_CONNECTIONS=4
        -DCONFIG_BT_NIMBLE_ATT_PREFERRED_MTU=247
        -DCONFIG_BT_NIMBLE_GATT_MAX_PROCS=4
    )
endif()

# Set C++ standard
set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD_REQUIRED ON)

# Optimization flags
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-missing-field-initializers
)

# Debug build specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${COMPONENT_LIB} PRIVATE
        -DDEBUG=1
        -DLOG_LOCAL_LEVEL=ESP_LOG_DEBUG
    )
else()
    target_compile_options(${COMPONENT_LIB} PRIVATE
        -DNDEBUG=1
        -DLOG_LOCAL_LEVEL=ESP_LOG_INFO
    )
endif()

# Print configuration summary
message(STATUS "=== ESP32C6 NimBLE Test Configuration ===")
message(STATUS "Target: ${CONFIG_IDF_TARGET}")
message(STATUS "ESP-IDF Version: ${IDF_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Required Components: ${COMPONENT_REQUIRES}")

if(CONFIG_IDF_TARGET_ESP32C6)
    message(STATUS "NimBLE Features:")
    message(STATUS "  - BLE Central/Peripheral roles")
    message(STATUS "  - BLE Broadcaster/Observer roles") 
    message(STATUS "  - GATT Client/Server")
    message(STATUS "  - Maximum MTU: 247 bytes")
    message(STATUS "  - Maximum Connections: 4")
endif()

message(STATUS "==========================================")