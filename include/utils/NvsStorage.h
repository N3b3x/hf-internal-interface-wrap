/**
 * @file NvsStorage.h
 * @brief Platform-agnostic NVS (Non-Volatile Storage) helper for key-value storage.
 *
 * This class provides a platform-agnostic interface for non-volatile storage
 * operations. The implementation adapts to the current platform's storage
 * mechanism while providing a consistent API.
 */
#ifndef NVSSTORAGE_H_
#define NVSSTORAGE_H_

#include "../mcu/McuTypes.h"
#include <cstdint>

/**
 * @class NvsStorage
 * @brief Platform-agnostic RAII wrapper for an NVS namespace.
 */
class NvsStorage {
public:
  /**
   * @brief Construct storage bound to a namespace.
   * @param ns Namespace string
   */
  explicit NvsStorage(const char *ns) noexcept;
  ~NvsStorage() noexcept;
  NvsStorage(const NvsStorage &) = delete;
  NvsStorage &operator=(const NvsStorage &) = delete;

  /** Open the namespace creating it if needed. */
  bool Open() noexcept;
  /** Close the handle if open. */
  void Close() noexcept;

  /** Store a 32-bit value under a key. */
  bool SetU32(const char *key, uint32_t value) noexcept;
  /** Retrieve a 32-bit value. */
  bool GetU32(const char *key, uint32_t &value) noexcept;
  /** Remove a key from storage. */
  bool EraseKey(const char *key) noexcept;
  /** Commit any pending writes. */
  bool Commit() noexcept;

private:
  const char *nsName;  ///< Namespace string
  void* handle;               // Platform-specific storage handle
};

#endif // NVSSTORAGE_H_
