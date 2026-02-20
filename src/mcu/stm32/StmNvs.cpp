/**
 * @file StmNvs.cpp
 * @brief STM32 NVS stub implementation.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#include "StmNvs.h"

StmNvs::StmNvs(const char* /*namespace_name*/) noexcept {}
StmNvs::~StmNvs() noexcept = default;

hf_nvs_err_t StmNvs::Initialize() noexcept { return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION; }
hf_nvs_err_t StmNvs::Deinitialize() noexcept { return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION; }

hf_nvs_err_t StmNvs::SetU32(const char*, hf_u32_t) noexcept {
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
}
hf_nvs_err_t StmNvs::GetU32(const char*, hf_u32_t&) noexcept {
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
}
hf_nvs_err_t StmNvs::SetString(const char*, const char*) noexcept {
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
}
hf_nvs_err_t StmNvs::GetString(const char*, char*, size_t, size_t*) noexcept {
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
}
hf_nvs_err_t StmNvs::SetBlob(const char*, const void*, size_t) noexcept {
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
}
hf_nvs_err_t StmNvs::GetBlob(const char*, void*, size_t, size_t*) noexcept {
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
}
hf_nvs_err_t StmNvs::EraseKey(const char*) noexcept {
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
}
hf_nvs_err_t StmNvs::Commit() noexcept {
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
}
bool StmNvs::KeyExists(const char*) noexcept { return false; }
hf_nvs_err_t StmNvs::GetSize(const char*, size_t&) noexcept {
    return hf_nvs_err_t::NVS_ERR_UNSUPPORTED_OPERATION;
}
const char* StmNvs::GetDescription() const noexcept { return "STM32 NVS (stub)"; }
size_t StmNvs::GetMaxKeyLength() const noexcept { return 0; }
size_t StmNvs::GetMaxValueSize() const noexcept { return 0; }
