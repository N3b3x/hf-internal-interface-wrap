/**
 * @file security_improvements_example.cpp
 * @brief Example demonstrating security improvements and best practices
 *
 * This file demonstrates fixes for common security and code quality issues:
 * 1. Safe alternatives to sscanf for user input parsing
 * 2. Named constants for magic numbers (WEP keys, UUID lengths)
 * 3. FreeRTOS delay instead of std::this_thread::sleep_for
 * 4. Consistent ESP_LOG usage instead of std::cout/std::cerr
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "SecurityGuidelines.h"

// BAD: Magic numbers without explanation
// constexpr size_t key_length = 16;
// GOOD: Named constants with clear documentation
using namespace Security;

static const char* TAG = "SecurityExample";

//==============================================================================
// DEMONSTRATION OF UNSAFE vs SAFE PARSING
//==============================================================================

/**
 * @brief Example of UNSAFE sscanf usage (DO NOT USE)
 */
void UnsafeParsingExample() {
    HF_LOG_ERROR(TAG, "=== UNSAFE PARSING EXAMPLE (DO NOT USE) ===");
    
    char user_input[] = "12345678901234567890"; // Potentially dangerous input
    int parsed_value;
    
    // UNSAFE: sscanf without bounds checking
    // sscanf(user_input, "%d", &parsed_value);
    HF_LOG_ERROR(TAG, "UNSAFE: sscanf without bounds checking - NEVER USE THIS!");
}

/**
 * @brief Example of SAFE parsing using std::from_chars
 */
void SafeParsingExample() {
    HF_LOG_INFO(TAG, "=== SAFE PARSING EXAMPLE ===");
    
    const char* user_inputs[] = {
        "12345",
        "invalid",
        "999999999999999999999", // Very large number
        "42",
        "-123"
    };
    
    for (const char* input : user_inputs) {
        int parsed_value;
        bool success = SafeParsing::ParseInteger(std::string_view(input), parsed_value);
        
        if (success) {
            HF_LOG_INFO(TAG, "Successfully parsed '%s' -> %d", input, parsed_value);
        } else {
            HF_LOG_WARN(TAG, "Failed to parse '%s' safely", input);
        }
    }
}

//==============================================================================
// DEMONSTRATION OF WEP KEY VALIDATION WITH NAMED CONSTANTS
//==============================================================================

/**
 * @brief Example of WEP key validation using named constants
 */
void WepKeyValidationExample() {
    HF_LOG_INFO(TAG, "=== WEP KEY VALIDATION EXAMPLE ===");
    
    // Test different key lengths
    uint8_t wep_key_64[WEP_KEY_LENGTH_64_BIT] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t wep_key_128[WEP_KEY_LENGTH_128_BIT] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D
    };
    uint8_t invalid_key[7] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    
    // Test with named constants instead of magic numbers
    if (SafeParsing::ValidateWepKeyLength(wep_key_64, WEP_KEY_LENGTH_64_BIT)) {
        HF_LOG_INFO(TAG, "WEP 64-bit key is valid (length: %d bytes)", WEP_KEY_LENGTH_64_BIT);
    }
    
    if (SafeParsing::ValidateWepKeyLength(wep_key_128, WEP_KEY_LENGTH_128_BIT)) {
        HF_LOG_INFO(TAG, "WEP 128-bit key is valid (length: %d bytes)", WEP_KEY_LENGTH_128_BIT);
    }
    
    if (!SafeParsing::ValidateWepKeyLength(invalid_key, sizeof(invalid_key))) {
        HF_LOG_WARN(TAG, "Invalid WEP key length: %d bytes", sizeof(invalid_key));
    }
}

//==============================================================================
// DEMONSTRATION OF UUID VALIDATION WITH NAMED CONSTANTS
//==============================================================================

/**
 * @brief Example of UUID validation using named constants
 */
