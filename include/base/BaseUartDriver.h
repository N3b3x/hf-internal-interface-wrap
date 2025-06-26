/**
 * @file BaseUartDriver.h
 * @brief Abstract base class for UART driver implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for UART communication
 * that provides a consistent API across different UART controller implementations.
 * Concrete implementations (like McuUartDriver for ESP32 UART) inherit from this class.
 *
 * @note This is a header-only abstract base class following the same pattern as BaseCan.
 * @note Users should program against this interface, not specific implementations.
 */

#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_BASEUARTDRIVER_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_BASEUARTDRIVER_H_

#include "mcu/McuTypes.h"
#include <cstdint>
#include <functional>
#include <string_view>

//--------------------------------------
//  HardFOC UART Error Codes (Table)
//--------------------------------------
/**
 * @brief HardFOC UART error codes
 * @details Comprehensive error enumeration for all UART operations in the system.
 *          This enumeration is used across all UART-related classes to provide
 *          consistent error reporting and handling.
 */

#define HF_UART_ERR_LIST(X) \
    /* Success codes */ \
    X(UART_SUCCESS, 0, "Success") \
    /* General errors */ \
    X(UART_ERR_FAILURE, 1, "General failure") \
    X(UART_ERR_NOT_INITIALIZED, 2, "Not initialized") \
    X(UART_ERR_ALREADY_INITIALIZED, 3, "Already initialized") \
    X(UART_ERR_INVALID_PARAMETER, 4, "Invalid parameter") \
    X(UART_ERR_NULL_POINTER, 5, "Null pointer") \
    X(UART_ERR_OUT_OF_MEMORY, 6, "Out of memory") \
    /* Communication errors */ \
    X(UART_ERR_TIMEOUT, 7, "Operation timeout") \
    X(UART_ERR_BUFFER_FULL, 8, "Buffer full") \
    X(UART_ERR_BUFFER_EMPTY, 9, "Buffer empty") \
    X(UART_ERR_TRANSMISSION_FAILED, 10, "Transmission failed") \
    X(UART_ERR_RECEPTION_FAILED, 11, "Reception failed") \
    /* Frame errors */ \
    X(UART_ERR_FRAME_ERROR, 12, "Frame error") \
    X(UART_ERR_PARITY_ERROR, 13, "Parity error") \
    X(UART_ERR_OVERRUN_ERROR, 14, "Overrun error") \
    X(UART_ERR_NOISE_ERROR, 15, "Noise error") \
    X(UART_ERR_BREAK_DETECTED, 16, "Break condition detected") \
    /* Hardware errors */ \
    X(UART_ERR_HARDWARE_FAULT, 17, "Hardware fault") \
    X(UART_ERR_COMMUNICATION_FAILURE, 18, "Communication failure") \
    X(UART_ERR_DEVICE_NOT_RESPONDING, 19, "Device not responding") \
    X(UART_ERR_VOLTAGE_OUT_OF_RANGE, 20, "Voltage out of range") \
    /* Configuration errors */ \
    X(UART_ERR_INVALID_CONFIGURATION, 21, "Invalid configuration") \
    X(UART_ERR_UNSUPPORTED_OPERATION, 22, "Unsupported operation") \
    X(UART_ERR_INVALID_BAUD_RATE, 23, "Invalid baud rate") \
    X(UART_ERR_INVALID_DATA_BITS, 24, "Invalid data bits") \
    X(UART_ERR_INVALID_PARITY, 25, "Invalid parity") \
    X(UART_ERR_INVALID_STOP_BITS, 26, "Invalid stop bits") \
    X(UART_ERR_PIN_CONFIGURATION_ERROR, 27, "Pin configuration error") \
    X(UART_ERR_FLOW_CONTROL_ERROR, 28, "Flow control error") \
    /* System errors */ \
    X(UART_ERR_SYSTEM_ERROR, 29, "System error") \
    X(UART_ERR_PERMISSION_DENIED, 30, "Permission denied") \
    X(UART_ERR_OPERATION_ABORTED, 31, "Operation aborted")

