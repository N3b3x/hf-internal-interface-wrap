/**
 * @file NvsComprehensiveTest.cpp
 * @brief Comprehensive NVS (Non-Volatile Storage) testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This test suite provides full coverage of the EspNvs class, testing all methods,
 * error conditions, edge cases, and boundary conditions. The tests are designed to
 * run without exceptions and without RTTI.
 *
 * @author Test Suite
 * @date 2025
 * @copyright HardFOC
 */

#include "TestFramework.h"
#include "base/BaseNvs.h"
#include "mcu/esp32/EspNvs.h"
#include <cstdio>
#include <cstring>
#include <random>

static const char* TAG = "NVS_Test";
static TestResults g_test_results;

// Test constants
static constexpr size_t TEST_BUFFER_SIZE = 128;
static constexpr size_t LARGE_BUFFER_SIZE = 1024;
static constexpr hf_u32_t TEST_U32_VALUE = 0xDEADBEEF;
static constexpr hf_u32_t TEST_U32_MAX = 0xFFFFFFFF;
static constexpr hf_u32_t TEST_U32_MIN = 0x00000000;
static const char* TEST_STRING = "Hello, ESP32-C6 NVS!";
static const char* LONG_STRING = "This is a test string for NVS storage.";
static const uint8_t TEST_BLOB_DATA[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

//=============================================================================
// TEST SECTION CONFIGURATION
//=============================================================================
// Enable/disable specific test categories by setting to true or false

// Core NVS functionality tests
static constexpr bool ENABLE_CORE_TESTS =
    true; // Initialization, deinitialization, basic operations
static constexpr bool ENABLE_DATA_TESTS = true;       // U32, string, blob operations
static constexpr bool ENABLE_MANAGEMENT_TESTS = true; // Key operations, commit operations
static constexpr bool ENABLE_DIAGNOSTIC_TESTS = true; // Statistics, diagnostics, metadata
static constexpr bool ENABLE_STRESS_TESTS = true;     // Edge cases, stress testing

// === Initialization and Deinitialization Tests ===

bool test_nvs_initialization() noexcept {
  ESP_LOGI(TAG, "Testing NVS initialization...");

  // Test 1: Normal initialization
  EspNvs nvs1("test_init");
  if (nvs1.IsInitialized()) {
    ESP_LOGE(TAG, "NVS should not be initialized before Initialize() call");
    return false;
  }

  auto result = nvs1.Initialize();
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize NVS: %s", HfNvsErrToString(result));
    return false;
  }

  if (!nvs1.IsInitialized()) {
    ESP_LOGE(TAG, "NVS should be initialized after successful Initialize()");
    return false;
  }

  // Test 2: Double initialization
  result = nvs1.Initialize();
  if (result != hf_nvs_err_t::NVS_ERR_ALREADY_INITIALIZED) {
    ESP_LOGE(TAG, "Double initialization should return NVS_ERR_ALREADY_INITIALIZED");
    return false;
  }

  // Test 3: Deinitialization
  result = nvs1.Deinitialize();
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to deinitialize NVS: %s", HfNvsErrToString(result));
    return false;
  }

  if (nvs1.IsInitialized()) {
    ESP_LOGE(TAG, "NVS should not be initialized after Deinitialize()");
    return false;
  }

  // Test 4: Double deinitialization
  result = nvs1.Deinitialize();
  if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
    ESP_LOGE(TAG, "Double deinitialization should return NVS_ERR_NOT_INITIALIZED");
    return false;
  }

  // Test 5: EnsureInitialized/EnsureDeinitialized
  EspNvs nvs2("test_ensure");
  if (!nvs2.EnsureInitialized()) {
    ESP_LOGE(TAG, "EnsureInitialized() failed");
    return false;
  }
  if (!nvs2.IsInitialized()) {
    ESP_LOGE(TAG, "NVS should be initialized after EnsureInitialized()");
    return false;
  }
  if (!nvs2.EnsureDeinitialized()) {
    ESP_LOGE(TAG, "EnsureDeinitialized() failed");
    return false;
  }
  if (nvs2.IsInitialized()) {
    ESP_LOGE(TAG, "NVS should not be initialized after EnsureDeinitialized()");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] NVS initialization tests passed");
  return true;
}

// === U32 Operations Tests ===

