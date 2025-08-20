#include "inc/mcu/esp32/EspI2c.h"
#include "inc/base/BaseI2c.h"

// Simple test to verify compilation
int main() {
    // Test basic types
    hf_i2c_master_bus_config_t bus_config = {};
    hf_i2c_device_config_t device_config = {};
    
    // Test bus creation
    EspI2cBus bus(bus_config);
    
    // Test device creation
    hf_i2c_device_config_t dev_cfg = {};
    dev_cfg.device_address = 0x48;
    dev_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
    dev_cfg.scl_speed_hz = 100000;
    
    // This should compile without errors
    return 0;
}