enum class HfUartErr : uint8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
    HF_UART_ERR_LIST(X)
#undef X
};

/**
 * @brief Convert HfUartErr to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfUartErrToString(HfUartErr err) noexcept {
    switch (err) {
#define X(NAME, VALUE, DESC) case HfUartErr::NAME: return DESC;
        HF_UART_ERR_LIST(X)
#undef X
        default: return "Unknown error";
    }
}

//--------------------------------------
//  UART Configuration Structure
//--------------------------------------

/**
 * @brief UART configuration structure.
 * @details Comprehensive configuration for UART initialization,
 *          supporting various MCU platforms and UART modes.
 */
struct UartConfig {
    uint32_t baud_rate;                      ///< Baud rate (bits per second)
    uint8_t data_bits;                       ///< Data bits (5-8, typically 8)
    uint8_t parity;                          ///< Parity: 0=None, 1=Even, 2=Odd
    uint8_t stop_bits;                       ///< Stop bits (1-2, typically 1)
    bool use_hardware_flow_control;          ///< Enable hardware flow control (RTS/CTS)
    hf_gpio_num_t tx_pin;                    ///< TX (transmit) pin
    hf_gpio_num_t rx_pin;                    ///< RX (receive) pin
    hf_gpio_num_t rts_pin;                   ///< RTS (Request To Send) pin (optional)
    hf_gpio_num_t cts_pin;                   ///< CTS (Clear To Send) pin (optional)
    uint16_t tx_buffer_size;                 ///< TX buffer size in bytes
    uint16_t rx_buffer_size;                 ///< RX buffer size in bytes
    uint32_t timeout_ms;                     ///< Default timeout for operations in milliseconds
    
    /**
     * @brief Default constructor with sensible defaults.
     */
    UartConfig() noexcept :
        baud_rate(115200),                  // Common baud rate
        data_bits(8),                       // 8 data bits
        parity(0),                          // No parity
        stop_bits(1),                       // 1 stop bit
        use_hardware_flow_control(false),   // No flow control by default
        tx_pin(HF_GPIO_INVALID),
        rx_pin(HF_GPIO_INVALID),
        rts_pin(HF_GPIO_INVALID),
        cts_pin(HF_GPIO_INVALID),
        tx_buffer_size(256),                // 256 bytes TX buffer
        rx_buffer_size(256),                // 256 bytes RX buffer
        timeout_ms(1000)                    // 1 second timeout
    {}
};

//--------------------------------------
//  Abstract Base Class
//--------------------------------------

/**
 * @class BaseUartDriver
 * @brief Abstract base class for UART driver implementations.
 * @details This class provides a comprehensive UART driver abstraction that serves as the base
 *          for all UART implementations in the HardFOC system. It supports:
 *          - Asynchronous serial communication
 *          - Configurable baud rates, data bits, parity, and stop bits
 *          - Hardware flow control (RTS/CTS)
 *          - Buffered TX/RX with configurable buffer sizes
 *          - Blocking and non-blocking I/O operations
 *          - Comprehensive error handling and status reporting
 *          - Printf-style formatted output
 *          - Lazy initialization pattern
 *          
 *          Derived classes implement platform-specific details for:
 *          - MCU UART controllers (ESP32, STM32, etc.)
 *          - USB-to-serial adapters
 *          - Bluetooth/WiFi serial bridges
 *          - Bit-banged software UART implementations
 * 
 * @note This is a header-only abstract base class - instantiate concrete implementations instead.
 * @note This class is not inherently thread-safe. Use appropriate synchronization if
 *       accessed from multiple contexts.
 */
class BaseUartDriver {
public:
    /**
     * @brief Constructor with port and configuration.
     * @param port UART port number
     * @param config UART configuration parameters
     */
    BaseUartDriver(hf_uart_port_t port, const UartConfig& config) noexcept :
        port_(port), config_(config), initialized_(false) {}

    /**
     * @brief Virtual destructor ensures proper cleanup in derived classes.
     */
    virtual ~BaseUartDriver() noexcept = default;
    
