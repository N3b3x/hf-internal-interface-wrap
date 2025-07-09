# 🎯 Practical Examples

This document provides practical, real-world examples of using the HardFOC Internal Interface Wrapper in various scenarios. Each example includes complete code, explanations, and best practices.

## 📋 Table of Contents

1. [Basic Hardware Control](#-basic-hardware-control)
2. [Motor Controller Integration](#-motor-controller-integration)
3. [Sensor Reading Systems](#-sensor-reading-systems)
4. [Communication Protocols](#-communication-protocols)
5. [Multi-Threading Examples](#-multi-threading-examples)
6. [Real-Time Applications](#-real-time-applications)
7. [Error Handling Patterns](#-error-handling-patterns)
8. [Performance Optimization](#-performance-optimization)

## 🔌 Basic Hardware Control

### Example 1: Smart LED Controller

```cpp
/**
 * @file SmartLedController.cpp
 * @brief Smart LED controller with multiple control modes
 */

#include "McuDigitalGpio.h"
#include "DigitalOutputGuard.h"
#include "PeriodicTimer.h"

class SmartLedController {
private:
    McuDigitalGpio led_gpio_;
    PeriodicTimer blink_timer_;
    bool is_blinking_;
    
    // Static callback for timer
    static void BlinkCallback(void* user_data) {
        auto* controller = static_cast<SmartLedController*>(user_data);
        controller->ToggleLed();
    }
    
    void ToggleLed() {
        static bool led_state = false;
        led_state = !led_state;
        
        if (led_state) {
            led_gpio_.SetActive();
        } else {
            led_gpio_.SetInactive();
        }
    }
    
public:
    SmartLedController(int pin) 
        : led_gpio_(pin, McuDigitalGpio::Direction::Output)
        , blink_timer_(BlinkCallback, this)
        , is_blinking_(false) {
    }
    
    bool Initialize() {
        if (!led_gpio_.Initialize()) {
            ESP_LOGE("SmartLED", "Failed to initialize LED GPIO");
            return false;
        }
        
        ESP_LOGI("SmartLED", "LED controller initialized on pin %d", 
                 led_gpio_.GetPinNumber());
        return true;
    }
    
    void TurnOn() {
        StopBlinking();
        led_gpio_.SetActive();
    }
    
    void TurnOff() {
        StopBlinking();
        led_gpio_.SetInactive();
    }
    
    void StartBlinking(uint32_t period_ms) {
        is_blinking_ = true;
        blink_timer_.Start(period_ms * 1000); // Convert to microseconds
    }
    
    void StopBlinking() {
        if (is_blinking_) {
            blink_timer_.Stop();
            is_blinking_ = false;
        }
    }
    
    void Flash(uint32_t duration_ms) {
        // Use RAII for temporary activation
        DigitalOutputGuard flash_guard(led_gpio_);
        
        if (flash_guard.IsValid()) {
            vTaskDelay(pdMS_TO_TICKS(duration_ms));
        }
        // LED automatically turns off when guard destructs
    }
    
    bool IsOn() const {
        return led_gpio_.IsActive();
    }
    
    bool IsBlinking() const {
        return is_blinking_;
    }
};

// Usage example
void led_demo_task(void* parameter) {
    SmartLedController status_led(2);
    SmartLedController error_led(13);
    
    if (!status_led.Initialize() || !error_led.Initialize()) {
        ESP_LOGE("Demo", "Failed to initialize LEDs");
        vTaskDelete(NULL);
        return;
    }
    
    while (true) {
        // Normal operation - status LED blinks slowly
        status_led.StartBlinking(1000);
        
        // Simulate some work
        vTaskDelay(pdMS_TO_TICKS(5000));
        
        // Error condition - error LED flashes rapidly
        error_led.StartBlinking(200);
        status_led.TurnOff();
        
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        // Recovery - flash both LEDs
        error_led.StopBlinking();
        status_led.Flash(100);
        error_led.Flash(100);
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
```

### Example 2: Multi-Button Input Handler

```cpp
/**
 * @file MultiButtonHandler.cpp
 * @brief Handles multiple buttons with debouncing and callbacks
 */

#include "McuDigitalGpio.h"
#include <map>
#include <functional>

class MultiButtonHandler {
public:
    using ButtonCallback = std::function<void(int button_id, bool pressed)>;
    
private:
    struct ButtonInfo {
        std::unique_ptr<McuDigitalGpio> gpio;
        ButtonCallback callback;
        bool last_state;
        uint32_t last_change_time;
        static constexpr uint32_t DEBOUNCE_MS = 50;
    };
    
    std::map<int, ButtonInfo> buttons_;
    TaskHandle_t monitor_task_handle_;
    bool monitoring_;
    
    static void MonitorTask(void* parameter) {
        auto* handler = static_cast<MultiButtonHandler*>(parameter);
        handler->MonitorButtons();
    }
    
    void MonitorButtons() {
        while (monitoring_) {
            uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
            
            for (auto& [button_id, info] : buttons_) {
                bool current_state = info.gpio->IsActive();
                
                // Check for state change
                if (current_state != info.last_state) {
                    // Debounce check
                    if (current_time - info.last_change_time >= ButtonInfo::DEBOUNCE_MS) {
                        info.last_state = current_state;
                        info.last_change_time = current_time;
                        
                        // Call callback if registered
                        if (info.callback) {
                            info.callback(button_id, current_state);
                        }
                    }
                }
            }
            
            vTaskDelay(pdMS_TO_TICKS(10)); // 10ms polling
        }
    }
    
public:
    MultiButtonHandler() : monitoring_(false), monitor_task_handle_(nullptr) {}
    
    ~MultiButtonHandler() {
        StopMonitoring();
    }
    
    bool AddButton(int button_id, int pin, ButtonCallback callback = nullptr) {
        if (buttons_.find(button_id) != buttons_.end()) {
            ESP_LOGW("ButtonHandler", "Button %d already exists", button_id);
            return false;
        }
        
        ButtonInfo info;
        info.gpio = std::make_unique<McuDigitalGpio>(pin, 
            McuDigitalGpio::Direction::Input,
            McuDigitalGpio::ActiveState::Low,  // Active low (typical for buttons)
            McuDigitalGpio::OutputMode::PushPull,
            McuDigitalGpio::PullMode::PullUp   // Enable pull-up
        );
        
        if (!info.gpio->Initialize()) {
            ESP_LOGE("ButtonHandler", "Failed to initialize button %d on pin %d", 
                     button_id, pin);
            return false;
        }
        
        info.callback = callback;
        info.last_state = info.gpio->IsActive();
        info.last_change_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        buttons_[button_id] = std::move(info);
        
        ESP_LOGI("ButtonHandler", "Added button %d on pin %d", button_id, pin);
        return true;
    }
    
    void SetCallback(int button_id, ButtonCallback callback) {
        auto it = buttons_.find(button_id);
        if (it != buttons_.end()) {
            it->second.callback = callback;
        }
    }
    
    bool IsButtonPressed(int button_id) const {
        auto it = buttons_.find(button_id);
        return (it != buttons_.end()) ? it->second.gpio->IsActive() : false;
    }
    
    void StartMonitoring() {
        if (monitoring_) return;
        
        monitoring_ = true;
        xTaskCreate(MonitorTask, "ButtonMonitor", 2048, this, 5, &monitor_task_handle_);
    }
    
    void StopMonitoring() {
        if (!monitoring_) return;
        
        monitoring_ = false;
        if (monitor_task_handle_) {
            vTaskDelete(monitor_task_handle_);
            monitor_task_handle_ = nullptr;
        }
    }
};

// Usage example
void button_demo() {
    MultiButtonHandler button_handler;
    
    // Add buttons with callbacks
    button_handler.AddButton(1, 4, [](int id, bool pressed) {
        ESP_LOGI("Button", "Button %d %s", id, pressed ? "PRESSED" : "RELEASED");
        if (pressed) {
            // Handle button 1 press
        }
    });
    
    button_handler.AddButton(2, 5, [](int id, bool pressed) {
        ESP_LOGI("Button", "Button %d %s", id, pressed ? "PRESSED" : "RELEASED");
        if (pressed) {
            // Handle button 2 press
        }
    });
    
    button_handler.StartMonitoring();
    
    // Buttons are now monitored automatically
}
```

## 🚗 Motor Controller Integration

### Example 3: PWM Motor Controller

```cpp
/**
 * @file PwmMotorController.cpp
 * @brief PWM-based motor controller with safety features
 */

#include "McuPwm.h"
#include "McuDigitalGpio.h"
#include "DigitalOutputGuard.h"
#include "PeriodicTimer.h"

class PwmMotorController {
private:
    McuPwm pwm_channel_;
    McuDigitalGpio direction_pin_;
    McuDigitalGpio brake_pin_;
    McuDigitalGpio fault_pin_;
    PeriodicTimer safety_timer_;
    
    float current_speed_;
    bool is_enabled_;
    bool fault_detected_;
    uint32_t last_command_time_;
    static constexpr uint32_t SAFETY_TIMEOUT_MS = 1000;
    
    static void SafetyCallback(void* user_data) {
        auto* controller = static_cast<PwmMotorController*>(user_data);
        controller->CheckSafety();
    }
    
    void CheckSafety() {
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        // Check for command timeout
        if (current_time - last_command_time_ > SAFETY_TIMEOUT_MS) {
            EmergencyStop("Command timeout");
        }
        
        // Check fault pin
        if (fault_pin_.IsActive()) {
            fault_detected_ = true;
            EmergencyStop("Hardware fault detected");
        }
    }
    
    void EmergencyStop(const char* reason) {
        ESP_LOGE("MotorController", "Emergency stop: %s", reason);
        
        // Immediately stop motor
        pwm_channel_.SetDutyCycle(0);
        brake_pin_.SetActive();
        is_enabled_ = false;
        current_speed_ = 0.0f;
    }
    
public:
    PwmMotorController(int pwm_pin, int direction_pin, int brake_pin, int fault_pin)
        : pwm_channel_(pwm_pin, 20000, 12) // 20kHz PWM, 12-bit resolution
        , direction_pin_(direction_pin, McuDigitalGpio::Direction::Output)
        , brake_pin_(brake_pin, McuDigitalGpio::Direction::Output)
        , fault_pin_(fault_pin, McuDigitalGpio::Direction::Input, 
                     McuDigitalGpio::ActiveState::High)
        , safety_timer_(SafetyCallback, this)
        , current_speed_(0.0f)
        , is_enabled_(false)
        , fault_detected_(false)
        , last_command_time_(0) {
    }
    
    bool Initialize() {
        // Initialize PWM
        if (!pwm_channel_.Initialize()) {
            ESP_LOGE("MotorController", "Failed to initialize PWM");
            return false;
        }
        
        // Initialize GPIO pins
        if (!direction_pin_.Initialize() || !brake_pin_.Initialize() || 
            !fault_pin_.Initialize()) {
            ESP_LOGE("MotorController", "Failed to initialize GPIO pins");
            return false;
        }
        
        // Start with motor stopped and brake engaged
        pwm_channel_.SetDutyCycle(0);
        brake_pin_.SetActive();
        
        // Start safety monitoring
        safety_timer_.Start(100000); // 100ms intervals
        
        ESP_LOGI("MotorController", "Motor controller initialized");
        return true;
    }
    
    bool Enable() {
        if (fault_detected_) {
            ESP_LOGE("MotorController", "Cannot enable - fault detected");
            return false;
        }
        
        // Release brake
        brake_pin_.SetInactive();
        is_enabled_ = true;
        last_command_time_ = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        ESP_LOGI("MotorController", "Motor enabled");
        return true;
    }
    
    void Disable() {
        is_enabled_ = false;
        pwm_channel_.SetDutyCycle(0);
        brake_pin_.SetActive();
        current_speed_ = 0.0f;
        
        ESP_LOGI("MotorController", "Motor disabled");
    }
    
    bool SetSpeed(float speed_percent) {
        if (!is_enabled_) {
            ESP_LOGW("MotorController", "Motor not enabled");
            return false;
        }
        
        if (fault_detected_) {
            ESP_LOGE("MotorController", "Cannot set speed - fault detected");
            return false;
        }
        
        // Clamp speed to valid range
        speed_percent = std::max(-100.0f, std::min(100.0f, speed_percent));
        
        // Set direction
        if (speed_percent >= 0) {
            direction_pin_.SetInactive(); // Forward
        } else {
            direction_pin_.SetActive();   // Reverse
            speed_percent = -speed_percent;
        }
        
        // Set PWM duty cycle
        uint32_t duty_cycle = static_cast<uint32_t>((speed_percent / 100.0f) * 4095);
        pwm_channel_.SetDutyCycle(duty_cycle);
        
        current_speed_ = speed_percent;
        last_command_time_ = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        return true;
    }
    
    float GetCurrentSpeed() const {
        return current_speed_;
    }
    
    bool IsEnabled() const {
        return is_enabled_;
    }
    
    bool IsFaultDetected() const {
        return fault_detected_;
    }
    
    void ClearFault() {
        if (!fault_pin_.IsActive()) {
            fault_detected_ = false;
            ESP_LOGI("MotorController", "Fault cleared");
        }
    }
    
    void PerformSafeOperation(std::function<void()> operation) {
        // Use RAII to ensure brake is applied during operation
        DigitalOutputGuard brake_guard(brake_pin_);
        
        if (brake_guard.IsValid()) {
            operation();
        }
        // Brake automatically released when guard destructs
    }
};

// Usage example
void motor_demo_task(void* parameter) {
    PwmMotorController motor(16, 17, 18, 19); // PWM, DIR, BRAKE, FAULT pins
    
    if (!motor.Initialize()) {
        ESP_LOGE("Demo", "Failed to initialize motor controller");
        vTaskDelete(NULL);
        return;
    }
    
    while (true) {
        // Enable motor
        if (motor.Enable()) {
            // Ramp up speed
            for (float speed = 0; speed <= 50; speed += 5) {
                motor.SetSpeed(speed);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            
            // Hold speed
            vTaskDelay(pdMS_TO_TICKS(2000));
            
            // Ramp down speed
            for (float speed = 50; speed >= 0; speed -= 5) {
                motor.SetSpeed(speed);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            
            // Reverse direction
            for (float speed = 0; speed >= -30; speed -= 5) {
                motor.SetSpeed(speed);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            
            // Stop
            motor.SetSpeed(0);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
        motor.Disable();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
```

## 📊 Sensor Reading Systems

### Example 4: Multi-Channel ADC Sensor System

```cpp
/**
 * @file MultiSensorSystem.cpp
 * @brief Multi-channel sensor reading system with calibration
 */

#include "McuAdc.h"
#include "NvsStorage.h"
#include <array>
#include <algorithm>

class MultiSensorSystem {
public:
    struct SensorInfo {
        const char* name;
        uint8_t channel;
        float scale_factor;
        float offset;
        float min_value;
        float max_value;
    };
    
    struct SensorReading {
        float value;
        uint32_t timestamp;
        bool valid;
    };
    
private:
    McuAdc adc_;
    NvsStorage calibration_storage_;
    std::array<SensorInfo, 4> sensors_;
    std::array<SensorReading, 4> latest_readings_;
    TaskHandle_t reading_task_handle_;
    bool continuous_reading_;
    
    static void ReadingTask(void* parameter) {
        auto* system = static_cast<MultiSensorSystem*>(parameter);
        system->ContinuousReadingLoop();
    }
    
    void ContinuousReadingLoop() {
        while (continuous_reading_) {
            ReadAllSensors();
            vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz sampling
        }
    }
    
    float ApplyCalibration(const SensorInfo& sensor, float raw_voltage) {
        float calibrated = (raw_voltage * sensor.scale_factor) + sensor.offset;
        return std::max(sensor.min_value, std::min(sensor.max_value, calibrated));
    }
    
    bool LoadCalibration() {
        if (!calibration_storage_.Open()) {
            ESP_LOGW("SensorSystem", "Failed to open calibration storage");
            return false;
        }
        
        for (size_t i = 0; i < sensors_.size(); ++i) {
            char key[32];
            snprintf(key, sizeof(key), "sensor_%zu_scale", i);
            
            uint32_t scale_bits;
            if (calibration_storage_.GetU32(key, scale_bits)) {
                sensors_[i].scale_factor = *reinterpret_cast<float*>(&scale_bits);
            }
            
            snprintf(key, sizeof(key), "sensor_%zu_offset", i);
            uint32_t offset_bits;
            if (calibration_storage_.GetU32(key, offset_bits)) {
                sensors_[i].offset = *reinterpret_cast<float*>(&offset_bits);
            }
        }
        
        return true;
    }
    
    bool SaveCalibration() {
        if (!calibration_storage_.Open()) {
            return false;
        }
        
        for (size_t i = 0; i < sensors_.size(); ++i) {
            char key[32];
            snprintf(key, sizeof(key), "sensor_%zu_scale", i);
            
            uint32_t scale_bits = *reinterpret_cast<uint32_t*>(&sensors_[i].scale_factor);
            calibration_storage_.SetU32(key, scale_bits);
            
            snprintf(key, sizeof(key), "sensor_%zu_offset", i);
            uint32_t offset_bits = *reinterpret_cast<uint32_t*>(&sensors_[i].offset);
            calibration_storage_.SetU32(key, offset_bits);
        }
        
        return true;
    }
    
public:
    MultiSensorSystem() 
        : adc_(HF_ADC_UNIT_1, 11, hf_adc_resolution_t::Bits12)
        , calibration_storage_("sensor_cal")
        , continuous_reading_(false)
        , reading_task_handle_(nullptr) {
        
        // Initialize sensor configuration
        sensors_[0] = {"Temperature", 0, 100.0f, -50.0f, -40.0f, 125.0f};
        sensors_[1] = {"Pressure", 1, 10.0f, 0.0f, 0.0f, 100.0f};
        sensors_[2] = {"Voltage", 2, 1.0f, 0.0f, 0.0f, 5.0f};
        sensors_[3] = {"Current", 3, 2.0f, -1.0f, -10.0f, 10.0f};
        
        // Initialize readings
        for (auto& reading : latest_readings_) {
            reading = {0.0f, 0, false};
        }
    }
    
    bool Initialize() {
        // Initialize ADC
        if (!adc_.Initialize()) {
            ESP_LOGE("SensorSystem", "Failed to initialize ADC");
            return false;
        }
        
        // Configure ADC channels
        for (const auto& sensor : sensors_) {
            HfAdcErr result = adc_.ConfigureChannel(sensor.channel, 11);
            if (result != HfAdcErr::ADC_OK) {
                ESP_LOGE("SensorSystem", "Failed to configure channel %d", sensor.channel);
                return false;
            }
        }
        
        // Load calibration data
        LoadCalibration();
        
        ESP_LOGI("SensorSystem", "Sensor system initialized");
        return true;
    }
    
    void ReadAllSensors() {
        uint32_t timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        for (size_t i = 0; i < sensors_.size(); ++i) {
            float voltage;
            HfAdcErr result = adc_.ReadChannelVoltage(
                sensors_[i].channel, 
                voltage, 
                10,    // Average 10 samples
                1000   // 1ms between samples
            );
            
            if (result == HfAdcErr::ADC_OK) {
                latest_readings_[i].value = ApplyCalibration(sensors_[i], voltage);
                latest_readings_[i].timestamp = timestamp;
                latest_readings_[i].valid = true;
            } else {
                latest_readings_[i].valid = false;
                ESP_LOGW("SensorSystem", "Failed to read sensor %s", sensors_[i].name);
            }
        }
    }
    
    SensorReading GetSensorReading(size_t sensor_index) const {
        if (sensor_index < sensors_.size()) {
            return latest_readings_[sensor_index];
        }
        return {0.0f, 0, false};
    }
    
    SensorReading GetSensorReading(const char* sensor_name) const {
        for (size_t i = 0; i < sensors_.size(); ++i) {
            if (strcmp(sensors_[i].name, sensor_name) == 0) {
                return latest_readings_[i];
            }
        }
        return {0.0f, 0, false};
    }
    
    void StartContinuousReading() {
        if (continuous_reading_) return;
        
        continuous_reading_ = true;
        xTaskCreate(ReadingTask, "SensorReading", 4096, this, 5, &reading_task_handle_);
    }
    
    void StopContinuousReading() {
        if (!continuous_reading_) return;
        
        continuous_reading_ = false;
        if (reading_task_handle_) {
            vTaskDelete(reading_task_handle_);
            reading_task_handle_ = nullptr;
        }
    }
    
    bool CalibrateSensor(size_t sensor_index, float reference_value) {
        if (sensor_index >= sensors_.size()) {
            return false;
        }
        
        // Read raw voltage
        float raw_voltage;
        HfAdcErr result = adc_.ReadChannelVoltage(
            sensors_[sensor_index].channel, 
            raw_voltage, 
            50,    // Many samples for calibration
            500    // 0.5ms between samples
        );
        
        if (result != HfAdcErr::ADC_OK) {
            ESP_LOGE("SensorSystem", "Failed to read calibration value");
            return false;
        }
        
        // Calculate new calibration parameters
        // Simple offset calibration: reference = raw * scale + offset
        // Assuming scale is correct, adjust offset
        sensors_[sensor_index].offset = reference_value - (raw_voltage * sensors_[sensor_index].scale_factor);
        
        // Save calibration
        SaveCalibration();
        
        ESP_LOGI("SensorSystem", "Calibrated sensor %s: offset = %.3f", 
                 sensors_[sensor_index].name, sensors_[sensor_index].offset);
        
        return true;
    }
    
    void PrintAllReadings() const {
        ESP_LOGI("SensorSystem", "=== Sensor Readings ===");
        for (size_t i = 0; i < sensors_.size(); ++i) {
            const auto& reading = latest_readings_[i];
            const auto& sensor = sensors_[i];
            
            if (reading.valid) {
                ESP_LOGI("SensorSystem", "%s: %.2f (age: %lu ms)", 
                         sensor.name, reading.value, 
                         (xTaskGetTickCount() * portTICK_PERIOD_MS) - reading.timestamp);
            } else {
                ESP_LOGW("SensorSystem", "%s: INVALID", sensor.name);
            }
        }
    }
};

// Usage example
void sensor_demo_task(void* parameter) {
    MultiSensorSystem sensors;
    
    if (!sensors.Initialize()) {
        ESP_LOGE("Demo", "Failed to initialize sensor system");
        vTaskDelete(NULL);
        return;
    }
    
    // Start continuous reading
    sensors.StartContinuousReading();
    
    while (true) {
        // Print all sensor readings
        sensors.PrintAllReadings();
        
        // Check specific sensors
        auto temp_reading = sensors.GetSensorReading("Temperature");
        if (temp_reading.valid && temp_reading.value > 80.0f) {
            ESP_LOGW("Demo", "High temperature detected: %.1f°C", temp_reading.value);
        }
        
        auto pressure_reading = sensors.GetSensorReading("Pressure");
        if (pressure_reading.valid && pressure_reading.value < 5.0f) {
            ESP_LOGW("Demo", "Low pressure detected: %.1f bar", pressure_reading.value);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
```

## 🌐 Communication Protocols

### Example 5: CAN Bus Communication System

```cpp
/**
 * @file CanCommunicationSystem.cpp
 * @brief CAN bus communication with message handling
 */

#include "McuCan.h"
#include "SfCan.h"
#include <map>
#include <functional>

class CanCommunicationSystem {
public:
    using MessageHandler = std::function<void(const hf_can_message_t& message)>;
    
private:
    std::unique_ptr<SfCan> can_driver_;
    std::map<uint32_t, MessageHandler> message_handlers_;
    TaskHandle_t receive_task_handle_;
    bool receiving_;
    
    static void ReceiveTask(void* parameter) {
        auto* system = static_cast<CanCommunicationSystem*>(parameter);
        system->ReceiveLoop();
    }
    
    void ReceiveLoop() {
        hf_can_message_t message;
        
        while (receiving_) {
            HfCanErr result = can_driver_->ReceiveMessage(message, 100); // 100ms timeout
            
            if (result == HfCanErr::CAN_OK) {
                ProcessMessage(message);
            } else if (result != HfCanErr::CAN_ERR_TIMEOUT) {
                ESP_LOGW("CanSystem", "CAN receive error: %d", static_cast<int>(result));
            }
        }
    }
    
    void ProcessMessage(const hf_can_message_t& message) {
        auto it = message_handlers_.find(message.id);
        if (it != message_handlers_.end()) {
            it->second(message);
        } else {
            ESP_LOGD("CanSystem", "Unhandled CAN message: ID=0x%X", message.id);
        }
    }
    
public:
    CanCommunicationSystem(hf_can_port_t port) {
        // Create MCU CAN implementation
        auto can_impl = std::make_unique<McuCan>(port);
        
        // Wrap with thread-safe interface
        can_driver_ = std::make_unique<SfCan>(std::move(can_impl));
        
        receiving_ = false;
        receive_task_handle_ = nullptr;
    }
    
    bool Initialize(uint32_t bitrate = 500000) {
        hf_can_config_t config = {
            .mode = HF_CAN_MODE_NORMAL,
            .bitrate = bitrate,
            .rx_queue_size = 64,
            .tx_queue_size = 32
        };
        
        HfCanErr result = can_driver_->Initialize(config);
        if (result != HfCanErr::CAN_OK) {
            ESP_LOGE("CanSystem", "Failed to initialize CAN: %d", static_cast<int>(result));
            return false;
        }
        
        ESP_LOGI("CanSystem", "CAN system initialized at %lu bps", bitrate);
        return true;
    }
    
    void RegisterMessageHandler(uint32_t message_id, MessageHandler handler) {
        message_handlers_[message_id] = handler;
        ESP_LOGI("CanSystem", "Registered handler for CAN ID 0x%X", message_id);
    }
    
    void UnregisterMessageHandler(uint32_t message_id) {
        message_handlers_.erase(message_id);
    }
    
    HfCanErr SendMessage(uint32_t id, const uint8_t* data, size_t length) {
        hf_can_message_t message = {};
        message.id = id;
        message.length = std::min(length, sizeof(message.data));
        message.extended = (id > 0x7FF);
        message.remote = false;
        
        memcpy(message.data, data, message.length);
        
        return can_driver_->SendMessage(message, 1000); // 1 second timeout
    }
    
    HfCanErr SendMessage(uint32_t id, const std::vector<uint8_t>& data) {
        return SendMessage(id, data.data(), data.size());
    }
    
    void StartReceiving() {
        if (receiving_) return;
        
        receiving_ = true;
        xTaskCreate(ReceiveTask, "CanReceive", 4096, this, 6, &receive_task_handle_);
    }
    
    void StopReceiving() {
        if (!receiving_) return;
        
        receiving_ = false;
        if (receive_task_handle_) {
            vTaskDelete(receive_task_handle_);
            receive_task_handle_ = nullptr;
        }
    }
    
    // Convenience methods for common message types
    HfCanErr SendHeartbeat(uint32_t node_id) {
        uint8_t data[1] = {0x00}; // Heartbeat data
        return SendMessage(0x700 + node_id, data, 1);
    }
    
    HfCanErr SendPDO(uint32_t node_id, uint8_t pdo_number, const uint8_t* data, size_t length) {
        uint32_t id = 0x180 + (pdo_number * 0x100) + node_id;
        return SendMessage(id, data, length);
    }
    
    HfCanErr SendSDO(uint32_t node_id, const uint8_t* sdo_data) {
        uint32_t id = 0x600 + node_id;
        return SendMessage(id, sdo_data, 8);
    }
};

// Example usage with CANopen-like protocol
class MotorControlNode {
private:
    CanCommunicationSystem can_system_;
    uint32_t node_id_;
    int32_t target_velocity_;
    int32_t actual_velocity_;
    uint16_t status_word_;
    
    void HandleControlMessage(const hf_can_message_t& message) {
        if (message.length >= 4) {
            // Extract target velocity from message
            target_velocity_ = *reinterpret_cast<const int32_t*>(message.data);
            ESP_LOGI("MotorNode", "New target velocity: %ld", target_velocity_);
        }
    }
    
    void HandleStatusRequest(const hf_can_message_t& message) {
        // Send status response
        uint8_t status_data[8];
        *reinterpret_cast<int32_t*>(&status_data[0]) = actual_velocity_;
        *reinterpret_cast<uint16_t*>(&status_data[4]) = status_word_;
        
        can_system_.SendMessage(0x580 + node_id_, status_data, 6);
    }
    
public:
    MotorControlNode(uint32_t node_id) 
        : can_system_(HF_CAN_PORT_0)
        , node_id_(node_id)
        , target_velocity_(0)
        , actual_velocity_(0)
        , status_word_(0x0000) {
    }
    
    bool Initialize() {
        if (!can_system_.Initialize()) {
            return false;
        }
        
        // Register message handlers
        can_system_.RegisterMessageHandler(0x200 + node_id_, 
            `this`(const hf_can_message_t& msg) { HandleControlMessage(msg); });
            
        can_system_.RegisterMessageHandler(0x600 + node_id_, 
            `this`(const hf_can_message_t& msg) { HandleStatusRequest(msg); });
        
        can_system_.StartReceiving();
        
        ESP_LOGI("MotorNode", "Motor control node %lu initialized", node_id_);
        return true;
    }
    
    void SendPeriodicUpdate() {
        // Send periodic status update
        uint8_t pdo_data[8];
        *reinterpret_cast<int32_t*>(&pdo_data[0]) = actual_velocity_;
        *reinterpret_cast<uint16_t*>(&pdo_data[4]) = status_word_;
        
        can_system_.SendPDO(node_id_, 1, pdo_data, 6);
    }
    
    void SetActualVelocity(int32_t velocity) {
        actual_velocity_ = velocity;
    }
    
    void SetStatusWord(uint16_t status) {
        status_word_ = status;
    }
    
    int32_t GetTargetVelocity() const {
        return target_velocity_;
    }
};

// Usage example
void can_demo_task(void* parameter) {
    MotorControlNode motor_node(1);
    
    if (!motor_node.Initialize()) {
        ESP_LOGE("Demo", "Failed to initialize motor node");
        vTaskDelete(NULL);
        return;
    }
    
    while (true) {
        // Simulate motor operation
        static int32_t simulated_velocity = 0;
        int32_t target = motor_node.GetTargetVelocity();
        
        // Simple velocity control simulation
        if (simulated_velocity < target) {
            simulated_velocity += 10;
        } else if (simulated_velocity > target) {
            simulated_velocity -= 10;
        }
        
        motor_node.SetActualVelocity(simulated_velocity);
        motor_node.SetStatusWord(0x0031); // Ready to switch on + switched on
        
        // Send periodic update
        motor_node.SendPeriodicUpdate();
        
        vTaskDelay(pdMS_TO_TICKS(10)); // 100Hz update rate
    }
}
```

---

*This examples document provides practical, production-ready code patterns for common embedded systems scenarios. Each example includes proper error handling, resource management, and follows best practices for the HardFOC Internal Interface Wrapper.*
