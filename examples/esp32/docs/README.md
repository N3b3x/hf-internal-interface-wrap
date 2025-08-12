# ESP32 HardFOC Interface Wrapper — Documentation Index

Welcome to the ESP32 examples documentation hub. Use the links below to navigate the guides.

## Core Guides
- [Centralized Configuration System](../README_CENTRALIZED_CONFIG.md)
- [Build & Flash Guide](./README_BUILD_SYSTEM.md)

## Example-Specific Docs
- [GPIO Test Guide](./README_GPIO_TEST.md)
- [ADC Test Guide](./README_ADC_TESTING.md)
- [NVS Test Guide](./README_NVS_TEST.md)
- [PIO Test Guide](./README_PIO_TEST.md)
- [Testing Infrastructure](./README_TESTING_INFRASTRUCTURE.md)
- [Timer Test Report](./TIMER_TEST_REPORT.md)

## Quick Links
- Build script: `examples/esp32/scripts/build_example.sh`
- Flash script: `examples/esp32/scripts/flash_example.sh`
- Central config file: `examples/esp32/examples_config.yml`
- Config helpers: `examples/esp32/scripts/`
- Available examples: Run `./scripts/build_example.sh list` to see all examples

## Available Scripts

All scripts are located in the `scripts/` directory:
- `build_example.sh` - Build examples with centralized configuration
- `flash_example.sh` - Flash and monitor examples with auto-build capability
- `config_loader.sh` - Configuration helper functions for bash scripts
- `get_example_info.py` - Python script for CMake integration
- `setup_ci.sh` - CI environment setup
- `setup_common.sh` - Common setup functions
- `setup_repo.sh` - Repository setup


