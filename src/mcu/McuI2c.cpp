/**
 * @file McuI2c.cpp
 * @brief Implementation of MCU-integrated I2C controller.
 */

#include "McuI2c.h"

// Platform-specific includes via centralized McuSelect.h
#ifdef HF_MCU_FAMILY_ESP32
#include "driver/i2c.h" 
#include "esp_log.h"
static const char* TAG = "McuI2c";
#endif

//==============================================//
// CONSTRUCTOR & DESTRUCTOR                     //
//==============================================//

McuI2c::McuI2c(const I2cBusConfig& config) noexcept
    : BaseI2cBus(config),
      platform_handle_(nullptr),
      last_error_(HfI2cErr::I2C_SUCCESS),
      transaction_count_(0),
      bus_locked_(false) {
}

McuI2c::~McuI2c() noexcept {
    if (initialized_) {
        Deinitialize();
    }
}

//==============================================//
// OVERRIDDEN PURE VIRTUAL FUNCTIONS            //
//==============================================//

bool McuI2c::Initialize() noexcept {
    if (initialized_) {
        return true;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    // Validate configuration
    if (config_.sda_pin == HF_GPIO_INVALID || config_.scl_pin == HF_GPIO_INVALID) {
        last_error_ = HfI2cErr::I2C_ERR_INVALID_CONFIGURATION;
        return false;
    }

    if (config_.clock_speed_hz == 0 || config_.clock_speed_hz > 1000000) {
        last_error_ = HfI2cErr::I2C_ERR_INVALID_CLOCK_SPEED;
        return false;
    }

    // Platform-specific initialization
    if (!PlatformInitialize()) {
        return false;
    }

    last_error_ = HfI2cErr::I2C_SUCCESS;
    return true;
}

bool McuI2c::Deinitialize() noexcept {
    if (!initialized_) {
        return true;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    bool result = PlatformDeinitialize();
    if (result) {
        last_error_ = HfI2cErr::I2C_SUCCESS;
    }
    
    return result;
}

HfI2cErr McuI2c::Write(uint8_t device_addr, const uint8_t* data,
                          uint16_t length, uint32_t timeout_ms) noexcept {
    if (!EnsureInitialized()) {
        return HfI2cErr::I2C_ERR_NOT_INITIALIZED;
    }

    if (!data && length > 0) {
        return HfI2cErr::I2C_ERR_NULL_POINTER;
    }

    if (!IsValidDeviceAddress(device_addr)) {
        return HfI2cErr::I2C_ERR_INVALID_ADDRESS;
    }

    std::lock_guard<std::mutex> lock(mutex_);

#ifdef HF_MCU_FAMILY_ESP32
    esp_err_t err;
    uint32_t timeout = GetTimeoutMs(timeout_ms);
    
    // Create I2C command link
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        last_error_ = HfI2cErr::I2C_ERR_OUT_OF_MEMORY;
        return last_error_;
    }

    // Start condition
    i2c_master_start(cmd);
    
    // Write device address with write bit
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, true);
    
    // Write data if any
    if (length > 0) {
        i2c_master_write(cmd, data, length, true);
    }
    
    // Stop condition
    i2c_master_stop(cmd);
    
    // Execute command
    err = i2c_master_cmd_begin(static_cast<i2c_port_t>(config_.port), cmd, 
                               pdMS_TO_TICKS(timeout));
    
    // Clean up
    i2c_cmd_link_delete(cmd);
    
    last_error_ = ConvertPlatformError(err);
    transaction_count_++;
    
    return last_error_;
#else
    last_error_ = HfI2cErr::I2C_ERR_UNSUPPORTED_OPERATION;
    return last_error_;
#endif
}

