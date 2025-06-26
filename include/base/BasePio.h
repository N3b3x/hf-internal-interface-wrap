/**
 * @file BasePio.h
 * @brief Abstract base class for Programmable IO Channel implementations in the HardFOC system.
 * 
 * This header defines the abstract base class for precise, buffered digital signal I/O
 * that provides a consistent API across different PIO implementations.
 * Concrete implementations (like McuPio for ESP32 RMT, RP2040 PIO, STM32 TIM+DMA) inherit from this class.
 * 
 * @note This is a header-only abstract base class following the same pattern as BaseCan/BaseAdc/BasePwm.
 * @note Users should program against this interface, not specific implementations.
 */

#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_BASEPIO_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_BASEPIO_H_

#include "mcu/McuTypes.h"
#include <cstdint>
#include <functional>
#include <string_view>

//--------------------------------------
//  HardFOC PIO Error Codes (Table)
//--------------------------------------
/**
 * @brief HardFOC PIO error codes
 * @details Comprehensive error enumeration for all PIO operations in the system.
 *          This enumeration is used across all PIO-related classes to provide
 *          consistent error reporting and handling.
 */

#define HF_PIO_ERR_LIST(X) \
    /* Success codes */ \
    X(PIO_SUCCESS, 0, "Success") \
    /* General errors */ \
    X(PIO_ERR_FAILURE, 1, "General failure") \
    X(PIO_ERR_NOT_INITIALIZED, 2, "Not initialized") \
    X(PIO_ERR_ALREADY_INITIALIZED, 3, "Already initialized") \
    X(PIO_ERR_INVALID_PARAMETER, 4, "Invalid parameter") \
    X(PIO_ERR_NULL_POINTER, 5, "Null pointer") \
    X(PIO_ERR_OUT_OF_MEMORY, 6, "Out of memory") \
    /* Channel errors */ \
    X(PIO_ERR_INVALID_CHANNEL, 7, "Invalid PIO channel") \
    X(PIO_ERR_CHANNEL_BUSY, 8, "Channel already in use") \
    X(PIO_ERR_CHANNEL_NOT_AVAILABLE, 9, "Channel not available") \
    X(PIO_ERR_INSUFFICIENT_CHANNELS, 10, "Insufficient channels available") \
    /* Timing errors */ \
    X(PIO_ERR_INVALID_RESOLUTION, 11, "Invalid time resolution") \
    X(PIO_ERR_RESOLUTION_TOO_HIGH, 12, "Time resolution too high") \
    X(PIO_ERR_RESOLUTION_TOO_LOW, 13, "Time resolution too low") \
    X(PIO_ERR_DURATION_TOO_LONG, 14, "Duration too long") \
    X(PIO_ERR_DURATION_TOO_SHORT, 15, "Duration too short") \
    /* Buffer errors */ \
    X(PIO_ERR_BUFFER_OVERFLOW, 16, "Buffer overflow") \
    X(PIO_ERR_BUFFER_UNDERFLOW, 17, "Buffer underflow") \
    X(PIO_ERR_BUFFER_TOO_SMALL, 18, "Buffer too small") \
    X(PIO_ERR_BUFFER_TOO_LARGE, 19, "Buffer too large") \
    /* Hardware errors */ \
    X(PIO_ERR_HARDWARE_FAULT, 20, "Hardware fault") \
    X(PIO_ERR_COMMUNICATION_TIMEOUT, 21, "Communication timeout") \
    X(PIO_ERR_COMMUNICATION_FAILURE, 22, "Communication failure") \
    X(PIO_ERR_DEVICE_NOT_RESPONDING, 23, "Device not responding") \
    /* Configuration errors */ \
    X(PIO_ERR_INVALID_CONFIGURATION, 24, "Invalid configuration") \
    X(PIO_ERR_UNSUPPORTED_OPERATION, 25, "Unsupported operation") \
    X(PIO_ERR_PIN_CONFLICT, 26, "Pin already in use") \
    X(PIO_ERR_RESOURCE_BUSY, 27, "Resource busy") \
    /* System errors */ \
    X(PIO_ERR_SYSTEM_ERROR, 28, "System error") \
    X(PIO_ERR_PERMISSION_DENIED, 29, "Permission denied") \
    X(PIO_ERR_OPERATION_ABORTED, 30, "Operation aborted")

enum class HfPioErr : uint8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
    HF_PIO_ERR_LIST(X)
#undef X
};

/**
 * @brief Convert HfPioErr to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfPioErrToString(HfPioErr err) noexcept {
    switch (err) {
#define X(NAME, VALUE, DESC) \
    case HfPioErr::NAME: \
        return DESC;
        HF_PIO_ERR_LIST(X)
#undef X
    default:
        return "Unknown error";
    }
}

//--------------------------------------
//  PIO Configuration Structures
//--------------------------------------

/**
 * @brief PIO channel direction
 */
