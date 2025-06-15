/**
  *
  *  Contains the declaration and definition of the AveragingFilter class that
  provides
  *      an averaging filter.
  *
  *
  *   Note:  These functions are not thread or interrupt-safe and should be
  called
  *          called with appropriate guards if used within an ISR or shared
  between tasks.
  -------------------------------------------------------------------------------------**/
//   Contains the declaration of the abstract Gpio class, which provides
//   features common to
//     GPIO classes.  Gpio derived classes are intended to employ lazy
//     initialization;
//    they are initialized the first time the pin in manipulated.
// </summary>
// -----------------------------------------------------------------------

#include "BaseGpio.h"

/**
 * @file BaseGpio.cpp
 * @brief Implementation of BaseGpio for ESP32-C6 using ESP-IDF.
 *
 * This file provides the base implementation for GPIO pin abstraction on
 * ESP32-C6.
 *
 * @note This implementation is intended for use with ESP-IDF. Pin configuration
 * is expected to be handled by derived classes or via the
 * PinCfg/gpio_config_esp32c6.hpp utilities.
 */

// For ESP32, pin configuration is typically handled in the derived class or via
// gpio_config(). This function can be overridden by derived classes if needed.
uint32_t BaseGpio::GetPinConfiguration() const noexcept {
  // No default configuration; override in derived class if needed.
  return 0;
}