HfI2cErr McuI2c::Read(uint8_t device_addr, uint8_t* data,
                         uint16_t length, uint32_t timeout_ms) noexcept {
    if (!EnsureInitialized()) {
        return HfI2cErr::I2C_ERR_NOT_INITIALIZED;
    }

    if (!data || length == 0) {
        return HfI2cErr::I2C_ERR_INVALID_PARAMETER;
    }

    if (!IsValidDeviceAddress(device_addr)) {
        return HfI2cErr::I2C_ERR_INVALID_ADDRESS;
    }

    std::lock_guard<std::mutex> lock(mutex_);

#ifdef HF_MCU_FAMILY_ESP32
    esp_err_t err;
    uint32_t timeout = GetTimeoutMs(timeout_ms);
    
    // Create I2C command link
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        last_error_ = HfI2cErr::I2C_ERR_OUT_OF_MEMORY;
        return last_error_;
    }

    // Start condition
    i2c_master_start(cmd);
    
    // Write device address with read bit
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_READ, true);
    
    // Read data
    if (length > 1) {
        i2c_master_read(cmd, data, length - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, &data[length - 1], I2C_MASTER_NACK);
    
    // Stop condition
    i2c_master_stop(cmd);
    
    // Execute command
    err = i2c_master_cmd_begin(static_cast<i2c_port_t>(config_.port), cmd, 
                               pdMS_TO_TICKS(timeout));
    
    // Clean up
    i2c_cmd_link_delete(cmd);
    
    last_error_ = ConvertPlatformError(err);
    transaction_count_++;
    
    return last_error_;
#else
    last_error_ = HfI2cErr::I2C_ERR_UNSUPPORTED_OPERATION;
    return last_error_;
#endif
}

HfI2cErr McuI2c::WriteRead(uint8_t device_addr, const uint8_t* tx_data, uint16_t tx_length,
                              uint8_t* rx_data, uint16_t rx_length, uint32_t timeout_ms) noexcept {
    if (!EnsureInitialized()) {
        return HfI2cErr::I2C_ERR_NOT_INITIALIZED;
    }

    if ((!tx_data && tx_length > 0) || (!rx_data && rx_length > 0)) {
        return HfI2cErr::I2C_ERR_NULL_POINTER;
    }

    if (!IsValidDeviceAddress(device_addr)) {
        return HfI2cErr::I2C_ERR_INVALID_ADDRESS;
    }

    std::lock_guard<std::mutex> lock(mutex_);

#ifdef HF_MCU_FAMILY_ESP32
    esp_err_t err;
    uint32_t timeout = GetTimeoutMs(timeout_ms);
    
    // Create I2C command link
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        last_error_ = HfI2cErr::I2C_ERR_OUT_OF_MEMORY;
        return last_error_;
    }

    // Write phase
    if (tx_length > 0) {
        // Start condition
        i2c_master_start(cmd);
        
        // Write device address with write bit
        i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, true);
        
        // Write data
        i2c_master_write(cmd, tx_data, tx_length, true);
    }
    
    // Read phase
    if (rx_length > 0) {
        // Repeated start condition
        i2c_master_start(cmd);
        
        // Write device address with read bit
        i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_READ, true);
        
        // Read data
        if (rx_length > 1) {
            i2c_master_read(cmd, rx_data, rx_length - 1, I2C_MASTER_ACK);
        }
        i2c_master_read_byte(cmd, &rx_data[rx_length - 1], I2C_MASTER_NACK);
    }
    
    // Stop condition
    i2c_master_stop(cmd);
    
    // Execute command
    err = i2c_master_cmd_begin(static_cast<i2c_port_t>(config_.port), cmd, 
                               pdMS_TO_TICKS(timeout));
    
    // Clean up
    i2c_cmd_link_delete(cmd);
    
    last_error_ = ConvertPlatformError(err);
    transaction_count_++;
    
    return last_error_;
#else
    last_error_ = HfI2cErr::I2C_ERR_UNSUPPORTED_OPERATION;
    return last_error_;
#endif
}

//==============================================//
// ENHANCED METHODS                             //
//==============================================//

bool McuI2c::IsBusy() noexcept {
    if (!initialized_) {
        return false;
    }

#ifdef HF_MCU_FAMILY_ESP32
    // Check if I2C bus is busy (platform specific implementation)
    // This is a simplified implementation
    return bus_locked_;
#else
    return false;
#endif
}

