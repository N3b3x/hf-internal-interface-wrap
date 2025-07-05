cmake_minimum_required(VERSION 3.23)

idf_component_register(    SRCS
        # MCU Implementation Sources
        "../../../../src/mcu/McuGpio.cpp"
        "../../../../src/mcu/McuCan.cpp"
        "../../../../src/mcu/McuPwm.cpp"
        "../../../../src/mcu/McuAdc.cpp"
        "../../../../src/mcu/McuPio.cpp"
        "../../../../src/mcu/McuI2c.cpp"
        "../../../../src/mcu/McuSpi.cpp"
        "../../../../src/mcu/McuUart.cpp"
        "../../../../src/mcu/McuNvsStorage.cpp"
        "../../../../src/mcu/McuPeriodicTimer.cpp"
        
        # Thread-Safe Wrapper Sources
        "../../../../src/thread_safe/SfCan.cpp"
        "../../../../src/thread_safe/SfI2cBus.cpp"
        "../../../../src/thread_safe/SfSpiBus.cpp"
        "../../../../src/thread_safe/SfUartDriver.cpp"
        "../../../../src/thread_safe/SfPwm.cpp"
        "../../../../src/thread_safe/SfAdc.cpp"
        "../../../../src/thread_safe/SfGpio.cpp"
        
        # Utility Sources
        "../../../../src/utils/DigitalOutputGuard.cpp"
        
    INCLUDE_DIRS 
        "../../../../inc"
        "../../../../inc/base"
        "../../../../inc/mcu"
        "../../../../inc/mcu/utils"
        "../../../../inc/thread_safe"
        "../../../../inc/utils"
        
    REQUIRES driver esp_adc freertos esp_timer nvs_flash nvs_sec_provider
)
