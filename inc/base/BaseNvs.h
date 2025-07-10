/**
 * @file BaseNvs.h
 * @brief Abstract base class for Non-Volatile Storage implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for non-volatile storage
 * that provides a consistent API across different storage implementations.
 * Concrete implementations for various storage devices inherit from this class
 * to provide key-value storage, settings persistence, and configuration management.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is a header-only abstract base class following the same pattern as BaseCan.
 * @note Users should program against this interface, not specific implementations.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <string_view>

//--------------------------------------
//  HardFOC NVS Error Codes (Table)
//--------------------------------------
/**
 * @brief HardFOC NVS error codes
 * @details Comprehensive error enumeration for all NVS operations in the system.
 *          This enumeration is used across all NVS-related classes to provide
 *          consistent error reporting and handling.
 */

#define HF_NVS_ERR_LIST(X)                                                                         \
  /* Success codes */                                                                              \
  X(NVS_SUCCESS, 0, "Success")                                                                     \
  /* General errors */                                                                             \
  X(NVS_ERR_FAILURE, 1, "General failure")                                                         \
  X(NVS_ERR_NOT_INITIALIZED, 2, "Not initialized")                                                 \
  X(NVS_ERR_ALREADY_INITIALIZED, 3, "Already initialized")                                         \
  X(NVS_ERR_INVALID_PARAMETER, 4, "Invalid parameter")                                             \
  X(NVS_ERR_NULL_POINTER, 5, "Null pointer")                                                       \
  X(NVS_ERR_OUT_OF_MEMORY, 6, "Out of memory")                                                     \
  /* Storage specific errors */                                                                    \
  X(NVS_ERR_KEY_NOT_FOUND, 7, "Key not found")                                                     \
  X(NVS_ERR_KEY_TOO_LONG, 8, "Key too long")                                                       \
  X(NVS_ERR_VALUE_TOO_LARGE, 9, "Value too large")                                                 \
  X(NVS_ERR_NAMESPACE_NOT_FOUND, 10, "Namespace not found")                                        \
  X(NVS_ERR_STORAGE_FULL, 11, "Storage full")                                                      \
  X(NVS_ERR_INVALID_DATA, 12, "Invalid data")                                                      \
  X(NVS_ERR_READ_ONLY, 13, "Read only mode")                                                       \
  X(NVS_ERR_CORRUPTED, 14, "Data corrupted")                                                       \
  /* ESP32-C6 encryption and advanced feature errors */                                           \
  X(NVS_ERR_ENCRYPTION_FAILED, 15, "Encryption operation failed")                                  \
  X(NVS_ERR_DECRYPTION_FAILED, 16, "Decryption operation failed")                                  \
  X(NVS_ERR_ENCRYPTION_NOT_CONFIGURED, 17, "Encryption not configured")                           \
  X(NVS_ERR_ENCRYPTION_NOT_SUPPORTED, 18, "Encryption not supported")                             \
  X(NVS_ERR_KEY_PARTITION_CORRUPTED, 19, "Key partition corrupted")                               \
  X(NVS_ERR_WRONG_ENCRYPTION_SCHEME, 20, "Wrong encryption scheme")                               \
  X(NVS_ERR_VERSION_MISMATCH, 21, "NVS version mismatch")                                         \
  X(NVS_ERR_NO_FREE_PAGES, 22, "No free pages available")                                         \
  X(NVS_ERR_PARTITION_NOT_FOUND, 23, "NVS partition not found")                                   \
  X(NVS_ERR_ITERATOR_INVALID, 24, "Iterator invalid or expired")                                  \
  X(NVS_ERR_SECURITY_VIOLATION, 25, "Security policy violation")                                  \
  X(NVS_ERR_UNSUPPORTED_OPERATION, 26, "Unsupported operation")   

// Generate enum class from X-macro
enum class hf_nvs_err_t : int32_t {
#define X(name, value, desc) name = value,
  HF_NVS_ERR_LIST(X)
#undef X
};

// Generate error description function
constexpr const char *HfNvsErrToString(hf_nvs_err_t err) noexcept {
  switch (err) {
#define X(name, value, desc)                                                                       \
  case hf_nvs_err_t::name:                                                                             \
    return desc;
    HF_NVS_ERR_LIST(X)
#undef X
  default:
    return "Unknown error";
  }
}

/**
 * @brief NVS operation statistics.
 */
