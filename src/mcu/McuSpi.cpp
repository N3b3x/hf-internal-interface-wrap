/**
 * @file McuSpi.cpp
 * @brief Implementation of MCU-integrated SPI controller.
 */

#include "McuSpi.h"

// Platform-specific includes
#ifdef HF_MCU_FAMILY_ESP32
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

static const char *TAG = "McuSpi";
#endif

//==============================================//
// CONSTRUCTOR & DESTRUCTOR                     //
//==============================================//

McuSpi::McuSpi(const SpiBusConfig &config) noexcept
    : BaseSpiBus(config), platform_handle_(nullptr), last_error_(HfSpiErr::SPI_SUCCESS),
      transaction_count_(0), cs_active_(false), dma_enabled_(false),
      max_transfer_size_(4092) { // ESP32 default max transfer size
}

McuSpi::~McuSpi() noexcept {
  if (initialized_) {
    Deinitialize();
  }
}

//==============================================//
// OVERRIDDEN PURE VIRTUAL FUNCTIONS            //
//==============================================//

bool McuSpi::Initialize() noexcept {
  if (initialized_) {
    return true;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  // Validate configuration
  if (config_.mosi_pin == HF_GPIO_INVALID && config_.miso_pin == HF_GPIO_INVALID) {
    last_error_ = HfSpiErr::SPI_ERR_INVALID_CONFIGURATION;
    return false;
  }

  if (config_.sclk_pin == HF_GPIO_INVALID) {
    last_error_ = HfSpiErr::SPI_ERR_INVALID_CONFIGURATION;
    return false;
  }

  if (!IsValidClockSpeed(config_.clock_speed_hz)) {
    last_error_ = HfSpiErr::SPI_ERR_INVALID_CLOCK_SPEED;
    return false;
  }

  if (!IsValidMode(config_.mode)) {
    last_error_ = HfSpiErr::SPI_ERR_INVALID_MODE;
    return false;
  }

  // Platform-specific initialization
  if (!PlatformInitialize()) {
    return false;
  }

  last_error_ = HfSpiErr::SPI_SUCCESS;
  return true;
}

bool McuSpi::Deinitialize() noexcept {
  if (!initialized_) {
    return true;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  bool result = PlatformDeinitialize();
  if (result) {
    last_error_ = HfSpiErr::SPI_SUCCESS;
  }

  return result;
}

HfSpiErr McuSpi::Transfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                          uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return HfSpiErr::SPI_ERR_NOT_INITIALIZED;
  }

  if (length == 0) {
    return HfSpiErr::SPI_ERR_INVALID_PARAMETER;
  }

  if (length > max_transfer_size_) {
    return HfSpiErr::SPI_ERR_TRANSFER_TOO_LONG;
  }

  return InternalTransfer(tx_data, rx_data, length, timeout_ms, true);
}

HfSpiErr McuSpi::SetChipSelect(bool active) noexcept {
  if (!EnsureInitialized()) {
    return HfSpiErr::SPI_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

#ifdef HF_MCU_FAMILY_ESP32
  if (config_.cs_pin != HF_GPIO_INVALID) {
    gpio_num_t cs_pin = static_cast<gpio_num_t>(config_.cs_pin);
    gpio_set_level(cs_pin, config_.cs_active_low ? !active : active);
    cs_active_ = active;
    last_error_ = HfSpiErr::SPI_SUCCESS;
    return last_error_;
  }
#endif

  last_error_ = HfSpiErr::SPI_ERR_CS_CONTROL_FAILED;
  return last_error_;
}

//==============================================//
// ENHANCED METHODS                             //
//==============================================//

bool McuSpi::IsBusy() noexcept {
  if (!initialized_) {
    return false;
  }

  // Simple implementation - could be enhanced with platform-specific busy checks
  return cs_active_;
}

bool McuSpi::SetClockSpeed(uint32_t clock_speed_hz) noexcept {
  if (!IsValidClockSpeed(clock_speed_hz)) {
    return false;
  }

  config_.clock_speed_hz = clock_speed_hz;

  // Reinitialize if already initialized
  if (initialized_) {
    bool was_initialized = initialized_;
    initialized_ = false;
    if (Deinitialize() && Initialize()) {
      return true;
    }
    initialized_ = was_initialized;
    return false;
  }

  return true;
}

bool McuSpi::SetMode(uint8_t mode) noexcept {
  if (!IsValidMode(mode)) {
    return false;
  }

  config_.mode = mode;

  // Reinitialize if already initialized
  if (initialized_) {
    bool was_initialized = initialized_;
    initialized_ = false;
    if (Deinitialize() && Initialize()) {
      return true;
    }
    initialized_ = was_initialized;
    return false;
  }

  return true;
}

bool McuSpi::SetDmaEnabled(bool enable) noexcept {
  dma_enabled_ = enable;
  return true; // DMA setting will be applied on next initialization
}

uint32_t McuSpi::GetBusStatus() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  // Return platform-specific status information
  return static_cast<uint32_t>(last_error_) | (cs_active_ ? 0x80000000 : 0);
#else
  return 0;
#endif
}

