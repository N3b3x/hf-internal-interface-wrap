/**
 * @file PwmUsageExample.cpp
 * @brief Example demonstrating the modernized PWM system usage.
 *
 * This example shows how to use the new PWM architecture with both
 * MCU-based and external IC-based PWM controllers, including thread-safe
 * wrappers for multi-threaded applications.
 */

#include "BasePwm.h"
#include "McuI2c.h"
#include "McuPwm.h"
#include "Pca9685Pwm.h"
#include "SfPwm.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

/**
 * @brief Example using ESP32C6 MCU PWM for motor control.
 */
void McuPwmExample() {
  std::cout << "=== MCU PWM Example ===" << std::endl;

  // Create MCU PWM controller
  auto mcu_pwm = std::make_unique<McuPwm>();

  if (mcu_pwm->Initialize() == HfPwmErr::PWM_SUCCESS) {
    std::cout << "MCU PWM initialized successfully" << std::endl;

    // Configure PWM for motor control
    PwmChannelConfig motor_config{};
    motor_config.output_pin = GPIO_NUM_2;
    motor_config.frequency_hz = 20000; // 20kHz for motor
    motor_config.resolution_bits = 12;
    motor_config.initial_duty_cycle = 0.0f; // Start stopped
    motor_config.timer_id = 0;
    motor_config.channel_id = 0;

    if (mcu_pwm->ConfigureChannel(0, motor_config) == HfPwmErr::PWM_SUCCESS) {
      std::cout << "Motor PWM channel configured" << std::endl;

      // Start PWM
      mcu_pwm->Start(0);

      // Gradually increase speed
      for (float duty = 0.0f; duty <= 0.8f; duty += 0.1f) {
        mcu_pwm->SetDutyCycle(0, duty);
        std::cout << "Motor duty cycle: " << (duty * 100) << "%" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }

      // Stop motor
      mcu_pwm->SetDutyCycle(0, 0.0f);
      mcu_pwm->Stop(0);
      std::cout << "Motor stopped" << std::endl;
    }

    mcu_pwm->Deinitialize();
  } else {
    std::cout << "Failed to initialize MCU PWM" << std::endl;
  }
}

/**
 * @brief Example using PCA9685 external PWM IC for servo control.
 */
void ExternalPwmExample() {
  std::cout << "\n=== External PWM Example ===" << std::endl;

  // Create I2C interface for PCA9685
  auto i2c_interface = std::make_unique<McuI2c>();

  // Create PCA9685 PWM controller
  auto pca9685_pwm = std::make_unique<Pca9685Pwm>(std::move(i2c_interface), 0x40);

  if (pca9685_pwm->Initialize() == HfPwmErr::PWM_SUCCESS) {
    std::cout << "PCA9685 PWM initialized successfully" << std::endl;

    // Configure PWM for servo control
    PwmChannelConfig servo_config{};
    servo_config.frequency_hz = 50; // 50Hz for servo
    servo_config.resolution_bits = 12;
    servo_config.initial_duty_cycle = 0.075f; // 1.5ms pulse (neutral)

    // Configure multiple servo channels
    uint8_t servo_channels[] = {0, 1, 2, 3};
    for (uint8_t channel : servo_channels) {
      if (pca9685_pwm->ConfigureChannel(channel, servo_config) == HfPwmErr::PWM_SUCCESS) {
        std::cout << "Servo channel " << static_cast<int>(channel) << " configured" << std::endl;
      }
    }

    // Start all servo channels
    pca9685_pwm->StartMultiple(servo_channels, 4);

    // Sweep servos
    std::cout << "Sweeping servos..." << std::endl;
    for (int sweep = 0; sweep < 3; sweep++) {
      // Move to minimum position (1ms pulse)
      float duty_cycles[] = {0.05f, 0.05f, 0.05f, 0.05f};
      pca9685_pwm->SetDutyCycleMultiple(servo_channels, duty_cycles, 4);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));

      // Move to maximum position (2ms pulse)
      float duty_cycles_max[] = {0.1f, 0.1f, 0.1f, 0.1f};
      pca9685_pwm->SetDutyCycleMultiple(servo_channels, duty_cycles_max, 4);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // Return to neutral
    float neutral_duty_cycles[] = {0.075f, 0.075f, 0.075f, 0.075f};
    pca9685_pwm->SetDutyCycleMultiple(servo_channels, neutral_duty_cycles, 4);
    std::cout << "Servos returned to neutral" << std::endl;

    // Stop all servos
    pca9685_pwm->StopMultiple(servo_channels, 4);
    pca9685_pwm->Deinitialize();
  } else {
    std::cout << "Failed to initialize PCA9685 PWM" << std::endl;
  }
}

