# HF-IID-ESPIDF
Internal Interface Drivers - wrappers for the espidf to be used in the HardFOC controller

# IID-ESPIDF

This component bundles the internal interface drivers and platform
specific utilities used by the HardFOC project.

The drivers provide low level access to GPIO, ADC and bus interfaces for
ESP-IDF targets.  Utility code such as the base thread framework and the
ThreadX compatibility layer are also included here as they depend on the
underlying RTOS implementation.

## Contents
- Internal interface driver implementations located in the parent
  directory (`BaseGpio`, `DigitalInput`, `SpiBus`, `I2cBus` ...).
- Thread-safe variants such as `SfSpiBus` and `SfI2cBus` which rely on
  the HF-RTOSW-ESPIDF mutex APIs.
- Platform dependent utilities from `UTILITIES/common` (e.g.
  `BaseThread`, mutex helpers, timing utilities).

## Usage
Add `iid-espidf` to your component requirements.  The component exports
the include directories for both the driver headers and the utilities.
It depends on the `hal` component and requires FreeRTOS.

```cmake
idf_component_register(
    REQUIRES iid-espidf
)
```

## License
This project is licensed under the GNU General Public License v3.0 or
later.