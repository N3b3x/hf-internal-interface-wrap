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

idf_component_register(
    SRCS 
        # Utility files
        "${SRC_ROOT}/src/utils/AsciiArtGenerator.cpp"
        "${SRC_ROOT}/src/utils/DigitalOutputGuard.cpp"
        
        # ESP32 implementation files
        "${SRC_ROOT}/src/mcu/esp32/EspAdc.cpp"
        "${SRC_ROOT}/src/mcu/esp32/EspBluetooth.cpp"
        "${SRC_ROOT}/src/mcu/esp32/EspCan.cpp"
        "${SRC_ROOT}/src/mcu/esp32/EspGpio.cpp"
        "${SRC_ROOT}/src/mcu/esp32/EspI2c.cpp"
        "${SRC_ROOT}/src/mcu/esp32/EspNvs.cpp"
        "${SRC_ROOT}/src/mcu/esp32/EspPeriodicTimer.cpp"
        "${SRC_ROOT}/src/mcu/esp32/EspPio.cpp"
        "${SRC_ROOT}/src/mcu/esp32/EspPwm.cpp"
        "${SRC_ROOT}/src/mcu/esp32/EspSpi.cpp"
        "${SRC_ROOT}/src/mcu/esp32/EspTemperature.cpp"
        "${SRC_ROOT}/src/mcu/esp32/EspUart.cpp"
        "${SRC_ROOT}/src/mcu/esp32/EspWifi.cpp"
        
    INCLUDE_DIRS 
        "${SRC_ROOT}/inc"
        "${SRC_ROOT}/inc/base"
        "${SRC_ROOT}/inc/mcu"
        "${SRC_ROOT}/inc/mcu/esp32"
        "${SRC_ROOT}/inc/mcu/esp32/utils"
        "${SRC_ROOT}/inc/utils"
        
    REQUIRES 
        # Core ESP-IDF components that we actually need
        driver 
        esp_timer 
        freertos
        nvs_flash
        esp_event
        esp_system
        esp_hw_support
        hal
        soc
        log
        esp_pm
        esp_wifi
        bt
)