/**
 * @brief Example using thread-safe PWM wrapper.
 */
void ThreadSafePwmExample() {
  std::cout << "\n=== Thread-Safe PWM Example ===" << std::endl;

  // Create thread-safe PWM wrapper around MCU PWM
  SfPwm sf_pwm(std::make_unique<McuPwm>());

  if (sf_pwm.Initialize() == HfPwmErr::PWM_SUCCESS) {
    std::cout << "Thread-safe PWM initialized successfully" << std::endl;

    // Configure LED PWM channels
    PwmChannelConfig led_config{};
    led_config.frequency_hz = 1000; // 1kHz for LED
    led_config.resolution_bits = 10;
    led_config.initial_duty_cycle = 0.0f;

    // Configure RGB LED channels
    uint8_t rgb_channels[] = {0, 1, 2}; // R, G, B
    for (uint8_t i = 0; i < 3; i++) {
      led_config.output_pin = static_cast<gpio_num_t>(GPIO_NUM_2 + i);
      led_config.channel_id = i;
      sf_pwm.ConfigureChannel(rgb_channels[i], led_config);
    }

    // Start RGB channels
    sf_pwm.StartMultiple(rgb_channels, 3);

    // Create threads to control different color channels
    bool running = true;

    std::thread red_thread([&sf_pwm, &running]() {
      float duty = 0.0f;
      float step = 0.01f;
      while (running) {
        sf_pwm.SetDutyCycle(0, duty);
        duty += step;
        if (duty >= 1.0f || duty <= 0.0f) {
          step = -step;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
      }
    });

    std::thread green_thread([&sf_pwm, &running]() {
      float duty = 0.5f;
      float step = 0.02f;
      while (running) {
        sf_pwm.SetDutyCycle(1, duty);
        duty += step;
        if (duty >= 1.0f || duty <= 0.0f) {
          step = -step;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
      }
    });

    std::thread blue_thread([&sf_pwm, &running]() {
      float duty = 1.0f;
      float step = -0.015f;
      while (running) {
        sf_pwm.SetDutyCycle(2, duty);
        duty += step;
        if (duty >= 1.0f || duty <= 0.0f) {
          step = -step;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
      }
    });

    // Run for 5 seconds
    std::cout << "Running RGB LED animation for 5 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Stop threads
    running = false;
    red_thread.join();
    green_thread.join();
    blue_thread.join();

    // Turn off LEDs
    sf_pwm.StopMultiple(rgb_channels, 3);
    sf_pwm.Deinitialize();
    std::cout << "RGB LED animation stopped" << std::endl;
  } else {
    std::cout << "Failed to initialize thread-safe PWM" << std::endl;
  }
}

/**
 * @brief Example using advanced ESP32C6 PWM features.
 */
void AdvancedPwmExample() {
  std::cout << "\n=== Advanced PWM Features Example ===" << std::endl;

  auto advanced_pwm = std::make_unique<McuPwm>();

  if (advanced_pwm->Initialize() == HfPwmErr::PWM_SUCCESS) {
    std::cout << "Advanced PWM initialized successfully" << std::endl;

    // Configure complementary PWM for H-bridge motor control
    PwmChannelConfig primary_config{};
    primary_config.output_pin = GPIO_NUM_2;
    primary_config.frequency_hz = 25000; // 25kHz
    primary_config.resolution_bits = 10;
    primary_config.initial_duty_cycle = 0.3f;
    primary_config.timer_id = 0;
    primary_config.channel_id = 0;

    PwmChannelConfig secondary_config{};
    secondary_config.output_pin = GPIO_NUM_3;
    secondary_config.frequency_hz = 25000;
    secondary_config.resolution_bits = 10;
    secondary_config.initial_duty_cycle = 0.3f;
    secondary_config.timer_id = 0;
    secondary_config.channel_id = 1;

    // Configure channels
    advanced_pwm->ConfigureChannel(0, primary_config);
    advanced_pwm->ConfigureChannel(1, secondary_config);

    // Configure complementary operation with dead time
    PwmComplementaryConfig comp_config{};
    comp_config.dead_time_ns = 1000; // 1μs dead time
    comp_config.enable_complementary = true;

    if (advanced_pwm->ConfigureComplementary(0, 1, comp_config) == HfPwmErr::PWM_SUCCESS) {
      std::cout << "Complementary PWM configured with dead time" << std::endl;

      // Set dead time
      advanced_pwm->SetDeadTime(0, 1000);
      advanced_pwm->SetDeadTime(1, 1000);

      // Start complementary PWM
      uint8_t comp_channels[] = {0, 1};
      advanced_pwm->StartMultiple(comp_channels, 2);

      // Test different duty cycles
      for (float duty = 0.1f; duty <= 0.9f; duty += 0.2f) {
        advanced_pwm->SetDutyCycle(0, duty);
        std::cout << "Primary duty: " << (duty * 100) << "%, Secondary: " << ((1.0f - duty) * 100)
                  << "%" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }

      // Stop complementary PWM
      advanced_pwm->StopMultiple(comp_channels, 2);
    }

    // Test fade functionality
    std::cout << "Testing PWM fade functionality..." << std::endl;

    PwmChannelConfig fade_config{};
    fade_config.output_pin = GPIO_NUM_4;
    fade_config.frequency_hz = 5000;
    fade_config.resolution_bits = 10;
    fade_config.initial_duty_cycle = 0.0f;
    fade_config.timer_id = 1;
    fade_config.channel_id = 2;

    advanced_pwm->ConfigureChannel(2, fade_config);

    // Configure fade
    PwmFadeConfig fade_settings{};
    fade_settings.target_duty_cycle = 1.0f;
    fade_settings.fade_time_ms = 2000;
    fade_settings.fade_mode = PwmFadeMode::LINEAR;

    if (advanced_pwm->ConfigureFade(2, fade_settings) == HfPwmErr::PWM_SUCCESS) {
      advanced_pwm->Start(2);
      advanced_pwm->StartFade(2);
      std::cout << "Fade started (0% to 100% over 2 seconds)" << std::endl;

      std::this_thread::sleep_for(std::chrono::milliseconds(2500));

      // Fade back down
      fade_settings.target_duty_cycle = 0.0f;
      advanced_pwm->ConfigureFade(2, fade_settings);
      advanced_pwm->StartFade(2);
      std::cout << "Fade down (100% to 0% over 2 seconds)" << std::endl;

      std::this_thread::sleep_for(std::chrono::milliseconds(2500));
      advanced_pwm->Stop(2);
    }

    advanced_pwm->Deinitialize();
    std::cout << "Advanced PWM features demonstration completed" << std::endl;
  } else {
    std::cout << "Failed to initialize advanced PWM" << std::endl;
  }
}

/**
 * @brief Main function demonstrating all PWM examples.
 */
int main() {
  std::cout << "HardFOC PWM System Examples" << std::endl;
  std::cout << "===========================" << std::endl;

  try {
    // Run all examples
    McuPwmExample();
    ExternalPwmExample();
    ThreadSafePwmExample();
    AdvancedPwmExample();

    std::cout << "\nAll PWM examples completed successfully!" << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Error running PWM examples: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
