/**
 * @file NvsStorage.cpp
 * @brief Implementation of the NvsStorage class.
 */

#include "NvsStorage.h"
#include <cstring>

NvsStorage::NvsStorage(const char* ns) noexcept
    : nsName(ns), handle(0) {}

NvsStorage::~NvsStorage() noexcept {
    if (handle) {
        Close();
    }
}

bool NvsStorage::Open() noexcept {
    if (handle)
        return true;
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK && err != ESP_ERR_NVS_NO_FREE_PAGES &&
        err != ESP_ERR_NVS_NEW_VERSION_FOUND) {
        return false;
    }
    if (err != ESP_OK) {
        nvs_flash_erase();
        if (nvs_flash_init() != ESP_OK)
            return false;
    }
    err = nvs_open(nsName, NVS_READWRITE, &handle);
    return err == ESP_OK;
}

void NvsStorage::Close() noexcept {
    if (handle) {
        nvs_close(handle);
        handle = 0;
    }
}

bool NvsStorage::SetU32(const char* key, uint32_t value) noexcept {
    if (!handle)
        return false;
    esp_err_t err = nvs_set_u32(handle, key, value);
    if (err != ESP_OK)
        return false;
    return Commit();
}

bool NvsStorage::GetU32(const char* key, uint32_t& value) noexcept {
    if (!handle)
        return false;
    esp_err_t err = nvs_get_u32(handle, key, &value);
    return err == ESP_OK;
}

bool NvsStorage::EraseKey(const char* key) noexcept {
    if (!handle)
        return false;
    esp_err_t err = nvs_erase_key(handle, key);
    if (err != ESP_OK)
        return false;
    return Commit();
}

bool NvsStorage::Commit() noexcept {
    if (!handle)
        return false;
    return nvs_commit(handle) == ESP_OK;
}