void UuidValidationExample() {
    HF_LOG_INFO(TAG, "=== UUID VALIDATION EXAMPLE ===");
    
    // Valid UUID (128-bit = 16 bytes)
    uint8_t valid_uuid[UUID_128_BYTE_LENGTH] = {
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0
    };
    
    // Invalid UUID (wrong length)
    uint8_t invalid_uuid[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    
    // Use named constant instead of magic number 16
    if (SafeParsing::ValidateUuidLength(valid_uuid, UUID_128_BYTE_LENGTH)) {
        HF_LOG_INFO(TAG, "UUID is valid (length: %d bytes)", UUID_128_BYTE_LENGTH);
    }
    
    if (!SafeParsing::ValidateUuidLength(invalid_uuid, sizeof(invalid_uuid))) {
        HF_LOG_WARN(TAG, "Invalid UUID length: %d bytes (expected: %d)", 
                   sizeof(invalid_uuid), UUID_128_BYTE_LENGTH);
    }
}

//==============================================================================
// DEMONSTRATION OF FREERTOS DELAY INSTEAD OF STD::THREAD
//==============================================================================

/**
 * @brief Example of UNSAFE std::this_thread::sleep_for usage (DO NOT USE)
 */
void UnsafeDelayExample() {
    HF_LOG_ERROR(TAG, "=== UNSAFE DELAY EXAMPLE (DO NOT USE) ===");
    
    // UNSAFE: std::this_thread::sleep_for in FreeRTOS environment
    // #include <thread>
    // #include <chrono>
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    HF_LOG_ERROR(TAG, "UNSAFE: std::this_thread::sleep_for - missing headers and not FreeRTOS compatible!");
}

/**
 * @brief Example of SAFE FreeRTOS delay usage
 */
void SafeDelayExample() {
    HF_LOG_INFO(TAG, "=== SAFE FREERTOS DELAY EXAMPLE ===");
    
    HF_LOG_INFO(TAG, "Starting delay demonstration...");
    
    // SAFE: Use FreeRTOS delay functions
    HF_LOG_INFO(TAG, "Delaying 1000ms using FreeRTOS...");
    FreeRtosUtils::DelayMs(1000);
    HF_LOG_INFO(TAG, "1000ms delay completed");
    
    HF_LOG_INFO(TAG, "Delaying 500us using FreeRTOS...");
    FreeRtosUtils::DelayUs(500);
    HF_LOG_INFO(TAG, "500us delay completed");
}

//==============================================================================
// DEMONSTRATION OF CONSISTENT LOGGING
//==============================================================================

/**
 * @brief Example of INCONSISTENT logging (AVOID THIS)
 */
void InconsistentLoggingExample() {
    HF_LOG_ERROR(TAG, "=== INCONSISTENT LOGGING EXAMPLE (AVOID) ===");
    
    // INCONSISTENT: Mixing std::cout/cerr with ESP_LOG
    // std::cout << "This is inconsistent with ESP_LOG usage" << std::endl;
    // std::cerr << "Error: This should use ESP_LOGE instead" << std::endl;
    HF_LOG_ERROR(TAG, "INCONSISTENT: Mixed std::cout/cerr with ESP_LOG - use ESP_LOG consistently!");
    
    ESP_LOGI(TAG, "This ESP_LOG call is good");
    // But then mixing with std::cout is bad practice
}

/**
 * @brief Example of CONSISTENT logging using ESP_LOG macros
 */
void ConsistentLoggingExample() {
    HF_LOG_INFO(TAG, "=== CONSISTENT LOGGING EXAMPLE ===");
    
    // GOOD: Consistent ESP_LOG usage
    HF_LOG_INFO(TAG, "Information message using HF_LOG_INFO");
    HF_LOG_WARN(TAG, "Warning message using HF_LOG_WARN");
    HF_LOG_ERROR(TAG, "Error message using HF_LOG_ERROR");
    HF_LOG_DEBUG(TAG, "Debug message using HF_LOG_DEBUG");
    
    // Also good: Direct ESP_LOG usage
    ESP_LOGI(TAG, "Direct ESP_LOGI usage is also consistent");
    ESP_LOGW(TAG, "Direct ESP_LOGW usage is also consistent");
    ESP_LOGE(TAG, "Direct ESP_LOGE usage is also consistent");
    ESP_LOGD(TAG, "Direct ESP_LOGD usage is also consistent");
}

//==============================================================================
// MAIN DEMONSTRATION FUNCTION
//==============================================================================

/**
 * @brief Main function demonstrating all security improvements
 */
extern "C" void security_improvements_demo() {
    HF_LOG_INFO(TAG, "=== SECURITY IMPROVEMENTS DEMONSTRATION ===");
    
    // 1. Safe parsing instead of sscanf
    UnsafeParsingExample();
    SafeParsingExample();
    
    FreeRtosUtils::DelayMs(1000);
    
    // 2. Named constants for WEP key magic numbers
    WepKeyValidationExample();
    
    FreeRtosUtils::DelayMs(1000);
    
    // 3. Named constants for UUID magic numbers
    UuidValidationExample();
    
    FreeRtosUtils::DelayMs(1000);
    
    // 4. FreeRTOS delay instead of std::this_thread::sleep_for
    UnsafeDelayExample();
    SafeDelayExample();
    
    FreeRtosUtils::DelayMs(1000);
    
    // 5. Consistent ESP_LOG usage
    InconsistentLoggingExample();
    ConsistentLoggingExample();
    
    HF_LOG_INFO(TAG, "=== SECURITY IMPROVEMENTS DEMONSTRATION COMPLETE ===");
}

/**
 * @brief Integration point for the main application
 */
void run_security_demo_task(void* parameter) {
    while (true) {
        security_improvements_demo();
        
        // Wait 10 seconds before running again
        FreeRtosUtils::DelayMs(10000);
    }
}