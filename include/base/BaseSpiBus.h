/**
 * @file BaseSpiBus.h
 * @brief Abstract base class for SPI bus implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for SPI bus communication
 * that provides a consistent API across different SPI controller implementations.
 * Concrete implementations (like McuSpiBus for ESP32 SPI) inherit from this class.
 *
 * @note This is a header-only abstract base class following the same pattern as BaseCan.
 * @note Users should program against this interface, not specific implementations.
 */

#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_BASESPIBUS_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_BASESPIBUS_H_

#include "mcu/McuTypes.h"
#include <cstdint>
#include <functional>
#include <string_view>

//--------------------------------------
//  HardFOC SPI Error Codes (Table)
//--------------------------------------
/**
 * @brief HardFOC SPI error codes
 * @details Comprehensive error enumeration for all SPI operations in the system.
 *          This enumeration is used across all SPI-related classes to provide
 *          consistent error reporting and handling.
 */

#define HF_SPI_ERR_LIST(X) \
    /* Success codes */ \
    X(SPI_SUCCESS, 0, "Success") \
    /* General errors */ \
    X(SPI_ERR_FAILURE, 1, "General failure") \
    X(SPI_ERR_NOT_INITIALIZED, 2, "Not initialized") \
    X(SPI_ERR_ALREADY_INITIALIZED, 3, "Already initialized") \
    X(SPI_ERR_INVALID_PARAMETER, 4, "Invalid parameter") \
    X(SPI_ERR_NULL_POINTER, 5, "Null pointer") \
    X(SPI_ERR_OUT_OF_MEMORY, 6, "Out of memory") \
    /* Bus errors */ \
    X(SPI_ERR_BUS_BUSY, 7, "Bus busy") \
    X(SPI_ERR_BUS_ERROR, 8, "Bus error") \
    X(SPI_ERR_BUS_NOT_AVAILABLE, 9, "Bus not available") \
    X(SPI_ERR_BUS_TIMEOUT, 10, "Bus timeout") \
    /* Transfer errors */ \
    X(SPI_ERR_TRANSFER_FAILED, 11, "Transfer failed") \
    X(SPI_ERR_TRANSFER_TIMEOUT, 12, "Transfer timeout") \
    X(SPI_ERR_TRANSFER_TOO_LONG, 13, "Transfer too long") \
    X(SPI_ERR_TRANSFER_SIZE_MISMATCH, 14, "Transfer size mismatch") \
    /* Device errors */ \
    X(SPI_ERR_DEVICE_NOT_FOUND, 15, "Device not found") \
    X(SPI_ERR_DEVICE_NOT_RESPONDING, 16, "Device not responding") \
    X(SPI_ERR_CS_CONTROL_FAILED, 17, "Chip select control failed") \
    /* Hardware errors */ \
    X(SPI_ERR_HARDWARE_FAULT, 18, "Hardware fault") \
    X(SPI_ERR_COMMUNICATION_FAILURE, 19, "Communication failure") \
    X(SPI_ERR_VOLTAGE_OUT_OF_RANGE, 20, "Voltage out of range") \
    X(SPI_ERR_CLOCK_ERROR, 21, "Clock error") \
    /* Configuration errors */ \
    X(SPI_ERR_INVALID_CONFIGURATION, 22, "Invalid configuration") \
    X(SPI_ERR_UNSUPPORTED_OPERATION, 23, "Unsupported operation") \
    X(SPI_ERR_INVALID_CLOCK_SPEED, 24, "Invalid clock speed") \
    X(SPI_ERR_INVALID_MODE, 25, "Invalid SPI mode") \
    X(SPI_ERR_PIN_CONFIGURATION_ERROR, 26, "Pin configuration error") \
    /* System errors */ \
    X(SPI_ERR_SYSTEM_ERROR, 27, "System error") \
    X(SPI_ERR_PERMISSION_DENIED, 28, "Permission denied") \
    X(SPI_ERR_OPERATION_ABORTED, 29, "Operation aborted")

enum class HfSpiErr : uint8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
    HF_SPI_ERR_LIST(X)
#undef X
};

/**
 * @brief Convert HfSpiErr to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfSpiErrToString(HfSpiErr err) noexcept {
    switch (err) {
#define X(NAME, VALUE, DESC) case HfSpiErr::NAME: return DESC;
        HF_SPI_ERR_LIST(X)
#undef X
        default: return "Unknown error";
    }
}

//--------------------------------------
//  SPI Configuration Structure
//--------------------------------------

/**
 * @brief SPI bus configuration structure.
 * @details Comprehensive configuration for SPI bus initialization,
 *          supporting various MCU platforms and SPI modes.
 */
