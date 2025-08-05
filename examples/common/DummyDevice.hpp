#pragma once
#include "mcu/esp32/EspI2c.h"

/**
 * @brief Example device class demonstrating proper I2C bus-device architecture usage
 *
 * This class shows how to use the new EspI2cBus/EspI2cDevice architecture
 * introduced in the HardFOC system for proper device abstraction.
 */
class DummyDevice {
public:
  /**
   * @brief Constructor for I2C device
   * @param bus Reference to the I2C bus
   * @param address I2C device address
   */
  DummyDevice(EspI2cBus& bus, uint8_t address) : bus_(bus), address_(address), device_index_(-1) {}

  /**
   * @brief Initialize the device
   * @return true if successful, false otherwise
   */
  bool Init() {
    // Create device configuration
    hf_i2c_device_config_t device_config = {};
    device_config.device_address = address_;
    device_config.scl_speed_hz = 100000; // 100kHz standard mode

    // Create device on the bus
    device_index_ = bus_.CreateDevice(device_config);
    if (device_index_ < 0) {
      return false;
    }

    // Try to read device ID register (common pattern)
    uint8_t device_id = 0;
    auto result = bus_.ReadDevice(device_index_, 0x00, &device_id, 1); // Read from register 0x00
    return result == hf_i2c_err_t::I2C_SUCCESS;
  }

  /**
   * @brief Read data from device
   * @param reg_addr Register address to read from
   * @param data Buffer to store read data
   * @param length Number of bytes to read
   * @return true if successful, false otherwise
   */
  bool Read(uint8_t reg_addr, uint8_t* data, size_t length) {
    if (device_index_ < 0) {
      return false;
    }

    auto result = bus_.ReadDevice(device_index_, reg_addr, data, length);
    return result == hf_i2c_err_t::I2C_SUCCESS;
  }

  /**
   * @brief Write data to device
   * @param reg_addr Register address to write to
   * @param data Data to write
   * @param length Number of bytes to write
   * @return true if successful, false otherwise
   */
  bool Write(uint8_t reg_addr, const uint8_t* data, size_t length) {
    if (device_index_ < 0) {
      return false;
    }

    auto result = bus_.WriteDevice(device_index_, reg_addr, data, length);
    return result == hf_i2c_err_t::I2C_SUCCESS;
  }

  /**
   * @brief Get device I2C address
   * @return Device address
   */
  uint8_t GetAddress() const {
    return address_;
  }

  /**
   * @brief Check if device is initialized
   * @return true if initialized, false otherwise
   */
  bool IsInitialized() const {
    return device_index_ >= 0;
  }

private:
  EspI2cBus& bus_;   ///< Reference to I2C bus
  uint8_t address_;  ///< Device I2C address
  int device_index_; ///< Device index on the bus (-1 if not initialized)
};
