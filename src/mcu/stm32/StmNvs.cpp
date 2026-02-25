/**
 * @file StmNvs.cpp
 * @brief STM32 NVS implementation — flash-backed key-value store.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmNvs.h"
#include <cstring>
#include <cstdio>

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL FORWARD DECLARATIONS (Flash operations)
// ═══════════════════════════════════════════════════════════════════════════════

extern "C" {
extern uint32_t HAL_FLASH_Unlock(void);
extern uint32_t HAL_FLASH_Lock(void);
extern uint32_t HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data);
extern uint32_t HAL_FLASHEx_Erase(void* pEraseInit, uint32_t* SectorError);
}

// Flash program type constants
namespace {
    constexpr uint32_t kFlashTypeProgramByte     = 0x00U;
    constexpr uint32_t kFlashTypeProgramHalfWord = 0x01U;
    constexpr uint32_t kFlashTypeProgramWord     = 0x02U;
    constexpr uint32_t kFlashTypeProgramDWord    = 0x03U;
}

// ═══════════════════════════════════════════════════════════════════════════════
// CONSTRUCTOR / DESTRUCTOR
// ═══════════════════════════════════════════════════════════════════════════════

StmNvs::StmNvs(const char* namespace_name,
               const hf_stm32_nvs_config_t& flash_config) noexcept
    : BaseNvs()
    , flash_config_(flash_config)
    , entry_count_(0)
    , write_offset_(0)
{
    std::memset(cache_, 0, sizeof(cache_));
    if (namespace_name) {
        strncpy(namespace_name_, namespace_name, sizeof(namespace_name_) - 1);
        namespace_name_[sizeof(namespace_name_) - 1] = '\0';
    }
}

StmNvs::~StmNvs() noexcept {
    if (initialized_) Deinitialize();
}

// ═══════════════════════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════

hf_nvs_err_t StmNvs::Initialize() noexcept {
    if (initialized_) return hf_nvs_err_t::NVS_SUCCESS;

    // Validate flash config
    if (flash_config_.flash_start_address == 0 || flash_config_.flash_size == 0) {
        // No flash configured — operate as RAM-only cache (volatile)
        // This is valid for testing or when persistent storage isn't needed
    }

    // Clear RAM cache
    std::memset(cache_, 0, sizeof(cache_));
    entry_count_ = 0;
    write_offset_ = 0;

    // Attempt to load existing data from flash
    if (flash_config_.flash_start_address != 0 && flash_config_.flash_size != 0) {
        LoadFromFlash();
    }

    initialized_ = true;
    return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t StmNvs::Deinitialize() noexcept {
    if (!initialized_) return hf_nvs_err_t::NVS_SUCCESS;

    // Flush any dirty entries before deinitializing
    FlushToFlash();

    initialized_ = false;
    return hf_nvs_err_t::NVS_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// KEY-VALUE OPERATIONS
// ═══════════════════════════════════════════════════════════════════════════════

hf_nvs_err_t StmNvs::SetU32(const char* key, hf_u32_t value) noexcept {
    if (!EnsureInitialized()) return hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED;
    if (!IsValidKey(key)) return hf_nvs_err_t::NVS_ERR_INVALID_KEY;

    int idx = AddOrUpdateCache(key, EntryType::U32, &value, sizeof(value));
    if (idx < 0) return hf_nvs_err_t::NVS_ERR_STORAGE_FULL;

    statistics_.total_writes++;
    return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t StmNvs::GetU32(const char* key, hf_u32_t& value) noexcept {
    if (!EnsureInitialized()) return hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED;
    if (!IsValidKey(key)) return hf_nvs_err_t::NVS_ERR_INVALID_KEY;

    int idx = FindCacheEntry(key);
    if (idx < 0 || cache_[idx].erased) return hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND;
    if (cache_[idx].type != EntryType::U32) return hf_nvs_err_t::NVS_ERR_TYPE_MISMATCH;
    if (cache_[idx].data_length != sizeof(hf_u32_t)) return hf_nvs_err_t::NVS_ERR_CORRUPTED_DATA;

    std::memcpy(&value, cache_[idx].data, sizeof(hf_u32_t));
    statistics_.total_reads++;
    return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t StmNvs::SetString(const char* key, const char* value) noexcept {
    if (!EnsureInitialized()) return hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED;
    if (!IsValidKey(key) || !value) return hf_nvs_err_t::NVS_ERR_INVALID_KEY;

    size_t len = std::strlen(value) + 1;  // Include null terminator
    if (len > hf::stm32::kNvsMaxValueSize) return hf_nvs_err_t::NVS_ERR_VALUE_TOO_LONG;

    int idx = AddOrUpdateCache(key, EntryType::STRING, value, len);
    if (idx < 0) return hf_nvs_err_t::NVS_ERR_STORAGE_FULL;

    statistics_.total_writes++;
    return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t StmNvs::GetString(const char* key, char* buffer, size_t buffer_size,
                                size_t* actual_size) noexcept {
    if (!EnsureInitialized()) return hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED;
    if (!IsValidKey(key) || !buffer) return hf_nvs_err_t::NVS_ERR_INVALID_KEY;

    int idx = FindCacheEntry(key);
    if (idx < 0 || cache_[idx].erased) return hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND;
    if (cache_[idx].type != EntryType::STRING) return hf_nvs_err_t::NVS_ERR_TYPE_MISMATCH;

    if (actual_size) *actual_size = cache_[idx].data_length;

    size_t copy_len = (cache_[idx].data_length < buffer_size)
                      ? cache_[idx].data_length : buffer_size - 1;
    std::memcpy(buffer, cache_[idx].data, copy_len);
    buffer[copy_len] = '\0';

    statistics_.total_reads++;
    return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t StmNvs::SetBlob(const char* key, const void* data, size_t data_size) noexcept {
    if (!EnsureInitialized()) return hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED;
    if (!IsValidKey(key) || !data) return hf_nvs_err_t::NVS_ERR_INVALID_KEY;
    if (data_size > sizeof(CacheEntry::data)) return hf_nvs_err_t::NVS_ERR_VALUE_TOO_LONG;

    int idx = AddOrUpdateCache(key, EntryType::BLOB, data, data_size);
    if (idx < 0) return hf_nvs_err_t::NVS_ERR_STORAGE_FULL;

    statistics_.total_writes++;
    return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t StmNvs::GetBlob(const char* key, void* buffer, size_t buffer_size,
                              size_t* actual_size) noexcept {
    if (!EnsureInitialized()) return hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED;
    if (!IsValidKey(key) || !buffer) return hf_nvs_err_t::NVS_ERR_INVALID_KEY;

    int idx = FindCacheEntry(key);
    if (idx < 0 || cache_[idx].erased) return hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND;
    if (cache_[idx].type != EntryType::BLOB) return hf_nvs_err_t::NVS_ERR_TYPE_MISMATCH;

    if (actual_size) *actual_size = cache_[idx].data_length;

    size_t copy_len = (cache_[idx].data_length < buffer_size)
                      ? cache_[idx].data_length : buffer_size;
    std::memcpy(buffer, cache_[idx].data, copy_len);

    statistics_.total_reads++;
    return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t StmNvs::EraseKey(const char* key) noexcept {
    if (!EnsureInitialized()) return hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED;
    if (!IsValidKey(key)) return hf_nvs_err_t::NVS_ERR_INVALID_KEY;

    int idx = FindCacheEntry(key);
    if (idx < 0) return hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND;

    cache_[idx].erased = true;
    cache_[idx].dirty = true;
    return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t StmNvs::Commit() noexcept {
    if (!EnsureInitialized()) return hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED;
    return FlushToFlash();
}

bool StmNvs::KeyExists(const char* key) noexcept {
    if (!initialized_ || !IsValidKey(key)) return false;
    int idx = FindCacheEntry(key);
    return idx >= 0 && !cache_[idx].erased;
}

hf_nvs_err_t StmNvs::GetSize(const char* key, size_t& size) noexcept {
    if (!EnsureInitialized()) return hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED;
    if (!IsValidKey(key)) return hf_nvs_err_t::NVS_ERR_INVALID_KEY;

    int idx = FindCacheEntry(key);
    if (idx < 0 || cache_[idx].erased) return hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND;

    size = cache_[idx].data_length;
    return hf_nvs_err_t::NVS_SUCCESS;
}

const char* StmNvs::GetDescription() const noexcept {
    return "STM32 Flash NVS";
}

size_t StmNvs::GetMaxKeyLength() const noexcept {
    return hf::stm32::kNvsMaxKeyLength;
}

size_t StmNvs::GetMaxValueSize() const noexcept {
    return sizeof(CacheEntry::data);
}

size_t StmNvs::GetFreeSpace() const noexcept {
    return (kMaxEntries > entry_count_) ? (kMaxEntries - entry_count_) * sizeof(CacheEntry) : 0;
}

// ═══════════════════════════════════════════════════════════════════════════════
// PRIVATE HELPERS
// ═══════════════════════════════════════════════════════════════════════════════

int StmNvs::FindCacheEntry(const char* key) const noexcept {
    for (size_t i = 0; i < entry_count_; ++i) {
        if (std::strcmp(cache_[i].key, key) == 0) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int StmNvs::AddOrUpdateCache(const char* key, EntryType type,
                              const void* data, size_t data_size) noexcept {
    if (data_size > sizeof(CacheEntry::data)) return -1;

    int idx = FindCacheEntry(key);
    if (idx < 0) {
        // New entry
        if (entry_count_ >= kMaxEntries) return -1;
        idx = static_cast<int>(entry_count_++);
        std::strncpy(cache_[idx].key, key, sizeof(CacheEntry::key) - 1);
        cache_[idx].key[sizeof(CacheEntry::key) - 1] = '\0';
    }

    cache_[idx].type = type;
    cache_[idx].data_length = static_cast<hf_u16_t>(data_size);
    std::memcpy(cache_[idx].data, data, data_size);
    cache_[idx].dirty = true;
    cache_[idx].erased = false;
    return idx;
}

bool StmNvs::IsValidKey(const char* key) const noexcept {
    if (!key || key[0] == '\0') return false;
    size_t len = std::strlen(key);
    return len > 0 && len <= hf::stm32::kNvsMaxKeyLength;
}

hf_nvs_err_t StmNvs::FlushToFlash() noexcept {
    if (flash_config_.flash_start_address == 0 || flash_config_.flash_size == 0) {
        // RAM-only mode — nothing to flush
        // Mark all as non-dirty
        for (size_t i = 0; i < entry_count_; ++i) {
            cache_[i].dirty = false;
        }
        return hf_nvs_err_t::NVS_SUCCESS;
    }

    // Unlock flash
    uint32_t status = HAL_FLASH_Unlock();
    if (!hf::stm32::IsHalOk(status)) {
        return hf_nvs_err_t::NVS_ERR_WRITE_FAILED;
    }

    // Write dirty entries to flash as word-aligned data
    hf_u32_t address = flash_config_.flash_start_address + write_offset_;
    for (size_t i = 0; i < entry_count_; ++i) {
        if (!cache_[i].dirty) continue;

        // Write entry header + key + data as sequential words
        EntryHeader header{};
        header.key_length = static_cast<hf_u8_t>(std::strlen(cache_[i].key));
        header.type = cache_[i].erased ? EntryType::ERASED : cache_[i].type;
        header.data_length = cache_[i].data_length;
        header.crc32 = ComputeCrc32(cache_[i].data, cache_[i].data_length);

        // Check space
        size_t entry_size = sizeof(EntryHeader) + header.key_length + header.data_length;
        entry_size = (entry_size + 3) & ~3U;  // Align to 4 bytes

        if (address + entry_size > flash_config_.flash_start_address + flash_config_.flash_size) {
            HAL_FLASH_Lock();
            return hf_nvs_err_t::NVS_ERR_STORAGE_FULL;
        }

        // Program header word by word
        const uint8_t* src = reinterpret_cast<const uint8_t*>(&header);
        for (size_t b = 0; b < sizeof(header); b += 4) {
            uint32_t word = 0;
            std::memcpy(&word, src + b, (b + 4 <= sizeof(header)) ? 4 : sizeof(header) - b);
            HAL_FLASH_Program(kFlashTypeProgramWord, address, word);
            address += 4;
        }

        // Program key
        for (size_t b = 0; b < header.key_length; b += 4) {
            uint32_t word = 0;
            size_t chunk = (b + 4 <= header.key_length) ? 4 : header.key_length - b;
            std::memcpy(&word, cache_[i].key + b, chunk);
            HAL_FLASH_Program(kFlashTypeProgramWord, address, word);
            address += 4;
        }

        // Program data
        for (size_t b = 0; b < header.data_length; b += 4) {
            uint32_t word = 0;
            size_t chunk = (b + 4 <= header.data_length) ? 4 : header.data_length - b;
            std::memcpy(&word, cache_[i].data + b, chunk);
            HAL_FLASH_Program(kFlashTypeProgramWord, address, word);
            address += 4;
        }

        cache_[i].dirty = false;
    }

    write_offset_ = address - flash_config_.flash_start_address;
    HAL_FLASH_Lock();
    return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t StmNvs::LoadFromFlash() noexcept {
    if (flash_config_.flash_start_address == 0 || flash_config_.flash_size == 0) {
        return hf_nvs_err_t::NVS_SUCCESS;
    }

    hf_u32_t address = flash_config_.flash_start_address;
    hf_u32_t end_address = address + flash_config_.flash_size;

    entry_count_ = 0;

    while (address + sizeof(EntryHeader) < end_address) {
        EntryHeader header{};
        std::memcpy(&header, reinterpret_cast<const void*>(address), sizeof(header));

        // Check for empty flash (all 0xFF)
        if (header.key_length == 0xFF || header.key_length == 0) break;
        if (header.type == EntryType::ERASED) {
            // Skip erased entry
            address += sizeof(EntryHeader) + header.key_length + header.data_length;
            address = (address + 3) & ~3U;
            continue;
        }

        if (entry_count_ >= kMaxEntries) break;

        address += sizeof(EntryHeader);

        // Read key
        size_t key_len = header.key_length;
        if (key_len >= sizeof(CacheEntry::key)) key_len = sizeof(CacheEntry::key) - 1;
        std::memcpy(cache_[entry_count_].key, reinterpret_cast<const void*>(address), key_len);
        cache_[entry_count_].key[key_len] = '\0';
        address += header.key_length;
        address = (address + 3) & ~3U;

        // Read data
        size_t data_len = header.data_length;
        if (data_len > sizeof(CacheEntry::data)) data_len = sizeof(CacheEntry::data);
        std::memcpy(cache_[entry_count_].data, reinterpret_cast<const void*>(address), data_len);
        cache_[entry_count_].data_length = static_cast<hf_u16_t>(data_len);
        cache_[entry_count_].type = header.type;
        cache_[entry_count_].dirty = false;
        cache_[entry_count_].erased = false;
        address += header.data_length;
        address = (address + 3) & ~3U;

        entry_count_++;
    }

    write_offset_ = address - flash_config_.flash_start_address;
    return hf_nvs_err_t::NVS_SUCCESS;
}

hf_u32_t StmNvs::ComputeCrc32(const void* data, size_t length) noexcept {
    // Simple CRC32 (no lookup table — minimal flash footprint)
    hf_u32_t crc = 0xFFFFFFFFU;
    const hf_u8_t* bytes = static_cast<const hf_u8_t*>(data);
    for (size_t i = 0; i < length; ++i) {
        crc ^= bytes[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320U & (-(crc & 1U)));
        }
    }
    return ~crc;
}
