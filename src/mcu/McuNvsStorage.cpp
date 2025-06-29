/**
 * @file McuNvsStorage.cpp
 * @brief Implementation of the McuNvsStorage class.
 *
 * This file provides the implementation for MCU-integrated non-volatile storage
 * using the platform's built-in NVS capabilities. On ESP32, this wraps the
 * NVS (Non-Volatile Storage) API for key-value storage, namespace management,
 * and data persistence across power cycles with comprehensive error handling.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "McuNvsStorage.h"
#include <cstring>

McuNvsStorage::McuNvsStorage(const char *ns) noexcept : nsName(ns), handle(0) {}

McuNvsStorage::~McuNvsStorage() noexcept {
  if (handle) {
    Close();
  }
}

bool McuNvsStorage::Open() noexcept {
  if (handle)
    return true;
  hf_err_t err = HF_NVS_FLASH_INIT();
  if (err != HF_OK && err != HF_ERR_NVS_NO_FREE_PAGES && err != HF_ERR_NVS_NEW_VERSION_FOUND) {
    return false;
  }
  if (err != HF_OK) {
    nvs_flash_erase();
    if (nvs_flash_init() != ESP_OK)
      return false;
  }
  err = nvs_open(nsName, NVS_READWRITE, &handle);
  return err == ESP_OK;
}

void McuNvsStorage::Close() noexcept {
  if (handle) {
    nvs_close(handle);
    handle = 0;
  }
}

bool McuNvsStorage::SetU32(const char *key, uint32_t value) noexcept {
  if (!handle)
    return false;
  esp_err_t err = nvs_set_u32(handle, key, value);
  if (err != ESP_OK)
    return false;
  return Commit();
}

bool McuNvsStorage::GetU32(const char *key, uint32_t &value) noexcept {
  if (!handle)
    return false;
  esp_err_t err = nvs_get_u32(handle, key, &value);
  return err == ESP_OK;
}

bool McuNvsStorage::EraseKey(const char *key) noexcept {
  if (!handle)
    return false;
  esp_err_t err = nvs_erase_key(handle, key);
  if (err != ESP_OK)
    return false;
  return Commit();
}

bool McuNvsStorage::Commit() noexcept {
  if (!handle)
    return false;
  return nvs_commit(handle) == ESP_OK;
}
