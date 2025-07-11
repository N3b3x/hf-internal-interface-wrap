/**
 * @file EspSpi.cpp
 * @brief Implementation of MCU-integrated SPI controller.
 *
 * This file provides the implementation for SPI bus communication using the
 * microcontroller's built-in SPI peripheral. The implementation supports
 * full-duplex communication, configurable clock speeds, multiple chip select
 * management, and DMA-accelerated transfers for high-performance applications
 * with comprehensive error handling and platform abstraction.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "EspSpi.h"

// C++ standard library headers (must be outside extern "C")
#include <algorithm>
#include <cstring>
#include <vector>


#ifdef HF_MCU_FAMILY_ESP32
// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

#ifdef __cplusplus
}
#endif

static const char *TAG = "EspSpi";

//==============================================//
// CONSTRUCTOR & DESTRUCTOR                     //
//==============================================//

EspSpi::EspSpi(const hf_spi_bus_config_t &config) noexcept
    : BaseSpi(), platform_handle_(nullptr), current_device_(nullptr),
      config_(config), use_advanced_config_(false), last_error_(hf_spi_err_t::SPI_SUCCESS), 
      transaction_count_(0), cs_active_(false), dma_enabled_(false), bus_suspended_(false),
      current_transfer_mode_(hf_spi_transfer_mode_t::HF_SPI_TRANSFER_MODE_SINGLE),
      max_transfer_size_(4092), next_operation_id_(1), event_callback_(nullptr),
      event_user_data_(nullptr), last_transfer_time_(0) {
  // Initialize basic config in advanced_config for unified handling
  advanced_config_.base_config = config;
  ESP_LOGD(TAG, "Creating EspSpi with basic configuration - Clock: %lu Hz, Mode: %d",
           config.clock_speed_hz, config.mode);
}

EspSpi::EspSpi(const hf_spi_advanced_config_t &config) noexcept
    : BaseSpi(), platform_handle_(nullptr), current_device_(nullptr),
      config_(config.base_config), advanced_config_(config), use_advanced_config_(true), 
      last_error_(hf_spi_err_t::SPI_SUCCESS), transaction_count_(0), cs_active_(false), 
      dma_enabled_(config.dma_enabled), bus_suspended_(false), current_transfer_mode_(config.transfer_mode),
      max_transfer_size_(config.max_transfer_size), next_operation_id_(1), event_callback_(nullptr),
      event_user_data_(nullptr), last_transfer_time_(0) {
  ESP_LOGD(TAG, "Creating EspSpi with advanced configuration - DMA: %s, Transfer mode: %d",
           dma_enabled_ ? "enabled" : "disabled", static_cast<int>(current_transfer_mode_));
}

EspSpi::~EspSpi() noexcept {
  if (initialized_) {
    Deinitialize();
  }
}

//==============================================//
// OVERRIDDEN PURE VIRTUAL FUNCTIONS            //
//==============================================//

bool EspSpi::Initialize() noexcept {
  if (initialized_) {
    return true;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  // Validate configuration
  const hf_spi_bus_config_t &cfg = use_advanced_config_ ? advanced_config_.base_config : config_;

  if (cfg.mosi_pin == HF_INVALID_PIN && cfg.miso_pin == HF_INVALID_PIN) {
    last_error_ = hf_spi_err_t::SPI_ERR_INVALID_CONFIGURATION;
    return false;
  }

  if (cfg.sclk_pin == HF_INVALID_PIN) {
    last_error_ = hf_spi_err_t::SPI_ERR_INVALID_CONFIGURATION;
    return false;
  }

  if (!IsValidClockSpeed(cfg.clock_speed_hz)) {
    last_error_ = hf_spi_err_t::SPI_ERR_INVALID_CLOCK_SPEED;
    return false;
  }

  if (!IsValidMode(cfg.mode)) {
    last_error_ = hf_spi_err_t::SPI_ERR_INVALID_MODE;
    return false;
  }

  // Platform-specific initialization
  if (!PlatformInitialize()) {
    return false;
  }

  last_error_ = hf_spi_err_t::SPI_SUCCESS;
  initialized_ = true; // Set initialized flag
  return initialized_;
}

bool EspSpi::Deinitialize() noexcept {
  if (!initialized_) {
    return true;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  bool result = PlatformDeinitialize();
  if (result) {
    last_error_ = hf_spi_err_t::SPI_SUCCESS;
  }

  return result;
}

hf_spi_err_t EspSpi::Transfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                              uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

  if (length == 0) {
    return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
  }

  if (length > max_transfer_size_) {
    return hf_spi_err_t::SPI_ERR_TRANSFER_TOO_LONG;
  }

  return InternalTransfer(tx_data, rx_data, length, timeout_ms, 
                         hf_spi_transfer_mode_t::HF_SPI_TRANSFER_MODE_SINGLE, true);
}

hf_spi_err_t EspSpi::SetChipSelect(bool active) noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }
  RtosUniqueLock<RtosMutex> lock(mutex_);

  const hf_spi_bus_config_t &cfg = use_advanced_config_ ? advanced_config_.base_config : config_;

  if (cfg.cs_pin != HF_INVALID_PIN) {
    gpio_num_t cs_pin = static_cast<gpio_num_t>(cfg.cs_pin);
    gpio_set_level(cs_pin, cfg.cs_active_low ? !active : active);
    cs_active_ = active;
    last_error_ = hf_spi_err_t::SPI_SUCCESS;
    return last_error_;
  }

  last_error_ = hf_spi_err_t::SPI_ERR_CS_CONTROL_FAILED;
  return last_error_;
}

//==============================================//
// ENHANCED METHODS                             //
//==============================================//

bool EspSpi::IsBusy() noexcept {
  if (!initialized_) {
    return false;
  }

  // Simple implementation - could be enhanced with platform-specific busy checks
  return cs_active_;
}

hf_spi_err_t EspSpi::SetClockSpeed(uint32_t clock_speed_hz) noexcept {
  if (!IsValidClockSpeed(clock_speed_hz)) {
    last_error_ = hf_spi_err_t::SPI_ERR_INVALID_CLOCK_SPEED;
    return last_error_;
  }

  if (use_advanced_config_) {
    advanced_config_.base_config.clock_speed_hz = clock_speed_hz;
  } else {
    config_.clock_speed_hz = clock_speed_hz;
  }

  // Reinitialize if already initialized
  if (initialized_) {
    bool was_initialized = initialized_;
    if (Deinitialize() && Initialize()) {
      return hf_spi_err_t::SPI_SUCCESS;
    }
    initialized_ = was_initialized;
    last_error_ = hf_spi_err_t::SPI_ERR_INVALID_CONFIGURATION;
    return last_error_;
  }

  return hf_spi_err_t::SPI_SUCCESS;
}

hf_spi_err_t EspSpi::SetMode(uint8_t mode) noexcept {
  if (!IsValidMode(mode)) {
    last_error_ = hf_spi_err_t::SPI_ERR_INVALID_MODE;
    return last_error_;
  }

  if (use_advanced_config_) {
    advanced_config_.base_config.mode = mode;
  } else {
    config_.mode = mode;
  }

  // Reinitialize if already initialized
  if (initialized_) {
    bool was_initialized = initialized_;
    if (Deinitialize() && Initialize()) {
      return hf_spi_err_t::SPI_SUCCESS;
    }
    initialized_ = was_initialized;
    last_error_ = hf_spi_err_t::SPI_ERR_INVALID_CONFIGURATION;
    return last_error_;
  }

  return hf_spi_err_t::SPI_SUCCESS;
}
hf_spi_err_t EspSpi::setDmaEnabled(bool enable) noexcept {
  dma_enabled_ = enable;
  if (use_advanced_config_) {
    advanced_config_.dma_enabled = enable;
  }
  return hf_spi_err_t::SPI_SUCCESS; // DMA setting will be applied on next initialization
}

uint32_t EspSpi::GetBusStatus() noexcept {
  // Return platform-specific status information
  return static_cast<uint32_t>(last_error_) | (cs_active_ ? 0x80000000 : 0);
}

// TransferSequence removed - not in header

// TransferWithTiming removed - not in header

// WriteRegister removed - not in header

// ReadRegister removed - not in header

//==============================================//
// ASYNCHRONOUS OPERATIONS                      //
//==============================================//

hf_spi_err_t EspSpi::transferAsync(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                                   hf_spi_async_callback_t callback, void *userData) noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

  if (length == 0 || !callback) {
    return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
  }

  if (length > max_transfer_size_) {
    return hf_spi_err_t::SPI_ERR_TRANSFER_TOO_LONG;
  }

  // For simplicity, implement as synchronous operation with callback
  // In a full implementation, this would use ESP-IDF async SPI features
  hf_spi_err_t result = Transfer(tx_data, rx_data, length, DEFAULT_TIMEOUT_MS);

  // Call the callback with the result
  callback(result, (result == hf_spi_err_t::SPI_SUCCESS) ? length : 0, userData);

  return result;
}

hf_spi_err_t EspSpi::cancelAsyncOperation(uint32_t operation_id) noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

  // Remove from active operations list
  RtosUniqueLock<RtosMutex> lock(mutex_);
  auto it = std::find(async_operations_.begin(), async_operations_.end(), operation_id);
  if (it != async_operations_.end()) {
    async_operations_.erase(it);
    return hf_spi_err_t::SPI_SUCCESS;
  }

  return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
}

//==============================================//
// PRIVATE METHODS                              //
//==============================================//

hf_spi_err_t EspSpi::ConvertPlatformError(int32_t platform_error) noexcept {
  switch (platform_error) {
  case ESP_OK:
    return hf_spi_err_t::SPI_SUCCESS;
  case ESP_ERR_INVALID_ARG:
    return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
  case ESP_ERR_TIMEOUT:
    return hf_spi_err_t::SPI_ERR_TRANSFER_TIMEOUT;
  case ESP_ERR_NO_MEM:
    return hf_spi_err_t::SPI_ERR_OUT_OF_MEMORY;
  case ESP_ERR_INVALID_STATE:
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  default:
    return hf_spi_err_t::SPI_ERR_TRANSFER_FAILED;
  }
}

bool EspSpi::PlatformInitialize() noexcept {
  const hf_spi_bus_config_t &cfg = use_advanced_config_ ? advanced_config_.base_config : config_;

  // Configure SPI bus
  spi_bus_config_t bus_cfg = {};
  bus_cfg.mosi_io_num = cfg.mosi_pin != HF_INVALID_PIN ? cfg.mosi_pin : -1;
  bus_cfg.miso_io_num = cfg.miso_pin != HF_INVALID_PIN ? cfg.miso_pin : -1;
  bus_cfg.sclk_io_num = cfg.sclk_pin;
  bus_cfg.quadwp_io_num = -1;
  bus_cfg.quadhd_io_num = -1;
  bus_cfg.max_transfer_sz = max_transfer_size_;

  spi_host_device_t host = static_cast<spi_host_device_t>(cfg.host);
  esp_err_t err =
      spi_bus_initialize(host, &bus_cfg, dma_enabled_ ? SPI_DMA_CH_AUTO : SPI_DMA_DISABLED);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(err));
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  // Configure CS pin if specified
  if (cfg.cs_pin != HF_INVALID_PIN) {
    gpio_config_t cs_cfg = {};
    cs_cfg.pin_bit_mask = (1ULL << cfg.cs_pin);
    cs_cfg.mode = GPIO_MODE_OUTPUT;
    cs_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    cs_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cs_cfg.intr_type = GPIO_INTR_DISABLE;

    err = gpio_config(&cs_cfg);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "CS pin configuration failed: %s", esp_err_to_name(err));
      spi_bus_free(host);
      last_error_ = ConvertPlatformError(err);
      return false;
    }

    // Set CS to inactive state
    gpio_set_level(static_cast<gpio_num_t>(cfg.cs_pin), cfg.cs_active_low ? 1 : 0);
  }

  // Add device and keep handle
  spi_device_interface_config_t dev_cfg = {};
  dev_cfg.clock_speed_hz = cfg.clock_speed_hz;
  dev_cfg.mode = cfg.mode;
  dev_cfg.spics_io_num = -1; // Software CS
  dev_cfg.queue_size =
      use_advanced_config_ ? advanced_config_.transaction_queue_size : DEFAULT_QUEUE_SIZE;

  err = spi_bus_add_device(host, &dev_cfg, &platform_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "spi_bus_add_device failed: %s", esp_err_to_name(err));
    spi_bus_free(host);
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  ESP_LOGI(
      TAG,
      "SPI bus initialized on host %d, MOSI=%d, MISO=%d, SCLK=%d, CS=%d, mode=%d, clock=%lu Hz",
      cfg.host, cfg.mosi_pin, cfg.miso_pin, cfg.sclk_pin, cfg.cs_pin, cfg.mode, cfg.clock_speed_hz);

  return true;
}

bool EspSpi::PlatformDeinitialize() noexcept {
  const hf_spi_bus_config_t &cfg = use_advanced_config_ ? advanced_config_.base_config : config_;

  if (platform_handle_) {
    spi_bus_remove_device(platform_handle_);
    platform_handle_ = nullptr;
  }

  spi_host_device_t host = static_cast<spi_host_device_t>(cfg.host);
  esp_err_t err = spi_bus_free(host);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "spi_bus_free failed: %s", esp_err_to_name(err));
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  ESP_LOGI(TAG, "SPI bus deinitialized on host %d", cfg.host);
  return true;
}

// Old InternalTransfer signature removed

hf_spi_err_t EspSpi::InternalTransfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                                      uint32_t timeout_ms, hf_spi_transfer_mode_t transfer_mode,
                                      bool manage_cs) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!platform_handle_) {
    last_error_ = hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
    return last_error_;
  }

  // Prepare transaction
  spi_transaction_t trans = {};
  trans.length = length * 8; // Length in bits
  trans.tx_buffer = tx_data;
  trans.rx_buffer = rx_data;

  // Set transfer mode flags if supported
  switch (transfer_mode) {
  case hf_spi_transfer_mode_t::HF_SPI_TRANSFER_MODE_SINGLE:
    // No special flags needed
    break;
  case hf_spi_transfer_mode_t::HF_SPI_TRANSFER_MODE_DUAL:
    // Dual mode not directly supported in basic ESP32 SPI
    ESP_LOGW(TAG, "Dual SPI mode not fully supported, using single mode");
    break;
  case hf_spi_transfer_mode_t::HF_SPI_TRANSFER_MODE_QUAD:
    // Quad mode would require different setup
    ESP_LOGW(TAG, "Quad SPI mode not fully supported, using single mode");
    break;
  case hf_spi_transfer_mode_t::HF_SPI_TRANSFER_MODE_OCTAL:
#ifdef HF_MCU_VARIANT_C6
    ESP_LOGW(TAG, "Octal SPI mode not implemented in this driver version");
#endif
    break;
  }

  // Manage CS if requested
  if (manage_cs && advanced_config_.base_config.cs_pin != HF_INVALID_PIN) {
    SetChipSelect(true);
  }

  // Record start time for statistics
  uint64_t start_time = esp_timer_get_time();

  // Acquire bus and perform transfer
  uint32_t timeout = GetTimeoutMs(timeout_ms);
  esp_err_t err = spi_device_acquire_bus(platform_handle_, pdMS_TO_TICKS(timeout));
  if (err == ESP_OK) {
    err = spi_device_transmit(platform_handle_, &trans);
    spi_device_release_bus(platform_handle_);
  }

  // Calculate transfer time
  uint64_t transfer_time = esp_timer_get_time() - start_time;

  // Release CS if we managed it
  if (manage_cs && advanced_config_.base_config.cs_pin != HF_INVALID_PIN) {
    SetChipSelect(false);
  }

  // Update statistics
  bool success = (err == ESP_OK);
  UpdateStatistics(success, success ? length : 0, transfer_time, dma_enabled_);

  last_error_ = ConvertPlatformError(err);
  transaction_count_++;

  return last_error_;
}

//==============================================//
// ADVANCED INITIALIZATION                      //
//==============================================//

hf_spi_err_t EspSpi::initializeAdvanced(const hf_spi_advanced_config_t &config) noexcept {
  if (initialized_) {
    return hf_spi_err_t::SPI_ERR_ALREADY_INITIALIZED;
  }

  advanced_config_ = config;
  use_advanced_config_ = true;
  dma_enabled_ = config.dma_enabled;
  max_transfer_size_ = config.max_transfer_size > 0 ? config.max_transfer_size : 4092;
  current_transfer_mode_ = config.transfer_mode;

  return Initialize() ? hf_spi_err_t::SPI_SUCCESS : last_error_;
}

hf_spi_err_t EspSpi::reconfigure(const hf_spi_advanced_config_t &config) noexcept {
  if (!initialized_) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

  // Store old state
  bool was_initialized = initialized_;
  hf_spi_advanced_config_t old_config = advanced_config_;

  // Apply new configuration
  if (Deinitialize()) {
    advanced_config_ = config;
    use_advanced_config_ = true;
    dma_enabled_ = config.dma_enabled;
    max_transfer_size_ = config.max_transfer_size > 0 ? config.max_transfer_size : 4092;
    current_transfer_mode_ = config.transfer_mode;

    if (Initialize()) {
      return hf_spi_err_t::SPI_SUCCESS;
    }
  }

  // Restore on failure
  advanced_config_ = old_config;
  initialized_ = was_initialized;
  return last_error_;
}

hf_spi_advanced_config_t EspSpi::getCurrentConfiguration() const noexcept {
  return advanced_config_;
}

hf_spi_err_t EspSpi::resetBus() noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

  // Reset the SPI bus
  if (Deinitialize() && Initialize()) {
    return hf_spi_err_t::SPI_SUCCESS;
  }
  return last_error_;
}

//==============================================//
// MULTI-DEVICE MANAGEMENT                      //
//==============================================//

hf_spi_device_handle_t
EspSpi::addDevice(const hf_spi_device_interface_config_t &device_config) noexcept {
  if (!EnsureInitialized()) {
    last_error_ = hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
    return nullptr;
  }

  const hf_spi_bus_config_t &cfg = use_advanced_config_ ? advanced_config_.base_config : config_;
  spi_host_device_t host = static_cast<spi_host_device_t>(cfg.host);
  spi_device_handle_t device_handle;

  esp_err_t err = spi_bus_add_device(host, &device_config, &device_handle);
  if (err == ESP_OK) {
    device_handles_.push_back(device_handle);
    last_error_ = hf_spi_err_t::SPI_SUCCESS;
    return device_handle;
  } else {
    last_error_ = ConvertPlatformError(err);
    return nullptr;
  }
}

hf_spi_err_t EspSpi::removeDevice(spi_device_handle_t device_handle) noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

  if (!device_handle) {
    return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
  }

  esp_err_t err = spi_bus_remove_device(device_handle);
  if (err == ESP_OK) {
    // Remove from our list
    auto it = std::find(device_handles_.begin(), device_handles_.end(), device_handle);
    if (it != device_handles_.end()) {
      device_handles_.erase(it);
    }
    last_error_ = hf_spi_err_t::SPI_SUCCESS;
    return last_error_;
  } else {
    last_error_ = ConvertPlatformError(err);
    return last_error_;
  }
}

hf_spi_err_t EspSpi::selectDevice(spi_device_handle_t device_handle) noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

  if (!device_handle) {
    return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
  }

  // Check if device handle is in our list
  auto it = std::find(device_handles_.begin(), device_handles_.end(), device_handle);
  if (it == device_handles_.end()) {
    return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
  }

  current_device_ = device_handle;
  last_error_ = hf_spi_err_t::SPI_SUCCESS;
  return last_error_;
}

//==============================================//
// ADVANCED TRANSFER METHODS                    //
//==============================================//

hf_spi_err_t EspSpi::transferQuad(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                                  uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

  // Store current mode and switch to quad
  hf_spi_transfer_mode_t old_mode = current_transfer_mode_;
  current_transfer_mode_ = hf_spi_transfer_mode_t::HF_SPI_TRANSFER_MODE_QUAD;

  hf_spi_err_t result =
      InternalTransfer(tx_data, rx_data, length, timeout_ms, current_transfer_mode_, true);

  // Restore previous mode
  current_transfer_mode_ = old_mode;
  return result;
}

hf_spi_err_t EspSpi::transferOctal(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                                   uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_VARIANT_C6
  // ESP32C6 supports octal SPI
  hf_spi_transfer_mode_t old_mode = current_transfer_mode_;
  current_transfer_mode_ = hf_spi_transfer_mode_t::HF_SPI_TRANSFER_MODE_OCTAL;

  hf_spi_err_t result =
      InternalTransfer(tx_data, rx_data, length, timeout_ms, current_transfer_mode_, true);

  current_transfer_mode_ = old_mode;
  return result;
#else
  // Not supported on other platforms
  (void)tx_data;
  (void)rx_data;
  (void)length;
  (void)timeout_ms;
  return hf_spi_err_t::SPI_ERR_UNSUPPORTED_OPERATION;
#endif
}

// transferAdvanced and transferPolling removed - not in header

//==============================================//
// MISSING FUNCTION IMPLEMENTATIONS            //
//==============================================//

// Duplicate setDmaEnabled removed

// GetTransferMode moved to header as inline function

// Duplicate function definitions removed

//==============================================//
// STATISTICS AND DIAGNOSTICS                  //
//==============================================//

hf_spi_err_t EspSpi::GetStatistics(hf_spi_statistics_t &statistics) const noexcept {
  statistics = statistics_;
  return hf_spi_err_t::SPI_SUCCESS;
}

void EspSpi::resetStatistics() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  statistics_ = hf_spi_statistics_t{}; // Reset to default values
}

hf_spi_err_t EspSpi::GetDiagnostics(hf_spi_diagnostics_t &diagnostics) const noexcept {
  hf_spi_diagnostics_t diag = {};

  diag.is_initialized = initialized_;
  diag.is_bus_suspended = bus_suspended_;
  diag.dma_enabled = dma_enabled_;
  diag.current_clock_speed = use_advanced_config_ ? advanced_config_.base_config.clock_speed_hz : config_.clock_speed_hz;
  diag.current_mode = use_advanced_config_ ? advanced_config_.base_config.mode : config_.mode;
  diag.max_transfer_size = max_transfer_size_;
  diag.device_count = device_handles_.size();
  diag.last_error = static_cast<uint32_t>(last_error_);
  diag.total_transactions = statistics_.total_transactions;
  diag.failed_transactions = statistics_.failed_transactions;

  diagnostics = diag;
  return hf_spi_err_t::SPI_SUCCESS;
}

bool EspSpi::isBusHealthy() noexcept {
  if (!initialized_) {
    return false;
  }

  // Check various health indicators
  bool healthy = true;

  // Check error rate
  if (statistics_.total_transactions > 0) {
    double error_rate =
        static_cast<double>(statistics_.failed_transactions) / statistics_.total_transactions;
    if (error_rate > 0.1) { // More than 10% error rate is unhealthy
      healthy = false;
    }
  }

  // Check if bus is not suspended unexpectedly
  if (bus_suspended_) {
    healthy = false;
  }

  // Check for recent timeout errors
  if (statistics_.timeout_transactions > (statistics_.total_transactions / 4)) {
    healthy = false; // Too many timeouts
  }

  return healthy;
}

void EspSpi::UpdateStatistics(bool success, size_t bytesTransferred, uint64_t transferTimeUs,
                              bool usedDma) noexcept {
  statistics_.total_transactions++;

  if (success) {
    statistics_.successful_transactions++;
    statistics_.total_bytes_sent += bytesTransferred;
    // For SPI, bytes_received equals bytes_sent in full-duplex mode
    statistics_.total_bytes_received += bytesTransferred;

    // Update timing statistics
    if (transferTimeUs < statistics_.min_transaction_time_us) {
      statistics_.min_transaction_time_us = transferTimeUs;
    }
    if (transferTimeUs > statistics_.max_transaction_time_us) {
      statistics_.max_transaction_time_us = transferTimeUs;
    }

    // Note: average_transfer_time_us, dma_transfers, and polling_transfers 
    // are not in the base class statistics structure
  } else {
    statistics_.failed_transactions++;
  }

  last_transfer_time_ = esp_timer_get_time();
}

//==============================================//
// MISSING ADVANCED TRANSFER IMPLEMENTATIONS   //
//==============================================//

hf_spi_err_t EspSpi::transferDma(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                                 uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

  // Enable DMA for this transfer if not already enabled
  bool old_dma_state = dma_enabled_;
  if (!dma_enabled_) {
    ESP_LOGD(TAG, "Temporarily enabling DMA for DMA transfer");
    dma_enabled_ = true;
  }

  hf_spi_err_t result = InternalTransfer(tx_data, rx_data, length, timeout_ms, 
                                        hf_spi_transfer_mode_t::HF_SPI_TRANSFER_MODE_SINGLE, true);

  // Restore previous DMA state
  dma_enabled_ = old_dma_state;
  return result;
}

hf_spi_err_t EspSpi::transferBatch(const hf_spi_transfer_descriptor_t *transfers,
                                   uint8_t count) noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

  if (!transfers || count == 0) {
    return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
  }

  hf_spi_err_t result = hf_spi_err_t::SPI_SUCCESS;

  // Assert CS once for the entire batch
  if (transfers[0].manage_cs && advanced_config_.base_config.cs_pin != HF_INVALID_PIN) {
    SetChipSelect(true);
  }

  // Execute all transfers in sequence
  for (uint8_t i = 0; i < count; i++) {
    const hf_spi_transfer_descriptor_t &transfer = transfers[i];
    uint32_t timeout = transfer.timeout_ms > 0 ? transfer.timeout_ms : GetTimeoutMs(0);

    result = InternalTransfer(transfer.tx_data, transfer.rx_data, transfer.length, timeout,
                             hf_spi_transfer_mode_t::HF_SPI_TRANSFER_MODE_SINGLE, false); // Don't manage CS for individual transfers

    if (result != hf_spi_err_t::SPI_SUCCESS) {
      ESP_LOGE(TAG, "Batch transfer failed at index %d: %d", i, static_cast<int>(result));
      break;
    }
  }

  // Release CS after the entire batch
  if (transfers[0].manage_cs && advanced_config_.base_config.cs_pin != HF_INVALID_PIN) {
    SetChipSelect(false);
  }

  return result;
}

//==============================================//
// POWER MANAGEMENT IMPLEMENTATIONS             //
//==============================================//

hf_spi_err_t EspSpi::suspendBus() noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (bus_suspended_) {
    return hf_spi_err_t::SPI_SUCCESS; // Already suspended
  }

  // On ESP32, we can't truly suspend the SPI bus, but we can release it
  // and mark it as suspended for power management awareness
  bus_suspended_ = true;
  ESP_LOGD(TAG, "SPI bus marked as suspended for power management");

  if (event_callback_) {
    event_callback_(static_cast<int>(hf_spi_event_type_t::HF_SPI_EVENT_BUS_SUSPENDED), nullptr,
                    event_user_data_);
  }

  last_error_ = hf_spi_err_t::SPI_SUCCESS;
  return last_error_;
}

hf_spi_err_t EspSpi::resumeBus() noexcept {
  if (!EnsureInitialized()) {
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!bus_suspended_) {
    return hf_spi_err_t::SPI_SUCCESS; // Already active
  }

  bus_suspended_ = false;
  ESP_LOGD(TAG, "SPI bus resumed from suspended state");

  if (event_callback_) {
    event_callback_(static_cast<int>(hf_spi_event_type_t::HF_SPI_EVENT_BUS_RESUMED), nullptr,
                    event_user_data_);
  }

  last_error_ = hf_spi_err_t::SPI_SUCCESS;
  return last_error_;
}

hf_spi_err_t EspSpi::setClockSource(hf_spi_clock_source_t clock_source) noexcept {
  if (use_advanced_config_) {
    advanced_config_.clock_source = clock_source;
  }

  // Clock source changes would require bus reconfiguration
  if (initialized_) {
    ESP_LOGW(TAG, "Clock source changed - reinitialize to apply");
  }

  last_error_ = hf_spi_err_t::SPI_SUCCESS;
  return last_error_;
}

//==============================================//
// REGISTER OPERATIONS IMPLEMENTATIONS         //
//==============================================//

hf_spi_err_t EspSpi::writeRegister(uint8_t reg_addr, uint8_t value) noexcept {
  uint8_t tx_data[2] = {reg_addr, value};
  return Transfer(tx_data, nullptr, 2);
}

hf_spi_err_t EspSpi::readRegister(uint8_t reg_addr, uint8_t &value) noexcept {
  uint8_t tx_data = reg_addr | 0x80; // Read command (MSB set)
  uint8_t rx_data[2];

  hf_spi_err_t result = Transfer(&tx_data, rx_data, 2);
  if (result == hf_spi_err_t::SPI_SUCCESS) {
    value = rx_data[1]; // Second byte contains the value
  }
  return result;
}

hf_spi_err_t EspSpi::writeMultipleRegisters(uint8_t start_reg_addr, const uint8_t *data,
                                            uint8_t count) noexcept {
  if (!data || count == 0) {
    return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
  }

  // Create transfer buffer with register address followed by data
  std::vector<uint8_t> tx_buffer(count + 1);
  tx_buffer[0] = start_reg_addr;
  std::memcpy(&tx_buffer[1], data, count);

  return Transfer(tx_buffer.data(), nullptr, count + 1);
}

hf_spi_err_t EspSpi::readMultipleRegisters(uint8_t start_reg_addr, uint8_t *data,
                                           uint8_t count) noexcept {
  if (!data || count == 0) {
    return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
  }

  uint8_t tx_addr = start_reg_addr | 0x80; // Read command
  std::vector<uint8_t> rx_buffer(count + 1);

  hf_spi_err_t result = Transfer(&tx_addr, rx_buffer.data(), count + 1);
  if (result == hf_spi_err_t::SPI_SUCCESS) {
    std::memcpy(data, &rx_buffer[1], count); // Skip address byte
  }
  return result;
}

//==============================================//
// EVENT CALLBACK IMPLEMENTATION               //
//==============================================//

void EspSpi::setEventCallback(hf_spi_event_callback_t callback, void *userData) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  event_callback_ = callback;
  event_user_data_ = userData;
}

#endif // HF_MCU_FAMILY_ESP32