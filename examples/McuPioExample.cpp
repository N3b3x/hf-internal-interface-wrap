/**
 * @file McuPioExample.cpp
 * @brief Example usage of the ESP32 RMT-based McuPio implementation
 *
 * This example demonstrates how to use the McuPio class for various
 * programmable I/O operations including:
 * - Basic symbol transmission
 * - WS2812 LED control
 * - IR remote control
 * - Custom protocol implementation
 * - Reception and callback handling
 */

#include "../mcu/McuPio.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

// Example: WS2812 LED control (NeoPixel)
class WS2812Controller {
private:
  McuPio pio_;
  uint8_t channel_id_;

  // WS2812 timing constants (in nanoseconds)
  static constexpr uint32_t T0H = 350;     // 0 bit high time
  static constexpr uint32_t T0L = 900;     // 0 bit low time
  static constexpr uint32_t T1H = 900;     // 1 bit high time
  static constexpr uint32_t T1L = 350;     // 1 bit low time
  static constexpr uint32_t RESET = 50000; // Reset pulse

public:
  WS2812Controller(uint8_t channel, hf_gpio_num_t pin) : channel_id_(channel) {
    // Initialize PIO
    if (pio_.Initialize() != HfPioErr::PIO_SUCCESS) {
      throw std::runtime_error("Failed to initialize PIO");
    }

    // Configure channel for WS2812
    PioChannelConfig config;
    config.gpio_pin = pin;
    config.direction = PioDirection::Transmit;
    config.resolution_ns = 125; // 125ns resolution for precise timing
    config.polarity = PioPolarity::Normal;
    config.idle_state = PioIdleState::Low;
    config.timeout_us = 1000;

    if (pio_.ConfigureChannel(channel_id_, config) != HfPioErr::PIO_SUCCESS) {
      throw std::runtime_error("Failed to configure PIO channel");
    }
  }

  ~WS2812Controller() {
    pio_.Deinitialize();
  }

  void SetPixelColor(uint8_t r, uint8_t g, uint8_t b) {
    std::vector<PioSymbol> symbols;

    // WS2812 uses GRB format
    uint32_t color = (g << 16) | (r << 8) | b;

    // Convert each bit to WS2812 timing
    for (int i = 23; i >= 0; i--) {
      bool bit = (color >> i) & 1;
      if (bit) {
        // Send '1' bit
        symbols.emplace_back(T1H / 125, true);  // High pulse
        symbols.emplace_back(T1L / 125, false); // Low pulse
      } else {
        // Send '0' bit
        symbols.emplace_back(T0H / 125, true);  // High pulse
        symbols.emplace_back(T0L / 125, false); // Low pulse
      }
    }

    // Add reset pulse
    symbols.emplace_back(RESET / 125, false);

    // Transmit symbols
    HfPioErr result = pio_.Transmit(channel_id_, symbols.data(), symbols.size(), true);
    if (result != HfPioErr::PIO_SUCCESS) {
      std::cerr << "Failed to transmit WS2812 data: " << static_cast<int>(result) << std::endl;
    }
  }
};

// Example: IR Remote Control
class IRController {
private:
  McuPio pio_;
  uint8_t channel_id_;

  // NEC protocol timing constants (in microseconds)
  static constexpr uint32_t NEC_HEADER_MARK = 9000;
  static constexpr uint32_t NEC_HEADER_SPACE = 4500;
  static constexpr uint32_t NEC_BIT_MARK = 560;
  static constexpr uint32_t NEC_ONE_SPACE = 1690;
  static constexpr uint32_t NEC_ZERO_SPACE = 560;

public:
  IRController(uint8_t channel, hf_gpio_num_t pin) : channel_id_(channel) {
    // Initialize PIO
    if (pio_.Initialize() != HfPioErr::PIO_SUCCESS) {
      throw std::runtime_error("Failed to initialize PIO");
    }

    // Configure channel for IR transmission
    PioChannelConfig config;
    config.gpio_pin = pin;
    config.direction = PioDirection::Transmit;
    config.resolution_ns = 1000; // 1μs resolution
    config.polarity = PioPolarity::Normal;
    config.idle_state = PioIdleState::Low;
    config.timeout_us = 100000; // 100ms timeout

    if (pio_.ConfigureChannel(channel_id_, config) != HfPioErr::PIO_SUCCESS) {
      throw std::runtime_error("Failed to configure PIO channel");
    }

    // Configure 38kHz carrier for IR
    if (pio_.ConfigureCarrier(channel_id_, 38000, 0.33f) != HfPioErr::PIO_SUCCESS) {
      throw std::runtime_error("Failed to configure IR carrier");
    }
  }

