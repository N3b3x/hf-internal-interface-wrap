# SfPio API Reference

## Overview

`SfPio` is a software-based implementation of the `BasePio` interface, providing PIO (Programmable I/O) functionality through software emulation. This implementation is useful for testing, simulation, or when hardware PIO controllers are not available.

## Features

- **Software Emulation** - Pure software implementation of PIO functionality
- **Testing Support** - Ideal for unit testing and simulation
- **Cross-Platform** - Works on any platform with basic I/O capabilities
- **Configurable** - Flexible configuration for different use cases
- **Debug Support** - Enhanced debugging and logging capabilities

## Header File

```cpp
#include "inc/mcu/software/SfPio.h"
```

## Class Definition

```cpp
class SfPio : public BasePio {
public:
    // Constructor with full configuration
    explicit SfPio(
        hf_pio_port_t port = hf_pio_port_t::HF_PIO_PORT_0,
        hf_pio_pin_t pin = hf_pio_pin_t::HF_PIO_PIN_0,
        hf_pio_direction_t direction = hf_pio_direction_t::HF_PIO_DIRECTION_OUTPUT
    ) noexcept;

    // Destructor
    ~SfPio() override;

    // BasePio implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // PIO operations
    hf_pio_err_t WritePin(hf_pio_state_t state) noexcept override;
    hf_pio_err_t ReadPin(hf_pio_state_t* state) const noexcept override;
    hf_pio_err_t TogglePin() noexcept override;
    hf_pio_err_t SetDirection(hf_pio_direction_t direction) noexcept override;
    hf_pio_err_t GetDirection(hf_pio_direction_t* direction) const noexcept override;
};
```

## Usage Examples

### Basic PIO Usage

```cpp
#include "inc/mcu/software/SfPio.h"

// Create software PIO instance
SfPio pio(HF_PIO_PORT_0, HF_PIO_PIN_0, HF_PIO_DIRECTION_OUTPUT);

// Initialize
if (!pio.Initialize()) {
    printf("Failed to initialize software PIO\n");
    return;
}

// Write to pin
hf_pio_err_t err = pio.WritePin(HF_PIO_STATE_HIGH);
if (err != HF_PIO_ERR_OK) {
    printf("Failed to write pin: %d\n", err);
    return;
}

// Read from pin
hf_pio_state_t state;
err = pio.ReadPin(&state);
if (err == HF_PIO_ERR_OK) {
    printf("Pin state: %s\n", state == HF_PIO_STATE_HIGH ? "HIGH" : "LOW");
}

// Toggle pin
err = pio.TogglePin();
if (err == HF_PIO_ERR_OK) {
    printf("Pin toggled\n");
}
```

## Related Documentation

- [BasePio API Reference](BasePio.md) - Base class interface
- [HardwareTypes Reference](HardwareTypes.md) - Platform-agnostic type definitions