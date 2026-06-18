/**
 * @file HostNvs.cpp
 * @brief In-memory NVS for host / unit-test builds.
 */

#include "HostNvs.h"

#include <cstring>

HostNvs::HostNvs(const char* name) : BaseNvs(name), name_(name) {}

hf_nvs_err_t HostNvs::Initialize() noexcept { return hf_nvs_err_t::NVS_SUCCESS; }
hf_nvs_err_t HostNvs::Deinitialize() noexcept { return hf_nvs_err_t::NVS_SUCCESS; }
hf_nvs_err_t HostNvs::Commit() noexcept { return hf_nvs_err_t::NVS_SUCCESS; }

hf_nvs_err_t HostNvs::SetBlob(const char* key, const void* data, size_t data_size) noexcept {
  if (!key || !data) return hf_nvs_err_t::NVS_ERR_NULL_POINTER;
  blobs_[key] = std::vector<uint8_t>(static_cast<const uint8_t*>(data),
                                     static_cast<const uint8_t*>(data) + data_size);
  return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t HostNvs::GetBlob(const char* key, void* buffer, size_t buffer_size,
                              size_t* actual_size) noexcept {
  auto it = blobs_.find(key);
  if (it == blobs_.end()) return hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND;
  if (!buffer) return hf_nvs_err_t::NVS_ERR_NULL_POINTER;
  if (buffer_size < it->second.size()) return hf_nvs_err_t::NVS_ERR_VALUE_TOO_LARGE;
  std::memcpy(buffer, it->second.data(), it->second.size());
  if (actual_size) *actual_size = it->second.size();
  return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t HostNvs::SetU32(const char* key, hf_u32_t value) noexcept {
  if (!key) return hf_nvs_err_t::NVS_ERR_NULL_POINTER;
  u32s_[key] = value;
  return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t HostNvs::GetU32(const char* key, hf_u32_t& value) noexcept {
  auto it = u32s_.find(key);
  if (it == u32s_.end()) return hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND;
  value = it->second;
  return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t HostNvs::SetString(const char* key, const char* value) noexcept {
  if (!key || !value) return hf_nvs_err_t::NVS_ERR_NULL_POINTER;
  strings_[key] = value;
  return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t HostNvs::GetString(const char* key, char* buffer, size_t buffer_size,
                                  size_t* actual_size) noexcept {
  auto it = strings_.find(key);
  if (it == strings_.end()) return hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND;
  if (!buffer) return hf_nvs_err_t::NVS_ERR_NULL_POINTER;
  if (buffer_size < it->second.size() + 1) return hf_nvs_err_t::NVS_ERR_VALUE_TOO_LARGE;
  std::strncpy(buffer, it->second.c_str(), buffer_size);
  if (actual_size) *actual_size = it->second.size();
  return hf_nvs_err_t::NVS_SUCCESS;
}

hf_nvs_err_t HostNvs::EraseKey(const char* key) noexcept {
  blobs_.erase(key);
  u32s_.erase(key);
  strings_.erase(key);
  return hf_nvs_err_t::NVS_SUCCESS;
}

bool HostNvs::KeyExists(const char* key) noexcept {
  return blobs_.count(key) || u32s_.count(key) || strings_.count(key);
}

hf_nvs_err_t HostNvs::GetSize(const char* key, size_t& size) noexcept {
  if (auto it = blobs_.find(key); it != blobs_.end()) {
    size = it->second.size();
    return hf_nvs_err_t::NVS_SUCCESS;
  }
  if (auto it = u32s_.find(key); it != u32s_.end()) {
    size = sizeof(hf_u32_t);
    return hf_nvs_err_t::NVS_SUCCESS;
  }
  if (auto it = strings_.find(key); it != strings_.end()) {
    size = it->second.size();
    return hf_nvs_err_t::NVS_SUCCESS;
  }
  return hf_nvs_err_t::NVS_ERR_KEY_NOT_FOUND;
}

const char* HostNvs::GetDescription() const noexcept { return "hf host NVS (in-memory)"; }
size_t HostNvs::GetMaxKeyLength() const noexcept { return 64; }
size_t HostNvs::GetMaxValueSize() const noexcept { return 4096; }
