/**
 * @file StmNvsBackend.h
 * @brief Weak product hooks for StmNvs flash I/O (external / QSPI-backed NVS).
 *
 * Addresses are absolute logical map addresses (e.g. memory-mapped QSPI slice).
 * The product firmware provides strong definitions for these symbols.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @return 0 on success. */
int PwStmNvsBackendRead(uint32_t abs_addr, void* buf, size_t len);
int PwStmNvsBackendWrite(uint32_t abs_addr, const void* buf, size_t len);
int PwStmNvsBackendSync(void);
/** @brief Erase [abs_addr, abs_addr+len) for compaction (4 KiB aligned OK). */
int PwStmNvsBackendErase(uint32_t abs_addr, size_t len);

#ifdef __cplusplus
}
#endif
