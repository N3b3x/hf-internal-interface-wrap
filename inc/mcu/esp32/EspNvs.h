/**
 * @file EspNvs.h
 * @brief World-class ESP32-C6 NVS storage implementation with ESP-IDF v5.5+ features.
 *
 * This header provides a production-ready NVS implementation for microcontrollers with
 * built-in non-volatile storage capabilities. On ESP32-C6, this leverages the modern
 * ESP-IDF v5.5+ NVS API with comprehensive security features, encryption support,
 * performance optimizations, and robust error handling.
 *
 * Key Features:
 * - Modern ESP-IDF v5.5+ NVS API with handle-based operations
 * - ESP32-C6 HMAC-based encryption support for secure storage
 * - Comprehensive error handling and mapping to HardFOC error codes
 * - Advanced NVS features: statistics, validation, performance monitoring
 * - Thread-safe operations with optional mutex protection
 * - Namespace isolation and management
 * - Key-value storage with multiple data type support
 * - Atomic operations and consistency guarantees
 *
 * Security Features:
 * - HMAC-based encryption scheme (ESP32-C6 specific)
 * - XTS encryption for data protection
 * - Secure key generation and eFuse-based key storage
 * - Flash encryption compatibility
 * - Tamper resistance and data integrity validation
 *
 * Performance Optimizations:
 * - Efficient handle management and validation
 * - Optimized error code mapping with comprehensive coverage
 * - Statistics tracking for performance monitoring
 * - Intelligent commit strategies for durability vs. performance
 * - Key validation caching and namespace management
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This implementation is specifically optimized for ESP32-C6 production environments.
 * @note Requires ESP-IDF v5.5 or later for full feature support.
 * @note All platform-specific types are abstracted through McuTypes.h.
 * @note Uses McuSelect.h for centralized platform configuration.
 */
#pragma once

#include "BaseNvs.h"
#include "RtosMutex.h"          // Thread-safe mutex support if enabled
#include "utils/EspTypes_NVS.h" // Centralized ESP32 NVS type definitions
#include <cstdint>

// ESP32-C6 NVS abstracted types for portability

/**
 * @class EspNvs
 * @brief Production-ready MCU-integrated non-volatile storage implementation.
 *
 * This class provides comprehensive non-volatile storage using the microcontroller's
 * built-in storage mechanisms with enterprise-grade features. On ESP32-C6, it leverages
 * the modern ESP-IDF v5.5+ NVS library with advanced security, performance optimizations,
 * and comprehensive error handling. The implementation provides the unified BaseNvs
 * API while exposing platform-specific advanced features.
 *
 * Core Features:
 * - Key-value storage using MCU's integrated NVS with namespace isolation
 * - Multiple data type support (uint32_t, string, blob) with type safety
 * - Atomic operations with consistency guarantees and durability
 * - Comprehensive error handling with detailed error reporting
 * - Performance monitoring and statistics tracking
 * - Thread-safe operations with optional mutex protection
 *
 * ESP32-C6 Advanced Features:
 * - HMAC-based encryption for secure storage without flash encryption
 * - XTS encryption with eFuse-based key management
 * - Handle-based modern ESP-IDF v5.5+ API with improved performance
 * - Comprehensive error mapping for all ESP32-C6 NVS error conditions
 * - Support for encrypted and non-encrypted partitions
 * - Advanced partition management and configuration options
 *
 * Performance Characteristics:
 * - Optimized for high-frequency read/write operations
 * - Intelligent commit strategies balancing durability vs. performance
 * - Efficient handle management with validation caching
 * - Statistics tracking with minimal performance overhead
 * - Key validation with comprehensive constraint checking
 *
 * Security Features:
 * - Hardware-backed encryption using ESP32-C6 HMAC peripheral
 * - Tamper-resistant key storage in eFuse blocks
 * - Data integrity validation and corruption detection
 * - Secure key generation and management
 * - Protection against unauthorized access and data tampering
 *
 * @note This implementation requires sufficient flash storage on the MCU.
 * @note ESP32-C6 encryption features require proper eFuse configuration.
 * @note Thread safety is optional and controlled by HF_THREAD_SAFE define.
 * @note All operations are atomic and provide consistency guarantees.
 *
 * @warning Encryption keys stored in eFuse are permanent and irreversible.
 * @warning Ensure proper backup and key management procedures.
 *
 * @see BaseNvs for the abstract interface definition
 * @see McuTypes.h for platform-specific type definitions
 * @see McuSelect.h for platform selection and configuration
 */
class EspNvs : public BaseNvs {
public:
  /**
   * @brief Constructor with namespace specification.
   * @param namespace_name Name of the storage namespace
   */
  explicit EspNvs(const char* namespace_name) noexcept;

  /**
   * @brief Destructor - ensures proper cleanup.
   */
  ~EspNvs() noexcept override;

  //==============================================//
  // OVERRIDDEN PURE VIRTUAL FUNCTIONS            //
  //==============================================//

