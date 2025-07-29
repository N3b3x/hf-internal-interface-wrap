/**
 * @file SecurityGuidelines.h
 * @brief Security and code quality guidelines for ESP32 development
 *
 * This file defines security best practices, named constants for magic numbers,
 * and safe utility functions to improve code maintainability and security.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#ifdef HF_MCU_FAMILY_ESP32

#include <charconv>
#include <string_view>
#include <cstring>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//==============================================================================
// NAMED CONSTANTS FOR MAGIC NUMBERS
//==============================================================================

namespace Security {

// WEP Key Length Constants
constexpr size_t WEP_KEY_LENGTH_64_BIT = 5;    ///< WEP 64-bit key length in bytes
constexpr size_t WEP_KEY_LENGTH_128_BIT = 13;  ///< WEP 128-bit key length in bytes
constexpr size_t WEP_KEY_LENGTH_152_BIT = 16;  ///< WEP 152-bit key length in bytes
constexpr size_t WEP_KEY_LENGTH_256_BIT = 29;  ///< WEP 256-bit key length in bytes

// UUID Constants
constexpr size_t UUID_128_BYTE_LENGTH = 16;    ///< UUID 128-bit length in bytes
constexpr size_t UUID_STRING_LENGTH = 36;      ///< UUID string representation length
constexpr size_t UUID_STRING_LENGTH_WITH_NULL = 37; ///< UUID string with null terminator

// Buffer size constants
constexpr size_t MAX_INPUT_BUFFER_SIZE = 256;  ///< Maximum input buffer size for parsing
constexpr size_t MAX_WEP_KEY_BUFFER = 64;      ///< Maximum WEP key buffer size

} // namespace Security

//==============================================================================
// SAFE INPUT PARSING UTILITIES
//==============================================================================

namespace SafeParsing {

/**
 * @brief Safe alternative to sscanf for parsing integers
 * 
 * @param input Input string to parse
 * @param output Reference to store parsed value
 * @return true if parsing successful, false otherwise
 */
template<typename T>
bool ParseInteger(std::string_view input, T& output) noexcept {
    // Validate input
    if (input.empty() || input.size() > Security::MAX_INPUT_BUFFER_SIZE) {
        return false;
    }
    
    // Use std::from_chars for safe parsing
    auto result = std::from_chars(input.data(), input.data() + input.size(), output);
    return result.ec == std::errc{} && result.ptr == input.data() + input.size();
}

/**
 * @brief Safe WEP key validation
 * 
 * @param key_data Pointer to key data
 * @param key_length Length of key data
 * @return true if key length is valid, false otherwise
 */
bool ValidateWepKeyLength(const uint8_t* key_data, size_t key_length) noexcept {
    if (!key_data) {
        return false;
    }
    
    return (key_length == Security::WEP_KEY_LENGTH_64_BIT ||
            key_length == Security::WEP_KEY_LENGTH_128_BIT ||
            key_length == Security::WEP_KEY_LENGTH_152_BIT ||
            key_length == Security::WEP_KEY_LENGTH_256_BIT);
}

/**
 * @brief Safe UUID validation
 * 
 * @param uuid_data Pointer to UUID data
 * @param data_length Length of UUID data
 * @return true if UUID length is valid, false otherwise
 */
bool ValidateUuidLength(const uint8_t* uuid_data, size_t data_length) noexcept {
    if (!uuid_data) {
        return false;
    }
    
    return data_length == Security::UUID_128_BYTE_LENGTH;
}

/**
 * @brief Safe string parsing with bounds checking
 * 
 * @param input Input string
 * @param max_length Maximum allowed length
 * @param output Output buffer
 * @param output_size Size of output buffer
 * @return true if parsing successful, false otherwise
 */
bool ParseBoundedString(const char* input, size_t max_length, 
                       char* output, size_t output_size) noexcept {
    if (!input || !output || output_size == 0) {
        return false;
    }
    
    size_t input_len = strnlen(input, max_length + 1);
    if (input_len > max_length || input_len >= output_size) {
        return false;
    }
    
    strncpy(output, input, output_size - 1);
    output[output_size - 1] = '\0';
    return true;
}

} // namespace SafeParsing

//==============================================================================
// FREERTOS DELAY UTILITIES
//==============================================================================

namespace FreeRtosUtils {

/**
 * @brief Safe FreeRTOS delay replacement for std::this_thread::sleep_for
 * 
 * @param delay_ms Delay in milliseconds
 */
inline void DelayMs(uint32_t delay_ms) noexcept {
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
}

/**
 * @brief Safe FreeRTOS delay in microseconds
 * 
 * @param delay_us Delay in microseconds
 */
inline void DelayUs(uint32_t delay_us) noexcept {
    if (delay_us < 1000) {
        // For very short delays, use esp_timer
        ets_delay_us(delay_us);
    } else {
        // Convert to ms for FreeRTOS
        vTaskDelay(pdMS_TO_TICKS(delay_us / 1000));
    }
}

} // namespace FreeRtosUtils

//==============================================================================
// LOGGING UTILITIES
//==============================================================================

namespace LoggingUtils {

/**
 * @brief Consistent ESP_LOG usage instead of std::cout/std::cerr
 */
class ConsistentLogger {
public:
    static void Info(const char* tag, const char* format, ...) {
        va_list args;
        va_start(args, format);
        esp_log_writev(ESP_LOG_INFO, tag, format, args);
        va_end(args);
    }
    
    static void Error(const char* tag, const char* format, ...) {
        va_list args;
        va_start(args, format);
        esp_log_writev(ESP_LOG_ERROR, tag, format, args);
        va_end(args);
    }
    
    static void Warning(const char* tag, const char* format, ...) {
        va_list args;
        va_start(args, format);
        esp_log_writev(ESP_LOG_WARN, tag, format, args);
        va_end(args);
    }
    
    static void Debug(const char* tag, const char* format, ...) {
        va_list args;
        va_start(args, format);
        esp_log_writev(ESP_LOG_DEBUG, tag, format, args);
        va_end(args);
    }
};

/**
 * @brief Macro to enforce ESP_LOG usage over std::cout/std::cerr
 */
#define HF_LOG_INFO(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#define HF_LOG_ERROR(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#define HF_LOG_WARN(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
#define HF_LOG_DEBUG(tag, format, ...) ESP_LOGD(tag, format, ##__VA_ARGS__)

} // namespace LoggingUtils

#endif // HF_MCU_FAMILY_ESP32