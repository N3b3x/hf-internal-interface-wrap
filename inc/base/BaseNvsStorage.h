/**
 * @file BaseNvsStorage.h
 * @brief Abstract base class for Non-Volatile Storage implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for non-volatile storage
 * that provides a consistent API across different storage implementations.
 * Concrete implementations (like McuNvsStorage for ESP32 NVS, external EEPROM) inherit from this
 * class.
 *
 * @note This is a header-only abstract base class following the same pattern as BaseCan.
 * @note Users should program against this interface, not specific implementations.
 */

#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_BASENVSSTORAGE_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_BASENVSSTORAGE_H_

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
  X(NVS_ERR_CORRUPTED, 14, "Data corrupted")

// Generate enum class from X-macro
enum class HfNvsErr : int32_t {
#define X(name, value, desc) name = value,
  HF_NVS_ERR_LIST(X)
#undef X
};

// Generate error description function
constexpr const char *HfNvsErrToString(HfNvsErr err) noexcept {
  switch (err) {
#define X(name, value, desc)                                                                       \
  case HfNvsErr::name:                                                                             \
    return desc;
    HF_NVS_ERR_LIST(X)
#undef X
  default:
    return "Unknown error";
  }
}

/**
 * @class BaseNvsStorage
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
 *
 * @note Implementations should handle platform-specific details internally
 * @note This class is designed to be thread-safe when properly implemented
 */
class BaseNvsStorage {
public:
  /**
   * @brief Constructor with namespace specification.
   * @param namespace_name Name of the storage namespace
   */
  explicit BaseNvsStorage(const char *namespace_name) noexcept
      : namespace_name_(namespace_name), initialized_(false) {}

  /**
   * @brief Virtual destructor to ensure proper cleanup.
   */
  virtual ~BaseNvsStorage() noexcept = default;

  // Disable copy constructor and assignment operator for safety
  BaseNvsStorage(const BaseNvsStorage &) = delete;
  BaseNvsStorage &operator=(const BaseNvsStorage &) = delete;

  //==============================================//
  // PURE VIRTUAL FUNCTIONS (MUST BE IMPLEMENTED) //
  //==============================================//

  /**
   * @brief Initialize the storage system and open the namespace.
   * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
   */
  virtual HfNvsErr Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the storage system and close the namespace.
   * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
   */
  virtual HfNvsErr Deinitialize() noexcept = 0;

  /**
   * @brief Store a 32-bit unsigned integer value.
   * @param key Storage key (null-terminated string)
   * @param value Value to store
   * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
   */
  virtual HfNvsErr SetU32(const char *key, uint32_t value) noexcept = 0;

  /**
   * @brief Retrieve a 32-bit unsigned integer value.
   * @param key Storage key (null-terminated string)
   * @param value Reference to store the retrieved value
   * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
   */
  virtual HfNvsErr GetU32(const char *key, uint32_t &value) noexcept = 0;

  /**
   * @brief Store a string value.
   * @param key Storage key (null-terminated string)
   * @param value String value to store
   * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
   */
  virtual HfNvsErr SetString(const char *key, const char *value) noexcept = 0;

  /**
   * @brief Retrieve a string value.
   * @param key Storage key (null-terminated string)
   * @param buffer Buffer to store the retrieved string
   * @param buffer_size Size of the buffer in bytes
   * @param actual_size Actual size of the string (optional)
   * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
   */
  virtual HfNvsErr GetString(const char *key, char *buffer, size_t buffer_size,
                             size_t *actual_size = nullptr) noexcept = 0;

  /**
   * @brief Store binary data (blob).
   * @param key Storage key (null-terminated string)
   * @param data Pointer to data to store
   * @param data_size Size of data in bytes
   * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
   */
  virtual HfNvsErr SetBlob(const char *key, const void *data, size_t data_size) noexcept = 0;

  /**
   * @brief Retrieve binary data (blob).
   * @param key Storage key (null-terminated string)
   * @param buffer Buffer to store the retrieved data
   * @param buffer_size Size of the buffer in bytes
   * @param actual_size Actual size of the data (optional)
   * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
   */
  virtual HfNvsErr GetBlob(const char *key, void *buffer, size_t buffer_size,
                           size_t *actual_size = nullptr) noexcept = 0;

  /**
   * @brief Remove a key from storage.
   * @param key Storage key to remove
   * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
   */
  virtual HfNvsErr EraseKey(const char *key) noexcept = 0;

  /**
   * @brief Commit any pending writes to non-volatile storage.
   * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
   */
  virtual HfNvsErr Commit() noexcept = 0;

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
   * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
   */
  virtual HfNvsErr GetSize(const char *key, size_t &size) noexcept = 0;

  //==============================================//
  // PUBLIC INTERFACE (IMPLEMENTED)               //
  //==============================================//

  /**
   * @brief Check if storage is initialized.
   * @return true if initialized, false otherwise
   */
  bool IsInitialized() const noexcept {
    return initialized_;
  }

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

protected:
  /**
   * @brief Set the initialized state.
   * @param initialized New initialization state
   */
  void SetInitialized(bool initialized) noexcept {
    initialized_ = initialized;
  }

private:
  const char *namespace_name_; ///< Storage namespace name
  bool initialized_;           ///< Initialization state flag
};

#endif // HAL_INTERNAL_INTERFACE_DRIVERS_BASENVSSTORAGE_H_
