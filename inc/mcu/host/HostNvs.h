/**
 * @file HostNvs.h
 * @brief In-memory `BaseNvs` for host / unit-test builds (hf-internal-interface-wrap).
 *
 * Follows the same layout as `EspNvs` and `StmNvs`: concrete `BaseNvs`
 * implementations live under hf-internal-interface-wrap, not product adapters.
 */

#pragma once

#include "BaseNvs.h"

#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Hash-map NVS namespace for host builds and deterministic tests.
 */
class HostNvs : public BaseNvs {
 public:
  explicit HostNvs(const char* name);
  ~HostNvs() override = default;

 protected:
  hf_nvs_err_t Initialize() noexcept override;
  hf_nvs_err_t Deinitialize() noexcept override;
  hf_nvs_err_t SetU32(const char* key, hf_u32_t value) noexcept override;
  hf_nvs_err_t GetU32(const char* key, hf_u32_t& value) noexcept override;
  hf_nvs_err_t SetString(const char* key, const char* value) noexcept override;
  hf_nvs_err_t GetString(const char* key, char* buffer, size_t buffer_size,
                         size_t* actual_size) noexcept override;
  hf_nvs_err_t SetBlob(const char* key, const void* data, size_t data_size) noexcept override;
  hf_nvs_err_t GetBlob(const char* key, void* buffer, size_t buffer_size,
                       size_t* actual_size) noexcept override;
  hf_nvs_err_t EraseKey(const char* key) noexcept override;
  hf_nvs_err_t Commit() noexcept override;
  bool KeyExists(const char* key) noexcept override;
  hf_nvs_err_t GetSize(const char* key, size_t& size) noexcept override;
  const char* GetDescription() const noexcept override;
  size_t GetMaxKeyLength() const noexcept override;
  size_t GetMaxValueSize() const noexcept override;

 private:
  const char* name_;
  std::unordered_map<std::string, std::vector<uint8_t>> blobs_;
  std::unordered_map<std::string, hf_u32_t> u32s_;
  std::unordered_map<std::string, std::string> strings_;
};