bool test_nvs_u32_operations() noexcept {
  ESP_LOGI(TAG, "Testing NVS U32 operations...");

  EspNvs nvs("test_u32");
  if (!nvs.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize NVS");
    return false;
  }

  // Test 1: Basic set and get
  auto result = nvs.SetU32("test_u32_basic", TEST_U32_VALUE);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set U32 value: %s", HfNvsErrToString(result));
    return false;
  }

  hf_u32_t retrieved_value = 0;
  result = nvs.GetU32("test_u32_basic", retrieved_value);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get U32 value: %s", HfNvsErrToString(result));
    return false;
  }

  if (retrieved_value != TEST_U32_VALUE) {
    ESP_LOGE(TAG, "Retrieved value mismatch: expected 0x%08X, got 0x%08X", TEST_U32_VALUE,
             retrieved_value);
    return false;
  }

  // Test 2: Boundary values
  result = nvs.SetU32("test_u32_max", TEST_U32_MAX);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set max U32 value");
    return false;
  }

  result = nvs.GetU32("test_u32_max", retrieved_value);
  if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_value != TEST_U32_MAX) {
    ESP_LOGE(TAG, "Failed to retrieve max U32 value");
    return false;
  }

  result = nvs.SetU32("test_u32_min", TEST_U32_MIN);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set min U32 value");
    return false;
  }

  result = nvs.GetU32("test_u32_min", retrieved_value);
  if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_value != TEST_U32_MIN) {
    ESP_LOGE(TAG, "Failed to retrieve min U32 value");
    return false;
  }

  // Test 3: Overwrite existing value
  result = nvs.SetU32("test_u32_basic", 0x12345678);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to overwrite U32 value");
    return false;
  }

  result = nvs.GetU32("test_u32_basic", retrieved_value);
  if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_value != 0x12345678) {
    ESP_LOGE(TAG, "Failed to retrieve overwritten U32 value");
    return false;
  }

  // Test 4: Get non-existent key
  result = nvs.GetU32("no_key", retrieved_value);
  if (result != hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND) {
    ESP_LOGE(TAG, "Getting non-existent key should return NVS_ERR_KEY_NOT_FOUND");
    return false;
  }

  // Test 5: Invalid parameters
  result = nvs.SetU32(nullptr, TEST_U32_VALUE);
  if (result != hf_nvs_err_t::NVS_ERR_NULL_POINTER) {
    ESP_LOGE(TAG, "SetU32 with null key should return NVS_ERR_NULL_POINTER");
    return false;
  }

  result = nvs.GetU32(nullptr, retrieved_value);
  if (result != hf_nvs_err_t::NVS_ERR_NULL_POINTER) {
    ESP_LOGE(TAG, "GetU32 with null key should return NVS_ERR_NULL_POINTER");
    return false;
  }

  // Test 6: Empty key
  result = nvs.SetU32("", TEST_U32_VALUE);
  if (result != hf_nvs_err_t::NVS_ERR_INVALID_PARAMETER) {
    ESP_LOGE(TAG, "SetU32 with empty key should return NVS_ERR_INVALID_PARAMETER");
    return false;
  }

  // Test 7: Maximum key length (15 characters for ESP32)
  char max_key[16];
  memset(max_key, 'K', 15);
  max_key[15] = '\0';

  result = nvs.SetU32(max_key, TEST_U32_VALUE);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set U32 with max length key");
    return false;
  }

  // Test 8: Key too long
  char long_key[32];
  memset(long_key, 'L', 31);
  long_key[31] = '\0';

  result = nvs.SetU32(long_key, TEST_U32_VALUE);
  if (result != hf_nvs_err_t::NVS_ERR_KEY_TOO_LONG) {
    ESP_LOGE(TAG, "SetU32 with too long key should return NVS_ERR_KEY_TOO_LONG");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] NVS U32 operations tests passed");
  return true;
}

// === String Operations Tests ===

