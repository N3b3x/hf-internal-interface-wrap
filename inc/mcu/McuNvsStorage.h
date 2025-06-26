/**
 * @file McuNvsStorage.h
 * @brief MCU-integrated Non-Volatile Storage implementation.
 *
 * This header provides an NVS implementation for microcontrollers with
 * built-in non-volatile storage. On ESP32, this wraps the NVS (Non-Volatile Storage) API,
 * on other MCUs it would wrap EEPROM or flash storage, etc.
 *
 * This is the primary NVS implementation for MCUs with integrated storage capabilities.
 */
#ifndef MCU_NVS_STORAGE_H_
#define MCU_NVS_STORAGE_H_

#include "BaseNvsStorage.h"
#include "McuTypes.h"
#include <cstdint>

/**
 * @class McuNvsStorage
 * @brief MCU-integrated non-volatile storage implementation.
 * 
 * This class provides non-volatile storage using the microcontroller's built-in
 * storage mechanisms. On ESP32, it uses the NVS (Non-Volatile Storage) library.
 * The implementation handles platform-specific details while providing the
 * unified BaseNvsStorage API.
 * 
 * Features:
 * - Key-value storage using MCU's integrated NVS
 * - Multiple data type support (uint32_t, string, blob)
 * - Namespace-based organization
 * - Atomic operations and error handling
 * - Comprehensive status reporting
 * - Lazy initialization support
 * 
 * @note This implementation requires sufficient flash storage on the MCU
 */
class McuNvsStorage : public BaseNvsStorage {
public:
    /**
     * @brief Constructor with namespace specification.
     * @param namespace_name Name of the storage namespace
     */
    explicit McuNvsStorage(const char* namespace_name) noexcept;

    /**
     * @brief Destructor - ensures proper cleanup.
     */
    ~McuNvsStorage() noexcept override;

    //==============================================//
    // OVERRIDDEN PURE VIRTUAL FUNCTIONS            //
    //==============================================//

    /**
     * @brief Initialize the NVS system and open the namespace.
     * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
     */
    HfNvsErr Initialize() noexcept override;

    /**
     * @brief Deinitialize the NVS system and close the namespace.
     * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
     */
    HfNvsErr Deinitialize() noexcept override;

    /**
     * @brief Store a 32-bit unsigned integer value.
     * @param key Storage key (null-terminated string)
     * @param value Value to store
     * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
     */
    HfNvsErr SetU32(const char* key, uint32_t value) noexcept override;

    /**
     * @brief Retrieve a 32-bit unsigned integer value.
     * @param key Storage key (null-terminated string)
     * @param value Reference to store the retrieved value
     * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
     */
    HfNvsErr GetU32(const char* key, uint32_t& value) noexcept override;

    /**
     * @brief Store a string value.
     * @param key Storage key (null-terminated string)
     * @param value String value to store
     * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
     */
    HfNvsErr SetString(const char* key, const char* value) noexcept override;

    /**
     * @brief Retrieve a string value.
     * @param key Storage key (null-terminated string)
     * @param buffer Buffer to store the retrieved string
     * @param buffer_size Size of the buffer in bytes
     * @param actual_size Actual size of the string (optional)
     * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
     */
    HfNvsErr GetString(const char* key, char* buffer, size_t buffer_size, 
                       size_t* actual_size = nullptr) noexcept override;

    /**
     * @brief Store binary data (blob).
     * @param key Storage key (null-terminated string)
     * @param data Pointer to data to store
     * @param data_size Size of data in bytes
     * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
     */
    HfNvsErr SetBlob(const char* key, const void* data, size_t data_size) noexcept override;

    /**
     * @brief Retrieve binary data (blob).
     * @param key Storage key (null-terminated string)
     * @param buffer Buffer to store the retrieved data
     * @param buffer_size Size of the buffer in bytes
     * @param actual_size Actual size of the data (optional)
     * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
     */
    HfNvsErr GetBlob(const char* key, void* buffer, size_t buffer_size,
                     size_t* actual_size = nullptr) noexcept override;

    /**
     * @brief Remove a key from storage.
     * @param key Storage key to remove
     * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
     */
    HfNvsErr EraseKey(const char* key) noexcept override;

    /**
     * @brief Commit any pending writes to non-volatile storage.
     * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
     */
    HfNvsErr Commit() noexcept override;

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
     * @return HfNvsErr::NVS_SUCCESS if successful, error code otherwise
     */
    HfNvsErr GetSize(const char* key, size_t& size) noexcept override;

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

private:
    /**
     * @brief Convert MCU-specific error to HfNvsErr.
     * @param mcu_error MCU-specific error code
     * @return Corresponding HfNvsErr value
     */
    HfNvsErr ConvertMcuError(int mcu_error) const noexcept;

    void* nvs_handle_;  ///< Platform-specific NVS handle
};

#endif // MCU_NVS_STORAGE_H_