bool McuI2c::ResetBus() noexcept {
    if (!initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);

#ifdef HF_MCU_FAMILY_ESP32
    // Reset I2C bus by deinitializing and reinitializing
    if (PlatformDeinitialize() && PlatformInitialize()) {
        last_error_ = HfI2cErr::I2C_SUCCESS;
        return true;
    }
    return false;
#else
    return false;
#endif
}

bool McuI2c::SetClockSpeed(uint32_t clock_speed_hz) noexcept {
    if (clock_speed_hz == 0 || clock_speed_hz > 1000000) {
        return false;
    }

    config_.clock_speed_hz = clock_speed_hz;
    
    // Reinitialize if already initialized
    if (initialized_) {
        return ResetBus();
    }
    
    return true;
}

bool McuI2c::SetPullUps(bool enable) noexcept {
    config_.enable_pullups = enable;
    
    // Reinitialize if already initialized
    if (initialized_) {
        return ResetBus();
    }
    
    return true;
}

uint32_t McuI2c::GetBusStatus() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
    // Return platform-specific status information
    return static_cast<uint32_t>(last_error_);
#else
    return 0;
#endif
}

bool McuI2c::RecoverBus() noexcept {
    // Implement bus recovery sequence (clock stretching, etc.)
    return ResetBus();
}

//==============================================//
// PRIVATE METHODS                              //
//==============================================//

HfI2cErr McuI2c::ConvertPlatformError(int32_t platform_error) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
    switch (platform_error) {
        case ESP_OK:
            return HfI2cErr::I2C_SUCCESS;
        case ESP_ERR_INVALID_ARG:
            return HfI2cErr::I2C_ERR_INVALID_PARAMETER;
        case ESP_ERR_TIMEOUT:
            return HfI2cErr::I2C_ERR_TIMEOUT;
        case ESP_FAIL:
            return HfI2cErr::I2C_ERR_FAILURE;
        case ESP_ERR_INVALID_STATE:
            return HfI2cErr::I2C_ERR_NOT_INITIALIZED;
        default:
            return HfI2cErr::I2C_ERR_COMMUNICATION_FAILURE;
    }
#else
    (void)platform_error;
    return HfI2cErr::I2C_ERR_UNSUPPORTED_OPERATION;
#endif
}

bool McuI2c::PlatformInitialize() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
    // Configure I2C parameters
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = static_cast<gpio_num_t>(config_.sda_pin);
    conf.scl_io_num = static_cast<gpio_num_t>(config_.scl_pin);
    conf.sda_pullup_en = config_.enable_pullups ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    conf.scl_pullup_en = config_.enable_pullups ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    conf.master.clk_speed = config_.clock_speed_hz;

    // Configure I2C
    esp_err_t err = i2c_param_config(static_cast<i2c_port_t>(config_.port), &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
        last_error_ = ConvertPlatformError(err);
        return false;
    }

    // Install I2C driver
    err = i2c_driver_install(static_cast<i2c_port_t>(config_.port), 
                            conf.mode, 
                            config_.rx_buffer_size, 
                            config_.tx_buffer_size, 
                            0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(err));
        last_error_ = ConvertPlatformError(err);
        return false;
    }

    ESP_LOGI(TAG, "I2C bus initialized on port %d, SDA=%d, SCL=%d, clock=%lu Hz",
             config_.port, config_.sda_pin, config_.scl_pin, config_.clock_speed_hz);
    
    return true;
#else
    last_error_ = HfI2cErr::I2C_ERR_UNSUPPORTED_OPERATION;
    return false;
#endif
}

bool McuI2c::PlatformDeinitialize() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
    esp_err_t err = i2c_driver_delete(static_cast<i2c_port_t>(config_.port));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_delete failed: %s", esp_err_to_name(err));
        last_error_ = ConvertPlatformError(err);
        return false;
    }
    
    ESP_LOGI(TAG, "I2C bus deinitialized on port %d", config_.port);
    return true;
#else
    return false;
#endif
}