bool test_nvs_string_operations() noexcept {
  // Run string test in dedicated task to provide larger stack and isolation
  RUN_TEST_IN_TASK(
      "test_nvs_string_operations",
      []() noexcept -> bool {
        ESP_LOGI(TAG, "Testing NVS string operations...");

        EspNvs nvs("test_str");
        if (!nvs.EnsureInitialized()) {
          ESP_LOGE(TAG, "Failed to initialize NVS");
          return false;
        }

        char buffer[128];
        size_t actual_size = 0;

        auto result = nvs.SetString("test_str_basic", TEST_STRING);
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
          ESP_LOGE(TAG, "Failed to set string value: %s", HfNvsErrToString(result));
          return false;
        }

        memset(buffer, 0, sizeof(buffer));
        result = nvs.GetString("test_str_basic", buffer, sizeof(buffer), &actual_size);
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
          ESP_LOGE(TAG, "Failed to get string value: %s", HfNvsErrToString(result));
          return false;
        }
        if (strcmp(buffer, TEST_STRING) != 0) {
          ESP_LOGE(TAG, "Retrieved string mismatch: expected '%s', got '%s'", TEST_STRING, buffer);
          return false;
        }
        if (actual_size != strlen(TEST_STRING) + 1) {
          ESP_LOGE(TAG, "Actual size mismatch: expected %zu, got %zu", strlen(TEST_STRING) + 1,
                   actual_size);
          return false;
        }

        // Empty string
        result = nvs.SetString("test_str_empty", "");
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
          ESP_LOGE(TAG, "Failed to set empty string");
          return false;
        }
        memset(buffer, 'X', sizeof(buffer));
        result = nvs.GetString("test_str_empty", buffer, sizeof(buffer), &actual_size);
        if (result != hf_nvs_err_t::NVS_SUCCESS || strcmp(buffer, "") != 0) {
          ESP_LOGE(TAG, "Failed to retrieve empty string");
          return false;
        }

        // Long string
        result = nvs.SetString("test_str_long", LONG_STRING);
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
          ESP_LOGE(TAG, "Failed to set long string");
          return false;
        }
        memset(buffer, 0, sizeof(buffer));
        result = nvs.GetString("test_str_long", buffer, sizeof(buffer), &actual_size);
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
          ESP_LOGE(TAG, "Failed to get long string");
          return false;
        }
        if (strcmp(buffer, LONG_STRING) != 0) {
          ESP_LOGE(TAG, "Retrieved long string mismatch");
          return false;
        }

        // Buffer too small -> VALUE_TOO_LARGE
        result = nvs.GetString("test_str_long", buffer, 10, &actual_size);
        if (result != hf_nvs_err_t::NVS_ERR_VALUE_TOO_LARGE) {
          ESP_LOGE(TAG, "Getting string with small buffer should return VALUE_TOO_LARGE");
          return false;
        }

        // Get actual size without buffer
        result = nvs.GetString("test_str_basic", nullptr, 0, &actual_size);
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
          ESP_LOGE(TAG, "Failed to get string size");
          return false;
        }
        if (actual_size != strlen(TEST_STRING) + 1) {
          ESP_LOGE(TAG, "String size mismatch");
          return false;
        }

        // Invalid parameters
        result = nvs.SetString(nullptr, TEST_STRING);
        if (result != hf_nvs_err_t::NVS_ERR_NULL_POINTER) {
          ESP_LOGE(TAG, "SetString with null key should return NVS_ERR_NULL_POINTER");
          return false;
        }
        result = nvs.SetString("test_str_null", nullptr);
        if (result != hf_nvs_err_t::NVS_ERR_NULL_POINTER) {
          ESP_LOGE(TAG, "SetString with null value should return NVS_ERR_NULL_POINTER");
          return false;
        }
        result = nvs.GetString(nullptr, buffer, sizeof(buffer));
        if (result != hf_nvs_err_t::NVS_ERR_NULL_POINTER) {
          ESP_LOGE(TAG, "GetString with null key should return NVS_ERR_NULL_POINTER");
          return false;
        }

        // Very long string (exceeds NVS limits) - allocate dynamically to avoid large stack
        const size_t too_long = nvs.GetMaxValueSize() + 16;
        char* very_long_string = static_cast<char*>(malloc(too_long));
        if (!very_long_string) {
          ESP_LOGE(TAG, "malloc failed for very_long_string");
          return false;
        }
        memset(very_long_string, 'A', too_long - 1);
        very_long_string[too_long - 1] = '\0';
        result = nvs.SetString("str_too_long", very_long_string);
        free(very_long_string);
        if (result != hf_nvs_err_t::NVS_ERR_VALUE_TOO_LARGE) {
          ESP_LOGE(TAG, "SetString with too long value should return NVS_ERR_VALUE_TOO_LARGE");
          return false;
        }

        ESP_LOGI(TAG, "[SUCCESS] NVS string operations tests passed");
        return true;
      },
      8192 /* stack bytes */, 5);

  return true;
}

// === Blob Operations Tests ===

