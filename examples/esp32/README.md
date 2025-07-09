# Advanced PWM Example

This example demonstrates all the advanced features of the refactored EspPwm implementation for ESP32 variants.

## 🎯 **Features Demonstrated**

### **Core Features**
- ✅ **Multi-Variant ESP32 Support**: Works on C6, Classic, S2, S3, C3, C2, H2
- ✅ **Unit Configuration**: Advanced PWM unit setup with different modes
- ✅ **Hardware Fade**: Smooth LED transitions using ESP32 LEDC fade
- ✅ **Complementary Outputs**: Motor control with deadtime insertion
- ✅ **Statistics & Diagnostics**: Real-time performance monitoring
- ✅ **Advanced Timer Management**: Automatic timer allocation and optimization
- ✅ **Interrupt-Driven Callbacks**: ISR-safe period and fault callbacks
- ✅ **Error Handling**: Comprehensive error detection and recovery

### **Application Examples**
- **LED Control**: Smooth fade effects with hardware acceleration
- **Motor Control**: Complementary PWM outputs for H-bridge drivers
- **Servo Control**: Precise position control with high resolution
- **Audio Generation**: Musical tone generation with frequency control

## 🚀 **Quick Start**

### **Prerequisites**
- ESP-IDF v5.4.2 or later
- ESP32 development board (any variant)
- Basic understanding of PWM concepts

### **Building the Example**

1. **Set up ESP-IDF environment**:
   ```bash
   . $HOME/esp/esp-idf/export.sh
   ```

2. **Navigate to the example directory**:
   ```bash
   cd examples/esp32/
   ```

3. **Configure the project**:
   ```bash
   idf.py set-target esp32c6  # or your target variant
   idf.py menuconfig
   ```

4. **Build the project**:
   ```bash
   idf.py build
   ```

5. **Flash to your device**:
   ```bash
   idf.py flash monitor
   ```

## 🔧 **Hardware Setup**

### **Required Connections**
Connect the following components to your ESP32:

| Component | GPIO Pin | Purpose |
|-----------|----------|---------|
| LED | GPIO 2 | PWM-controlled LED with fade effects |
| Motor Phase A | GPIO 3 | H-bridge motor control |
| Motor Phase B | GPIO 4 | Complementary motor control |
| Servo | GPIO 5 | Servo motor position control |
| Audio Output | GPIO 6 | Audio tone generation |

### **Optional Components**
- **LED with current-limiting resistor** (220Ω-1kΩ)
- **H-bridge motor driver** (e.g., L298N, DRV8833)
- **Servo motor** (standard 5V servo)
- **Audio amplifier** (for audio output)

## 📊 **Example Output**

When you run the example, you'll see output similar to this:

```
I (1234) AdvancedPwmExample: === Advanced PWM Example Starting ===
I (1234) AdvancedPwmExample: ESP32 Variant: ESP32-C6
I (1234) AdvancedPwmExample: PWM Configuration:
I (1234) AdvancedPwmExample:   Max Channels: 8
I (1234) AdvancedPwmExample:   Max Timers: 4
I (1234) AdvancedPwmExample:   Max Resolution: 14 bits
I (1234) AdvancedPwmExample:   Frequency Range: 1 - 40000000 Hz
I (1234) AdvancedPwmExample: PWM system initialized successfully
I (1234) AdvancedPwmExample: All channels started successfully

I (1234) AdvancedPwmExample: === LED Fade Demonstration ===
I (1234) AdvancedPwmExample: Fading LED from 0% to 100% over 2 seconds...
I (4234) AdvancedPwmExample: Fading LED from 100% to 0% over 1 second...

I (5734) AdvancedPwmExample: === Motor Control Demonstration ===
I (5734) AdvancedPwmExample: Ramping up motor speed...
I (5734) AdvancedPwmExample: Motor duty cycle: 10.0%
I (6234) AdvancedPwmExample: Motor duty cycle: 20.0%
...

I (12334) AdvancedPwmExample: === Final Status ===
I (12334) AdvancedPwmExample: === PWM Statistics ===
I (12334) AdvancedPwmExample: Total duty updates: 45
I (12334) AdvancedPwmExample: Total frequency changes: 12
I (12334) AdvancedPwmExample: Total fades completed: 8
I (12334) AdvancedPwmExample: Total errors: 0
I (12334) AdvancedPwmExample: Last operation time: 12345678 us
I (12334) AdvancedPwmExample: Last error: Success
```

## 🔍 **Code Structure**

### **Main Components**

1. **Configuration Section** (lines 30-50)
   - PWM frequencies and resolutions for different applications
   - GPIO pin assignments for ESP32-C6