HfSpiErr McuSpi::TransferSequence(const SpiTransfer *transfers, uint8_t num_transfers) noexcept {
  if (!EnsureInitialized()) {
    return HfSpiErr::SPI_ERR_NOT_INITIALIZED;
  }

  if (!transfers || num_transfers == 0) {
    return HfSpiErr::SPI_ERR_INVALID_PARAMETER;
  }

  // Assert CS before sequence
  HfSpiErr result = SetChipSelect(true);
  if (result != HfSpiErr::SPI_SUCCESS) {
    return result;
  }

  // Perform transfers
  for (uint8_t i = 0; i < num_transfers; i++) {
    result = InternalTransfer(transfers[i].tx_data, transfers[i].rx_data, transfers[i].length,
                              transfers[i].timeout_ms, false);
    if (result != HfSpiErr::SPI_SUCCESS) {
      break;
    }
  }

  // Deassert CS after sequence
  SetChipSelect(false);

  return result;
}

HfSpiErr McuSpi::TransferWithTiming(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                                    uint32_t cs_hold_time_us, uint32_t timeout_ms) noexcept {
  HfSpiErr result = Transfer(tx_data, rx_data, length, timeout_ms);

  if (result == HfSpiErr::SPI_SUCCESS && cs_hold_time_us > 0) {
    // Platform-specific delay implementation
#ifdef HF_MCU_FAMILY_ESP32
    esp_rom_delay_us(cs_hold_time_us);
#endif
  }

  return result;
}

HfSpiErr McuSpi::WriteRegister(uint8_t reg_addr, const uint8_t *data, uint16_t length) noexcept {
  if (!data || length == 0) {
    return HfSpiErr::SPI_ERR_INVALID_PARAMETER;
  }

  // Create buffer with register address + data
  uint16_t total_length = length + 1;
  if (total_length > max_transfer_size_) {
    return HfSpiErr::SPI_ERR_TRANSFER_TOO_LONG;
  }

  uint8_t *tx_buffer = new uint8_t[total_length];
  if (!tx_buffer) {
    return HfSpiErr::SPI_ERR_OUT_OF_MEMORY;
  }

  tx_buffer[0] = reg_addr;
  std::memcpy(&tx_buffer[1], data, length);

  HfSpiErr result = Transfer(tx_buffer, nullptr, total_length);

  delete[] tx_buffer;
  return result;
}

HfSpiErr McuSpi::ReadRegister(uint8_t reg_addr, uint8_t *data, uint16_t length) noexcept {
  if (!data || length == 0) {
    return HfSpiErr::SPI_ERR_INVALID_PARAMETER;
  }

  uint16_t total_length = length + 1;
  if (total_length > max_transfer_size_) {
    return HfSpiErr::SPI_ERR_TRANSFER_TOO_LONG;
  }

  uint8_t *tx_buffer = new uint8_t[total_length];
  uint8_t *rx_buffer = new uint8_t[total_length];

  if (!tx_buffer || !rx_buffer) {
    delete[] tx_buffer;
    delete[] rx_buffer;
    return HfSpiErr::SPI_ERR_OUT_OF_MEMORY;
  }

  tx_buffer[0] = reg_addr;
  std::memset(&tx_buffer[1], 0, length);

  HfSpiErr result = Transfer(tx_buffer, rx_buffer, total_length);

  if (result == HfSpiErr::SPI_SUCCESS) {
    std::memcpy(data, &rx_buffer[1], length);
  }

  delete[] tx_buffer;
  delete[] rx_buffer;
  return result;
}

//==============================================//
// PRIVATE METHODS                              //
//==============================================//

HfSpiErr McuSpi::ConvertPlatformError(int32_t platform_error) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  switch (platform_error) {
  case ESP_OK:
    return HfSpiErr::SPI_SUCCESS;
  case ESP_ERR_INVALID_ARG:
    return HfSpiErr::SPI_ERR_INVALID_PARAMETER;
  case ESP_ERR_TIMEOUT:
    return HfSpiErr::SPI_ERR_TRANSFER_TIMEOUT;
  case ESP_ERR_NO_MEM:
    return HfSpiErr::SPI_ERR_OUT_OF_MEMORY;
  case ESP_ERR_INVALID_STATE:
    return HfSpiErr::SPI_ERR_NOT_INITIALIZED;
  default:
    return HfSpiErr::SPI_ERR_TRANSFER_FAILED;
  }