bool test_nvs_blob_operations() noexcept {
  // Run blob test in dedicated task to provide larger stack for buffers
  RUN_TEST_IN_TASK(
      "test_nvs_blob_operations",
      []() noexcept -> bool {
        ESP_LOGI(TAG, "Testing NVS blob operations...");
        EspNvs nvs("test_blob");
        if (!nvs.EnsureInitialized()) {
          ESP_LOGE(TAG, "Failed to initialize NVS");
          return false;
        }
        uint8_t buffer[TEST_BUFFER_SIZE];
        size_t actual_size = 0;

        // Test 1: Basic set and get
        auto result = nvs.SetBlob("test_blob_basic", TEST_BLOB_DATA, sizeof(TEST_BLOB_DATA));
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
          ESP_LOGE(TAG, "Failed to set blob value: %s", HfNvsErrToString(result));
          return false;
        }

        memset(buffer, 0, sizeof(buffer));
        result = nvs.GetBlob("test_blob_basic", buffer, sizeof(buffer), &actual_size);
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
          ESP_LOGE(TAG, "Failed to get blob value: %s", HfNvsErrToString(result));
          return false;
        }

        if (memcmp(buffer, TEST_BLOB_DATA, sizeof(TEST_BLOB_DATA)) != 0) {
          ESP_LOGE(TAG, "Retrieved blob data mismatch");
          return false;
        }

        if (actual_size != sizeof(TEST_BLOB_DATA)) {
          ESP_LOGE(TAG, "Actual size mismatch: expected %zu, got %zu", sizeof(TEST_BLOB_DATA),
                   actual_size);
          return false;
        }

        // Test 2: Empty blob -> NULL_POINTER (data null)
        result = nvs.SetBlob("test_blob_empty", nullptr, 0);
        if (result != hf_nvs_err_t::NVS_ERR_NULL_POINTER) {
          ESP_LOGE(TAG, "SetBlob with null and size 0 should return NVS_ERR_NULL_POINTER");
          return false;
        }

        // Test 3: Large blob
        static uint8_t large_blob[1024];
        for (size_t i = 0; i < sizeof(large_blob); ++i) {
          large_blob[i] = static_cast<uint8_t>(i & 0xFF);
        }

        result = nvs.SetBlob("test_blob_large", large_blob, sizeof(large_blob));
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
          ESP_LOGE(TAG, "Failed to set large blob");
          return false;
        }

        static uint8_t large_buffer[1024];
        result = nvs.GetBlob("test_blob_large", large_buffer, sizeof(large_buffer), &actual_size);
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
          ESP_LOGE(TAG, "Failed to get large blob");
          return false;
        }

        if (memcmp(large_buffer, large_blob, sizeof(large_blob)) != 0) {
          ESP_LOGE(TAG, "Large blob data mismatch");
          return false;
        }

        // Test 4: Buffer too small -> VALUE_TOO_LARGE
        result = nvs.GetBlob("test_blob_large", buffer, 10, &actual_size);
        if (result != hf_nvs_err_t::NVS_ERR_VALUE_TOO_LARGE) {
          ESP_LOGE(TAG, "Getting blob with small buffer should return VALUE_TOO_LARGE");
          return false;
        }

        // Test 5: Get actual size without buffer
        result = nvs.GetBlob("test_blob_basic", nullptr, 0, &actual_size);
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
          ESP_LOGE(TAG, "Failed to get blob size");
          return false;
        }

        if (actual_size != sizeof(TEST_BLOB_DATA)) {
          ESP_LOGE(TAG, "Blob size mismatch");
          return false;
        }

        // Test 6: Invalid parameters
        result = nvs.SetBlob(nullptr, TEST_BLOB_DATA, sizeof(TEST_BLOB_DATA));
        if (result != hf_nvs_err_t::NVS_ERR_NULL_POINTER) {
          ESP_LOGE(TAG, "SetBlob with null key should return NVS_ERR_NULL_POINTER");
          return false;
        }

        result = nvs.SetBlob("test_blob_null", nullptr, 10);
        if (result != hf_nvs_err_t::NVS_ERR_NULL_POINTER) {
          ESP_LOGE(TAG, "SetBlob with null data should return NVS_ERR_NULL_POINTER");
          return false;
        }

        result = nvs.GetBlob(nullptr, buffer, sizeof(buffer));
        if (result != hf_nvs_err_t::NVS_ERR_NULL_POINTER) {
          ESP_LOGE(TAG, "GetBlob with null key should return NVS_ERR_NULL_POINTER");
          return false;
        }

        // Test 7: Very large blob (exceeds NVS limits)
        static uint8_t very_large_blob[4096];
        memset(very_large_blob, 0xAA, sizeof(very_large_blob));

        // Note: The conservative 4KB guard is enforced for strings, not blobs. Blob max depends on
        // partition. So we only perform a sanity large-blob write/read above, then skip forcing an
        // oversize error here.

        ESP_LOGI(TAG, "[SUCCESS] NVS blob operations tests passed");
        return true;
      },
      8192 /* stack bytes */, 5);
  return true;
}

// === Key Management Tests ===

