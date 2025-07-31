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
        driver 
        esp_timer 
        freertos
        nvs_flash
        esp_adc
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_spi
        esp_driver_uart
        esp_driver_ledc
        esp_driver_pcnt
        esp_driver_rmt
        esp_driver_mcpwm
        esp_driver_gptimer
        esp_driver_sdmmc
        esp_driver_sdspi
        esp_driver_sdio
        esp_driver_dac
        esp_driver_tsens
        esp_driver_sdm
        esp_driver_ana_cmpr
        esp_driver_bitscrambler
        esp_driver_cam
        esp_driver_isp
        esp_driver_jpeg
        esp_driver_parlio
        esp_driver_ppa
        esp_driver_touch_sens
        esp_driver_usb_serial_jtag
        esp_eth
        esp_event
        esp_gdbstub
        esp_hid
        esp_http_client
        esp_http_server
        esp_https_ota
        esp_https_server
        esp_hw_support
        esp_lcd
        esp_local_ctrl
        esp_mm
        esp_netif
        esp_netif_stack
        esp_partition
        esp_phy
        esp_pm
        esp_psram
        esp_ringbuf
        esp_rom
        esp_security
        esp_system
        esp_vfs_console
        esp_wifi
        bt
        espcoredump
        esptool_py
        fatfs
        hal
        heap
        http_parser
        idf_test
        ieee802154
        json
        log
        lwip
        mbedtls
        mqtt
        newlib
        nvs_sec_provider
        openthread
        partition_table
        protobuf-c
        protocomm
        pthread
        rt
        sdmmc
        soc
        spi_flash
        spiffs
        tcp_transport
        ulp
        unity
        usb
        vfs
        wear_levelling
        wifi_provisioning
        wpa_supplicant
)
