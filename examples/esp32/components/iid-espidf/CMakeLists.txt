cmake_minimum_required(VERSION 3.23)

# Get the absolute path to the project root
# In CI, the files are copied to the build directory, so we can use relative paths from CMAKE_SOURCE_DIR
get_filename_component(PROJECT_ROOT "${CMAKE_SOURCE_DIR}" ABSOLUTE)

# Check if we're in the CI environment (files copied to build directory)
if(EXISTS "${PROJECT_ROOT}/src" AND EXISTS "${PROJECT_ROOT}/inc")
    # We're in CI or local build with copied files
    set(SRC_ROOT "${PROJECT_ROOT}")
else()
    # We're in development with original structure
    get_filename_component(SRC_ROOT "${CMAKE_SOURCE_DIR}/../../" ABSOLUTE)
endif()

message(STATUS "HF Internal Interface Wrap - ESP32 Component Configuration")
message(STATUS "Project Root: ${PROJECT_ROOT}")
message(STATUS "Source Root: ${SRC_ROOT}")

# List all potential ESP32 implementation source files
set(HF_ESP32_SRCS
    "${SRC_ROOT}/src/mcu/esp32/EspGpio.cpp"
    "${SRC_ROOT}/src/mcu/esp32/EspI2c.cpp"
    "${SRC_ROOT}/src/mcu/esp32/EspNvs.cpp"
    "${SRC_ROOT}/src/mcu/esp32/EspPeriodicTimer.cpp"
    "${SRC_ROOT}/src/mcu/esp32/EspPwm.cpp"
    "${SRC_ROOT}/src/mcu/esp32/EspSpi.cpp"
    "${SRC_ROOT}/src/mcu/esp32/EspUart.cpp"
    "${SRC_ROOT}/src/mcu/esp32/EspBluetooth.cpp"    # ESP32C6 NimBLE Bluetooth implementation
    # Temporarily excluded due to missing dependencies:
    "${SRC_ROOT}/src/mcu/esp32/EspAdc.cpp"          # Missing esp_adc headers for ESP32-C6
      
    "${SRC_ROOT}/src/mcu/esp32/EspCan.cpp"          # May have TWAI compatibility issues
    "${SRC_ROOT}/src/mcu/esp32/EspPio.cpp"          # May have RMT compatibility issues
    "${SRC_ROOT}/src/mcu/esp32/EspTemperature.cpp"  # May have sensor compatibility issues
    "${SRC_ROOT}/src/mcu/esp32/EspWifi.cpp"         # May have WiFi config compatibility issues
)

# List utility source files
set(HF_UTIL_SRCS
    "${SRC_ROOT}/src/utils/AsciiArtGenerator.cpp"
    "${SRC_ROOT}/src/utils/DigitalOutputGuard.cpp"
)

# Check which source files actually exist and add them
set(EXISTING_SRCS "")
foreach(src_file ${HF_ESP32_SRCS} ${HF_UTIL_SRCS})
    if(EXISTS "${src_file}")
        list(APPEND EXISTING_SRCS "${src_file}")
        message(STATUS "✓ Found source: ${src_file}")
    else()
        message(WARNING "✗ Source file not found: ${src_file}")
    endif()
endforeach()

idf_component_register(
    SRCS 
        ${EXISTING_SRCS}
        
    INCLUDE_DIRS 
        "${SRC_ROOT}/inc"
        "${SRC_ROOT}/inc/base"
        "${SRC_ROOT}/inc/mcu"
        "${SRC_ROOT}/inc/mcu/esp32"
        "${SRC_ROOT}/inc/mcu/esp32/utils"
        "${SRC_ROOT}/inc/utils"
        
    REQUIRES 
        # ESP-IDF core components
        driver              # GPIO, I2C, SPI, UART drivers
        esp_timer           # EspPeriodicTimer
        freertos            # RTOS support
        nvs_flash           # EspNvs
        esp_event           # Event system
        esp_system          # System utilities
        esp_hw_support      # Hardware support
        hal                 # Hardware abstraction
        soc                 # SoC definitions
        log                 # Logging
        
        # ADC components
        esp_adc
        
        # Driver components
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_spi
        esp_driver_uart
        esp_driver_ledc
        esp_driver_rmt
        esp_driver_gptimer
        esp_driver_tsens
        
        # Network components
        esp_wifi
        esp_netif
        esp_event
        lwip
        
        # Bluetooth components (available on most ESP32 targets)
        bt
        
        # Storage components
        nvs_flash
        nvs_sec_provider
        
        # Additional components for full functionality
        esp_common          # Common ESP32 utilities
        bootloader_support  # Bootloader support utilities
        app_update          # Application update support
        spi_flash           # SPI flash support
        partition_table     # Partition table support
        
    PRIV_REQUIRES
        # Private requirements (internal use only)
        esp_pm              # Power management (private)
        esp_phy
        wpa_supplicant
)

# Set C++ standard
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)