struct hf_nvs_statistics_t {
  uint32_t totalOperations;      ///< Total NVS operations performed
  uint32_t successfulOperations; ///< Successful operations
  uint32_t failedOperations;     ///< Failed operations
  uint32_t readOperations;       ///< Number of read operations
  uint32_t writeOperations;      ///< Number of write operations
  uint32_t eraseOperations;      ///< Number of erase operations
  uint32_t commitOperations;     ///< Number of commit operations
  uint32_t averageOperationTimeUs; ///< Average operation time (microseconds)
  uint32_t maxOperationTimeUs;   ///< Maximum operation time
  uint32_t minOperationTimeUs;   ///< Minimum operation time

  hf_nvs_statistics_t()
      : totalOperations(0), successfulOperations(0), failedOperations(0),
        readOperations(0), writeOperations(0), eraseOperations(0), commitOperations(0),
        averageOperationTimeUs(0), maxOperationTimeUs(0), minOperationTimeUs(UINT32_MAX) {}
};

/**
 * @brief NVS diagnostic information.
 */
struct hf_nvs_diagnostics_t {
  bool nvsHealthy;               ///< Overall NVS health status
  hf_nvs_err_t lastErrorCode;    ///< Last error code
  uint32_t lastErrorTimestamp;   ///< Last error timestamp
  uint32_t consecutiveErrors;    ///< Consecutive error count
  bool nvsInitialized;           ///< NVS initialization status
  size_t usedSpace;              ///< Used space in bytes
  size_t totalSpace;             ///< Total space in bytes
  uint32_t wearLevel;            ///< Wear level indicator

  hf_nvs_diagnostics_t()
      : nvsHealthy(true), lastErrorCode(hf_nvs_err_t::NVS_SUCCESS), lastErrorTimestamp(0), 
          consecutiveErrors(0), nvsInitialized(false), usedSpace(0), totalSpace(0), wearLevel(0) {}
};

/**
 * @class BaseNvs
 * @brief Abstract base class for non-volatile storage operations.
 *
 * This class provides a consistent interface for non-volatile storage across different
 * hardware platforms and storage mechanisms. It supports key-value storage with
 * various data types and namespace organization.
 *
 * Key Features:
 * - Namespace-based organization
 * - Multiple data type support (uint32_t, string, blob)
 * - Atomic operations
 * - Error handling and status reporting
 * - Platform-agnostic interface
 * - Lazy initialization pattern
 *
 * @note Implementations should handle platform-specific details internally
 * @note This class is designed to be thread-safe when properly implemented
 */
class BaseNvs {
public:
  /**
   * @brief Virtual destructor to ensure proper cleanup.
   */
  virtual ~BaseNvs() noexcept = default;

  // Disable copy constructor and assignment operator for safety
  BaseNvs(const BaseNvs &) = delete;
  BaseNvs &operator=(const BaseNvs &) = delete;

  //==============================================//
  // LAZY-INITIALIZATION PATTERN                  //
  //==============================================//

  /**
   * @brief Ensures that the NVS storage is initialized (lazy initialization).
   * @return true if the NVS storage is initialized, false otherwise.
   * @note This method follows the HardFOC HAL contract pattern used by all peripherals.
   */
  bool EnsureInitialized() noexcept {
    if (!initialized_) {
      initialized_ = Initialize();
    }
    return initialized_;
  }

  /**
   * @brief Ensures that the NVS storage is deinitialized.
   * @return true if the NVS storage is deinitialized, false otherwise.
   */
  bool EnsureDeinitialized() noexcept {
    if (initialized_) {
      initialized_ = !Deinitialize();
    }
    return !initialized_;
  }

  /**
   * @brief Check if storage is initialized.
   * @return true if initialized, false otherwise
   */
  bool IsInitialized() const noexcept {
    return initialized_;
  }

  //==============================================//
  // PURE VIRTUAL FUNCTIONS (MUST BE IMPLEMENTED) //
  //==============================================//

