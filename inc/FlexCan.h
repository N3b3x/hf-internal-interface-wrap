/**
 * @file FlexCan.h
 * @brief Simple FlexCAN controller driver abstraction.
 *
 * This class provides a lightweight wrapper around a FlexCAN peripheral.
 * The interface is modeled after other internal interface drivers in this
 * repository. It does not implement thread safety.
 */
#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_FLEXCAN_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_FLEXCAN_H_

#include "driver/gpio.h"
#include "driver/twai.h"
#include <cstdint>

/**
 * @class FlexCan
 * @brief Minimal FlexCAN controller wrapper.
 */
class FlexCan {
public:
  /**
   * @brief CAN frame structure.
   */
  struct Frame {
    uint32_t id;     ///< Identifier of the frame
    uint8_t data[8]; ///< Data bytes
    uint8_t dlc;     ///< Data length code
    bool extended;   ///< True if extended ID
    bool rtr;        ///< Remote transmission request
  };
  /**
   * @brief Constructor.
   * @param portArg CAN controller port number (currently unused on ESP32)
   * @param baudRateArg Bus baud rate in bit/s
   * @param txPinArg GPIO pin for TWAI TX (default: GPIO_NUM_21)
   * @param rxPinArg GPIO pin for TWAI RX (default: GPIO_NUM_22)
   */
  FlexCan(uint8_t portArg, uint32_t baudRateArg, gpio_num_t txPinArg = GPIO_NUM_21,
          gpio_num_t rxPinArg = GPIO_NUM_22) noexcept;

  /**
   * @brief Destructor.
   */
  ~FlexCan() noexcept;

  /**
   * @brief Open and initialize the CAN controller.
   * @return true if successful, false otherwise
   */
  bool Open() noexcept;

  /**
   * @brief Close the CAN controller.
   * @return true if closed, false otherwise
   */
  bool Close() noexcept;

  /**
   * @brief Write a CAN frame.
   * @param frame Frame to transmit
   * @return true if transmitted
   */
  bool Write(const Frame &frame) noexcept;
  /**
   * @brief Read a CAN frame.
   * @param frame Destination frame
   * @param timeoutMs Timeout in milliseconds
   * @return true if a frame was received
   */
  bool Read(Frame &frame, uint32_t timeoutMs = 0) noexcept;

  /**
   * @brief Check if there are pending messages in the receive queue.
   * @return Number of pending messages, or 0 if none
   */
  uint32_t Available() noexcept;

  /**
   * @brief Get the current bus state.
   * @return TWAI state
   */
  twai_state_t GetState() noexcept;
  /**
   * @brief Get the configured baud rate.
   */
  uint32_t GetBaudRate() const noexcept {
    return baudRate;
  }

  /**
   * @brief Get the configured TX pin.
   */
  gpio_num_t GetTxPin() const noexcept {
    return txPin;
  }

  /**
   * @brief Get the configured RX pin.
   */
  gpio_num_t GetRxPin() const noexcept {
    return rxPin;
  }

  /**
   * @brief Check if the controller is initialized.
   */
  bool IsInitialized() const noexcept {
    return initialized;
  }

private:
  uint8_t port;      ///< Controller port
  uint32_t baudRate; ///< Bus baud rate
  bool initialized;  ///< Initialization flag
  gpio_num_t txPin;  ///< TX pin used by TWAI driver
  gpio_num_t rxPin;  ///< RX pin used by TWAI driver
};

#endif // HAL_INTERNAL_INTERFACE_DRIVERS_FLEXCAN_H_