# Add ESP32 family definitions based on target
if(CONFIG_IDF_TARGET_ESP32)
    target_compile_definitions(${COMPONENT_LIB} PUBLIC
        HF_TARGET_MCU_ESP32=1
        HF_MCU_ESP32=1
        HF_MCU_FAMILY_ESP32=1
        HF_THREAD_SAFE=1
        ESP32=1
    )
elseif(CONFIG_IDF_TARGET_ESP32C6)
    target_compile_definitions(${COMPONENT_LIB} PUBLIC
        HF_TARGET_MCU_ESP32C6=1
        HF_MCU_ESP32C6=1
        HF_MCU_FAMILY_ESP32=1
        HF_THREAD_SAFE=1
        ESP32C6=1
    )
elseif(CONFIG_IDF_TARGET_ESP32S3)
    target_compile_definitions(${COMPONENT_LIB} PUBLIC
        HF_TARGET_MCU_ESP32S3=1
        HF_MCU_ESP32S3=1
        HF_MCU_FAMILY_ESP32=1
        HF_THREAD_SAFE=1
        ESP32S3=1
    )
else()
    # Default to ESP32
    target_compile_definitions(${COMPONENT_LIB} PUBLIC
        HF_TARGET_MCU_ESP32=1
        HF_MCU_ESP32=1
        HF_MCU_FAMILY_ESP32=1
        HF_THREAD_SAFE=1
        ESP32=1
    )
endif()

# Add target-specific Bluetooth support definitions
if(CONFIG_IDF_TARGET_ESP32C6)
    message(STATUS "Configuring ESP32C6 with NimBLE BLE-only support")
    target_compile_definitions(${COMPONENT_LIB} PUBLIC
        HAS_BLE_SUPPORT=1
        HAS_NIMBLE_SUPPORT=1
        HAS_CLASSIC_BLUETOOTH=0
        CONFIG_BT_NIMBLE_ENABLED=1
    )
elseif(CONFIG_IDF_TARGET_ESP32 OR CONFIG_IDF_TARGET_ESP32S3)
    message(STATUS "Configuring ${CONFIG_IDF_TARGET} with full Bluetooth support")
    target_compile_definitions(${COMPONENT_LIB} PUBLIC
        HAS_BLE_SUPPORT=1
        HAS_CLASSIC_BLUETOOTH=1
        HAS_NIMBLE_SUPPORT=1
        HAS_BLUEDROID_SUPPORT=1
    )
elseif(CONFIG_IDF_TARGET_ESP32C3 OR CONFIG_IDF_TARGET_ESP32H2)
    message(STATUS "Configuring ${CONFIG_IDF_TARGET} with BLE-only support")
    target_compile_definitions(${COMPONENT_LIB} PUBLIC
        HAS_BLE_SUPPORT=1
        HAS_CLASSIC_BLUETOOTH=0
        HAS_NIMBLE_SUPPORT=0
        HAS_BLUEDROID_SUPPORT=1
    )
elseif(CONFIG_IDF_TARGET_ESP32S2)
    message(STATUS "ESP32S2 has no Bluetooth support")
    target_compile_definitions(${COMPONENT_LIB} PUBLIC
        HAS_BLE_SUPPORT=0
        HAS_CLASSIC_BLUETOOTH=0
        HAS_NIMBLE_SUPPORT=0
        HAS_BLUEDROID_SUPPORT=0
    )
endif()

# Compiler flags for ESP32-C6 optimization
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wall
    -Wextra
    -O2
    -g
    -ffunction-sections
    -fdata-sections
    -Wno-pedantic
)

# Add specific flags for C++ files
target_compile_options(${COMPONENT_LIB} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-error=pedantic>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-pedantic>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-variadic-macros>
)

# Link time optimization for better performance
if(CONFIG_COMPILER_OPTIMIZATION_SIZE OR CONFIG_COMPILER_OPTIMIZATION_PERF)
    target_compile_options(${COMPONENT_LIB} PRIVATE -flto)
    target_link_options(${COMPONENT_LIB} PRIVATE -flto)
endif()

# Final status messages
list(LENGTH EXISTING_SRCS NUM_SOURCES)
message(STATUS "=== HF Internal Interface Wrap Component Summary ===")
message(STATUS "✓ Component configured successfully")
message(STATUS "✓ Included ${NUM_SOURCES} source files")
if(CONFIG_IDF_TARGET_ESP32)
    message(STATUS "✓ Target MCU: ESP32")
elseif(CONFIG_IDF_TARGET_ESP32C6)
    message(STATUS "✓ Target MCU: ESP32-C6")
elseif(CONFIG_IDF_TARGET_ESP32S3)
    message(STATUS "✓ Target MCU: ESP32-S3")
else()
    message(STATUS "✓ Target MCU: ESP32 (default)")
endif()
message(STATUS "✓ C++ Standard: C++17")
message(STATUS "✓ Thread Safety: ENABLED")
message(STATUS "✓ Optimization: LTO enabled (if configured)")
message(STATUS "===============================================")