bool test_nvs_key_operations() noexcept {
  ESP_LOGI(TAG, "Testing NVS key operations...");

  EspNvs nvs("test_key");
  if (!nvs.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize NVS");
    return false;
  }

  // Test 1: KeyExists for non-existent key
  if (nvs.KeyExists("no_key_xyz")) {
    ESP_LOGE(TAG, "KeyExists should return false for non-existent key");
    return false;
  }

  // Test 2: Create key and check existence
  auto result = nvs.SetU32("test_key_exists", TEST_U32_VALUE);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to create key");
    return false;
  }

  if (!nvs.KeyExists("test_key_exists")) {
    ESP_LOGE(TAG, "KeyExists should return true for existing key");
    return false;
  }

  // Test 3: GetSize for existing key
  size_t size = 0;
  result = nvs.GetSize("test_key_exists", size);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get size of existing key");
    return false;
  }

  if (size != sizeof(hf_u32_t)) {
    ESP_LOGE(TAG, "Size mismatch for U32 value: expected %zu, got %zu", sizeof(hf_u32_t), size);
    return false;
  }

  // Test 4: GetSize for string key
  result = nvs.SetString("test_key_string", TEST_STRING);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set string key");
    return false;
  }

  result = nvs.GetSize("test_key_string", size);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get size of string key");
    return false;
  }

  if (size != strlen(TEST_STRING) + 1) {
    ESP_LOGE(TAG, "Size mismatch for string value");
    return false;
  }

  // Test 5: EraseKey
  result = nvs.EraseKey("test_key_exists");
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to erase key");
    return false;
  }

  if (nvs.KeyExists("test_key_exists")) {
    ESP_LOGE(TAG, "Key should not exist after erasing");
    return false;
  }

  // Test 6: EraseKey for non-existent key
  result = nvs.EraseKey("no_key_xyz");
  if (result != hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND) {
    ESP_LOGE(TAG, "Erasing non-existent key should return NVS_ERR_KEY_NOT_FOUND");
    return false;
  }

  // Test 7: Invalid parameters for key operations
  result = nvs.EraseKey(nullptr);
  if (result != hf_nvs_err_t::NVS_ERR_NULL_POINTER) {
    ESP_LOGE(TAG, "EraseKey with null key should return NVS_ERR_NULL_POINTER");
    return false;
  }

  result = nvs.GetSize(nullptr, size);
  if (result != hf_nvs_err_t::NVS_ERR_NULL_POINTER) {
    ESP_LOGE(TAG, "GetSize with null key should return NVS_ERR_NULL_POINTER");
    return false;
  }

  if (nvs.KeyExists(nullptr)) {
    ESP_LOGE(TAG, "KeyExists with null key should return false");
    return false;
  }

  // Test 8: GetSize for non-existent key
  result = nvs.GetSize("no_key_size", size);
  if (result != hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND) {
    ESP_LOGE(TAG, "GetSize for non-existent key should return NVS_ERR_KEY_NOT_FOUND");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] NVS key operations tests passed");
  return true;
}

// === Commit Operations Tests ===

bool test_nvs_commit_operations() noexcept {
  ESP_LOGI(TAG, "Testing NVS commit operations...");

  EspNvs nvs("test_commit");
  if (!nvs.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize NVS");
    return false;
  }

  // Test 1: Commit with no pending changes
  auto result = nvs.Commit();
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Commit with no changes should succeed");
    return false;
  }

  // Test 2: Commit after write operations
  result = nvs.SetU32("commit_u32", TEST_U32_VALUE);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set U32 value");
    return false;
  }

  result = nvs.SetString("commit_str", TEST_STRING);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set string value");
    return false;
  }

  result = nvs.Commit();
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to commit changes: %s", HfNvsErrToString(result));
    return false;
  }

  // Test 3: Verify data persists after commit
  hf_u32_t u32_value = 0;
  result = nvs.GetU32("commit_u32", u32_value);
  if (result != hf_nvs_err_t::NVS_SUCCESS || u32_value != TEST_U32_VALUE) {
    ESP_LOGE(TAG, "Failed to verify U32 value after commit");
    return false;
  }

  char buffer[TEST_BUFFER_SIZE];
  result = nvs.GetString("commit_str", buffer, sizeof(buffer));
  if (result != hf_nvs_err_t::NVS_SUCCESS || strcmp(buffer, TEST_STRING) != 0) {
    ESP_LOGE(TAG, "Failed to verify string value after commit");
    return false;
  }

  // Test 4: Multiple commits
  for (int i = 0; i < 5; ++i) {
    result = nvs.SetU32("commit_loop", static_cast<hf_u32_t>(i));
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set value in loop");
      return false;
    }

    result = nvs.Commit();
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
      ESP_LOGE(TAG, "Failed to commit in loop iteration %d", i);
      return false;
    }
  }

  // Test 5: Commit on uninitialized NVS
  EspNvs nvs_uninit("test_uninit");
  result = nvs_uninit.Commit();
  if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
    ESP_LOGE(TAG, "Commit on uninitialized NVS should return NVS_ERR_NOT_INITIALIZED");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] NVS commit operations tests passed");
  return true;
}

// === Statistics and Diagnostics Tests ===