struct SpiBusConfig {
    hf_spi_host_t host;                 ///< SPI host/controller
    hf_gpio_num_t mosi_pin;             ///< MOSI (Master Out Slave In) pin
    hf_gpio_num_t miso_pin;             ///< MISO (Master In Slave Out) pin
    hf_gpio_num_t sclk_pin;             ///< SCLK (Serial Clock) pin
    hf_gpio_num_t cs_pin;               ///< CS (Chip Select) pin
    uint32_t clock_speed_hz;            ///< Clock speed in Hz
    uint8_t mode;                       ///< SPI mode (0-3: CPOL/CPHA combinations)
    uint8_t bits_per_word;              ///< Bits per transfer (typically 8 or 16)
    bool cs_active_low;                 ///< True if CS is active low, false if active high
    uint16_t timeout_ms;                ///< Default timeout for operations in milliseconds
    
    /**
     * @brief Default constructor with sensible defaults.
     */
    SpiBusConfig() noexcept :
        host(0),
        mosi_pin(HF_GPIO_INVALID),
        miso_pin(HF_GPIO_INVALID),
        sclk_pin(HF_GPIO_INVALID),
        cs_pin(HF_GPIO_INVALID),
        clock_speed_hz(1000000),    // 1MHz default
        mode(0),                    // Mode 0 (CPOL=0, CPHA=0)
        bits_per_word(8),           // 8-bit transfers
        cs_active_low(true),        // Most devices use active-low CS
        timeout_ms(1000)
    {}
};

//--------------------------------------
//  Abstract Base Class
//--------------------------------------

/**
 * @class BaseSpiBus
 * @brief Abstract base class for SPI bus implementations.
 * @details This class provides a comprehensive SPI bus abstraction that serves as the base
 *          for all SPI implementations in the HardFOC system. It supports:
 *          - Master mode SPI communication
 *          - Configurable SPI modes (0-3)
 *          - Full-duplex, write-only, and read-only transfers
 *          - Configurable clock speeds and timing
 *          - Chip select control
 *          - Configurable word sizes
 *          - Comprehensive error handling
 *          - Lazy initialization pattern
 *          
 *          Derived classes implement platform-specific details for:
 *          - MCU SPI controllers (ESP32, STM32, etc.)
 *          - Bit-banged SPI implementations
 *          - SPI bridge/adapter hardware
 * 
 * @note This is a header-only abstract base class - instantiate concrete implementations instead.
 * @note This class is not inherently thread-safe. Use appropriate synchronization if
 *       accessed from multiple contexts.
 */
class BaseSpiBus {
public:
    /**
     * @brief Constructor with configuration.
     * @param config SPI bus configuration parameters
     */
    explicit BaseSpiBus(const SpiBusConfig& config) noexcept :
        config_(config), initialized_(false) {}

    /**
     * @brief Virtual destructor ensures proper cleanup in derived classes.
     */
    virtual ~BaseSpiBus() noexcept = default;
    
    // Non-copyable, non-movable (can be changed in derived classes if needed)
    BaseSpiBus(const BaseSpiBus&) = delete;
    BaseSpiBus& operator=(const BaseSpiBus&) = delete;
    BaseSpiBus(BaseSpiBus&&) = delete;
    BaseSpiBus& operator=(BaseSpiBus&&) = delete;

    /**
     * @brief Ensures that the SPI bus is initialized (lazy initialization).
     * @return true if the SPI bus is initialized, false otherwise.
     */
    bool EnsureInitialized() noexcept {
        if (!initialized_) {
            initialized_ = Initialize();
        }
        return initialized_;
    }

    /**
     * @brief Checks if the bus is initialized.
     * @return true if initialized, false otherwise
     */
    [[nodiscard]] bool IsInitialized() const noexcept {
        return initialized_;
    }
    
    /**
     * @brief Get the bus configuration.
     * @return Reference to the current configuration
     */
    [[nodiscard]] const SpiBusConfig& GetConfig() const noexcept {
        return config_;
    }
    
    //==============================================//
    // PURE VIRTUAL FUNCTIONS - MUST BE OVERRIDDEN  //
    //==============================================//
    
    /**
     * @brief Initialize the SPI bus.
     * @return true if successful, false otherwise
     * @note Must be implemented by concrete classes.
     */
    virtual bool Initialize() noexcept = 0;
    
    /**
     * @brief Deinitialize the SPI bus.
     * @return true if successful, false otherwise
     * @note Must be implemented by concrete classes.
     */
    virtual bool Deinitialize() noexcept = 0;
    
    /**
     * @brief Perform a full-duplex SPI transfer.
     * @param tx_data Transmit data buffer (can be nullptr for read-only)
     * @param rx_data Receive data buffer (can be nullptr for write-only)
     * @param length Number of bytes to transfer
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfSpiErr result code
     * @note Must be implemented by concrete classes.
     */
    virtual HfSpiErr Transfer(const uint8_t* tx_data, uint8_t* rx_data, 
                             uint16_t length, uint32_t timeout_ms = 0) noexcept = 0;