enum class PioDirection : uint8_t {
    Transmit = 0,     ///< Transmit mode (output)
    Receive = 1,      ///< Receive mode (input)
    Bidirectional = 2 ///< Bidirectional mode (if supported)
};

/**
 * @brief PIO signal polarity
 */
enum class PioPolarity : uint8_t {
    Normal = 0,       ///< Normal polarity (idle low, active high)
    Inverted = 1      ///< Inverted polarity (idle high, active low)
};

/**
 * @brief PIO idle state
 */
enum class PioIdleState : uint8_t {
    Low = 0,          ///< Idle state is low
    High = 1          ///< Idle state is high
};

/**
 * @brief PIO channel configuration structure
 */
struct PioChannelConfig {
    hf_gpio_num_t gpio_pin;           ///< GPIO pin for PIO signal
    PioDirection direction;           ///< Channel direction
    uint32_t resolution_ns;           ///< Time resolution in nanoseconds
    PioPolarity polarity;             ///< Signal polarity
    PioIdleState idle_state;          ///< Idle state
    uint32_t timeout_us;              ///< Operation timeout in microseconds
    size_t buffer_size;               ///< Buffer size for symbols/durations
    
    PioChannelConfig() noexcept
        : gpio_pin(-1), direction(PioDirection::Transmit)
        , resolution_ns(1000), polarity(PioPolarity::Normal)
        , idle_state(PioIdleState::Low), timeout_us(10000)
        , buffer_size(64) {}
};

/**
 * @brief PIO symbol structure for precise timing
 */
struct PioSymbol {
    uint32_t duration;                ///< Duration in resolution units
    bool level;                       ///< Signal level (true = high, false = low)
    
    PioSymbol() noexcept : duration(0), level(false) {}
    PioSymbol(uint32_t dur, bool lvl) noexcept : duration(dur), level(lvl) {}
};

/**
 * @brief PIO channel status information
 */
struct PioChannelStatus {
    bool is_initialized;              ///< Channel is initialized
    bool is_busy;                     ///< Channel is currently busy
    bool is_transmitting;             ///< Channel is transmitting
    bool is_receiving;                ///< Channel is receiving
    size_t symbols_queued;            ///< Number of symbols in queue
    size_t symbols_processed;         ///< Number of symbols processed
    HfPioErr last_error;              ///< Last error that occurred
    uint32_t timestamp_us;            ///< Timestamp of last operation
};

/**
 * @brief PIO capability information
 */
struct PioCapabilities {
    uint8_t max_channels;             ///< Maximum number of channels
    uint32_t min_resolution_ns;       ///< Minimum time resolution
    uint32_t max_resolution_ns;       ///< Maximum time resolution
    uint32_t max_duration;            ///< Maximum single duration
    size_t max_buffer_size;           ///< Maximum buffer size
    bool supports_bidirectional;     ///< Supports bidirectional mode
    bool supports_loopback;           ///< Supports loopback mode
    bool supports_carrier;            ///< Supports carrier modulation
};

//--------------------------------------
//  Callback Types
//--------------------------------------

/**
 * @brief Callback for PIO transmission complete events
 * @param channel_id Channel that completed transmission
 * @param symbols_sent Number of symbols transmitted
 * @param user_data User-provided data
 */
using PioTransmitCallback = std::function<void(uint8_t channel_id, size_t symbols_sent, void* user_data)>;

/**
 * @brief Callback for PIO reception complete events
 * @param channel_id Channel that received data
 * @param symbols Received symbols
 * @param symbol_count Number of symbols received
 * @param user_data User-provided data
 */
using PioReceiveCallback = std::function<void(uint8_t channel_id, const PioSymbol* symbols, size_t symbol_count, void* user_data)>;

/**
 * @brief Callback for PIO error events
 * @param channel_id Channel that encountered error
 * @param error Error that occurred
 * @param user_data User-provided data
 */
using PioErrorCallback = std::function<void(uint8_t channel_id, HfPioErr error, void* user_data)>;

//--------------------------------------
//  Abstract Base Class
//--------------------------------------

/**
 * @class BasePio
 * @brief Abstract base class for Programmable IO Channel implementations.
 * 
 * This class defines the interface for precise, buffered digital signal I/O
 * that can handle timing-critical operations like WS2812 LED driving,
 * IR communication, stepper motor control, and custom protocols.
 * 
 * The abstraction is designed to work with various hardware backends:
 * - ESP32: RMT (Remote Control Transceiver) peripheral
 * - RP2040: PIO (Programmable I/O) state machines
 * - STM32: Timer + DMA + GPIO combinations
 * - Generic: Software-based implementations
 * 
 * Key features:
 * - Precise timing control down to nanosecond resolution
 * - Buffered symbol transmission and reception
 * - Asynchronous operation with callbacks
 * - Platform-agnostic interface
 * - Support for complex waveform generation and decoding
 */
