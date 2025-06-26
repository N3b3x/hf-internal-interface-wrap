cmake_minimum_required(VERSION 3.23)

idf_component_register(    SRCS
        # MCU Implementation Sources
        "../../../src/mcu/McuDigitalGpio.cpp"
        "../../../src/mcu/McuCan.cpp"
        "../../../src/mcu/McuPwm.cpp"
        "../../../src/mcu/McuAdc.cpp"
        "../../../src/mcu/McuPio.cpp"
        
        # Thread-Safe Wrapper Sources
        "../../../src/thread_safe/SfCan.cpp"
        "../../../src/thread_safe/SfI2cBus.cpp"
        "../../../src/thread_safe/SfSpiBus.cpp"
        "../../../src/thread_safe/SfUartDriver.cpp"
        "../../../src/thread_safe/SfPwm.cpp"
        
        # Driver Sources
        "../../../src/drivers/I2cBus.cpp"
        "../../../src/drivers/SpiBus.cpp"
        "../../../src/drivers/UartDriver.cpp"
          # Utility Sources
        "../../../src/utils/DigitalOutputGuard.cpp"
        "../../../src/utils/PeriodicTimer.cpp"
        "../../../src/utils/NvsStorage.cpp"
        
        # Modern MCU Sources
        "../../../src/mcu/McuInit.cpp"
        
    INCLUDE_DIRS 
        "../../../include"
        "../../../include/base"
        "../../../include/mcu"
        "../../../include/thread_safe"
        "../../../include/drivers"
        "../../../include/utils"
        "../../../include/platform"
        
    REQUIRES driver esp_adc freertos esp_timer nvs_flash
)
