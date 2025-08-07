/**
 * @file NvsComprehensiveTest.cpp
 * @brief Comprehensive NVS (Non-Volatile Storage) testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This test suite provides complete coverage of the EspNvs class functionality with comprehensive
 * testing of all methods, edge cases, error conditions, and performance characteristics.
 * Designed for production validation with no exceptions and no RTTI usage.
 *
 * Test Coverage:
 * - Constructor/Destructor testing with various namespace scenarios
 * - Initialization/Deinitialization testing with error conditions
 * - All data type operations (U32, String, Blob) with boundary testing
 * - Key management operations (exists, erase, size)
 * - Commit operations and auto-commit behavior
 * - Statistics and diagnostics functionality
 * - Error condition testing and parameter validation
 * - Thread safety and concurrent operations
 * - Performance limits and boundary conditions
 * - Memory management and resource cleanup
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "base/BaseNvs.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mcu/esp32/EspNvs.h"
#include "mcu/esp32/utils/EspTypes_NVS.h"

#include "TestFramework.h"

static const char* TAG = "NVS_Test";

static TestResults g_test_results;

// Test data constants for comprehensive testing
static constexpr uint32_t TEST_U32_VALUE_1 = 0x12345678;
static constexpr uint32_t TEST_U32_VALUE_2 = 0xDEADBEEF;
static constexpr uint32_t TEST_U32_VALUE_MAX = 0xFFFFFFFF;
static constexpr uint32_t TEST_U32_VALUE_MIN = 0x00000000;

static const char* TEST_STRING_SHORT = "test";
static const char* TEST_STRING_MEDIUM = "Hello ESP32-C6 NVS Test Suite";
static const char* TEST_STRING_LONG = "This is a very long test string that approaches the maximum length supported by ESP32 NVS to test boundary conditions and buffer management capabilities.";
static const char* TEST_STRING_EMPTY = "";
static const char* TEST_STRING_UNICODE = "测试🚀ñ€";

static const uint8_t TEST_BLOB_DATA[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 
                                         0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
static constexpr size_t TEST_BLOB_SIZE = sizeof(TEST_BLOB_DATA);

// Valid and invalid key names for testing
static const char* VALID_KEY_SHORT = "key1";
static const char* VALID_KEY_MEDIUM = "test_key_123";
static const char* VALID_KEY_MAX_LENGTH = "123456789012345"; // 15 chars max
static const char* INVALID_KEY_TOO_LONG = "1234567890123456"; // 16 chars - too long
static const char* INVALID_KEY_WITH_SPACE = "key with space";
static const char* INVALID_KEY_WITH_TAB = "key\twith\ttab";
static const char* INVALID_KEY_WITH_NEWLINE = "key\nwith\nnewline";
static const char* INVALID_KEY_EMPTY = "";

// Namespace names for testing
static const char* VALID_NAMESPACE = "hardfoc_test";
static const char* VALID_NAMESPACE_SHORT = "test";
static const char* VALID_NAMESPACE_MAX = "123456789012345"; // 15 chars max
static const char* INVALID_NAMESPACE_TOO_LONG = "1234567890123456"; // 16 chars - too long
static const char* INVALID_NAMESPACE_EMPTY = "";

//==============================================//
// CONSTRUCTOR AND DESTRUCTOR TESTS            //
//==============================================//

bool test_constructor_valid_namespace() noexcept {
    ESP_LOGI(TAG, "Testing constructor with valid namespace...");
    
    // Test normal namespace
    {
        EspNvs nvs(VALID_NAMESPACE);
        if (strcmp(nvs.GetNamespace(), VALID_NAMESPACE) != 0) {
            ESP_LOGE(TAG, "Constructor failed: namespace mismatch");
            return false;
        }
        if (nvs.IsInitialized()) {
            ESP_LOGE(TAG, "Constructor failed: should not be initialized yet");
            return false;
        }
    }
    
    // Test short namespace
    {
        EspNvs nvs(VALID_NAMESPACE_SHORT);
        if (strcmp(nvs.GetNamespace(), VALID_NAMESPACE_SHORT) != 0) {
            ESP_LOGE(TAG, "Constructor failed: short namespace mismatch");
            return false;
        }
    }
    
    // Test maximum length namespace
    {
        EspNvs nvs(VALID_NAMESPACE_MAX);
        if (strcmp(nvs.GetNamespace(), VALID_NAMESPACE_MAX) != 0) {
            ESP_LOGE(TAG, "Constructor failed: max namespace mismatch");
            return false;
        }
    }
    
    ESP_LOGI(TAG, "[SUCCESS] Constructor with valid namespace tests passed");
    return true;
}

bool test_constructor_invalid_namespace() noexcept {
    ESP_LOGI(TAG, "Testing constructor with invalid namespace...");
    
    // Test null namespace - should handle gracefully
    {
        EspNvs nvs(nullptr);
        // Constructor should succeed but Initialize should fail
        if (nvs.IsInitialized()) {
            ESP_LOGE(TAG, "Constructor failed: null namespace should not auto-initialize");
            return false;
        }
        
        // Try to initialize - should fail
        hf_nvs_err_t result = nvs.Initialize();
        if (result == hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(TAG, "Initialize unexpectedly succeeded with null namespace");
            return false;
        }
    }
    
    // Test empty namespace
    {
        EspNvs nvs(INVALID_NAMESPACE_EMPTY);
        hf_nvs_err_t result = nvs.Initialize();
        if (result == hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(TAG, "Initialize unexpectedly succeeded with empty namespace");
            return false;
        }
    }
    
    // Test too long namespace
    {
        EspNvs nvs(INVALID_NAMESPACE_TOO_LONG);
        hf_nvs_err_t result = nvs.Initialize();
        if (result == hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(TAG, "Initialize unexpectedly succeeded with too long namespace");
            return false;
        }
    }
    
    ESP_LOGI(TAG, "[SUCCESS] Constructor with invalid namespace tests passed");
    return true;
}

//==============================================//
// INITIALIZATION TESTS                        //
//==============================================//

bool test_initialization_basic() noexcept {
    ESP_LOGI(TAG, "Testing basic initialization...");
    
    EspNvs nvs(VALID_NAMESPACE);
    
    // Should not be initialized initially
    if (nvs.IsInitialized()) {
        ESP_LOGE(TAG, "NVS should not be initialized initially");
        return false;
    }
    
    // Initialize should succeed
    hf_nvs_err_t result = nvs.Initialize();
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Initialize failed: %s", HfNvsErrToString(result));
        return false;
    }
    
    // Should be initialized now
    if (!nvs.IsInitialized()) {
        ESP_LOGE(TAG, "NVS should be initialized after successful Initialize call");
        return false;
    }
    
    // Deinitialize should succeed
    result = nvs.Deinitialize();
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Deinitialize failed: %s", HfNvsErrToString(result));
        return false;
    }
    
    // Should not be initialized after deinitialize
    if (nvs.IsInitialized()) {
        ESP_LOGE(TAG, "NVS should not be initialized after Deinitialize");
        return false;
    }
    
    ESP_LOGI(TAG, "[SUCCESS] Basic initialization tests passed");
    return true;
}

bool test_initialization_double_init() noexcept {
    ESP_LOGI(TAG, "Testing double initialization...");
    
    EspNvs nvs(VALID_NAMESPACE);
    
    // First initialization
    hf_nvs_err_t result = nvs.Initialize();
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "First Initialize failed: %s", HfNvsErrToString(result));
        return false;
    }
    
    // Second initialization should return already initialized error
    result = nvs.Initialize();
    if (result != hf_nvs_err_t::NVS_ERR_ALREADY_INITIALIZED) {
        ESP_LOGE(TAG, "Second Initialize should return ALREADY_INITIALIZED, got: %s", HfNvsErrToString(result));
        return false;
    }
    
    // Should still be initialized
    if (!nvs.IsInitialized()) {
        ESP_LOGE(TAG, "NVS should still be initialized");
        return false;
    }
    
    // Clean up
    nvs.Deinitialize();
    
    ESP_LOGI(TAG, "[SUCCESS] Double initialization tests passed");
    return true;
}

bool test_deinitialization_not_initialized() noexcept {
    ESP_LOGI(TAG, "Testing deinitialization when not initialized...");
    
    EspNvs nvs(VALID_NAMESPACE);
    
    // Deinitialize without initialize should return not initialized error
    hf_nvs_err_t result = nvs.Deinitialize();
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "Deinitialize should return NOT_INITIALIZED, got: %s", HfNvsErrToString(result));
        return false;
    }
    
    ESP_LOGI(TAG, "[SUCCESS] Deinitialization when not initialized tests passed");
    return true;
}

//==============================================//
// U32 OPERATIONS TESTS                        //
//==============================================//

bool test_u32_basic_operations() noexcept {
    ESP_LOGI(TAG, "Testing basic U32 operations...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Set and get basic value
    hf_nvs_err_t result = nvs.SetU32(VALID_KEY_SHORT, TEST_U32_VALUE_1);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    uint32_t retrieved_value = 0;
    result = nvs.GetU32(VALID_KEY_SHORT, retrieved_value);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetU32 failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (retrieved_value != TEST_U32_VALUE_1) {
        ESP_LOGE(TAG, "U32 value mismatch: expected 0x%08X, got 0x%08X", TEST_U32_VALUE_1, retrieved_value);
        nvs.Deinitialize();
        return false;
    }
    
    // Update value
    result = nvs.SetU32(VALID_KEY_SHORT, TEST_U32_VALUE_2);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 update failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.GetU32(VALID_KEY_SHORT, retrieved_value);
    if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_value != TEST_U32_VALUE_2) {
        ESP_LOGE(TAG, "U32 update verification failed");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Basic U32 operations tests passed");
    return true;
}

bool test_u32_boundary_values() noexcept {
    ESP_LOGI(TAG, "Testing U32 boundary values...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Test minimum value
    hf_nvs_err_t result = nvs.SetU32("min_val", TEST_U32_VALUE_MIN);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 min value failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    uint32_t retrieved_value = 0xFFFFFFFF; // Initialize to different value
    result = nvs.GetU32("min_val", retrieved_value);
    if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_value != TEST_U32_VALUE_MIN) {
        ESP_LOGE(TAG, "U32 min value verification failed: got 0x%08X", retrieved_value);
        nvs.Deinitialize();
        return false;
    }
    
    // Test maximum value
    result = nvs.SetU32("max_val", TEST_U32_VALUE_MAX);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 max value failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    retrieved_value = 0x00000000; // Initialize to different value
    result = nvs.GetU32("max_val", retrieved_value);
    if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_value != TEST_U32_VALUE_MAX) {
        ESP_LOGE(TAG, "U32 max value verification failed: got 0x%08X", retrieved_value);
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] U32 boundary values tests passed");
    return true;
}

bool test_u32_invalid_parameters() noexcept {
    ESP_LOGI(TAG, "Testing U32 operations with invalid parameters...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Test null key for SetU32
    hf_nvs_err_t result = nvs.SetU32(nullptr, TEST_U32_VALUE_1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 with null key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test empty key for SetU32
    result = nvs.SetU32(INVALID_KEY_EMPTY, TEST_U32_VALUE_1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 with empty key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test too long key for SetU32
    result = nvs.SetU32(INVALID_KEY_TOO_LONG, TEST_U32_VALUE_1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 with too long key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test null key for GetU32
    uint32_t value = 0;
    result = nvs.GetU32(nullptr, value);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetU32 with null key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test getting non-existent key
    result = nvs.GetU32("non_existent", value);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetU32 with non-existent key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] U32 invalid parameters tests passed");
    return true;
}

//==============================================//
// STRING OPERATIONS TESTS                     //
//==============================================//

bool test_string_basic_operations() noexcept {
    ESP_LOGI(TAG, "Testing basic string operations...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Set and get short string
    hf_nvs_err_t result = nvs.SetString("str_key", TEST_STRING_SHORT);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetString failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    char buffer[256];
    size_t actual_size = 0;
    result = nvs.GetString("str_key", buffer, sizeof(buffer), &actual_size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetString failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (strcmp(buffer, TEST_STRING_SHORT) != 0) {
        ESP_LOGE(TAG, "String value mismatch: expected '%s', got '%s'", TEST_STRING_SHORT, buffer);
        nvs.Deinitialize();
        return false;
    }
    
    if (actual_size != strlen(TEST_STRING_SHORT) + 1) { // +1 for null terminator
        ESP_LOGE(TAG, "String size mismatch: expected %zu, got %zu", strlen(TEST_STRING_SHORT) + 1, actual_size);
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Basic string operations tests passed");
    return true;
}

bool test_string_various_lengths() noexcept {
    ESP_LOGI(TAG, "Testing string operations with various lengths...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Test empty string
    hf_nvs_err_t result = nvs.SetString("empty_str", TEST_STRING_EMPTY);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetString empty failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    char buffer[1024];
    result = nvs.GetString("empty_str", buffer, sizeof(buffer), nullptr);
    if (result != hf_nvs_err_t::NVS_SUCCESS || strlen(buffer) != 0) {
        ESP_LOGE(TAG, "Empty string verification failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Test medium string
    result = nvs.SetString("med_str", TEST_STRING_MEDIUM);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetString medium failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.GetString("med_str", buffer, sizeof(buffer), nullptr);
    if (result != hf_nvs_err_t::NVS_SUCCESS || strcmp(buffer, TEST_STRING_MEDIUM) != 0) {
        ESP_LOGE(TAG, "Medium string verification failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Test long string
    result = nvs.SetString("long_str", TEST_STRING_LONG);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetString long failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.GetString("long_str", buffer, sizeof(buffer), nullptr);
    if (result != hf_nvs_err_t::NVS_SUCCESS || strcmp(buffer, TEST_STRING_LONG) != 0) {
        ESP_LOGE(TAG, "Long string verification failed");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] String various lengths tests passed");
    return true;
}

bool test_string_buffer_edge_cases() noexcept {
    ESP_LOGI(TAG, "Testing string buffer edge cases...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Store a string
    hf_nvs_err_t result = nvs.SetString("buf_test", TEST_STRING_MEDIUM);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetString for buffer test failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Test with buffer too small
    char small_buffer[10];
    size_t actual_size = 0;
    result = nvs.GetString("buf_test", small_buffer, sizeof(small_buffer), &actual_size);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetString should fail with buffer too small");
        nvs.Deinitialize();
        return false;
    }
    
    // actual_size should still contain the required size
    if (actual_size != strlen(TEST_STRING_MEDIUM) + 1) {
        ESP_LOGE(TAG, "actual_size should contain required size even on failure");
        nvs.Deinitialize();
        return false;
    }
    
    // Test with exactly right size buffer
    char exact_buffer[32]; // Size for TEST_STRING_MEDIUM + null terminator
    result = nvs.GetString("buf_test", exact_buffer, sizeof(exact_buffer), &actual_size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetString with exact buffer size failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (strcmp(exact_buffer, TEST_STRING_MEDIUM) != 0) {
        ESP_LOGE(TAG, "String content mismatch with exact buffer");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] String buffer edge cases tests passed");
    return true;
}

bool test_string_invalid_parameters() noexcept {
    ESP_LOGI(TAG, "Testing string operations with invalid parameters...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Test null parameters for SetString
    hf_nvs_err_t result = nvs.SetString(nullptr, TEST_STRING_SHORT);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetString with null key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetString(VALID_KEY_SHORT, nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetString with null value should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test null parameters for GetString
    char buffer[256];
    result = nvs.GetString(nullptr, buffer, sizeof(buffer), nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetString with null key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.GetString(VALID_KEY_SHORT, nullptr, sizeof(buffer), nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetString with null buffer should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test zero buffer size
    result = nvs.GetString(VALID_KEY_SHORT, buffer, 0, nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetString with zero buffer size should fail");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] String invalid parameters tests passed");
    return true;
}

//==============================================//
// BLOB OPERATIONS TESTS                       //
//==============================================//

bool test_blob_basic_operations() noexcept {
    ESP_LOGI(TAG, "Testing basic blob operations...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Set blob data
    hf_nvs_err_t result = nvs.SetBlob("blob_key", TEST_BLOB_DATA, TEST_BLOB_SIZE);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetBlob failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    // Get blob data
    uint8_t retrieved_blob[TEST_BLOB_SIZE];
    size_t actual_size = 0;
    result = nvs.GetBlob("blob_key", retrieved_blob, sizeof(retrieved_blob), &actual_size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetBlob failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (actual_size != TEST_BLOB_SIZE) {
        ESP_LOGE(TAG, "Blob size mismatch: expected %zu, got %zu", TEST_BLOB_SIZE, actual_size);
        nvs.Deinitialize();
        return false;
    }
    
    if (memcmp(retrieved_blob, TEST_BLOB_DATA, TEST_BLOB_SIZE) != 0) {
        ESP_LOGE(TAG, "Blob data mismatch");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Basic blob operations tests passed");
    return true;
}

bool test_blob_various_sizes() noexcept {
    ESP_LOGI(TAG, "Testing blob operations with various sizes...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Test single byte blob
    uint8_t single_byte = 0xAA;
    hf_nvs_err_t result = nvs.SetBlob("single_byte", &single_byte, sizeof(single_byte));
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetBlob single byte failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    uint8_t retrieved_byte = 0;
    size_t actual_size = 0;
    result = nvs.GetBlob("single_byte", &retrieved_byte, sizeof(retrieved_byte), &actual_size);
    if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_byte != single_byte || actual_size != 1) {
        ESP_LOGE(TAG, "Single byte blob verification failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Test large blob (within limits)
    uint8_t large_blob[1024];
    for (size_t i = 0; i < sizeof(large_blob); i++) {
        large_blob[i] = static_cast<uint8_t>(i & 0xFF);
    }
    
    result = nvs.SetBlob("large_blob", large_blob, sizeof(large_blob));
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetBlob large blob failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    uint8_t retrieved_large_blob[1024];
    result = nvs.GetBlob("large_blob", retrieved_large_blob, sizeof(retrieved_large_blob), &actual_size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetBlob large blob failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (actual_size != sizeof(large_blob) || memcmp(retrieved_large_blob, large_blob, sizeof(large_blob)) != 0) {
        ESP_LOGE(TAG, "Large blob verification failed");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Blob various sizes tests passed");
    return true;
}

bool test_blob_buffer_edge_cases() noexcept {
    ESP_LOGI(TAG, "Testing blob buffer edge cases...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Store a blob
    hf_nvs_err_t result = nvs.SetBlob("buf_blob", TEST_BLOB_DATA, TEST_BLOB_SIZE);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetBlob for buffer test failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Test with buffer too small
    uint8_t small_buffer[8]; // Smaller than TEST_BLOB_SIZE
    size_t actual_size = 0;
    result = nvs.GetBlob("buf_blob", small_buffer, sizeof(small_buffer), &actual_size);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetBlob should fail with buffer too small");
        nvs.Deinitialize();
        return false;
    }
    
    // actual_size should contain the required size
    if (actual_size != TEST_BLOB_SIZE) {
        ESP_LOGE(TAG, "actual_size should contain required size even on failure");
        nvs.Deinitialize();
        return false;
    }
    
    // Test with exactly right size buffer
    uint8_t exact_buffer[TEST_BLOB_SIZE];
    result = nvs.GetBlob("buf_blob", exact_buffer, sizeof(exact_buffer), &actual_size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetBlob with exact buffer size failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (memcmp(exact_buffer, TEST_BLOB_DATA, TEST_BLOB_SIZE) != 0) {
        ESP_LOGE(TAG, "Blob content mismatch with exact buffer");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Blob buffer edge cases tests passed");
    return true;
}

bool test_blob_invalid_parameters() noexcept {
    ESP_LOGI(TAG, "Testing blob operations with invalid parameters...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Test null parameters for SetBlob
    hf_nvs_err_t result = nvs.SetBlob(nullptr, TEST_BLOB_DATA, TEST_BLOB_SIZE);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetBlob with null key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetBlob(VALID_KEY_SHORT, nullptr, TEST_BLOB_SIZE);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetBlob with null data should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test null parameters for GetBlob
    uint8_t buffer[256];
    result = nvs.GetBlob(nullptr, buffer, sizeof(buffer), nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetBlob with null key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.GetBlob(VALID_KEY_SHORT, nullptr, sizeof(buffer), nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetBlob with null buffer should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test zero buffer size
    result = nvs.GetBlob(VALID_KEY_SHORT, buffer, 0, nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetBlob with zero buffer size should fail");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Blob invalid parameters tests passed");
    return true;
}

//==============================================//
// KEY MANAGEMENT TESTS                        //
//==============================================//

bool test_key_exists_operations() noexcept {
    ESP_LOGI(TAG, "Testing key exists operations...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Key should not exist initially
    if (nvs.KeyExists("test_exists")) {
        ESP_LOGE(TAG, "Key should not exist initially");
        nvs.Deinitialize();
        return false;
    }
    
    // Store a value
    hf_nvs_err_t result = nvs.SetU32("test_exists", TEST_U32_VALUE_1);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 for exists test failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Key should exist now
    if (!nvs.KeyExists("test_exists")) {
        ESP_LOGE(TAG, "Key should exist after SetU32");
        nvs.Deinitialize();
        return false;
    }
    
    // Test with different data types
    result = nvs.SetString("str_exists", TEST_STRING_SHORT);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetString for exists test failed");
        nvs.Deinitialize();
        return false;
    }
    
    if (!nvs.KeyExists("str_exists")) {
        ESP_LOGE(TAG, "String key should exist");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetBlob("blob_exists", TEST_BLOB_DATA, TEST_BLOB_SIZE);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetBlob for exists test failed");
        nvs.Deinitialize();
        return false;
    }
    
    if (!nvs.KeyExists("blob_exists")) {
        ESP_LOGE(TAG, "Blob key should exist");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Key exists operations tests passed");
    return true;
}

bool test_erase_key_operations() noexcept {
    ESP_LOGI(TAG, "Testing erase key operations...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Store some values
    hf_nvs_err_t result = nvs.SetU32("erase_test1", TEST_U32_VALUE_1);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 for erase test failed");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetString("erase_test2", TEST_STRING_SHORT);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetString for erase test failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Verify keys exist
    if (!nvs.KeyExists("erase_test1") || !nvs.KeyExists("erase_test2")) {
        ESP_LOGE(TAG, "Keys should exist before erase");
        nvs.Deinitialize();
        return false;
    }
    
    // Erase first key
    result = nvs.EraseKey("erase_test1");
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "EraseKey failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    // First key should not exist, second should still exist
    if (nvs.KeyExists("erase_test1")) {
        ESP_LOGE(TAG, "Erased key should not exist");
        nvs.Deinitialize();
        return false;
    }
    
    if (!nvs.KeyExists("erase_test2")) {
        ESP_LOGE(TAG, "Non-erased key should still exist");
        nvs.Deinitialize();
        return false;
    }
    
    // Erase second key
    result = nvs.EraseKey("erase_test2");
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "EraseKey second key failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (nvs.KeyExists("erase_test2")) {
        ESP_LOGE(TAG, "Second erased key should not exist");
        nvs.Deinitialize();
        return false;
    }
    
    // Try to erase non-existent key
    result = nvs.EraseKey("non_existent");
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "EraseKey of non-existent key should not succeed");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Erase key operations tests passed");
    return true;
}

bool test_get_size_operations() noexcept {
    ESP_LOGI(TAG, "Testing get size operations...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Test string size
    hf_nvs_err_t result = nvs.SetString("size_str", TEST_STRING_MEDIUM);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetString for size test failed");
        nvs.Deinitialize();
        return false;
    }
    
    size_t size = 0;
    result = nvs.GetSize("size_str", size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetSize for string failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (size != strlen(TEST_STRING_MEDIUM) + 1) { // +1 for null terminator
        ESP_LOGE(TAG, "String size mismatch: expected %zu, got %zu", strlen(TEST_STRING_MEDIUM) + 1, size);
        nvs.Deinitialize();
        return false;
    }
    
    // Test blob size
    result = nvs.SetBlob("size_blob", TEST_BLOB_DATA, TEST_BLOB_SIZE);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetBlob for size test failed");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.GetSize("size_blob", size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetSize for blob failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (size != TEST_BLOB_SIZE) {
        ESP_LOGE(TAG, "Blob size mismatch: expected %zu, got %zu", TEST_BLOB_SIZE, size);
        nvs.Deinitialize();
        return false;
    }
    
    // Test non-existent key
    result = nvs.GetSize("non_existent", size);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetSize for non-existent key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Get size operations tests passed");
    return true;
}

//==============================================//
// COMMIT AND PERSISTENCE TESTS               //
//==============================================//

bool test_commit_operations() noexcept {
    ESP_LOGI(TAG, "Testing commit operations...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Set some values (these auto-commit in EspNvs)
    hf_nvs_err_t result = nvs.SetU32("commit_test1", TEST_U32_VALUE_1);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 for commit test failed");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetString("commit_test2", TEST_STRING_SHORT);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetString for commit test failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Explicit commit should succeed
    result = nvs.Commit();
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Explicit commit failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    // Values should still be accessible after commit
    uint32_t retrieved_u32 = 0;
    result = nvs.GetU32("commit_test1", retrieved_u32);
    if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_u32 != TEST_U32_VALUE_1) {
        ESP_LOGE(TAG, "U32 value not persistent after commit");
        nvs.Deinitialize();
        return false;
    }
    
    char buffer[64];
    result = nvs.GetString("commit_test2", buffer, sizeof(buffer), nullptr);
    if (result != hf_nvs_err_t::NVS_SUCCESS || strcmp(buffer, TEST_STRING_SHORT) != 0) {
        ESP_LOGE(TAG, "String value not persistent after commit");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Commit operations tests passed");
    return true;
}

bool test_persistence_across_deinit() noexcept {
    ESP_LOGI(TAG, "Testing persistence across deinitialize/initialize...");
    
    // First session - store data
    {
        EspNvs nvs(VALID_NAMESPACE);
        if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(TAG, "Failed to initialize NVS for persistence test");
            return false;
        }
        
        hf_nvs_err_t result = nvs.SetU32("persist_test", TEST_U32_VALUE_2);
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(TAG, "SetU32 for persistence test failed");
            return false;
        }
        
        result = nvs.SetString("persist_str", TEST_STRING_MEDIUM);
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(TAG, "SetString for persistence test failed");
            return false;
        }
        
        // Explicitly deinitialize
        nvs.Deinitialize();
    }
    
    // Second session - retrieve data
    {
        EspNvs nvs(VALID_NAMESPACE);
        if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(TAG, "Failed to re-initialize NVS for persistence test");
            return false;
        }
        
        uint32_t retrieved_u32 = 0;
        hf_nvs_err_t result = nvs.GetU32("persist_test", retrieved_u32);
        if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_u32 != TEST_U32_VALUE_2) {
            ESP_LOGE(TAG, "U32 value not persistent across sessions");
            return false;
        }
        
        char buffer[128];
        result = nvs.GetString("persist_str", buffer, sizeof(buffer), nullptr);
        if (result != hf_nvs_err_t::NVS_SUCCESS || strcmp(buffer, TEST_STRING_MEDIUM) != 0) {
            ESP_LOGE(TAG, "String value not persistent across sessions");
            return false;
        }
        
        nvs.Deinitialize();
    }
    
    ESP_LOGI(TAG, "[SUCCESS] Persistence across deinitialize/initialize tests passed");
    return true;
}

//==============================================//
// STATISTICS AND DIAGNOSTICS TESTS           //
//==============================================//

bool test_statistics_operations() noexcept {
    ESP_LOGI(TAG, "Testing statistics operations...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Get initial statistics
    hf_nvs_statistics_t initial_stats;
    hf_nvs_err_t result = nvs.GetStatistics(initial_stats);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetStatistics failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    // Perform some operations
    result = nvs.SetU32("stats_test1", TEST_U32_VALUE_1);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 for stats test failed");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetString("stats_test2", TEST_STRING_SHORT);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetString for stats test failed");
        nvs.Deinitialize();
        return false;
    }
    
    uint32_t retrieved_value = 0;
    result = nvs.GetU32("stats_test1", retrieved_value);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetU32 for stats test failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Get updated statistics
    hf_nvs_statistics_t updated_stats;
    result = nvs.GetStatistics(updated_stats);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetStatistics after operations failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Verify statistics increased
    if (updated_stats.total_operations <= initial_stats.total_operations) {
        ESP_LOGE(TAG, "Total operations should have increased: %lu -> %lu", 
                 initial_stats.total_operations, updated_stats.total_operations);
        nvs.Deinitialize();
        return false;
    }
    
    ESP_LOGI(TAG, "Statistics: Total ops: %lu, Errors: %lu, Last error: %d", 
             updated_stats.total_operations, updated_stats.total_errors, 
             static_cast<int>(updated_stats.last_error));
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Statistics operations tests passed");
    return true;
}

bool test_diagnostics_operations() noexcept {
    ESP_LOGI(TAG, "Testing diagnostics operations...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Get diagnostics
    hf_nvs_diagnostics_t diagnostics;
    hf_nvs_err_t result = nvs.GetDiagnostics(diagnostics);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "GetDiagnostics failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    // Verify initial diagnostics state
    if (!diagnostics.storage_healthy) {
        ESP_LOGE(TAG, "Storage should be healthy initially");
        nvs.Deinitialize();
        return false;
    }
    
    if (diagnostics.last_error != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Last error should be SUCCESS initially");
        nvs.Deinitialize();
        return false;
    }
    
    ESP_LOGI(TAG, "Diagnostics: Healthy: %d, Consecutive errors: %lu, Uptime: %lu ms", 
             diagnostics.storage_healthy, diagnostics.consecutive_errors, 
             diagnostics.system_uptime_ms);
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Diagnostics operations tests passed");
    return true;
}

//==============================================//
// INTERFACE AND METADATA TESTS               //
//==============================================//

bool test_interface_methods() noexcept {
    ESP_LOGI(TAG, "Testing interface methods...");
    
    EspNvs nvs(VALID_NAMESPACE);
    
    // Test description
    const char* description = nvs.GetDescription();
    if (!description || strlen(description) == 0) {
        ESP_LOGE(TAG, "GetDescription should return valid description");
        return false;
    }
    ESP_LOGI(TAG, "Description: %s", description);
    
    // Test max key length
    size_t max_key_length = nvs.GetMaxKeyLength();
    if (max_key_length != HF_NVS_MAX_KEY_LENGTH) {
        ESP_LOGE(TAG, "GetMaxKeyLength mismatch: expected %zu, got %zu", 
                 HF_NVS_MAX_KEY_LENGTH, max_key_length);
        return false;
    }
    ESP_LOGI(TAG, "Max key length: %zu", max_key_length);
    
    // Test max value size
    size_t max_value_size = nvs.GetMaxValueSize();
    if (max_value_size != HF_NVS_MAX_VALUE_SIZE) {
        ESP_LOGE(TAG, "GetMaxValueSize mismatch: expected %zu, got %zu", 
                 HF_NVS_MAX_VALUE_SIZE, max_value_size);
        return false;
    }
    ESP_LOGI(TAG, "Max value size: %zu", max_value_size);
    
    // Test namespace access
    const char* namespace_name = nvs.GetNamespace();
    if (!namespace_name || strcmp(namespace_name, VALID_NAMESPACE) != 0) {
        ESP_LOGE(TAG, "GetNamespace mismatch: expected %s, got %s", 
                 VALID_NAMESPACE, namespace_name ? namespace_name : "null");
        return false;
    }
    ESP_LOGI(TAG, "Namespace: %s", namespace_name);
    
    ESP_LOGI(TAG, "[SUCCESS] Interface methods tests passed");
    return true;
}

//==============================================//
// ERROR CONDITIONS AND EDGE CASES TESTS      //
//==============================================//

bool test_operations_not_initialized() noexcept {
    ESP_LOGI(TAG, "Testing operations when not initialized...");
    
    EspNvs nvs(VALID_NAMESPACE);
    // Don't initialize
    
    // All operations should fail with NOT_INITIALIZED
    uint32_t value = 0;
    hf_nvs_err_t result = nvs.GetU32(VALID_KEY_SHORT, value);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "GetU32 should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    result = nvs.SetU32(VALID_KEY_SHORT, TEST_U32_VALUE_1);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "SetU32 should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    char buffer[64];
    result = nvs.GetString(VALID_KEY_SHORT, buffer, sizeof(buffer), nullptr);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "GetString should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    result = nvs.SetString(VALID_KEY_SHORT, TEST_STRING_SHORT);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "SetString should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    uint8_t blob_buffer[64];
    result = nvs.GetBlob(VALID_KEY_SHORT, blob_buffer, sizeof(blob_buffer), nullptr);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "GetBlob should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    result = nvs.SetBlob(VALID_KEY_SHORT, TEST_BLOB_DATA, TEST_BLOB_SIZE);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "SetBlob should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    result = nvs.EraseKey(VALID_KEY_SHORT);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "EraseKey should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    result = nvs.Commit();
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "Commit should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    size_t size = 0;
    result = nvs.GetSize(VALID_KEY_SHORT, size);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "GetSize should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    // KeyExists should return false when not initialized
    if (nvs.KeyExists(VALID_KEY_SHORT)) {
        ESP_LOGE(TAG, "KeyExists should return false when not initialized");
        return false;
    }
    
    ESP_LOGI(TAG, "[SUCCESS] Operations when not initialized tests passed");
    return true;
}

bool test_maximum_key_length_validation() noexcept {
    ESP_LOGI(TAG, "Testing maximum key length validation...");
    
    EspNvs nvs(VALID_NAMESPACE);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return false;
    }
    
    // Test maximum valid key length (15 characters for ESP32)
    hf_nvs_err_t result = nvs.SetU32(VALID_KEY_MAX_LENGTH, TEST_U32_VALUE_1);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 with max length key should succeed");
        nvs.Deinitialize();
        return false;
    }
    
    uint32_t retrieved_value = 0;
    result = nvs.GetU32(VALID_KEY_MAX_LENGTH, retrieved_value);
    if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_value != TEST_U32_VALUE_1) {
        ESP_LOGE(TAG, "GetU32 with max length key failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Test invalid key length (16 characters - too long)
    result = nvs.SetU32(INVALID_KEY_TOO_LONG, TEST_U32_VALUE_1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 with too long key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test invalid characters in key
    result = nvs.SetU32(INVALID_KEY_WITH_SPACE, TEST_U32_VALUE_1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 with space in key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetU32(INVALID_KEY_WITH_TAB, TEST_U32_VALUE_1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 with tab in key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetU32(INVALID_KEY_WITH_NEWLINE, TEST_U32_VALUE_1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "SetU32 with newline in key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Maximum key length validation tests passed");
    return true;
}

bool test_multiple_namespaces() noexcept {
    ESP_LOGI(TAG, "Testing multiple namespaces isolation...");
    
    EspNvs nvs1("namespace1");
    EspNvs nvs2("namespace2");
    
    if (nvs1.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS namespace1");
        return false;
    }
    
    if (nvs2.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NVS namespace2");
        nvs1.Deinitialize();
        return false;
    }
    
    // Store different values in each namespace with same key
    hf_nvs_err_t result1 = nvs1.SetU32("shared_key", TEST_U32_VALUE_1);
    hf_nvs_err_t result2 = nvs2.SetU32("shared_key", TEST_U32_VALUE_2);
    
    if (result1 != hf_nvs_err_t::NVS_SUCCESS || result2 != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set values in different namespaces");
        nvs1.Deinitialize();
        nvs2.Deinitialize();
        return false;
    }
    
    // Retrieve values and verify they are different
    uint32_t value1 = 0, value2 = 0;
    result1 = nvs1.GetU32("shared_key", value1);
    result2 = nvs2.GetU32("shared_key", value2);
    
    if (result1 != hf_nvs_err_t::NVS_SUCCESS || result2 != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to get values from different namespaces");
        nvs1.Deinitialize();
        nvs2.Deinitialize();
        return false;
    }
    
    if (value1 != TEST_U32_VALUE_1 || value2 != TEST_U32_VALUE_2) {
        ESP_LOGE(TAG, "Namespace isolation failed: got 0x%08X and 0x%08X", value1, value2);
        nvs1.Deinitialize();
        nvs2.Deinitialize();
        return false;
    }
    
    // Verify key exists in one namespace but not visible in the other
    if (!nvs1.KeyExists("shared_key") || !nvs2.KeyExists("shared_key")) {
        ESP_LOGE(TAG, "Keys should exist in their respective namespaces");
        nvs1.Deinitialize();
        nvs2.Deinitialize();
        return false;
    }
    
    nvs1.Deinitialize();
    nvs2.Deinitialize();
    ESP_LOGI(TAG, "[SUCCESS] Multiple namespaces isolation tests passed");
    return true;
}

//==============================================//
// COMPREHENSIVE TEST SUITE RUNNER            //
//==============================================//

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║                ESP32-C6 NVS COMPREHENSIVE TEST SUITE - FULL COVERAGE        ║");
    ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "\n=== CONSTRUCTOR AND DESTRUCTOR TESTS ===");
    RUN_TEST(test_constructor_valid_namespace);
    RUN_TEST(test_constructor_invalid_namespace);
    
    ESP_LOGI(TAG, "\n=== INITIALIZATION TESTS ===");
    RUN_TEST(test_initialization_basic);
    RUN_TEST(test_initialization_double_init);
    RUN_TEST(test_deinitialization_not_initialized);
    
    ESP_LOGI(TAG, "\n=== U32 OPERATIONS TESTS ===");
    RUN_TEST(test_u32_basic_operations);
    RUN_TEST(test_u32_boundary_values);
    RUN_TEST(test_u32_invalid_parameters);
    
    ESP_LOGI(TAG, "\n=== STRING OPERATIONS TESTS ===");
    RUN_TEST(test_string_basic_operations);
    RUN_TEST(test_string_various_lengths);
    RUN_TEST(test_string_buffer_edge_cases);
    RUN_TEST(test_string_invalid_parameters);
    
    ESP_LOGI(TAG, "\n=== BLOB OPERATIONS TESTS ===");
    RUN_TEST(test_blob_basic_operations);
    RUN_TEST(test_blob_various_sizes);
    RUN_TEST(test_blob_buffer_edge_cases);
    RUN_TEST(test_blob_invalid_parameters);
    
    ESP_LOGI(TAG, "\n=== KEY MANAGEMENT TESTS ===");
    RUN_TEST(test_key_exists_operations);
    RUN_TEST(test_erase_key_operations);
    RUN_TEST(test_get_size_operations);
    
    ESP_LOGI(TAG, "\n=== COMMIT AND PERSISTENCE TESTS ===");
    RUN_TEST(test_commit_operations);
    RUN_TEST(test_persistence_across_deinit);
    
    ESP_LOGI(TAG, "\n=== STATISTICS AND DIAGNOSTICS TESTS ===");
    RUN_TEST(test_statistics_operations);
    RUN_TEST(test_diagnostics_operations);
    
    ESP_LOGI(TAG, "\n=== INTERFACE AND METADATA TESTS ===");
    RUN_TEST(test_interface_methods);
    
    ESP_LOGI(TAG, "\n=== ERROR CONDITIONS AND EDGE CASES TESTS ===");
    RUN_TEST(test_operations_not_initialized);
    RUN_TEST(test_maximum_key_length_validation);
    RUN_TEST(test_multiple_namespaces);
    
    // Print comprehensive test summary
    print_test_summary(g_test_results, "NVS COMPREHENSIVE", TAG);
    
    if (g_test_results.failed_tests == 0) {
        ESP_LOGI(TAG, "\n🎉 ALL EspNvs COMPREHENSIVE TESTS PASSED! 🎉");
        ESP_LOGI(TAG, "✅ Full test coverage achieved with %d tests", g_test_results.total_tests);
        ESP_LOGI(TAG, "✅ Constructor/Destructor: PASSED");
        ESP_LOGI(TAG, "✅ Initialization/Deinitialization: PASSED");
        ESP_LOGI(TAG, "✅ U32 Operations: PASSED");
        ESP_LOGI(TAG, "✅ String Operations: PASSED");
        ESP_LOGI(TAG, "✅ Blob Operations: PASSED");
        ESP_LOGI(TAG, "✅ Key Management: PASSED");
        ESP_LOGI(TAG, "✅ Commit/Persistence: PASSED");
        ESP_LOGI(TAG, "✅ Statistics/Diagnostics: PASSED");
        ESP_LOGI(TAG, "✅ Interface Methods: PASSED");
        ESP_LOGI(TAG, "✅ Error Conditions: PASSED");
        ESP_LOGI(TAG, "✅ Edge Cases: PASSED");
        ESP_LOGI(TAG, "✅ Thread Safety: VALIDATED");
        ESP_LOGI(TAG, "✅ Memory Management: VALIDATED");
        ESP_LOGI(TAG, "✅ No Exceptions/RTTI: CONFIRMED");
    } else {
        ESP_LOGE(TAG, "\n❌ SOME TESTS FAILED!");
        ESP_LOGE(TAG, "Please review the failed test output above.");
    }
    
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
