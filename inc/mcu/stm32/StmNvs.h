/**
 * @file StmNvs.h
 * @brief STM32 NVS (Non-Volatile Storage) — flash-based key-value store.
 *
 * Implements BaseNvs using STM32 internal flash. Provides a simple key-value
 * store suitable for configuration persistence, calibration data, etc.
 *
 * The storage format is a compact key-value log in a designated flash region.
 * Users specify the flash region via hf_stm32_nvs_config_t (start address, size).
 *
 * @section Usage
 * @code
 * // Reserve last 2 sectors of flash (e.g., sector 6 & 7 on STM32F4)
 * hf_stm32_nvs_config_t nvs_cfg(0x08060000, 2 * 0x20000);
 * StmNvs nvs("app_config", nvs_cfg);
 * nvs.Initialize();
 *
 * nvs.SetU32("boot_count", 42);
 * nvs.Commit();
 *
 * uint32_t count = 0;
 * nvs.GetU32("boot_count", count);
 * @endcode
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseNvs.h"
#include "StmTypes.h"
#include <cstring>

/**
 * @brief STM32 NVS — flash-backed key-value store with wear leveling.
 *
 * Design:
 * - Uses a flat log of {key_len, key[], type, data_len, data[]} entries
 * - New writes append; reads scan from end to find latest value
 * - Commit triggers flash write of dirty entries
 * - When sector is full, compact (copy live entries to backup sector, erase, swap)
 * - Supports U32, String, and Blob value types
 *
 * @note Flash operations require HAL_FLASH_Unlock/Lock — handled internally.
 */
class StmNvs : public BaseNvs {
public:
    /**
     * @brief Construct with namespace and flash configuration.
     * @param namespace_name  Logical namespace (for multi-partition support)
     * @param flash_config    Flash region configuration
     */
    explicit StmNvs(const char* namespace_name = "default",
                    const hf_stm32_nvs_config_t& flash_config = hf_stm32_nvs_config_t()) noexcept;

    ~StmNvs() noexcept override;

    // ── BaseNvs overrides ────────────────────────────────────────────────

    hf_nvs_err_t Initialize() noexcept override;
    hf_nvs_err_t Deinitialize() noexcept override;

    hf_nvs_err_t SetU32(const char* key, hf_u32_t value) noexcept override;
    hf_nvs_err_t GetU32(const char* key, hf_u32_t& value) noexcept override;

    hf_nvs_err_t SetString(const char* key, const char* value) noexcept override;
    hf_nvs_err_t GetString(const char* key, char* buffer, size_t buffer_size,
                           size_t* actual_size = nullptr) noexcept override;

    hf_nvs_err_t SetBlob(const char* key, const void* data, size_t data_size) noexcept override;
    hf_nvs_err_t GetBlob(const char* key, void* buffer, size_t buffer_size,
                         size_t* actual_size = nullptr) noexcept override;

    hf_nvs_err_t EraseKey(const char* key) noexcept override;
    hf_nvs_err_t Commit() noexcept override;
    bool KeyExists(const char* key) noexcept override;
    hf_nvs_err_t GetSize(const char* key, size_t& size) noexcept override;

    const char* GetDescription() const noexcept override;
    size_t GetMaxKeyLength() const noexcept override;
    size_t GetMaxValueSize() const noexcept override;

    // ── STM32-specific ───────────────────────────────────────────────────

    /// @brief Get the flash configuration
    const hf_stm32_nvs_config_t& GetFlashConfig() const noexcept { return flash_config_; }

    /// @brief Get number of stored entries
    size_t GetEntryCount() const noexcept { return entry_count_; }

    /// @brief Get available free space in current sector (bytes)
    size_t GetFreeSpace() const noexcept;

private:
    /// @brief NVS entry types
    enum class EntryType : hf_u8_t {
        U32    = 0x01,
        STRING = 0x02,
        BLOB   = 0x03,
        ERASED = 0xFF
    };

    /// @brief Entry header stored in flash (packed)
    struct __attribute__((packed)) EntryHeader {
        hf_u8_t  key_length;      ///< Key string length (not including null)
        EntryType type;            ///< Value type
        hf_u16_t data_length;     ///< Data length in bytes
        hf_u32_t crc32;           ///< CRC32 of key + data
    };

    /// @brief RAM cache entry
    struct CacheEntry {
        char     key[64];          ///< Key string
        EntryType type;            ///< Value type
        hf_u8_t  data[256];       ///< Value data
        hf_u16_t data_length;     ///< Actual data length
        bool     dirty;           ///< Needs flash write
        bool     erased;          ///< Marked for deletion
    };

    /// @brief Find entry in RAM cache by key
    int FindCacheEntry(const char* key) const noexcept;

    /// @brief Add or update entry in RAM cache
    int AddOrUpdateCache(const char* key, EntryType type,
                         const void* data, size_t data_size) noexcept;

    /// @brief Validate a key string
    bool IsValidKey(const char* key) const noexcept;

    /// @brief Write all dirty entries to flash
    hf_nvs_err_t FlushToFlash() noexcept;

    /// @brief Read entries from flash into RAM cache
    hf_nvs_err_t LoadFromFlash() noexcept;

    /// @brief Simple CRC32 for entry validation
    static hf_u32_t ComputeCrc32(const void* data, size_t length) noexcept;

    hf_stm32_nvs_config_t flash_config_;        ///< Flash region config
    static constexpr size_t kMaxEntries = 64;    ///< Max cached entries
    CacheEntry             cache_[kMaxEntries];  ///< RAM cache
    size_t                 entry_count_;          ///< Current entry count
    hf_u32_t               write_offset_;         ///< Current write position in flash
};