    // Non-copyable, non-movable (can be changed in derived classes if needed)
    BaseUartDriver(const BaseUartDriver&) = delete;
    BaseUartDriver& operator=(const BaseUartDriver&) = delete;
    BaseUartDriver(BaseUartDriver&&) = delete;
    BaseUartDriver& operator=(BaseUartDriver&&) = delete;

    /**
     * @brief Ensures that the UART is initialized (lazy initialization).
     * @return true if the UART is initialized, false otherwise.
     */
    bool EnsureInitialized() noexcept {
        if (!initialized_) {
            initialized_ = Initialize();
        }
        return initialized_;
    }

    /**
     * @brief Checks if the driver is initialized.
     * @return true if initialized, false otherwise
     */
    [[nodiscard]] bool IsInitialized() const noexcept {
        return initialized_;
    }
    
    /**
     * @brief Get the UART configuration.
     * @return Reference to the current configuration
     */
    [[nodiscard]] const UartConfig& GetConfig() const noexcept {
        return config_;
    }
    
    /**
     * @brief Get the UART port number.
     * @return UART port number
     */
    [[nodiscard]] hf_uart_port_t GetPort() const noexcept { 
        return port_; 
    }
    
    //==============================================//
    // PURE VIRTUAL FUNCTIONS - MUST BE OVERRIDDEN  //
    //==============================================//
    
    /**
     * @brief Initialize the UART driver.
     * @return true if successful, false otherwise
     * @note Must be implemented by concrete classes.
     */
    virtual bool Initialize() noexcept = 0;
    
    /**
     * @brief Deinitialize the UART driver.
     * @return true if successful, false otherwise
     * @note Must be implemented by concrete classes.
     */
    virtual bool Deinitialize() noexcept = 0;
    
    /**
     * @brief Write data to the UART.
     * @param data Data buffer to transmit
     * @param length Number of bytes to write
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfUartErr result code
     * @note Must be implemented by concrete classes.
     */
    virtual HfUartErr Write(const uint8_t* data, uint16_t length, 
                           uint32_t timeout_ms = 0) noexcept = 0;

    /**
     * @brief Read data from the UART.
     * @param data Buffer to store received data
     * @param length Number of bytes to read
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfUartErr result code
     * @note Must be implemented by concrete classes.
     */
    virtual HfUartErr Read(uint8_t* data, uint16_t length, 
                          uint32_t timeout_ms = 0) noexcept = 0;

    /**
     * @brief Get the number of bytes available to read.
     * @return Number of bytes available in the receive buffer
     * @note Must be implemented by concrete classes.
     */
    virtual uint16_t BytesAvailable() noexcept = 0;

    /**
     * @brief Flush the transmit buffer.
     * @return HfUartErr result code
     * @note Must be implemented by concrete classes.
     */
    virtual HfUartErr FlushTx() noexcept = 0;

    /**
     * @brief Flush the receive buffer.
     * @return HfUartErr result code
     * @note Must be implemented by concrete classes.
     */
    virtual HfUartErr FlushRx() noexcept = 0;
    
    //==============================================//
    // CONVENIENCE METHODS WITH DEFAULT IMPLEMENTATIONS //
    //==============================================//
    
    /**
     * @brief Legacy compatibility: Open and initialize the UART.
     * @return true if open, false otherwise.
     */
    virtual bool Open() noexcept {
        return EnsureInitialized();
    }

    /**
     * @brief Legacy compatibility: Close and de-initialize the UART.
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
     * @brief Legacy compatibility: Write with boolean return.
     * @param data Data buffer to transmit
     * @param length Number of bytes to write
     * @return true if write succeeded
     */
    virtual bool Write(const uint8_t *data, uint16_t length) noexcept {
        if (!EnsureInitialized()) {
            return false;
        }
        return Write(data, length, 0) == HfUartErr::UART_SUCCESS;
    }