  ~IRController() {
    pio_.Deinitialize();
  }

  void SendNECCommand(uint8_t address, uint8_t command) {
    std::vector<PioSymbol> symbols;

    // NEC header
    symbols.emplace_back(NEC_HEADER_MARK, true);
    symbols.emplace_back(NEC_HEADER_SPACE, false);

    // Address (8 bits)
    SendByte(symbols, address);

    // Inverted address (8 bits)
    SendByte(symbols, ~address);

    // Command (8 bits)
    SendByte(symbols, command);

    // Inverted command (8 bits)
    SendByte(symbols, ~command);

    // Final bit mark
    symbols.emplace_back(NEC_BIT_MARK, true);

    // Transmit IR command
    HfPioErr result = pio_.Transmit(channel_id_, symbols.data(), symbols.size(), true);
    if (result != HfPioErr::PIO_SUCCESS) {
      std::cerr << "Failed to transmit IR command: " << static_cast<int>(result) << std::endl;
    }
  }

private:
  void SendByte(std::vector<PioSymbol> &symbols, uint8_t byte) {
    for (int i = 0; i < 8; i++) {
      symbols.emplace_back(NEC_BIT_MARK, true);

      if (byte & (1 << i)) {
        symbols.emplace_back(NEC_ONE_SPACE, false);
      } else {
        symbols.emplace_back(NEC_ZERO_SPACE, false);
      }
    }
  }
};

// Example: Custom Protocol with Reception
class CustomProtocolExample {
private:
  McuPio pio_;
  uint8_t tx_channel_;
  uint8_t rx_channel_;
  std::vector<PioSymbol> received_data_;
  bool reception_complete_;

  // Custom protocol timing
  static constexpr uint32_t SYNC_PULSE = 1000;
  static constexpr uint32_t DATA_BIT_HIGH = 500;
  static constexpr uint32_t DATA_BIT_LOW = 250;

public:
  CustomProtocolExample(uint8_t tx_ch, hf_gpio_num_t tx_pin, uint8_t rx_ch, hf_gpio_num_t rx_pin)
      : tx_channel_(tx_ch), rx_channel_(rx_ch), reception_complete_(false) {

    // Initialize PIO
    if (pio_.Initialize() != HfPioErr::PIO_SUCCESS) {
      throw std::runtime_error("Failed to initialize PIO");
    }

    // Configure TX channel
    PioChannelConfig tx_config;
    tx_config.gpio_pin = tx_pin;
    tx_config.direction = PioDirection::Transmit;
    tx_config.resolution_ns = 100; // 100ns resolution
    tx_config.polarity = PioPolarity::Normal;
    tx_config.idle_state = PioIdleState::Low;

    if (pio_.ConfigureChannel(tx_channel_, tx_config) != HfPioErr::PIO_SUCCESS) {
      throw std::runtime_error("Failed to configure TX channel");
    }

    // Configure RX channel
    PioChannelConfig rx_config;
    rx_config.gpio_pin = rx_pin;
    rx_config.direction = PioDirection::Receive;
    rx_config.resolution_ns = 100; // 100ns resolution
    rx_config.polarity = PioPolarity::Normal;
    rx_config.idle_state = PioIdleState::Low;
    rx_config.buffer_size = 128;

    if (pio_.ConfigureChannel(rx_channel_, rx_config) != HfPioErr::PIO_SUCCESS) {
      throw std::runtime_error("Failed to configure RX channel");
    }

    // Set up callbacks
    pio_.SetReceiveCallback([this](uint8_t channel, const PioSymbol *symbols, size_t count,
                                   void *user_data) { OnDataReceived(channel, symbols, count); });

    pio_.SetErrorCallback(
        [this](uint8_t channel, HfPioErr error, void *user_data) { OnError(channel, error); });
  }

