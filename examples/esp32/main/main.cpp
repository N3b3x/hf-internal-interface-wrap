#include "DigitalOutput.h"
#include "DummyDevice.hpp"
#include "I2cBus.h"
#include "McuSelect.h"
#include <iostream>

extern "C" void app_main(void) {
  // Test MCU selection system first
  std::cout << "=== ESP32-C6 MCU Selection Test ===" << std::endl;
  std::cout << "Selected MCU: " << HF_MCU_NAME << std::endl;
  std::cout << "Architecture: " << HF_MCU_ARCHITECTURE << std::endl;
  
  #if defined(HF_MCU_FAMILY_ESP32)
    std::cout << "Platform Family: ESP32" << std::endl;
    std::cout << "GPIO Max Pins: " << HF_MCU_GPIO_MAX_PINS << std::endl;
    std::cout << "ADC Max Channels: " << HF_MCU_ADC_MAX_CHANNELS << std::endl;
    std::cout << "CAN Protocol: " << HF_MCU_CAN_PROTOCOL << std::endl;
    
    #ifdef HF_MCU_ESP32C6
      std::cout << "ESP32-C6 specific features detected" << std::endl;
      static_assert(HF_MCU_GPIO_MAX_PINS == 31, "ESP32-C6 should have 31 GPIO pins");
      static_assert(HF_MCU_ADC_MAX_CHANNELS == 7, "ESP32-C6 should have 7 ADC channels");
    #endif
  #endif
  
  // Test MCU capabilities
  std::cout << "\n=== MCU Capability Test ===" << std::endl;
  std::cout << "GPIO Support: " << (HF_MCU_HAS_GPIO ? "YES" : "NO") << std::endl;
  std::cout << "I2C Support: " << (HF_MCU_HAS_I2C ? "YES" : "NO") << std::endl;
  std::cout << "CAN Support: " << (HF_MCU_HAS_CAN ? "YES" : "NO") << std::endl;
  
  // ESP32-C6 specific GPIO configuration
  i2c_config_t cfg{};
  cfg.sda_io_num = GPIO_NUM_6;  // ESP32-C6 default SDA
  cfg.scl_io_num = GPIO_NUM_7;  // ESP32-C6 default SCL
  
  I2cBus bus(I2C_NUM_0, cfg);
  bus.Open();
  
  DummyDevice dev(bus, 0x50);
  dev.Init();
  
  // Use GPIO_NUM_8 for LED on ESP32-C6 (built-in LED pin)
  DigitalOutput led(GPIO_NUM_8, DigitalGpio::ActiveState::High);
  led.SetActive();
  
  std::cout << "\n=== ESP32-C6 Hardware Initialization Complete ===" << std::endl;
  std::cout << "I2C Bus: GPIO6 (SDA), GPIO7 (SCL)" << std::endl;
  std::cout << "LED: GPIO8 (Active)" << std::endl;
}
