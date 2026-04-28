/**
 * @file iiwrap_version.h
 * @brief Version macros for the hf-internal-interface-wrap submodule.
 *
 * Manually-curated semantic version. Bump on releases together with
 * the matching git tag in this repo. Consumers (e.g. the parent app's
 * version manifest) pick this up via __has_include of `iiwrap_version.h`.
 */
#pragma once

#define HF_IIWRAP_VERSION_MAJOR  1
#define HF_IIWRAP_VERSION_MINOR  0
#define HF_IIWRAP_VERSION_PATCH  0
#define HF_IIWRAP_VERSION_STRING "1.0.0"