  ~CustomProtocolExample() {
    pio_.Deinitialize();
  }

  void SendData(const std::vector<uint8_t> &data) {
    std::vector<PioSymbol> symbols;

    // Sync pulse
    symbols.emplace_back(SYNC_PULSE / 100, true);
    symbols.emplace_back(SYNC_PULSE / 100, false);

    // Data bits
    for (uint8_t byte : data) {
      for (int i = 7; i >= 0; i--) {
        bool bit = (byte >> i) & 1;
        if (bit) {
          symbols.emplace_back(DATA_BIT_HIGH / 100, true);
          symbols.emplace_back(DATA_BIT_LOW / 100, false);
        } else {
          symbols.emplace_back(DATA_BIT_LOW / 100, true);
          symbols.emplace_back(DATA_BIT_HIGH / 100, false);
        }
      }
    }

    // Transmit
    HfPioErr result = pio_.Transmit(tx_channel_, symbols.data(), symbols.size(), false);
    if (result != HfPioErr::PIO_SUCCESS) {
      std::cerr << "Failed to transmit custom protocol data: " << static_cast<int>(result)
                << std::endl;
    }
  }

  void StartReceiving() {
    received_data_.clear();
    received_data_.resize(128); // Allocate buffer
    reception_complete_ = false;

    HfPioErr result = pio_.StartReceive(rx_channel_, received_data_.data(), received_data_.size(),
                                        10000); // 10ms timeout
    if (result != HfPioErr::PIO_SUCCESS) {
      std::cerr << "Failed to start reception: " << static_cast<int>(result) << std::endl;
    }
  }

  bool IsReceptionComplete() const {
    return reception_complete_;
  }

  const std::vector<PioSymbol> &GetReceivedData() const {
    return received_data_;
  }

private:
  void OnDataReceived(uint8_t channel, const PioSymbol *symbols, size_t count) {
    std::cout << "Received " << count << " symbols on channel " << static_cast<int>(channel)
              << std::endl;

    // Process received symbols
    for (size_t i = 0; i < count; i++) {
      std::cout << "Symbol " << i << ": level=" << symbols[i].level
                << ", duration=" << symbols[i].duration << std::endl;
    }

    reception_complete_ = true;
  }

  void OnError(uint8_t channel, HfPioErr error) {
    std::cerr << "PIO error on channel " << static_cast<int>(channel) << ": "
              << static_cast<int>(error) << std::endl;
  }
};

// Main example function
int main() {
  try {
    std::cout << "=== McuPio Examples ===" << std::endl;

    // Example 1: WS2812 LED control
    std::cout << "\n1. WS2812 LED Control Example" << std::endl;
    WS2812Controller led(0, 18); // Channel 0, GPIO 18

    // Set LED to red
    led.SetPixelColor(255, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Set LED to green
    led.SetPixelColor(0, 255, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Set LED to blue
    led.SetPixelColor(0, 0, 255);
    std::cout << "WS2812 LED colors sent successfully!" << std::endl;

    // Example 2: IR Remote Control
    std::cout << "\n2. IR Remote Control Example" << std::endl;
    IRController ir(1, 19); // Channel 1, GPIO 19

    // Send NEC command (e.g., TV power button)
    ir.SendNECCommand(0x02, 0x20); // Address 0x02, Command 0x20
    std::cout << "IR command sent successfully!" << std::endl;

    // Example 3: Custom Protocol with Reception
    std::cout << "\n3. Custom Protocol Example" << std::endl;
    CustomProtocolExample custom(2, 20, 3, 21); // TX: Ch2/GPIO20, RX: Ch3/GPIO21

    // Start receiving
    custom.StartReceiving();

    // Send some data
    std::vector<uint8_t> test_data = {0xAA, 0x55, 0x12, 0x34};
    custom.SendData(test_data);

    // Wait for reception (in real application, this would be event-driven)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (custom.IsReceptionComplete()) {
      std::cout << "Custom protocol data received successfully!" << std::endl;
    } else {
      std::cout << "Custom protocol reception timeout" << std::endl;
    }

    std::cout << "\n=== All examples completed ===" << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
