cmake_minimum_required(VERSION 3.23)

idf_component_register(
    SRCS
        "../../../src/DigitalExternalIRQ.cpp"
        "../../../src/DigitalGpio.cpp"
        "../../../src/DigitalInput.cpp"
        "../../../src/DigitalOutput.cpp"
        "../../../src/Esp32C6Adc.cpp"
        "../../../src/FlexCan.cpp"
        "../../../src/SfFlexCan.cpp"
        "../../../src/I2cBus.cpp"
        "../../../src/SfSpiBus.cpp"
        "../../../src/SfI2cBus.cpp"
        "../../../src/SpiBus.cpp"
        "../../../src/PwmOutput.cpp"
        "../../../src/PeriodicTimer.cpp"
        "../../../src/UartDriver.cpp"
        "../../../src/SfUartDriver.cpp"
        "../../../src/RMT.cpp"
        "../../../src/NvsStorage.cpp"
    INCLUDE_DIRS "../../../inc"
    REQUIRES driver esp_adc freertos esp_timer nvs nvs_flash
)