2. **Callback Functions** (lines 60-80)
   - `PeriodCompleteCallback`: ISR-safe period completion handler
   - `FaultCallback`: Error handling and recovery

3. **Helper Functions** (lines 85-150)
   - `PrintStatistics()`: Display performance metrics
   - `PrintDiagnostics()`: Show system health
   - `PrintChannelStatus()`: Individual channel information

4. **Configuration Functions** (lines 155-300)
   - `ConfigureLedChannel()`: LED with fade support
   - `ConfigureMotorChannels()`: Complementary motor outputs
   - `ConfigureServoChannel()`: High-resolution servo control
   - `ConfigureAudioChannel()`: Audio tone generation

5. **Demonstration Functions** (lines 305-490)
   - `DemonstrateLedFade()`: Hardware fade effects
   - `DemonstrateMotorControl()`: Motor speed ramping
   - `DemonstrateServoControl()`: Servo position control
   - `DemonstrateAudioGeneration()`: Musical scale generation
   - `DemonstrateAdvancedFeatures()`: Advanced feature showcase

## 🎛️ **Key Features Explained**

### **1. Multi-Variant Support**
The example automatically detects your ESP32 variant and uses appropriate configurations:

```cpp
#ifdef HF_MCU_ESP32C6
    // ESP32-C6 specific settings
#elif defined(HF_MCU_ESP32)
    // ESP32 Classic specific settings
// ... other variants
#endif
```

### **2. Unit Configuration**
Advanced PWM setup with mode selection:

```cpp
hf_pwm_unit_config_t pwm_config;
pwm_config.unit_id = 0;
pwm_config.mode = hf_pwm_mode_t::Fade;  // Enable fade mode
pwm_config.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
pwm_config.enable_fade = true;
pwm_config.enable_interrupts = true;

EspPwm pwm(pwm_config);
```

### **3. Hardware Fade**
Smooth LED transitions using ESP32 LEDC hardware:

```cpp
// Fade from 0% to 100% over 2 seconds
pwm.SetHardwareFade(0, 1.0f, 2000);
```

### **4. Complementary Outputs**
Motor control with automatic deadtime insertion:

```cpp
// Configure complementary outputs
pwm.SetComplementaryOutput(1, 2, 1000); // 1μs deadtime
```

### **5. Statistics & Diagnostics**
Real-time performance monitoring:

```cpp
hf_pwm_statistics_t stats;
pwm.GetStatistics(stats);
printf("Total duty updates: %llu\n", stats.total_duty_updates);
```

## 🔧 **Customization**

### **Changing GPIO Pins**
Modify the pin assignments in the configuration section:

```cpp
static constexpr hf_pin_num_t LED_PIN = 2;      // Change to your LED pin
static constexpr hf_pin_num_t MOTOR_A_PIN = 3;  // Change to your motor pin
```

### **Adjusting Frequencies**
Modify the frequency constants for your application:

```cpp
static constexpr uint32_t LED_FREQUENCY_HZ = 1000;    // LED PWM frequency
static constexpr uint32_t MOTOR_FREQUENCY_HZ = 20000; // Motor PWM frequency
```

### **Adding New Channels**
To add a new PWM channel:

1. Define the configuration
2. Configure the channel
3. Enable and control it

```cpp
// Configure new channel
hf_pwm_channel_config_t new_config;
new_config.output_pin = YOUR_PIN;
new_config.frequency_hz = YOUR_FREQUENCY;
// ... other settings

pwm.ConfigureChannel(5, new_config);
pwm.EnableChannel(5);
pwm.SetDutyCycle(5, 0.5f);
```

## 🐛 **Troubleshooting**

### **Common Issues**

1. **"Failed to initialize PWM system"**
   - Check ESP-IDF version (requires v5.4.2+)
   - Verify target configuration matches your hardware

2. **"Failed to configure channel"**
   - Check GPIO pin availability
   - Verify frequency and resolution are within limits
   - Ensure no pin conflicts

3. **"Hardware fade not working"**
   - Ensure fade mode is enabled in unit configuration
   - Check that fade functionality is installed

4. **"Motor not responding"**
   - Verify H-bridge connections
   - Check complementary output configuration
   - Ensure proper power supply

### **Debug Information**
Enable debug logging by setting log level in `menuconfig`:
- Component config → Log output → Default log verbosity → Debug

## 📚 **Further Reading**

- [ESP32 LEDC Documentation](https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32c6/api-reference/peripherals/ledc.html)
- [HardFOC PWM Architecture](docs/ESP32_PWM_ARCHITECTURAL_AUDIT_COMPLETE.md)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32c6/)

## 🤝 **Contributing**

This example is part of the HardFOC project. To contribute:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## 📄 **License**

This example is licensed under the same license as the HardFOC project. 