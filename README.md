# HF Internal Interface Wrap

Platform-agnostic internal interface drivers – handy wrappers that provide unified APIs across different MCU platforms used by the HardFOC controller. 🏎️ These abstractions keep your code tidy and portable across different microcontroller families.

> **Note:** This component supports multiple platforms through the hf_platform system. Currently includes ESP-IDF support (requires v5.5 or newer) with extensibility for other platforms.

For detailed API guides see [docs/index.md](docs/index.md).

## Platform-Agnostic Architecture

This component provides a truly platform-agnostic hardware abstraction layer. All platform-specific types and functionality are centralized in `PlatformTypes.h` using a consistent `hf_*` naming convention. The interface classes are completely free of conditional compilation and platform-specific includes.

### Key Features
- **True Platform Independence**: All interface headers use only `hf_*` types from `PlatformTypes.h`
- **No Conditional Compilation**: Interface classes are clean and portable
- **Layered CAN Stack**: CanBus → FlexCan → SfCan with proper abstraction
- **Legacy Compatibility**: Preserved useful utility functions like `IsActiveHigh()`
- **Thread-Safe Variants**: Built-in thread safety using standard C++ mutexes

Each abstraction is intentionally compact and header-focused where possible. Create an object, call `Open()` or `Start()` and off you go. All the platform-specific ceremony is performed behind the scenes so your code stays compact and portable between MCU families.


### Interface Classes with Legacy Support
- **GPIO**: `BaseGpio`, `DigitalInput`, `DigitalOutput`, `DigitalGpio` with legacy helpers like `IsActiveHigh()`
- **Communication**: `I2cBus`, `SpiBus`, `UartDriver` with convenience methods like `WriteByte()`, `ReadRegister()`
- **CAN Stack**: `CanBus` (platform-agnostic) → `FlexCan` (compatibility) → `SfCan` (thread-safe)
- **ADC**: `McuAdc` with legacy functions like `ReadVoltage()`, `ReadAveraged()`
- **PWM**: `PwmOutput` with utilities like `SetDutyPercent()`, `GenerateSquareWave()`
- **Thread-Safe Variants**: `SfI2cBus`, `SfSpiBus`, `SfUartDriver`, `SfCan` using standard C++ mutexes
- **Timers**: `PeriodicTimer` with platform-agnostic callback support
- **Storage**: `NvsStorage` for non-volatile key-value storage


### Usage
Add the component to your project requirements. The component provides platform-agnostic hardware interfaces with automatic platform detection.

```cmake
idf_component_register(
    SRCS ...
    REQUIRES hf-internal-interface-wrap
)
```

### Platform-Agnostic Code Examples

All examples use `hf_*` types from `PlatformTypes.h` for maximum portability:

### PwmOutput example
```cpp
#include "PwmOutput.h"

PwmOutput pwm(HF_GPIO_NUM_4, HF_LEDC_CHANNEL_0, HF_LEDC_TIMER_0, 5000,
              HF_LEDC_TIMER_13_BIT);

void app_main() {
    pwm.Start();
    pwm.SetDuty(0.5f); // 50% duty cycle
    pwm.SetDutyPercent(75); // 75% duty cycle
}
```

### DigitalOutput example
```cpp
#include "DigitalOutput.h"

DigitalOutput led(HF_GPIO_NUM_2, true); // Active high LED

void app_main() {
    led.Open();
    led.SetHigh(); // Turn on LED
    led.SetLow();  // Turn off LED
    led.Toggle();  // Toggle LED state
}
```

### PeriodicTimer example
```cpp
#include "PeriodicTimer.h"

static void BlinkCallback(void*) {
    // Toggle LED or perform periodic task
}

PeriodicTimer timer(&BlinkCallback);

void app_main() {
    timer.Start(500000); // 500ms period
}
```

### I2C example
```cpp
#include "I2cBus.h"

I2cBus i2c(HF_I2C_NUM_0, HF_GPIO_NUM_21, HF_GPIO_NUM_22, 400000);

void app_main() {
    i2c.Open();
    
    // Write single byte
    i2c.WriteByte(0x50, 0x10, 0xFF);
    
    // Read register
    hf_u8_t data;
    i2c.ReadRegister(0x50, 0x10, data);
}
```

### SfUartDriver example
```cpp
#include "SfUartDriver.h"
#include <mutex>

std::mutex uart_mutex;
hf_uart_config_t cfg = {
    .baud_rate = 115200,
    .data_bits = HF_UART_DATA_8_BITS,
    .parity = HF_UART_PARITY_DISABLE,
    .stop_bits = HF_UART_STOP_BITS_1,
    .flow_ctrl = HF_UART_HW_FLOWCTRL_DISABLE
};

SfUartDriver serial(HF_UART_NUM_1, cfg, HF_GPIO_NUM_1, HF_GPIO_NUM_3, uart_mutex);

void app_main() {
    serial.Open();
    serial.Write(reinterpret_cast<const hf_u8_t*>("Hello"), 5);
}
```

### Thread-Safe CAN example
```cpp
#include "SfCan.h"
#include <mutex>

std::mutex can_mutex;
SfCan can(0, 500000, can_mutex);

void app_main() {
    can.Open();
    
    hf_can_message_t msg = {};
    msg.identifier = 0x100;
    msg.data[0] = 0x01;
    msg.data_length_code = 1;
    
    can.Transmit(msg);
}
```

### NvsStorage example
```cpp
#include "NvsStorage.h"

NvsStorage storage("app");

void app_main() {
    storage.Open();
    
    // Store configuration
    storage.SetU32("count", 42);
    storage.SetString("name", "HardFOC");
    
    // Read configuration
    hf_u32_t count;
    storage.GetU32("count", count);
    
    std::string name;
    storage.GetString("name", name);
}
```

### ADC example

```cpp
#include "McuAdc.h"

McuAdc adc(HF_ADC_UNIT_1, HF_ADC_CHANNEL_0, HF_ADC_ATTEN_DB_11);

void app_main() {
    adc.Open();
    
    // Read raw value
    hf_u32_t raw = adc.Read();
    
    // Read voltage
    float voltage = adc.ReadVoltage();
    
    // Read averaged
    hf_u32_t avg = adc.ReadAveraged(10);
}
```
### Building & Testing

Follow these steps to build the library and run the unit tests:

```bash
$IDF_PATH/export.sh
cd tests
mkdir build && cd build
cmake .. && make
./test_runner
```


### License

This project is licensed under the GNU General Public License v3.0 or later.
