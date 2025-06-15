/**
 * @file NvsStorage.h
 * @brief Lightweight NVS helper for key-value storage.
 */
#ifndef NVSSTORAGE_H_
#define NVSSTORAGE_H_

#include <cstdint>
#include <nvs.h>
#include <nvs_flash.h>

/**
 * @class NvsStorage
 * @brief Minimal RAII wrapper for an NVS namespace.
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

  /** Check if the handle is valid. */
  bool IsOpened() const noexcept {
    return handle != 0;
  }

private:
  const char *nsName;  ///< Namespace string
  nvs_handle_t handle; ///< Open handle or 0
};

#endif // NVSSTORAGE_H_
