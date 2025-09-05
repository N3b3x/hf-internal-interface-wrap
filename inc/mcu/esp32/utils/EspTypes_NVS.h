/**
 * @file EspTypes_NVS.h
 * @brief ESP32 NVS type definitions for hardware abstraction.
 *
 * This header defines only the essential NVS-specific types used by
 * the EspNvs implementation. Clean and minimal approach.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "BaseNvs.h" // For hf_nvs_err_t
#include "EspTypes_Base.h"
#include "HardwareTypes.h" // For basic hardware types
#include "McuSelect.h"     // Central MCU platform selection (includes all ESP-IDF)
#include <cstring>

//==============================================================================
// ESP32 NVS TYPE MAPPINGS
//==============================================================================

// Direct ESP-IDF type usage - no unnecessary aliases
// These types are used internally by EspNvs implementation

//==============================================================================
// ESP32 NVS CONSTANTS
//==============================================================================

static constexpr size_t HF_NVS_MAX_KEY_LENGTH = 15;
static constexpr size_t HF_NVS_MAX_VALUE_SIZE = 4000;
static constexpr size_t HF_NVS_MAX_NAMESPACE_LENGTH = 15;
static constexpr size_t HF_NVS_MAX_NAMESPACES = 256;
static constexpr size_t HF_NVS_FLASH_SECTOR_SIZE = 4096;
static constexpr size_t HF_NVS_PAGE_SIZE = 4096;
static constexpr size_t HF_NVS_ENTRY_SIZE = 32;

// NVS operation timeouts
static constexpr uint32_t HF_NVS_OPERATION_TIMEOUT_MS = 1000; ///< Default operation timeout
static constexpr uint32_t HF_NVS_INIT_TIMEOUT_MS = 5000;      ///< Initialization timeout
static constexpr uint32_t HF_NVS_COMMIT_TIMEOUT_MS = 2000;    ///< Commit operation timeout

//==============================================================================
// ESP32 NVS ENUMS
//==============================================================================

/**
 * @brief ESP32 NVS data types.
 */
enum class hf_nvs_type_t : uint8_t {
  HF_NVS_TYPE_U8 = 0,  ///< 8-bit unsigned integer
  HF_NVS_TYPE_I8 = 1,  ///< 8-bit signed integer
  HF_NVS_TYPE_U16 = 2, ///< 16-bit unsigned integer
  HF_NVS_TYPE_I16 = 3, ///< 16-bit signed integer
  HF_NVS_TYPE_U32 = 4, ///< 32-bit unsigned integer
  HF_NVS_TYPE_I32 = 5, ///< 32-bit signed integer
  HF_NVS_TYPE_U64 = 6, ///< 64-bit unsigned integer
  HF_NVS_TYPE_I64 = 7, ///< 64-bit signed integer
  HF_NVS_TYPE_STR = 8, ///< String
  HF_NVS_TYPE_BLOB = 9 ///< Binary blob
};

/**
 * @brief ESP32 NVS open modes.
 */
enum class hf_nvs_open_mode_t : uint8_t {
  HF_NVS_READONLY = 0, ///< Read-only mode
  HF_NVS_READWRITE = 1 ///< Read-write mode
};

/**
 * @brief ESP32 NVS encryption modes.
 */
enum class hf_nvs_encryption_mode_t : uint8_t {
  HF_NVS_ENCRYPTION_NONE = 0, ///< No encryption
  HF_NVS_ENCRYPTION_HMAC = 1, ///< HMAC encryption
  HF_NVS_ENCRYPTION_XTS = 2   ///< XTS encryption
};

//==============================================================================
// ESP32 NVS CONFIGURATION STRUCTURES
//==============================================================================

/**
 * @brief ESP32 NVS partition configuration.
 */
struct hf_nvs_partition_config_t {
  const char* partition_label;         ///< Partition label
  const char* namespace_name;          ///< Namespace name
  hf_nvs_open_mode_t open_mode;        ///< Open mode
  hf_nvs_encryption_mode_t encryption; ///< Encryption mode
  size_t max_entries;                  ///< Maximum entries
  bool auto_commit;                    ///< Auto-commit flag

  hf_nvs_partition_config_t() noexcept
      : partition_label("nvs"), namespace_name("default"),
        open_mode(hf_nvs_open_mode_t::HF_NVS_READWRITE),
        encryption(hf_nvs_encryption_mode_t::HF_NVS_ENCRYPTION_NONE), max_entries(256),
        auto_commit(true) {}
};

/**
 * @brief ESP32 NVS capabilities information.
 */
