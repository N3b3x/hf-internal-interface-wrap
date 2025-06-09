/**
 * @file FlexCan.cpp
 * @brief Implementation of the FlexCan class.
 */

#include "FlexCan.h"
#include <PinCfg/gpio_config_esp32c6.hpp>
#include <cstdio>

FlexCan::FlexCan(uint8_t portArg, uint32_t baudRateArg) noexcept
    : port(portArg), baudRate(baudRateArg), initialized(false),
      txPin((gpio_num_t)TWAI_TX_PIN), rxPin((gpio_num_t)TWAI_RX_PIN) {}

FlexCan::~FlexCan() noexcept {
  if (initialized) {
    Close();
  }
}

bool FlexCan::Open() noexcept {
  if (initialized)
    return true;

  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(txPin, rxPin, TWAI_MODE_NORMAL);
  g_config.tx_queue_len = 5;
  g_config.rx_queue_len = 5;

  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  twai_timing_config_t t_config;
  switch (baudRate) {
  case 1000000:
    t_config = TWAI_TIMING_CONFIG_1MBITS();
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
  default:
    // Fallback to 500 Kbit/s
    t_config = TWAI_TIMING_CONFIG_500KBITS();
    break;
  }

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    return false;
  }
  if (twai_start() != ESP_OK) {
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

  twai_message_t msg = {};
  msg.identifier = frame.id;
  msg.extd = frame.extended ? 1 : 0;
  msg.rtr = frame.rtr ? 1 : 0;
  msg.data_length_code = frame.dlc;
  for (uint8_t i = 0; i < frame.dlc && i < 8; ++i) {
    msg.data[i] = frame.data[i];
  }

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

  frame.id = msg.identifier;
  frame.extended = msg.extd;
  frame.rtr = msg.rtr;
  frame.dlc = msg.data_length_code;
  for (uint8_t i = 0; i < frame.dlc && i < 8; ++i) {
    frame.data[i] = msg.data[i];
  }

  return true;
}