    /**
     * @brief Legacy compatibility: Read with boolean return.
     * @param data Buffer to store received data
     * @param length Number of bytes to read
     * @param timeout_ms Timeout in milliseconds
     * @return true if read succeeded
     */
    virtual bool Read(uint8_t *data, uint16_t length, uint32_t timeout_ms = UINT32_MAX) noexcept {
        if (!EnsureInitialized()) {
            return false;
        }
        uint32_t timeout = (timeout_ms == UINT32_MAX) ? config_.timeout_ms : timeout_ms;
        return Read(data, length, timeout) == HfUartErr::UART_SUCCESS;
    }

    /**
     * @brief Write string data.
     * @param str Null-terminated string to write
     * @return true if successful, false otherwise
     */
    virtual bool WriteString(const char* str) noexcept {
        if (!str) {
            return false;
        }
        // Calculate string length (simple strlen implementation)
        uint16_t len = 0;
        while (str[len] != '\0') {
            len++;
        }
        return Write(reinterpret_cast<const uint8_t*>(str), len) == HfUartErr::UART_SUCCESS;
    }

    /**
     * @brief Write single byte.
     * @param byte Byte to write
     * @return true if successful, false otherwise
     */
    virtual bool WriteByte(uint8_t byte) noexcept {
        return Write(&byte, 1) == HfUartErr::UART_SUCCESS;
    }

    /**
     * @brief Read single byte.
     * @param byte Output: byte read
     * @param timeout_ms Timeout in milliseconds
     * @return true if successful, false otherwise
     */
    virtual bool ReadByte(uint8_t& byte, uint32_t timeout_ms = 1000) noexcept {
        return Read(&byte, 1, timeout_ms) == HfUartErr::UART_SUCCESS;
    }

    /**
     * @brief Legacy compatibility: Flush transmit buffer with boolean return.
     * @return true if successful, false otherwise
     */
    virtual bool FlushTx() noexcept {
        if (!EnsureInitialized()) {
            return false;
        }
        return FlushTx() == HfUartErr::UART_SUCCESS;
    }

    /**
     * @brief Legacy compatibility: Flush receive buffer with boolean return.
     * @return true if successful, false otherwise
     */
    virtual bool FlushRx() noexcept {
        if (!EnsureInitialized()) {
            return false;
        }
        return FlushRx() == HfUartErr::UART_SUCCESS;
    }

    /**
     * @brief Set timeout for read operations.
     * @param timeout_ms Timeout in milliseconds
     */
    virtual void SetReadTimeout(uint32_t timeout_ms) noexcept {
        config_.timeout_ms = timeout_ms;
    }

    /**
     * @brief Get the configured baud rate.
     * @return Baud rate in bits per second
     */
    [[nodiscard]] virtual uint32_t GetBaudRate() const noexcept {
        return config_.baud_rate;
    }

    /**
     * @brief Get the configured data bits.
     * @return Number of data bits
     */
    [[nodiscard]] virtual uint8_t GetDataBits() const noexcept {
        return config_.data_bits;
    }

    /**
     * @brief Get the configured parity setting.
     * @return Parity setting (0=None, 1=Even, 2=Odd)
     */
    [[nodiscard]] virtual uint8_t GetParity() const noexcept {
        return config_.parity;
    }

    /**
     * @brief Get the configured stop bits.
     * @return Number of stop bits
     */
    [[nodiscard]] virtual uint8_t GetStopBits() const noexcept {
        return config_.stop_bits;
    }

    /**
     * @brief Check if hardware flow control is enabled.
     * @return true if hardware flow control is enabled
     */
    [[nodiscard]] virtual bool IsFlowControlEnabled() const noexcept {
        return config_.use_hardware_flow_control;
    }

    /**
     * @brief Printf-style formatted output.
     * @param format Format string
     * @param ... Format arguments
     * @return Number of characters written, or -1 on error
     * @note This is a virtual method that can be overridden for platform-specific implementations
     */
    virtual int Printf(const char* format, ...) noexcept = 0;

protected:
    hf_uart_port_t port_;                ///< UART port number
    UartConfig config_;                  ///< UART configuration
    bool initialized_;                   ///< Initialization state
};

#endif // HAL_INTERNAL_INTERFACE_DRIVERS_BASEUARTDRIVER_H_
