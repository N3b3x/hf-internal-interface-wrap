cmake_minimum_required(VERSION 3.16)

# HardFOC Internal Interface Wrapper Component
# This component provides base classes and ESP32 implementations for WiFi

set(COMPONENT_SRCDIR "src/mcu/esp32")
set(COMPONENT_INCLUDE_DIRS "inc" "inc/base" "inc/mcu/esp32" "inc/utils" "inc/mcu/esp32/utils")

# Source files
set(COMPONENT_SRCS
    # Base classes would be here if they had implementations
    # For now, they are header-only abstract classes
    
    # ESP32 implementations (if they exist)
    # "src/mcu/esp32/EspWifi.cpp"
    
    # "src/mcu/esp32/EspGpio.cpp"
    # ... other ESP implementations
)

# Add all available source files from the ESP32 directory
file(GLOB_RECURSE ESP32_SOURCES "src/mcu/esp32/*.cpp")
list(APPEND COMPONENT_SRCS ${ESP32_SOURCES})

# Component dependencies
set(COMPONENT_REQUIRES
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
)

# Optional dependencies
set(COMPONENT_PRIV_REQUIRES
    esp_netif
    esp_event
    esp_timer
    lwip
)

# Register the component
idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS ${COMPONENT_INCLUDE_DIRS}
    REQUIRES ${COMPONENT_REQUIRES}
    PRIV_REQUIRES ${COMPONENT_PRIV_REQUIRES}
)

# Set C++ standard
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)

# Add compiler flags for better C++ support
target_compile_options(${COMPONENT_LIB} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
)

# Define component version (optional)
set_property(TARGET ${COMPONENT_LIB} PROPERTY VERSION "1.0.0")