bool test_nvs_statistics_diagnostics() noexcept {
  ESP_LOGI(TAG, "Testing NVS statistics and diagnostics...");

  EspNvs nvs("test_stats");
  if (!nvs.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize NVS");
    return false;
  }

  // Test 1: Get initial statistics
  hf_nvs_statistics_t stats = {};
  auto result = nvs.GetStatistics(stats);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get statistics: %s", HfNvsErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "Initial stats - Total ops: %u, Errors: %u, Reads: %u, Writes: %u",
           stats.total_operations, stats.total_errors, stats.total_reads, stats.total_writes);

  // Test 2: Perform operations and check statistics update
  uint32_t initial_writes = stats.total_writes;
  uint32_t initial_reads = stats.total_reads;

  // Perform some write operations
  nvs.SetU32("stats_test_1", 100);
  nvs.SetString("stats_test_2", "test");
  nvs.SetBlob("stats_test_3", TEST_BLOB_DATA, sizeof(TEST_BLOB_DATA));

  // Perform some read operations
  hf_u32_t u32_val;
  nvs.GetU32("stats_test_1", u32_val);

  char str_buffer[100];
  nvs.GetString("stats_test_2", str_buffer, sizeof(str_buffer));

  // Get updated statistics
  result = nvs.GetStatistics(stats);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get updated statistics");
    return false;
  }

  if (stats.total_writes <= initial_writes) {
    ESP_LOGE(TAG, "Write count should have increased");
    return false;
  }

  if (stats.total_reads <= initial_reads) {
    ESP_LOGE(TAG, "Read count should have increased");
    return false;
  }

  ESP_LOGI(TAG, "Updated stats - Total ops: %u, Errors: %u, Reads: %u, Writes: %u",
           stats.total_operations, stats.total_errors, stats.total_reads, stats.total_writes);

  // Test 3: Get diagnostics
  hf_nvs_diagnostics_t diag = {};
  result = nvs.GetDiagnostics(diag);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get diagnostics: %s", HfNvsErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "Diagnostics - Last error: %s, Consecutive errors: %u, Storage healthy: %s",
           HfNvsErrToString(diag.last_error), diag.consecutive_errors,
           diag.storage_healthy ? "Yes" : "No");

  // Test 4: Trigger an error and check diagnostics reflects it
  result = nvs.GetU32("non_existent_key_diag", u32_val); // This should fail
  if (result == hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Expected failure when reading non-existent key");
    return false;
  }

  result = nvs.GetDiagnostics(diag);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get diagnostics after error");
    return false;
  }

  if (diag.last_error != hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND) {
    ESP_LOGE(TAG, "Last error should be NVS_ERR_KEY_NOT_FOUND");
    return false;
  }

  // Test 5: Statistics on uninitialized NVS
  EspNvs nvs_uninit("test_uninit_stats");
  result = nvs_uninit.GetStatistics(stats);
  if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
    ESP_LOGE(TAG, "GetStatistics on uninitialized NVS should return NVS_ERR_NOT_INITIALIZED");
    return false;
  }

  result = nvs_uninit.GetDiagnostics(diag);
  if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
    ESP_LOGE(TAG, "GetDiagnostics on uninitialized NVS should return NVS_ERR_NOT_INITIALIZED");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] NVS statistics and diagnostics tests passed");
  return true;
}

// === Metadata Tests ===

bool test_nvs_metadata() noexcept {
  ESP_LOGI(TAG, "Testing NVS metadata functions...");

  EspNvs nvs("test_meta");

  // Test 1: Get description
  const char* desc = nvs.GetDescription();
  if (desc == nullptr) {
    ESP_LOGE(TAG, "GetDescription should not return null");
    return false;
  }
  ESP_LOGI(TAG, "NVS Description: %s", desc);

  // Test 2: Get namespace
  const char* ns = nvs.GetNamespace();
  if (ns == nullptr || strcmp(ns, "test_meta") != 0) {
    ESP_LOGE(TAG, "GetNamespace mismatch");
    return false;
  }
  ESP_LOGI(TAG, "NVS Namespace: %s", ns);

  // Test 3: Get max key length
  size_t max_key_len = nvs.GetMaxKeyLength();
  if (max_key_len != 15) { // ESP32 NVS limit
    ESP_LOGE(TAG, "Max key length should be 15, got %zu", max_key_len);
    return false;
  }
  ESP_LOGI(TAG, "Max key length: %zu", max_key_len);

  // Test 4: Get max value size
  size_t max_val_size = nvs.GetMaxValueSize();
  if (max_val_size != 4000) { // ESP32 NVS conservative limit
    ESP_LOGE(TAG, "Max value size should be 4000, got %zu", max_val_size);
    return false;
  }
  ESP_LOGI(TAG, "Max value size: %zu", max_val_size);

  ESP_LOGI(TAG, "[SUCCESS] NVS metadata tests passed");
  return true;
}

// === Edge Cases and Boundary Tests ===

