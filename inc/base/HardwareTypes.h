/**
 * @file HardwareTypes.h
 * @brief Platform-agnostic hardware type definitions for the HardFOC system.
 *
 * This file defines platform-agnostic types used by base interface classes.
 * These types provide a consistent API across different hardware platforms
 * without exposing MCU-specific implementation details. All hardware
 * abstraction layers use these common types for portability and consistency.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note These types are designed to be platform-independent and should not include
 *       any MCU-specific headers or definitions.
 */

#pragma once

#include <cstdint>
#include <limits>

//==============================================================================
// PLATFORM-AGNOSTIC GPIO TYPES
//==============================================================================

/**
 * @brief Platform-agnostic GPIO pin number type.
 * @details Generic pin identifier that works across different hardware platforms.
 *          Uses int32_t to handle both positive pin numbers and invalid (-1) values.
 */
using hf_pin_num_t = int32_t;

/**
 * @brief Invalid pin constant for unassigned or invalid pins.
 */
constexpr hf_pin_num_t HF_INVALID_PIN = -1;

/**
 * @brief Maximum pin number supported by the abstraction layer.
 */
constexpr hf_pin_num_t HF_MAX_PIN_NUMBER = 255;

//==============================================================================
// PLATFORM-AGNOSTIC PORT/CONTROLLER TYPES
//==============================================================================

/**
 * @brief Platform-agnostic port/controller identifier type.
 * @details Generic identifier for communication ports (I2C, SPI, UART, etc.).
 *          Uses uint32_t to accommodate various controller numbering schemes.
 */
using hf_port_num_t = uint32_t;

/**
 * @brief Invalid port constant for unassigned or invalid ports.
 */
constexpr hf_port_num_t HF_INVALID_PORT = std::numeric_limits<hf_port_num_t>::max();

/**
 * @brief Platform-agnostic host/controller identifier type.
 * @details Used for SPI hosts, I2C controllers, etc.
 */
using hf_host_id_t = uint32_t;

/**
 * @brief Invalid host constant for unassigned or invalid hosts.
 */
constexpr hf_host_id_t HF_INVALID_HOST = std::numeric_limits<hf_host_id_t>::max();

//==============================================================================
// COMMUNICATION FREQUENCY TYPES
//==============================================================================

/**
 * @brief Platform-agnostic frequency type (in Hz).
 */
using hf_frequency_hz_t = uint32_t;

///@brief Backward compatibility alias
using hf_frequency_t = hf_frequency_hz_t;

/**
 * @brief Platform-agnostic baud rate type.
 */
using hf_baud_rate_t = uint32_t;

//==============================================================================
// CHANNEL AND TIMING TYPES
//==============================================================================

/**
 * @brief Platform-agnostic channel identifier type.
 * @details Used for ADC channels, PWM channels, etc.
 */
using hf_channel_id_t = uint32_t;

/**
 * @brief Invalid channel constant for unassigned or invalid channels.
 */
constexpr hf_channel_id_t HF_INVALID_CHANNEL = std::numeric_limits<hf_channel_id_t>::max();

/**
 * @brief Platform-agnostic time type in milliseconds.
 */
using hf_time_t = uint32_t;

/**
 * @brief Timeout value in milliseconds.
 */
using hf_timeout_ms_t = hf_time_t;

//==============================================================================
// TIMEOUT CONSTANTS
//==============================================================================

/**
 * @brief Default timeout value in milliseconds.
 */
constexpr hf_time_t HF_TIMEOUT_DEFAULT_MS = 1000;

/**
 * @brief No timeout (wait indefinitely).
 */
constexpr hf_time_t HF_TIMEOUT_NONE = 0;

/**
 * @brief Maximum timeout value.
 */
constexpr hf_time_t HF_TIMEOUT_MAX = std::numeric_limits<hf_time_t>::max();

//==============================================================================
// VALIDATION FUNCTIONS
//==============================================================================

/**
 * @brief Check if a pin number is valid.
 * @param pin Pin number to validate
 * @return true if valid, false otherwise
 */
constexpr bool IsValidPin(hf_pin_num_t pin) noexcept {
  return pin >= 0 && pin <= HF_MAX_PIN_NUMBER;
}

/**
 * @brief Check if a port number is valid.
 * @param port Port number to validate
 * @return true if valid, false otherwise
 */
constexpr bool IsValidPort(hf_port_num_t port) noexcept {
  return port != HF_INVALID_PORT;
}

/**
 * @brief Check if a host ID is valid.
 * @param host Host ID to validate
 * @return true if valid, false otherwise
 */
constexpr bool IsValidHost(hf_host_id_t host) noexcept {
  return host != HF_INVALID_HOST;
}

/**
 * @brief Check if a channel ID is valid.
 * @param channel Channel ID to validate
 * @return true if valid, false otherwise
 */
constexpr bool IsValidChannel(hf_channel_id_t channel) noexcept {
  return channel != HF_INVALID_CHANNEL;
}