  /**
   * @brief Initialize the storage system and open the namespace.
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  virtual hf_nvs_err_t Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the storage system and close the namespace.
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  virtual hf_nvs_err_t Deinitialize() noexcept = 0;

  /**
   * @brief Store a 32-bit unsigned integer value.
   * @param key Storage key (null-terminated string)
   * @param value Value to store
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  virtual hf_nvs_err_t SetU32(const char *key, uint32_t value) noexcept = 0;

  /**
   * @brief Retrieve a 32-bit unsigned integer value.
   * @param key Storage key (null-terminated string)
   * @param value Reference to store the retrieved value
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  virtual hf_nvs_err_t GetU32(const char *key, uint32_t &value) noexcept = 0;

  /**
   * @brief Store a string value.
   * @param key Storage key (null-terminated string)
   * @param value String value to store
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  virtual hf_nvs_err_t SetString(const char *key, const char *value) noexcept = 0;

  /**
   * @brief Retrieve a string value.
   * @param key Storage key (null-terminated string)
   * @param buffer Buffer to store the retrieved string
   * @param buffer_size Size of the buffer in bytes
   * @param actual_size Actual size of the string (optional)
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  virtual hf_nvs_err_t GetString(const char *key, char *buffer, size_t buffer_size,
                             size_t *actual_size = nullptr) noexcept = 0;

  /**
   * @brief Store binary data (blob).
   * @param key Storage key (null-terminated string)
   * @param data Pointer to data to store
   * @param data_size Size of data in bytes
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  virtual hf_nvs_err_t SetBlob(const char *key, const void *data, size_t data_size) noexcept = 0;

  /**
   * @brief Retrieve binary data (blob).
   * @param key Storage key (null-terminated string)
   * @param buffer Buffer to store the retrieved data
   * @param buffer_size Size of the buffer in bytes
   * @param actual_size Actual size of the data (optional)
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  virtual hf_nvs_err_t GetBlob(const char *key, void *buffer, size_t buffer_size,
                           size_t *actual_size = nullptr) noexcept = 0;

  /**
   * @brief Remove a key from storage.
   * @param key Storage key to remove
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  virtual hf_nvs_err_t EraseKey(const char *key) noexcept = 0;

  /**
   * @brief Commit any pending writes to non-volatile storage.
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  virtual hf_nvs_err_t Commit() noexcept = 0;

  /**
   * @brief Check if a key exists in storage.
   * @param key Storage key to check
   * @return true if key exists, false otherwise
   */
  virtual bool KeyExists(const char *key) noexcept = 0;

  /**
   * @brief Get the size of a stored value.
   * @param key Storage key
   * @param size Reference to store the size
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  virtual hf_nvs_err_t GetSize(const char *key, size_t &size) noexcept = 0;

  //==============================================//
  // PUBLIC INTERFACE (IMPLEMENTED)               //
  //==============================================//

  /**
   * @brief Get the namespace name.
   * @return Namespace name string
   */
  const char *GetNamespace() const noexcept {
    return namespace_name_;
  }

  /**
   * @brief Get description of this storage implementation.
   * @return Description string
   */
  virtual const char *GetDescription() const noexcept = 0;

  /**
   * @brief Get maximum key length supported.
   * @return Maximum key length in characters
   */
  virtual size_t GetMaxKeyLength() const noexcept = 0;

  /**
   * @brief Get maximum value size supported.
   * @return Maximum value size in bytes
   */
  virtual size_t GetMaxValueSize() const noexcept = 0;

  //==============================================//
  // STATISTICS AND DIAGNOSTICS
  //==============================================//

  /**
   * @brief Reset NVS operation statistics.
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific statistics reset
   */
  virtual hf_nvs_err_t ResetStatistics() noexcept {
    statistics_ = hf_nvs_statistics_t{}; // Reset statistics to default values
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset NVS diagnostic information.
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific diagnostics reset
   */
  virtual hf_nvs_err_t ResetDiagnostics() noexcept {
    diagnostics_ = hf_nvs_diagnostics_t{}; // Reset diagnostics to default values
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get NVS operation statistics
   * @param statistics Reference to store statistics data
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, NVS_ERR_NOT_SUPPORTED if not implemented
   */
  virtual hf_nvs_err_t GetStatistics(hf_nvs_statistics_t &statistics) const noexcept {
    statistics = statistics_; // Return statistics by default
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get NVS diagnostic information
   * @param diagnostics Reference to store diagnostics data
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, NVS_ERR_NOT_SUPPORTED if not implemented
   */
  virtual hf_nvs_err_t GetDiagnostics(hf_nvs_diagnostics_t &diagnostics) const noexcept {
    diagnostics = diagnostics_; // Return diagnostics by default
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
  }

protected:
  
  /**
   * @brief Protected constructor with namespace specification.
   * @param namespace_name Name of the storage namespace
   */
  explicit BaseNvs(const char *namespace_name) noexcept
      : namespace_name_(namespace_name), initialized_(false), statistics_{}, diagnostics_{} {}

  /**
   * @brief Set the initialized state.
   * @param initialized New initialization state
   */
  void SetInitialized(bool initialized) noexcept {
    initialized_ = initialized;
  }

  const char *namespace_name_; ///< Namespace name
  bool initialized_; ///< Initialization status
  hf_nvs_statistics_t statistics_; ///< NVS operation statistics
  hf_nvs_diagnostics_t diagnostics_; ///< NVS diagnostic information

private:
};