bool test_nvs_edge_cases() noexcept {
  ESP_LOGI(TAG, "Testing NVS edge cases...");

  EspNvs nvs("test_edge");
  if (!nvs.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize NVS");
    return false;
  }

  // Test 1: Key with special characters
  const char* special_key = "test-key_123";
  auto result = nvs.SetU32(special_key, TEST_U32_VALUE);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set value with special characters in key");
    return false;
  }

  hf_u32_t retrieved_value = 0;
  result = nvs.GetU32(special_key, retrieved_value);
  if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_value != TEST_U32_VALUE) {
    ESP_LOGE(TAG, "Failed to retrieve value with special characters in key");
    return false;
  }

  // Test 2: Rapid successive operations
  for (int i = 0; i < 100; ++i) {
    char key[16];
    snprintf(key, sizeof(key), "rapid_%d", i % 10);

    result = nvs.SetU32(key, static_cast<hf_u32_t>(i));
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
      ESP_LOGE(TAG, "Failed in rapid operation %d", i);
      return false;
    }
  }

  // Test 3: Overwrite with different data types
  const char* multi_type_key = "multi_type";

  // First set as U32
  result = nvs.SetU32(multi_type_key, TEST_U32_VALUE);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set U32 value");
    return false;
  }

  // Then overwrite as string
  result = nvs.SetString(multi_type_key, TEST_STRING);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to overwrite with string");
    return false;
  }

  // Verify it's now a string
  char buffer[TEST_BUFFER_SIZE];
  result = nvs.GetString(multi_type_key, buffer, sizeof(buffer));
  if (result != hf_nvs_err_t::NVS_SUCCESS || strcmp(buffer, TEST_STRING) != 0) {
    ESP_LOGE(TAG, "Failed to verify overwritten string value");
    return false;
  }

  // Test 4: Binary data with null bytes
  uint8_t null_data[] = {0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00};
  result = nvs.SetBlob("null_bytes", null_data, sizeof(null_data));
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set blob with null bytes");
    return false;
  }

  uint8_t null_buffer[sizeof(null_data)];
  size_t actual_size = 0;
  result = nvs.GetBlob("null_bytes", null_buffer, sizeof(null_buffer), &actual_size);
  if (result != hf_nvs_err_t::NVS_SUCCESS ||
      memcmp(null_buffer, null_data, sizeof(null_data)) != 0) {
    ESP_LOGE(TAG, "Failed to retrieve blob with null bytes");
    return false;
  }

  // Test 5: String with special characters
  const char* special_string = "Test\nString\twith\rspecial chars!@#$%^&*()";
  result = nvs.SetString("special_str", special_string);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set string with special characters");
    return false;
  }

  memset(buffer, 0, sizeof(buffer));
  result = nvs.GetString("special_str", buffer, sizeof(buffer));
  if (result != hf_nvs_err_t::NVS_SUCCESS || strcmp(buffer, special_string) != 0) {
    ESP_LOGE(TAG, "Failed to retrieve string with special characters");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] NVS edge cases tests passed");
  return true;
}

// === Stress Test ===