struct hf_nvs_capabilities_t {
  size_t max_namespaces;           ///< Maximum namespaces
  size_t max_keys_per_namespace;   ///< Maximum keys per namespace
  size_t max_key_length;           ///< Maximum key length
  size_t max_value_size;           ///< Maximum value size
  size_t flash_sector_size;        ///< Flash sector size
  bool supports_encryption;        ///< Encryption support
  bool supports_hmac_encryption;   ///< HMAC encryption support
  bool supports_xts_encryption;    ///< XTS encryption support
  bool supports_atomic_operations; ///< Atomic operations support
  bool supports_wear_leveling;     ///< Wear leveling support

  hf_nvs_capabilities_t() noexcept
      : max_namespaces(HF_NVS_MAX_NAMESPACES), max_keys_per_namespace(256),
        max_key_length(HF_NVS_MAX_KEY_LENGTH), max_value_size(HF_NVS_MAX_VALUE_SIZE),
        flash_sector_size(HF_NVS_FLASH_SECTOR_SIZE), supports_encryption(true),
        supports_hmac_encryption(true), supports_xts_encryption(true),
        supports_atomic_operations(true), supports_wear_leveling(true) {}
};

/**
 * @brief ESP32 NVS iterator configuration.
 */
struct hf_nvs_iterator_config_t {
  const char* namespace_name; ///< Namespace name
  hf_nvs_type_t type;         ///< Data type filter
  const char* key_prefix;     ///< Key prefix filter
  size_t max_entries;         ///< Maximum entries to iterate

  hf_nvs_iterator_config_t() noexcept
      : namespace_name(nullptr), type(hf_nvs_type_t::HF_NVS_TYPE_U8), key_prefix(nullptr),
        max_entries(100) {}
};

/**
 * @brief ESP32 NVS entry information.
 */
struct hf_nvs_entry_info_t {
  char key[HF_NVS_MAX_KEY_LENGTH + 1]; ///< Entry key
  hf_nvs_type_t type;                  ///< Entry type
  size_t size;                         ///< Entry size
  uint32_t crc;                        ///< Entry CRC

  hf_nvs_entry_info_t() noexcept : type(hf_nvs_type_t::HF_NVS_TYPE_U8), size(0), crc(0) {
    key[0] = '\0';
  }
};

//==============================================================================
// ESP32 NVS VALIDATION MACROS
//==============================================================================

#define HF_NVS_IS_VALID_KEY_LENGTH(len) ((len) > 0 && (len) <= HF_NVS_MAX_KEY_LENGTH)
#define HF_NVS_IS_VALID_VALUE_SIZE(size) ((size) <= HF_NVS_MAX_VALUE_SIZE)
#define HF_NVS_IS_VALID_NAMESPACE_LENGTH(len) ((len) > 0 && (len) <= HF_NVS_MAX_NAMESPACE_LENGTH)

//==============================================================================
// ESP32 NVS UTILITY FUNCTIONS
//==============================================================================

/**
 * @brief Validate NVS key name for ESP32
 * @param key Key name to validate
 * @return true if valid, false otherwise
 */
inline constexpr bool IsValidNvsKey(const char* key) noexcept {
  if (!key)
    return false;
  size_t len = strlen(key);
  return HF_NVS_IS_VALID_KEY_LENGTH(len);
}

/**
 * @brief Validate NVS namespace name for ESP32
 * @param namespace_name Namespace name to validate
 * @return true if valid, false otherwise
 */
inline constexpr bool IsValidNvsNamespace(const char* namespace_name) noexcept {
  if (!namespace_name)
    return false;
  size_t len = strlen(namespace_name);
  return HF_NVS_IS_VALID_NAMESPACE_LENGTH(len);
}

/**
 * @brief Validate NVS value size for ESP32
 * @param size Value size to validate
 * @return true if valid, false otherwise
 */
inline constexpr bool IsValidNvsValueSize(size_t size) noexcept {
  return HF_NVS_IS_VALID_VALUE_SIZE(size);
}

/**
 * @brief Get maximum supported value size for given type
 * @param type NVS data type
 * @return Maximum size in bytes
 */
inline constexpr size_t GetMaxValueSizeForType(hf_nvs_type_t type) noexcept {
  switch (type) {
  case hf_nvs_type_t::HF_NVS_TYPE_U8:
  case hf_nvs_type_t::HF_NVS_TYPE_I8:
    return 1;
  case hf_nvs_type_t::HF_NVS_TYPE_U16:
  case hf_nvs_type_t::HF_NVS_TYPE_I16:
    return 2;
  case hf_nvs_type_t::HF_NVS_TYPE_U32:
  case hf_nvs_type_t::HF_NVS_TYPE_I32:
    return 4;
  case hf_nvs_type_t::HF_NVS_TYPE_U64:
  case hf_nvs_type_t::HF_NVS_TYPE_I64:
    return 8;
  case hf_nvs_type_t::HF_NVS_TYPE_STR:
  case hf_nvs_type_t::HF_NVS_TYPE_BLOB:
    return HF_NVS_MAX_VALUE_SIZE;
  default:
    return 0;
  }
}

//==============================================================================
// END OF ESPNVS TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================
