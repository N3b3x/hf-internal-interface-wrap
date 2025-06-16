/**
 * @file FlexCan.cpp
 * @brief Implementation of the FlexCan class.
 */

#include "FlexCan.h"
#include <cstdio>

FlexCan::FlexCan(uint8_t portArg, uint32_t baudRateArg, gpio_num_t txPinArg,
                 gpio_num_t rxPinArg) noexcept
    : port(portArg), baudRate(baudRateArg), initialized(false), txPin(txPinArg), rxPin(rxPinArg) {}

FlexCan::~FlexCan() noexcept {
  if (initialized) {
    Close();
  }
}

bool FlexCan::Open() noexcept {
  if (initialized)
    return true;

  // Configure TWAI general settings
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(txPin, rxPin, TWAI_MODE_NORMAL);
  g_config.tx_queue_len = 10; // Increased queue length for better performance
  g_config.rx_queue_len = 10;

  // Accept all messages - can be customized later if needed
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Configure timing based on baud rate
  twai_timing_config_t t_config;
  switch (baudRate) {
  case 1000000:
    t_config = TWAI_TIMING_CONFIG_1MBITS();
    break;
  case 800000:
    t_config = TWAI_TIMING_CONFIG_800KBITS();
    break;
  case 500000:
    t_config = TWAI_TIMING_CONFIG_500KBITS();
    break;
  case 250000:
    t_config = TWAI_TIMING_CONFIG_250KBITS();
    break;
  case 125000:
    t_config = TWAI_TIMING_CONFIG_125KBITS();
    break;
  case 100000:
    t_config = TWAI_TIMING_CONFIG_100KBITS();
    break;
  case 50000:
    t_config = TWAI_TIMING_CONFIG_50KBITS();
    break;
  case 25000:
    t_config = TWAI_TIMING_CONFIG_25KBITS();
    break;
  default:
    // Fallback to 500 Kbit/s for unsupported rates
    t_config = TWAI_TIMING_CONFIG_500KBITS();
    break;
  }

  // Install the TWAI driver
  esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
  if (ret != ESP_OK) {
    return false;
  }

  // Start the TWAI driver
  ret = twai_start();
  if (ret != ESP_OK) {
    twai_driver_uninstall();
    return false;
  }

  initialized = true;
  return true;
}

bool FlexCan::Close() noexcept {
  if (!initialized)
    return true;
  twai_stop();
  twai_driver_uninstall();
  initialized = false;
  return true;
}

bool FlexCan::Write(const Frame &frame) noexcept {
  if (!initialized)
    return false;

  // Validate frame data
  if (frame.dlc > 8) {
    return false;
  }

  // Prepare TWAI message
  twai_message_t msg = {};
  msg.identifier = frame.id;
  msg.extd = frame.extended ? 1 : 0;
  msg.rtr = frame.rtr ? 1 : 0;
  msg.data_length_code = frame.dlc;

  // Copy data bytes
  for (uint8_t i = 0; i < frame.dlc && i < 8; ++i) {
    msg.data[i] = frame.data[i];
  }

  // Transmit with timeout
  esp_err_t ret = twai_transmit(&msg, pdMS_TO_TICKS(100));
  return (ret == ESP_OK);
}

bool FlexCan::Read(Frame &frame, uint32_t timeoutMs) noexcept {
  if (!initialized)
    return false;

  twai_message_t msg;
  esp_err_t ret = twai_receive(&msg, pdMS_TO_TICKS(timeoutMs));
  if (ret != ESP_OK) {
    return false;
  }

  // Extract frame information
  frame.id = msg.identifier;
  frame.extended = (msg.extd != 0);
  frame.rtr = (msg.rtr != 0);
  frame.dlc = msg.data_length_code;

  // Clear data array first
  for (uint8_t i = 0; i < 8; ++i) {
    frame.data[i] = 0;
  }

  // Copy received data
  for (uint8_t i = 0; i < frame.dlc && i < 8; ++i) {
    frame.data[i] = msg.data[i];
  }

  return true;
}

uint32_t FlexCan::Available() noexcept {
  if (!initialized)
    return 0;

  twai_status_info_t status_info;
  esp_err_t ret = twai_get_status_info(&status_info);
  if (ret != ESP_OK) {
    return 0;
  }

  return status_info.msgs_to_rx;
}

twai_state_t FlexCan::GetState() noexcept {
  if (!initialized)
    return TWAI_STATE_STOPPED;

  twai_status_info_t status_info;
  esp_err_t ret = twai_get_status_info(&status_info);
  if (ret != ESP_OK) {
    return TWAI_STATE_STOPPED;
  }

  return status_info.state;
}