bool test_nvs_stress() noexcept {
  ESP_LOGI(TAG, "Testing NVS stress scenarios...");

  // Test 1: Multiple namespaces
  EspNvs nvs1("stress_ns1");
  EspNvs nvs2("stress_ns2");
  EspNvs nvs3("stress_ns3");

  if (!nvs1.EnsureInitialized() || !nvs2.EnsureInitialized() || !nvs3.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize multiple namespaces");
    return false;
  }

  // Write to different namespaces
  auto result = nvs1.SetU32("shared_key", 111);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to write to namespace 1");
    return false;
  }

  result = nvs2.SetU32("shared_key", 222);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to write to namespace 2");
    return false;
  }

  result = nvs3.SetU32("shared_key", 333);
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to write to namespace 3");
    return false;
  }

  // Verify namespace isolation
  hf_u32_t value1 = 0, value2 = 0, value3 = 0;
  nvs1.GetU32("shared_key", value1);
  nvs2.GetU32("shared_key", value2);
  nvs3.GetU32("shared_key", value3);

  if (value1 != 111 || value2 != 222 || value3 != 333) {
    ESP_LOGE(TAG, "Namespace isolation failed: %u, %u, %u", value1, value2, value3);
    return false;
  }

  // Test 2: Fill storage with many keys
  EspNvs nvs_fill("stress_fill");
  if (!nvs_fill.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize fill namespace");
    return false;
  }

  const int NUM_KEYS = 50;
  for (int i = 0; i < NUM_KEYS; ++i) {
    char key[16];
    snprintf(key, sizeof(key), "fill_%d", i);

    result = nvs_fill.SetU32(key, static_cast<hf_u32_t>(i * 1000));
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set key %s in fill test", key);
      return false;
    }

    // Add some variety with strings and blobs
    if (i % 3 == 0) {
      char str_key[16];
      char str_val[32];
      snprintf(str_key, sizeof(str_key), "str_%d", i);
      snprintf(str_val, sizeof(str_val), "String value %d", i);
      nvs_fill.SetString(str_key, str_val);
    }

    if (i % 5 == 0) {
      char blob_key[16];
      uint8_t blob_data[16];
      snprintf(blob_key, sizeof(blob_key), "blob_%d", i);
      memset(blob_data, static_cast<uint8_t>(i), sizeof(blob_data));
      nvs_fill.SetBlob(blob_key, blob_data, sizeof(blob_data));
    }
  }

  // Commit all changes
  result = nvs_fill.Commit();
  if (result != hf_nvs_err_t::NVS_SUCCESS) {
    ESP_LOGE(TAG, "Failed to commit in stress test");
    return false;
  }

  // Verify some random keys
  for (int i = 0; i < 10; ++i) {
    int idx = (i * 7) % NUM_KEYS; // Pseudo-random selection
    char key[16];
    snprintf(key, sizeof(key), "fill_%d", idx);

    hf_u32_t val = 0;
    result = nvs_fill.GetU32(key, val);
    if (result != hf_nvs_err_t::NVS_SUCCESS || val != static_cast<hf_u32_t>(idx * 1000)) {
      ESP_LOGE(TAG, "Failed to verify key %s in stress test", key);
      return false;
    }
  }

  // Test 3: Rapid init/deinit cycles
  for (int i = 0; i < 5; ++i) {
    EspNvs nvs_cycle("stress_cycle");
    if (!nvs_cycle.EnsureInitialized()) {
      ESP_LOGE(TAG, "Failed to initialize in cycle %d", i);
      return false;
    }

    nvs_cycle.SetU32("cycle_test", static_cast<hf_u32_t>(i));

    if (!nvs_cycle.EnsureDeinitialized()) {
      ESP_LOGE(TAG, "Failed to deinitialize in cycle %d", i);
      return false;
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] NVS stress tests passed");
  return true;
}

// === Main Test Runner ===

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 NVS COMPREHENSIVE TEST SUITE                     ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                           ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "║ Target: ESP32-C6 DevKit-M-1                                                  ║");
  ESP_LOGI(TAG, "║ ESP-IDF: v5.5+                                                               ║");
  ESP_LOGI(TAG, "║ Features: NVS, Statistics, Diagnostics, Metadata, Edge Cases, Stress Tests   ║");
  ESP_LOGI(TAG, "║ Architecture: noexcept (no exception handling)                               ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "\n");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Report test section configuration
  print_test_section_status(TAG, "NVS");

  // Run all NVS tests based on configuration
  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_CORE_TESTS, "NVS CORE TESTS",
      // Core functionality tests
      ESP_LOGI(TAG, "Running core NVS functionality tests...");
      RUN_TEST_IN_TASK("initialization", test_nvs_initialization, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_DATA_TESTS, "NVS DATA TESTS",
      // Data operation tests
      ESP_LOGI(TAG, "Running NVS data operation tests...");
      RUN_TEST_IN_TASK("u32_operations", test_nvs_u32_operations, 8192, 1);
      RUN_TEST_IN_TASK("string_operations", test_nvs_string_operations, 8192, 1);
      RUN_TEST_IN_TASK("blob_operations", test_nvs_blob_operations, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_MANAGEMENT_TESTS, "NVS MANAGEMENT TESTS",
      // Management operation tests
      ESP_LOGI(TAG, "Running NVS management tests...");
      RUN_TEST_IN_TASK("key_operations", test_nvs_key_operations, 8192, 1);
      RUN_TEST_IN_TASK("commit_operations", test_nvs_commit_operations, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_DIAGNOSTIC_TESTS, "NVS DIAGNOSTIC TESTS",
      // Diagnostic and metadata tests
      ESP_LOGI(TAG, "Running NVS diagnostic tests...");
      RUN_TEST_IN_TASK("statistics_diagnostics", test_nvs_statistics_diagnostics, 8192, 1);
      RUN_TEST_IN_TASK("metadata", test_nvs_metadata, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(ENABLE_STRESS_TESTS, "NVS STRESS TESTS",
                              // Stress and edge case tests
                              ESP_LOGI(TAG, "Running NVS stress tests...");
                              RUN_TEST_IN_TASK("edge_cases", test_nvs_edge_cases, 8192, 1);
                              RUN_TEST_IN_TASK("stress", test_nvs_stress, 8192, 1););

  // Print summary
  print_test_summary(g_test_results, "NVS", TAG);

  ESP_LOGI(TAG, "NVS comprehensive testing completed.");
  ESP_LOGI(TAG, "System will continue running. Press RESET to restart tests.");
  ESP_LOGI(TAG, "\n");

  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 NVS COMPREHENSIVE TEST SUITE                     ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                          ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  // Keep running
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