    /**
     * @brief Assert/deassert the chip select signal.
     * @param active True to assert CS, false to deassert
     * @return HfSpiErr result code
     * @note Must be implemented by concrete classes.
     */
    virtual HfSpiErr SetChipSelect(bool active) noexcept = 0;
    
    //==============================================//
    // CONVENIENCE METHODS WITH DEFAULT IMPLEMENTATIONS //
    //==============================================//
    
    /**
     * @brief Legacy compatibility: Open and initialize the SPI bus.
     * @return true if open, false otherwise.
     */
    virtual bool Open() noexcept {
        return EnsureInitialized();
    }

    /**
     * @brief Legacy compatibility: Close and de-initialize the SPI bus.
     * @return true if closed, false otherwise.
     */
    virtual bool Close() noexcept {
        if (initialized_) {
            initialized_ = !Deinitialize();
            return !initialized_;
        }
        return true;
    }

    /**
     * @brief Legacy compatibility: Transfer with boolean return.
     * @param tx_data Transmit data buffer
     * @param rx_data Receive data buffer
     * @param length Number of bytes to transfer
     * @return true if transfer succeeded
     */
    virtual bool Transfer(const uint8_t* tx_data, uint8_t* rx_data, 
                         uint16_t length) noexcept {
        if (!EnsureInitialized()) {
            return false;
        }
        return Transfer(tx_data, rx_data, length, 0) == HfSpiErr::SPI_SUCCESS;
    }

    /**
     * @brief Write data to SPI bus.
     * @param data Data buffer to transmit
     * @param length Number of bytes to write
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfSpiErr result code
     */
    virtual HfSpiErr Write(const uint8_t* data, uint16_t length, 
                          uint32_t timeout_ms = 0) noexcept {
        return Transfer(data, nullptr, length, timeout_ms);
    }

    /**
     * @brief Read data from SPI bus.
     * @param data Buffer to store received data
     * @param length Number of bytes to read
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfSpiErr result code
     */
    virtual HfSpiErr Read(uint8_t* data, uint16_t length, 
                         uint32_t timeout_ms = 0) noexcept {
        return Transfer(nullptr, data, length, timeout_ms);
    }

    /**
     * @brief Legacy compatibility: Write with boolean return.
     * @param data Data buffer to transmit
     * @param length Number of bytes to write
     * @return true if write succeeded
     */
    virtual bool Write(const uint8_t* data, uint16_t length) noexcept {
        if (!EnsureInitialized()) {
            return false;
        }
        return Write(data, length, 0) == HfSpiErr::SPI_SUCCESS;
    }

    /**
     * @brief Legacy compatibility: Read with boolean return.
     * @param data Buffer to store received data
     * @param length Number of bytes to read
     * @return true if read succeeded
     */
    virtual bool Read(uint8_t* data, uint16_t length) noexcept {
        if (!EnsureInitialized()) {
            return false;
        }
        return Read(data, length, 0) == HfSpiErr::SPI_SUCCESS;
    }

    /**
     * @brief Get the configured clock speed.
     * @return Clock speed in Hz
     */
    [[nodiscard]] virtual uint32_t GetClockHz() const noexcept {
        return config_.clock_speed_hz;
    }

    /**
     * @brief Get the configured SPI mode.
     * @return SPI mode (0-3)
     */
    [[nodiscard]] virtual uint8_t GetMode() const noexcept {
        return config_.mode;
    }

    /**
     * @brief Get the configured bits per word.
     * @return Bits per word
     */
    [[nodiscard]] virtual uint8_t GetBitsPerWord() const noexcept {
        return config_.bits_per_word;
    }

    /**
     * @brief Get the SPI host/controller.
     * @return SPI host number
     */
    [[nodiscard]] virtual hf_spi_host_t GetHost() const noexcept { 
        return config_.host; 
    }

    /**
     * @brief Write single byte to SPI bus.
     * @param data Byte to write
     * @return true if successful, false otherwise
     */
    virtual bool WriteByte(uint8_t data) noexcept {
        return Write(&data, 1) == HfSpiErr::SPI_SUCCESS;
    }

    /**
     * @brief Read single byte from SPI bus.
     * @param data Output: byte read
     * @return true if successful, false otherwise
     */
    virtual bool ReadByte(uint8_t& data) noexcept {
        return Read(&data, 1) == HfSpiErr::SPI_SUCCESS;
    }

    /**
     * @brief Write single byte and read response.
     * @param tx_data Byte to write
     * @param rx_data Output: byte read
     * @return true if successful, false otherwise
     */
    virtual bool TransferByte(uint8_t tx_data, uint8_t& rx_data) noexcept {
        return Transfer(&tx_data, &rx_data, 1) == HfSpiErr::SPI_SUCCESS;
    }

protected:
    SpiBusConfig config_;                ///< Bus configuration
    bool initialized_;                   ///< Initialization state
};

#endif // HAL_INTERNAL_INTERFACE_DRIVERS_BASESPIBUS_H_
