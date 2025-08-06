cmake_minimum_required(VERSION 3.16)

# ESP-IDF Mocking component for unit testing
# This component provides mock implementations of ESP-IDF hardware functions
# to enable testing of EspGpio and EspAdc without actual hardware

set(MOCK_SOURCES
    # Core ESP-IDF mocks
    "src/mock_esp_log.c"
    "src/mock_esp_system.c"
    "src/mock_freertos.c"
    
    # GPIO mocks
    "src/mock_gpio.c"
    "src/mock_esp_attr.c"
    
    # ADC mocks
    "src/mock_adc_oneshot.c"
    "src/mock_adc_continuous.c"
    "src/mock_adc_cali.c"
    "src/mock_adc_filter.c"
    "src/mock_adc_monitor.c"
    
    # Timer and system mocks
    "src/mock_esp_timer.c"
    
    # Hardware abstraction mocks
    "src/mock_hal.c"
    "src/mock_soc.c"
    
    # Mock state management
    "src/mock_state_manager.c"
)

set(MOCK_INCLUDE_DIRS
    "include"
    "include/mock"
    "include/driver"
    "include/esp_adc"
    "include/freertos"
    "include/hal"
    "include/soc"
)

# Register the mocking component
idf_component_register(
    SRCS ${MOCK_SOURCES}
    INCLUDE_DIRS ${MOCK_INCLUDE_DIRS}
    REQUIRES unity
)

# Set compile definitions for mocking
target_compile_definitions(${COMPONENT_LIB} PUBLIC
    ESP_IDF_MOCKING=1
    UNIT_TESTING=1
    CONFIG_FREERTOS_HZ=1000
    CONFIG_ESP_TASK_WDT_TIMEOUT_S=5
    CONFIG_ESP_MAIN_TASK_STACK_SIZE=8192
    SOC_GPIO_PIN_COUNT=50
    SOC_ADC_MAX_CHANNEL_NUM=10
    SOC_ADC_PERIPH_NUM=2
)

# Disable warnings for mock functions
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-function
    -Wno-missing-field-initializers
)

# Add include path mapping for ESP-IDF headers
target_include_directories(${COMPONENT_LIB} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)