/**
 * @file McuTypes_NVS.h
 * @brief MCU-specific NVS type definitions for hardware abstraction.
 *
 * This header defines all NVS-specific types and constants that are used
 * throughout the internal interface wrap layer for NVS operations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "McuTypes_Base.h"
#include "BaseNvsStorage.h" // For HfNvsErr

#ifdef HF_MCU_FAMILY_ESP32
// ESP32-C6 NVS native type aliases for abstraction
using hf_nvs_handle_native_t = nvs_handle_t;
using hf_nvs_open_mode_native_t = nvs_open_mode_t;
using hf_nvs_type_native_t = nvs_type_t;
using hf_nvs_iterator_native_t = nvs_iterator_t;

// ESP32-C6 NVS constants
static constexpr size_t HF_NVS_MAX_KEY_LENGTH = 15;         ///< Maximum NVS key length (ESP32 limit)
static constexpr size_t HF_NVS_MAX_VALUE_SIZE = 4000;      ///< Maximum NVS value size (conservative)
static constexpr size_t HF_NVS_MAX_NAMESPACE_LENGTH = 15;  ///< Maximum namespace length
static constexpr size_t HF_NVS_MAX_NAMESPACES = 254;       ///< Maximum number of namespaces

// ESP32-C6 NVS operation timeouts
static constexpr uint32_t HF_NVS_OPERATION_TIMEOUT_MS = 1000;    ///< Default operation timeout
static constexpr uint32_t HF_NVS_INIT_TIMEOUT_MS = 5000;         ///< Initialization timeout
static constexpr uint32_t HF_NVS_COMMIT_TIMEOUT_MS = 2000;       ///< Commit operation timeout

// ESP32-C6 NVS validation macros
#define HF_NVS_IS_VALID_KEY_LENGTH(len) ((len) > 0 && (len) <= HF_NVS_MAX_KEY_LENGTH)
#define HF_NVS_IS_VALID_VALUE_SIZE(size) ((size) <= HF_NVS_MAX_VALUE_SIZE)
#define HF_NVS_IS_VALID_NAMESPACE_LENGTH(len) ((len) > 0 && (len) <= HF_NVS_MAX_NAMESPACE_LENGTH)


#else
// Generic implementations for non-ESP32 platforms
using hf_nvs_handle_native_t = uint32_t;
using hf_nvs_open_mode_native_t = int;
using hf_nvs_type_native_t = int;
using hf_nvs_iterator_native_t = void*;

static constexpr size_t HF_NVS_MAX_KEY_LENGTH = 32;
static constexpr size_t HF_NVS_MAX_VALUE_SIZE = 1024;
static constexpr size_t HF_NVS_MAX_NAMESPACE_LENGTH = 32;
static constexpr size_t HF_NVS_MAX_NAMESPACES = 256;

static constexpr uint32_t HF_NVS_OPERATION_TIMEOUT_MS = 1000;
static constexpr uint32_t HF_NVS_INIT_TIMEOUT_MS = 5000;
static constexpr uint32_t HF_NVS_COMMIT_TIMEOUT_MS = 2000;

#define HF_NVS_IS_VALID_KEY_LENGTH(len) ((len) > 0 && (len) <= HF_NVS_MAX_KEY_LENGTH)
#define HF_NVS_IS_VALID_VALUE_SIZE(size) ((size) <= HF_NVS_MAX_VALUE_SIZE)
#define HF_NVS_IS_VALID_NAMESPACE_LENGTH(len) ((len) > 0 && (len) <= HF_NVS_MAX_NAMESPACE_LENGTH)

#endif