  /**
   * @brief Initialize the NVS system and open the namespace.
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t Initialize() noexcept override;

  /**
   * @brief Deinitialize the NVS system and close the namespace.
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t Deinitialize() noexcept override;

  /**
   * @brief Store a 32-bit unsigned integer value.
   * @param key Storage key (null-terminated string)
   * @param value Value to store
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t SetU32(const char* key, hf_u32_t value) noexcept override;

  /**
   * @brief Retrieve a 32-bit unsigned integer value.
   * @param key Storage key (null-terminated string)
   * @param value Reference to store the retrieved value
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t GetU32(const char* key, hf_u32_t& value) noexcept override;

  /**
   * @brief Store a string value.
   * @param key Storage key (null-terminated string)
   * @param value String value to store
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t SetString(const char* key, const char* value) noexcept override;

  /**
   * @brief Retrieve a string value.
   * @param key Storage key (null-terminated string)
   * @param buffer Buffer to store the retrieved string
   * @param buffer_size Size of the buffer in bytes
   * @param actual_size Actual size of the string (optional)
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t GetString(const char* key, char* buffer, size_t buffer_size,
                         size_t* actual_size = nullptr) noexcept override;

  /**
   * @brief Store binary data (blob).
   * @param key Storage key (null-terminated string)
   * @param data Pointer to data to store
   * @param data_size Size of data in bytes
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t SetBlob(const char* key, const void* data, size_t data_size) noexcept override;

  /**
   * @brief Retrieve binary data (blob).
   * @param key Storage key (null-terminated string)
   * @param buffer Buffer to store the retrieved data
   * @param buffer_size Size of the buffer in bytes
   * @param actual_size Actual size of the data (optional)
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t GetBlob(const char* key, void* buffer, size_t buffer_size,
                       size_t* actual_size = nullptr) noexcept override;

  /**
   * @brief Remove a key from storage.
   * @param key Storage key to remove
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t EraseKey(const char* key) noexcept override;

  /**
   * @brief Commit any pending writes to non-volatile storage.
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t Commit() noexcept override;

  /**
   * @brief Check if a key exists in storage.
   * @param key Storage key to check
   * @return true if key exists, false otherwise
   */
  bool KeyExists(const char* key) noexcept override;

  /**
   * @brief Get the size of a stored value.
   * @param key Storage key
   * @param size Reference to store the size
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t GetSize(const char* key, size_t& size) noexcept override;

  /**
   * @brief Get description of this NVS implementation.
   * @return Description string
   */
  const char* GetDescription() const noexcept override;

  /**
   * @brief Get maximum key length supported.
   * @return Maximum key length in characters
   */
  size_t GetMaxKeyLength() const noexcept override;

  /**
   * @brief Get maximum value size supported.
   * @return Maximum value size in bytes
   */
  size_t GetMaxValueSize() const noexcept override;

  //==============================================//
  // STATISTICS AND DIAGNOSTICS                   //
  //==============================================//

  /**
   * @brief Get NVS operation statistics.
   * @param statistics Reference to statistics structure to fill
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t GetStatistics(hf_nvs_statistics_t& statistics) const noexcept override;

  /**
   * @brief Get NVS diagnostic information.
   * @param diagnostics Reference to diagnostics structure to fill
   * @return hf_nvs_err_t::NVS_SUCCESS if successful, error code otherwise
   */
  hf_nvs_err_t GetDiagnostics(hf_nvs_diagnostics_t& diagnostics) const noexcept override;

private:
  //==============================================//
  // PRIVATE HELPER FUNCTIONS                     //
  //==============================================//

  /**
   * @brief Convert MCU-specific error code to HardFOC NVS error.
   * @details Provides comprehensive mapping from ESP32-C6 NVS error codes
   *          to unified HardFOC error enumeration, including all encryption
   *          and advanced feature error conditions.
   * @param mcu_error MCU-specific error code (esp_err_t on ESP32)
   * @return Corresponding hf_nvs_err_t enumeration value
   * @note Supports all ESP-IDF v5.5+ NVS error codes including encryption
   */
  hf_nvs_err_t ConvertMcuError(int mcu_error) const noexcept;

  /**
   * @brief Update operation statistics and performance counters.
   * @details Tracks operation counts, error rates, and timing for
   *          performance monitoring and debugging purposes.
   * @param error_occurred Whether the operation resulted in an error
   */
  void UpdateStatistics(bool error_occurred) noexcept;

  /**
   * @brief Validate key name according to ESP32 NVS constraints.
   * @param key Key name to validate
   * @return true if key is valid, false otherwise
   */
  bool IsValidKey(const char* key) const noexcept;

  /**
   * @brief Safely extract NVS handle from void pointer with ESP32-C6 RISC-V validation.
   * @param[out] handle Reference to store the extracted handle
   * @return true if handle is valid, false otherwise
   */
  bool ExtractValidHandle(nvs_handle_t& handle) const noexcept;

  //==============================================//
  // PRIVATE MEMBER VARIABLES                     //
  //==============================================//

  void* nvs_handle_;            ///< Platform-specific NVS handle (nvs_handle_t on ESP32)
  mutable int last_error_code_; ///< Last MCU-specific error code for debugging

  // Statistics and performance monitoring
  mutable hf_nvs_statistics_t statistics_;   ///< Operation statistics
  mutable hf_nvs_diagnostics_t diagnostics_; ///< Diagnostic information

  // Thread safety
  mutable RtosMutex mutex_; ///< Mutex for thread-safe operations
};
