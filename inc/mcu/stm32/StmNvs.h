/**
 * @file StmNvs.h
 * @brief STM32 NVS (non-volatile storage) stub — placeholder for flash/EEPROM integration.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseNvs.h"

/**
 * @brief STM32 NVS — stub implementation.
 */
class StmNvs : public BaseNvs {
public:
    explicit StmNvs(const char* namespace_name = "default") noexcept;
    ~StmNvs() noexcept override;

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
};
