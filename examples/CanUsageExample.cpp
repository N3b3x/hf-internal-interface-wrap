/**
 * @file CanUsageExample.cpp
 * @brief Comprehensive examples demonstrating the modernized CAN interface usage.
 *
 * This file demonstrates the proper usage of the new CAN architecture with both
 * raw hardware access (McuCan) and thread-safe wrappers (SfCan) for various
 * scenarios including multi-threaded applications and performance-critical code.
 */

#include "BaseCan.h"
#include "McuCan.h"
#include "SfCan.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

/**
 * @brief Example using raw McuCan for single-threaded, performance-critical applications.
 */
void RawCanExample() {
  std::cout << "=== Raw CAN Example (McuCan) ===" << std::endl;

  // Create CAN configuration for 500kbps
  CanBusConfig config{};
  config.baud_rate = 500000;
  config.tx_pin = static_cast<hf_gpio_num_t>(21); // Example GPIO
  config.rx_pin = static_cast<hf_gpio_num_t>(22); // Example GPIO
  config.mode = CAN_MODE_NORMAL;
  config.tx_queue_len = 10;
  config.rx_queue_len = 10;

  // Create raw MCU CAN interface (no thread safety)
  auto mcu_can = std::make_unique<McuCan>(config);

  if (mcu_can->Initialize()) {
    std::cout << "Raw CAN initialized successfully" << std::endl;

    if (mcu_can->Start()) {
      std::cout << "Raw CAN started" << std::endl;

      // Send a message (maximum performance, no mutex overhead)
      hf_can_message_t msg{};
      msg.id = 0x123;
      msg.dlc = 8;
      msg.data[0] = 0x01;
      msg.data[1] = 0x02;
      msg.data[2] = 0x03;
      msg.data[3] = 0x04;
      msg.data[4] = 0x05;
      msg.data[5] = 0x06;
      msg.data[6] = 0x07;
      msg.data[7] = 0x08;

      bool sent = mcu_can->SendMessage(msg, 1000);
      std::cout << "Message sent: " << (sent ? "Success" : "Failed") << std::endl;

      // Try to receive a message
      hf_can_message_t rx_msg{};
      bool received = mcu_can->ReceiveMessage(rx_msg, 100);
      if (received) {
        std::cout << "Received message ID: 0x" << std::hex << rx_msg.id << std::endl;
      }

      mcu_can->Stop();
    }

    mcu_can->Deinitialize();
  }
}

/**
 * @brief Example using thread-safe SfCan for multi-threaded applications.
 */
void ThreadSafeCanExample() {
  std::cout << "\n=== Thread-Safe CAN Example (SfCan) ===" << std::endl;

  // Create CAN configuration
  CanBusConfig config{};
  config.baud_rate = 500000;
  config.tx_pin = static_cast<hf_gpio_num_t>(21);
  config.rx_pin = static_cast<hf_gpio_num_t>(22);
  config.mode = CAN_MODE_NORMAL;
  config.tx_queue_len = 10;
  config.rx_queue_len = 10;

  // Create thread-safe CAN interface
  auto sf_can = std::make_unique<SfCan>(std::make_unique<McuCan>(config));

  // Configure mutex timeout for this application
  sf_can->SetMutexTimeout(std::chrono::milliseconds(100));

  if (sf_can->Initialize()) {
    std::cout << "Thread-safe CAN initialized successfully" << std::endl;

    if (sf_can->Start()) {
      std::cout << "Thread-safe CAN started" << std::endl;

      // Test different sending methods
      hf_can_message_t msg{};
      msg.id = 0x456;
      msg.dlc = 4;
      msg.data[0] = 0xAA;
      msg.data[1] = 0xBB;
      msg.data[2] = 0xCC;
      msg.data[3] = 0xDD;

      // Non-blocking send (timeout = 0)
      bool sent_nb = sf_can->SendMessageNonBlocking(msg);
      std::cout << "Non-blocking send: " << (sent_nb ? "Success" : "Failed") << std::endl;

      // Blocking send (timeout = infinite)
      msg.id = 0x457;
      bool sent_b = sf_can->SendMessageBlocking(msg);
      std::cout << "Blocking send: " << (sent_b ? "Success" : "Failed") << std::endl;

      // Batch send multiple messages
      std::vector<hf_can_message_t> messages;
      for (int i = 0; i < 5; ++i) {
        hf_can_message_t batch_msg{};
        batch_msg.id = 0x500 + i;
        batch_msg.dlc = 1;
        batch_msg.data[0] = static_cast<uint8_t>(i);
        messages.push_back(batch_msg);
      }

      bool batch_sent = sf_can->SendMultipleMessages(messages);
      std::cout << "Batch send (5 messages): " << (batch_sent ? "Success" : "Failed") << std::endl;

      // Lock-free status checks
      bool initialized = sf_can->IsInitialized();
      bool tx_full = sf_can->IsTransmitQueueFull();
      bool rx_empty = sf_can->IsReceiveQueueEmpty();

      std::cout << "Status - Initialized: " << initialized << ", TX Full: " << tx_full
                << ", RX Empty: " << rx_empty << std::endl;

      // Get threading statistics
      auto stats = sf_can->GetThreadingStats();
      std::cout << "Threading Stats:" << std::endl;
      std::cout << "  Total operations: " << stats.total_operations << std::endl;
      std::cout << "  Lock contentions: " << stats.lock_contentions << std::endl;
      std::cout << "  Average lock time: " << stats.average_lock_time_us << " μs" << std::endl;
      std::cout << "  Max lock time: " << stats.max_lock_time_us << " μs" << std::endl;

      sf_can->Stop();
    }

    sf_can->Deinitialize();
  }
}