class BasePio {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~BasePio() noexcept = default;

    // Disable copy constructor and assignment operator for safety
    BasePio(const BasePio&) = delete;
    BasePio& operator=(const BasePio&) = delete;

    // Allow move operations
    BasePio(BasePio&&) noexcept = default;
    BasePio& operator=(BasePio&&) noexcept = default;

    //==============================================//
    // PURE VIRTUAL FUNCTIONS - MUST BE OVERRIDDEN  //
    //==============================================//

    /**
     * @brief Initialize the PIO peripheral
     * @return Error code indicating success or failure
     */
    virtual HfPioErr Initialize() noexcept = 0;

    /**
     * @brief Deinitialize the PIO peripheral
     * @return Error code indicating success or failure
     */
    virtual HfPioErr Deinitialize() noexcept = 0;

    /**
     * @brief Check if the PIO is initialized
     * @return true if initialized, false otherwise
     */
    virtual bool IsInitialized() const noexcept = 0;

    /**
     * @brief Configure a PIO channel
     * @param channel_id Channel identifier
     * @param config Channel configuration
     * @return Error code indicating success or failure
     */
    virtual HfPioErr ConfigureChannel(uint8_t channel_id, const PioChannelConfig& config) noexcept = 0;

    /**
     * @brief Transmit a sequence of symbols
     * @param channel_id Channel identifier
     * @param symbols Array of symbols to transmit
     * @param symbol_count Number of symbols in the array
     * @param wait_completion If true, block until transmission is complete
     * @return Error code indicating success or failure
     */
    virtual HfPioErr Transmit(uint8_t channel_id, const PioSymbol* symbols, size_t symbol_count, bool wait_completion = false) noexcept = 0;

    /**
     * @brief Start receiving symbols
     * @param channel_id Channel identifier
     * @param buffer Buffer to store received symbols
     * @param buffer_size Size of the buffer
     * @param timeout_us Timeout in microseconds (0 = no timeout)
     * @return Error code indicating success or failure
     */
    virtual HfPioErr StartReceive(uint8_t channel_id, PioSymbol* buffer, size_t buffer_size, uint32_t timeout_us = 0) noexcept = 0;

    /**
     * @brief Stop receiving and get the number of symbols received
     * @param channel_id Channel identifier
     * @param symbols_received [out] Number of symbols actually received
     * @return Error code indicating success or failure
     */
    virtual HfPioErr StopReceive(uint8_t channel_id, size_t& symbols_received) noexcept = 0;

    /**
     * @brief Check if a channel is currently busy
     * @param channel_id Channel identifier
     * @return true if channel is busy, false otherwise
     */
    virtual bool IsChannelBusy(uint8_t channel_id) const noexcept = 0;

    /**
     * @brief Get channel status information
     * @param channel_id Channel identifier
     * @param status [out] Status information
     * @return Error code indicating success or failure
     */
    virtual HfPioErr GetChannelStatus(uint8_t channel_id, PioChannelStatus& status) const noexcept = 0;

    /**
     * @brief Get PIO capabilities
     * @param capabilities [out] Capability information
     * @return Error code indicating success or failure
     */
    virtual HfPioErr GetCapabilities(PioCapabilities& capabilities) const noexcept = 0;

    /**
     * @brief Set callback for transmission complete events
     * @param callback Callback function
     * @param user_data User data to pass to callback
     */
    virtual void SetTransmitCallback(PioTransmitCallback callback, void* user_data = nullptr) noexcept = 0;

    /**
     * @brief Set callback for reception complete events
     * @param callback Callback function
     * @param user_data User data to pass to callback
     */
    virtual void SetReceiveCallback(PioReceiveCallback callback, void* user_data = nullptr) noexcept = 0;

    /**
     * @brief Set callback for error events
     * @param callback Callback function
     * @param user_data User data to pass to callback
     */
    virtual void SetErrorCallback(PioErrorCallback callback, void* user_data = nullptr) noexcept = 0;

    /**
     * @brief Clear all callbacks
     */
    virtual void ClearCallbacks() noexcept = 0;

protected:
    /**
     * @brief Protected constructor
     */
    BasePio() noexcept = default;

private:
    bool initialized_{false};
};

#endif // HAL_INTERNAL_INTERFACE_DRIVERS_BASEPIO_H_
