#pragma once

#include <memory>
#include <new>
#include <utility>

namespace hf {
namespace utils {

/**
 * @brief Creates a unique_ptr using nothrow new for exception-free design
 *
 * This function template provides a safe alternative to std::make_unique
 * for environments that require no-exception guarantees. It uses nothrow
 * new to allocate memory and returns nullptr on allocation failure instead
 * of throwing std::bad_alloc.
 *
 * @tparam T The type to create
 * @tparam Args Parameter pack for constructor arguments
 * @param args Arguments to forward to T's constructor
 * @return std::unique_ptr<T> Valid pointer on success, nullptr on allocation failure
 *
 * @example
 * @code
 * auto ptr = make_unique_nothrow<MyClass>(arg1, arg2);
 * if (!ptr) {
 *     // Handle allocation failure
 *     return false;
 * }
 * // Use ptr safely
 * @endcode
 */
template <typename T, typename... Args>
std::unique_ptr<T> make_unique_nothrow(Args&&... args) {
  T* raw_ptr = new (std::nothrow) T(std::forward<Args>(args)...);
  if (!raw_ptr) {
    return nullptr;
  }
  return std::unique_ptr<T>(raw_ptr);
}

/**
 * @brief Creates a unique_ptr for arrays using nothrow new
 *
 * Specialized version for creating arrays with nothrow allocation.
 *
 * @tparam T The array element type
 * @param size Number of elements to allocate
 * @return std::unique_ptr<T[]> Valid pointer on success, nullptr on allocation failure
 *
 * @par Array Allocation Example:
 * @code
 * auto arr = make_unique_array_nothrow<int>(1000);
 * if (!arr) {
 *     // Handle allocation failure
 *     return false;
 * }
 * // Use arr safely
 * @endcode
 */
template <typename T>
std::unique_ptr<T[]> make_unique_array_nothrow(size_t size) {
  T* raw_ptr = new (std::nothrow) T[size];
  if (!raw_ptr) {
    return nullptr;
  }
  return std::unique_ptr<T[]>(raw_ptr);
}

} // namespace utils
} // namespace hf