/**
 * @brief Example demonstrating multi-threaded CAN communication.
 */
void MultiThreadedCanExample() {
  std::cout << "\n=== Multi-Threaded CAN Example ===" << std::endl;

  // Create shared CAN interface
  CanBusConfig config{};
  config.baud_rate = 500000;
  config.tx_pin = static_cast<hf_gpio_num_t>(21);
  config.rx_pin = static_cast<hf_gpio_num_t>(22);
  config.mode = CAN_MODE_NORMAL;
  config.tx_queue_len = 20;
  config.rx_queue_len = 20;

  auto sf_can = std::make_shared<SfCan>(std::make_unique<McuCan>(config));

  if (!sf_can->Initialize() || !sf_can->Start()) {
    std::cout << "Failed to initialize CAN for multi-threaded example" << std::endl;
    return;
  }

  std::cout << "Multi-threaded CAN initialized" << std::endl;

  // Sender thread
  std::thread sender_thread([sf_can]() {
    for (int i = 0; i < 10; ++i) {
      hf_can_message_t msg{};
      msg.id = 0x600 + i;
      msg.dlc = 8;
      for (int j = 0; j < 8; ++j) {
        msg.data[j] = static_cast<uint8_t>(i * 8 + j);
      }

      if (sf_can->SendMessage(msg, 1000)) {
        std::cout << "Sender: Sent message " << i << std::endl;
      } else {
        std::cout << "Sender: Failed to send message " << i << std::endl;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });

  // Receiver thread
  std::thread receiver_thread([sf_can]() {
    for (int i = 0; i < 5; ++i) {
      hf_can_message_t msg{};
      if (sf_can->ReceiveMessage(msg, 2000)) {
        std::cout << "Receiver: Got message ID 0x" << std::hex << msg.id << ", DLC " << std::dec
                  << static_cast<int>(msg.dlc) << std::endl;
      } else {
        std::cout << "Receiver: Timeout waiting for message" << std::endl;
      }
    }
  });

  // Monitor thread
  std::thread monitor_thread([sf_can]() {
    for (int i = 0; i < 20; ++i) {
      // Use lock-free operations for monitoring
      bool tx_full = sf_can->IsTransmitQueueFull();
      bool rx_empty = sf_can->IsReceiveQueueEmpty();

      if (tx_full) {
        std::cout << "Monitor: TX queue full!" << std::endl;
      }
      if (!rx_empty) {
        std::cout << "Monitor: RX queue has data" << std::endl;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  });

  // Wait for all threads to complete
  sender_thread.join();
  receiver_thread.join();
  monitor_thread.join();

  // Final statistics
  auto final_stats = sf_can->GetThreadingStats();
  std::cout << "Final Threading Stats:" << std::endl;
  std::cout << "  Total operations: " << final_stats.total_operations << std::endl;
  std::cout << "  Lock contentions: " << final_stats.lock_contentions << std::endl;
  std::cout << "  Contention rate: "
            << (100.0 * final_stats.lock_contentions / final_stats.total_operations) << "%"
            << std::endl;

  sf_can->Stop();
  sf_can->Deinitialize();
}

/**
 * @brief Example demonstrating advanced SfCan features.
 */
void AdvancedCanExample() {
  std::cout << "\n=== Advanced CAN Features Example ===" << std::endl;

  CanBusConfig config{};
  config.baud_rate = 1000000; // 1Mbps for high-speed example
  config.tx_pin = static_cast<hf_gpio_num_t>(21);
  config.rx_pin = static_cast<hf_gpio_num_t>(22);
  config.mode = CAN_MODE_NORMAL;
  config.tx_queue_len = 50;
  config.rx_queue_len = 50;

  auto sf_can = std::make_unique<SfCan>(std::make_unique<McuCan>(config));

  // Configure for high-performance operation
  sf_can->SetMutexTimeout(std::chrono::milliseconds(10));

  if (sf_can->Initialize() && sf_can->Start()) {
    std::cout << "Advanced CAN example started" << std::endl;

    // Test manual locking for complex operations
    if (sf_can->TryLock()) {
      std::cout << "Acquired exclusive lock for complex operation" << std::endl;

      // Perform multiple operations without lock contention
      for (int i = 0; i < 5; ++i) {
        hf_can_message_t msg{};
        msg.id = 0x700 + i;
        msg.dlc = 8;
        // Fill with pattern
        for (int j = 0; j < 8; ++j) {
          msg.data[j] = static_cast<uint8_t>(i ^ j);
        }

        // Direct access to underlying implementation (already locked)
        auto *impl = sf_can->GetImplementation();
        if (impl) {
          impl->SendMessage(msg, 100);
        }
      }

      sf_can->Unlock();
      std::cout << "Released exclusive lock" << std::endl;
    }

    // Test partial batch sending
    std::vector<hf_can_message_t> large_batch;
    for (int i = 0; i < 100; ++i) {
      hf_can_message_t msg{};
      msg.id = 0x800 + i;
      msg.dlc = 2;
      msg.data[0] = static_cast<uint8_t>(i);
      msg.data[1] = static_cast<uint8_t>(i >> 8);
      large_batch.push_back(msg);
    }

    size_t sent_count = sf_can->SendMultipleMessagesPartial(large_batch, 10);
    std::cout << "Partial batch send: " << sent_count << "/" << large_batch.size()
              << " messages sent" << std::endl;

    // Test callback functionality
    bool callback_received = false;
    auto callback = [&callback_received](const hf_can_message_t &msg) {
      std::cout << "Callback: Received message ID 0x" << std::hex << msg.id << std::endl;
      callback_received = true;
    };

    if (sf_can->SetReceiveCallback(callback)) {
      std::cout << "Receive callback set successfully" << std::endl;

      // Send a test message that might trigger the callback
      hf_can_message_t test_msg{};
      test_msg.id = 0x999;
      test_msg.dlc = 1;
      test_msg.data[0] = 0xFF;
      sf_can->SendMessage(test_msg);

      // Wait briefly for potential callback
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      sf_can->ClearReceiveCallback();
      std::cout << "Callback cleared" << std::endl;
    }

    sf_can->Stop();
    sf_can->Deinitialize();
  }
}

/**
 * @brief Main function demonstrating all CAN examples.
 */
extern "C" void RunCanExamples() {
  // Run all CAN usage examples sequentially
  RawCanExample();
  ThreadSafeCanExample();
  MultiThreadedCanExample();
  AdvancedCanExample();
}

#ifdef RUN_CAN_EXAMPLE_MAIN
int main() {
  std::cout << "HardFOC CAN Interface Examples" << std::endl;
  std::cout << "==============================" << std::endl;

  try {
    RunCanExamples();
    std::cout << "\n=== All CAN Examples Completed ===" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Exception in CAN examples: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
#endif