#else
  (void)platform_error;
  return HfSpiErr::SPI_ERR_UNSUPPORTED_OPERATION;
#endif
}

bool McuSpi::PlatformInitialize() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  // Configure SPI bus
  spi_bus_config_t bus_cfg = {};
  bus_cfg.mosi_io_num = config_.mosi_pin != HF_GPIO_INVALID ? config_.mosi_pin : -1;
  bus_cfg.miso_io_num = config_.miso_pin != HF_GPIO_INVALID ? config_.miso_pin : -1;
  bus_cfg.sclk_io_num = config_.sclk_pin;
  bus_cfg.quadwp_io_num = -1;
  bus_cfg.quadhd_io_num = -1;
  bus_cfg.max_transfer_sz = max_transfer_size_;

  esp_err_t err = spi_bus_initialize(static_cast<spi_host_device_t>(config_.host), &bus_cfg,
                                     dma_enabled_ ? SPI_DMA_CH_AUTO : SPI_DMA_DISABLED);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(err));
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  // Configure CS pin if specified
  if (config_.cs_pin != HF_GPIO_INVALID) {
    gpio_config_t cs_cfg = {};
    cs_cfg.pin_bit_mask = (1ULL << config_.cs_pin);
    cs_cfg.mode = GPIO_MODE_OUTPUT;
    cs_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    cs_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cs_cfg.intr_type = GPIO_INTR_DISABLE;

    err = gpio_config(&cs_cfg);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "CS pin configuration failed: %s", esp_err_to_name(err));
      spi_bus_free(static_cast<spi_host_device_t>(config_.host));
      last_error_ = ConvertPlatformError(err);
      return false;
    }

    // Set CS to inactive state
    gpio_set_level(static_cast<gpio_num_t>(config_.cs_pin), config_.cs_active_low ? 1 : 0);
  }

  ESP_LOGI(
      TAG,
      "SPI bus initialized on host %d, MOSI=%d, MISO=%d, SCLK=%d, CS=%d, mode=%d, clock=%lu Hz",
      config_.host, config_.mosi_pin, config_.miso_pin, config_.sclk_pin, config_.cs_pin,
      config_.mode, config_.clock_speed_hz);

  return true;
#else
  last_error_ = HfSpiErr::SPI_ERR_UNSUPPORTED_OPERATION;
  return false;
#endif
}

bool McuSpi::PlatformDeinitialize() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err = spi_bus_free(static_cast<spi_host_device_t>(config_.host));
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "spi_bus_free failed: %s", esp_err_to_name(err));
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  ESP_LOGI(TAG, "SPI bus deinitialized on host %d", config_.host);
  return true;
#else
  return false;
#endif
}

HfSpiErr McuSpi::InternalTransfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                                  uint32_t timeout_ms, bool manage_cs) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

#ifdef HF_MCU_FAMILY_ESP32
  // Configure device for this transfer
  spi_device_interface_config_t dev_cfg = {};
  dev_cfg.clock_speed_hz = config_.clock_speed_hz;
  dev_cfg.mode = config_.mode;
  dev_cfg.spics_io_num = -1; // We manage CS manually
  dev_cfg.queue_size = 1;

  spi_device_handle_t dev_handle;
  esp_err_t err =
      spi_bus_add_device(static_cast<spi_host_device_t>(config_.host), &dev_cfg, &dev_handle);
  if (err != ESP_OK) {
    last_error_ = ConvertPlatformError(err);
    return last_error_;
  }

  // Prepare transaction
  spi_transaction_t trans = {};
  trans.length = length * 8; // Length in bits
  trans.tx_buffer = tx_data;
  trans.rx_buffer = rx_data;

  // Manage CS if requested
  if (manage_cs && config_.cs_pin != HF_GPIO_INVALID) {
    SetChipSelect(true);
  }

  // Perform transfer
  uint32_t timeout = GetTimeoutMs(timeout_ms);
  err = spi_device_transmit(dev_handle, &trans);

  // Release CS if we managed it
  if (manage_cs && config_.cs_pin != HF_GPIO_INVALID) {
    SetChipSelect(false);
  }

  // Clean up
  spi_bus_remove_device(dev_handle);

  last_error_ = ConvertPlatformError(err);
  transaction_count_++;

  return last_error_;
#else
  (void)tx_data;
  (void)rx_data;
  (void)length;
  (void)timeout_ms;
  (void)manage_cs;
  last_error_ = HfSpiErr::SPI_ERR_UNSUPPORTED_OPERATION;
  return last_error_;
#endif
}
