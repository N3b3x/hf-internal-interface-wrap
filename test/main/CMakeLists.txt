cmake_minimum_required(VERSION 3.16)

# Test main component CMakeLists.txt
# This component contains all the unit test source files

set(TEST_SOURCES
    # Test runner
    "test_main.cpp"
    
    # EspGpio tests
    "test_esp_gpio_basic.cpp"
    "test_esp_gpio_interrupts.cpp"
    "test_esp_gpio_advanced.cpp"
    "test_esp_gpio_power_management.cpp"
    
    # EspAdc tests
    "test_esp_adc_basic.cpp"
    "test_esp_adc_oneshot.cpp"
    "test_esp_adc_continuous.cpp"
    "test_esp_adc_calibration.cpp"
    "test_esp_adc_filters.cpp"
    "test_esp_adc_monitors.cpp"
    
    # Base class tests
    "test_base_gpio.cpp"
    "test_base_adc.cpp"
    
    # Integration tests
    "test_integration_gpio_adc.cpp"
)

# Include directories
set(TEST_INCLUDE_DIRS
    "."
    "../include"
    "../../inc"
    "../../inc/base"
    "../../inc/mcu/esp32"
    "../../inc/mcu/esp32/utils"
    "../../inc/utils"
)

# Component requirements
set(TEST_REQUIRES
    unity
    esp_idf_mocks
    freertos
    esp_common
    esp_hw_support
    esp_system
    log
    soc
    hal
    driver
    esp_adc
)

# Register the test component
idf_component_register(
    SRCS ${TEST_SOURCES}
    INCLUDE_DIRS ${TEST_INCLUDE_DIRS}
    REQUIRES ${TEST_REQUIRES}
    WHOLE_ARCHIVE  # Ensure all test functions are linked
)

# Set C++ standard and compile options
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)

# Add test-specific compile options
target_compile_options(${COMPONENT_LIB} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
    -Wall
    -Wextra
    -Werror
    -DUNITY_INCLUDE_CONFIG_H
)

# Add test definitions
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    UNIT_TESTING=1
    ESP32_TESTING=1
    UNITY_SUPPORT_64=1
    UNITY_INCLUDE_DOUBLE=1
    UNITY_INCLUDE_FLOAT=1
)

# Link against source files from main project (mocked for testing)
target_link_libraries(${COMPONENT_LIB} PRIVATE
    esp_idf_mocks
)