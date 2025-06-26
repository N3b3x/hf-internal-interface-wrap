/**
 * @file HardwareTypes.h
 * @brief Platform-agnostic hardware type definitions for the HardFOC system.
 *
 * @details This file defines platform-agnostic types used by base interface classes.
 *          These types provide a consistent API across different hardware platforms
 *          without exposing MCU-specific implementation details..
 *
 * @note These types are designed to be platform-independent and should not include
 *       any MCU-specific headers or definitions.
 */

#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_HARDWARETYPES_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_HARDWARETYPES_H_

#include <cstdint>
#include <limits>

//==============================================================================
// PLATFORM-AGNOSTIC GPIO TYPES
//==============================================================================

/**
 * @brief Platform-agnostic GPIO pin number type.
 * @details Uses int32_t to accommodate various MCU pin numbering schemes.
 *          Negative values indicate invalid or unassigned pins.
 */
using HfPinNumber = int32_t;

/**
 * @brief Invalid pin constant for unassigned or invalid pins.
 */
constexpr HfPinNumber HF_INVALID_PIN = -1;

/**
 * @brief Maximum pin number supported by the abstraction layer.
 */
constexpr HfPinNumber HF_MAX_PIN_NUMBER = 255;

//==============================================================================
// PLATFORM-AGNOSTIC PORT/CONTROLLER TYPES
//==============================================================================

/**
 * @brief Platform-agnostic port/controller identifier type.
 * @details Generic identifier for communication ports (I2C, SPI, UART, etc.).
 *          Uses uint32_t to accommodate various controller numbering schemes.
 */
using HfPortNumber = uint32_t;

/**
 * @brief Invalid port constant for unassigned or invalid ports.
 */
constexpr HfPortNumber HF_INVALID_PORT = std::numeric_limits<HfPortNumber>::max();

/**
 * @brief Platform-agnostic host/controller identifier type.
 * @details Used for SPI hosts, I2C controllers, etc.
 */
using HfHostId = uint32_t;

/**
 * @brief Invalid host constant for unassigned or invalid hosts.
 */
constexpr HfHostId HF_INVALID_HOST = std::numeric_limits<HfHostId>::max();

//==============================================================================
// COMMUNICATION FREQUENCY TYPES
//==============================================================================

/**
 * @brief Platform-agnostic frequency type in Hz.
 */
using HfFrequencyHz = uint32_t;

/**
 * @brief Platform-agnostic baud rate type.
 */
using HfBaudRate = uint32_t;

//==============================================================================
// CHANNEL AND TIMING TYPES
//==============================================================================

/**
 * @brief Platform-agnostic channel identifier type.
 * @details Used for ADC channels, PWM channels, PIO channels, etc.
 */
using HfChannelId = uint32_t;

/**
 * @brief Invalid channel constant.
 */
constexpr HfChannelId HF_INVALID_CHANNEL = std::numeric_limits<HfChannelId>::max();

/**
 * @brief Platform-agnostic timeout type in milliseconds.
 */
using HfTimeoutMs = uint32_t;

/**
 * @brief Platform-agnostic timestamp type in microseconds.
 */
using HfTimestampUs = uint32_t;

//==============================================================================
// VALIDATION FUNCTIONS
//==============================================================================

/**
 * @brief Check if a pin number is valid.
 * @param pin Pin number to validate
 * @return true if valid, false otherwise
 */
constexpr bool IsValidPin(HfPinNumber pin) noexcept {
    return pin >= 0 && pin <= HF_MAX_PIN_NUMBER;
}

/**
 * @brief Check if a port number is valid.
 * @param port Port number to validate
 * @return true if valid, false otherwise
 */
constexpr bool IsValidPort(HfPortNumber port) noexcept {
    return port != HF_INVALID_PORT;
}

/**
 * @brief Check if a host ID is valid.
 * @param host Host ID to validate
 * @return true if valid, false otherwise
 */
constexpr bool IsValidHost(HfHostId host) noexcept {
    return host != HF_INVALID_HOST;
}

/**
 * @brief Check if a channel ID is valid.
 * @param channel Channel ID to validate
 * @return true if valid, false otherwise
 */
constexpr bool IsValidChannel(HfChannelId channel) noexcept {
    return channel != HF_INVALID_CHANNEL;
}

#endif // HAL_INTERNAL_INTERFACE_DRIVERS_HARDWARETYPES_H_