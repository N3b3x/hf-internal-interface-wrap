# ESP32 Port Detection and Troubleshooting Guide

This guide explains how to use the improved port detection system for ESP32 devices on both macOS and Linux systems.

---

**Navigation**: [← Previous: Utility Scripts](README_UTILITY_SCRIPTS.md) | [Back to Scripts](../README.md) | [Next: Scripts Overview →](README_SCRIPTS_OVERVIEW.md)

---

## Overview

The flash system has been enhanced with intelligent port detection that automatically finds ESP32 devices on your system. This solves common issues where users couldn't find the correct USB port for their ESP32 device.

## Quick Start

### 1. Automatic Port Detection (Recommended)

The `flash_app.sh` script now automatically detects ESP32 devices:

```bash
# From project root
./examples/esp32/scripts/flash_app.sh <example_name> <build_type> <operation>
```

**Example:**
```bash
./examples/esp32/scripts/flash_app.sh gpio Release flash_monitor
```

The script will:
- Automatically detect your operating system (macOS/Linux)
- Search for ESP32 devices using system-specific methods
- Select the best available port
- Handle permissions automatically
- Provide detailed error messages if something goes wrong

### 2. Manual Port Detection

If you want to check what ports are available before flashing:

```bash
# From project root
./examples/esp32/scripts/detect_ports.sh
```

**Options:**
```bash
./examples/esp32/scripts/detect_ports.sh --verbose          # Show detailed device info
./examples/esp32/scripts/detect_ports.sh --test-connection  # Test port accessibility
./examples/esp32/scripts/detect_ports.sh --help             # Show help
```

### 3. Manual Port Specification

You can also manually specify a port:

```bash
export ESPPORT="/dev/your_port_here"
./examples/esp32/scripts/flash_app.sh <example_name> <build_type> <operation>
```

## Platform-Specific Details

### macOS

**Device Detection:**
- ESP32 devices typically appear as `/dev/cu.usbmodem*` or `/dev/tty.usbmodem*`
- The script prefers `/dev/cu.*` (callout devices) over `/dev/tty.*` (terminal devices)
- Common patterns: `usbmodem`, `usbserial`, `SLAB_USBtoUART`, `CP210`, `CH340`

**Troubleshooting:**
- Check System Information > USB for connected devices
- Look for devices named 'USB Serial' or similar
- Ensure you have necessary USB-to-UART drivers:
  - Silicon Labs CP210x: Usually built into macOS
  - CH340: May need driver installation
- Check System Preferences > Security & Privacy > Privacy > Full Disk Access

### Linux

**Device Detection:**
- ESP32-C6 typically uses `/dev/ttyACM*` (CDC ACM)
- Older ESP32 uses `/dev/ttyUSB*` (USB-to-UART bridge)
- The script uses `lsusb` if available to identify ESP32 devices

**Troubleshooting:**
- Check if device appears: `ls /dev/ttyACM* /dev/ttyUSB*`
- Check USB enumeration: `lsusb` (if available)
- Check kernel messages: `dmesg | tail` (after connecting device)
- Fix permissions: `sudo chmod 666 /dev/ttyACM*`
- Add user to dialout group: `sudo usermod -a -G dialout $USER`

## Common Issues and Solutions

### Issue: "No suitable serial ports found"

**Solutions:**
1. Ensure your ESP32 device is connected via USB
2. Check if the device appears in your system (use `detect_ports.sh`)
3. Try disconnecting and reconnecting the device
4. Check if you need to install USB-to-UART drivers
5. Ensure the device is not being used by another application

### Issue: "Port is not readable"

**Solutions:**
- **Linux:** Run `sudo chmod 666 /dev/port_name` or add user to dialout group
- **macOS:** Check System Preferences > Security & Privacy > Privacy > Full Disk Access

### Issue: "Port configuration failed"

**Solutions:**
1. Check if another application is using the port
2. Close any serial monitors or terminal applications
3. Disconnect and reconnect the device
4. Try a different USB port or cable

### Issue: Device not appearing at all

**Solutions:**
1. Try a different USB cable
2. Try a different USB port
3. Ensure the ESP32 is in download mode (hold BOOT button while connecting)
4. Check if you need to install specific drivers for your USB-to-UART chip
5. On macOS, check System Information > USB

## Testing Your Setup

### 1. Test Port Detection

```bash
./examples/esp32/scripts/test_port_detection.sh
```

This script tests the port detection functions without actually flashing.

### 2. Test Port Accessibility

```bash
./examples/esp32/scripts/detect_ports.sh --test-connection
```

This tests if the detected ports are actually accessible.

### 3. Test Full Flash Process

```bash
./examples/esp32/scripts/flash_app.sh <example_name> Release flash
```

This will test the complete process including port detection and flashing.

## Advanced Usage

### Custom Port Detection

If you have a custom setup, you can modify the port detection functions in `flash_app.sh`:

- `detect_os()`: Operating system detection
- `find_esp32_devices()`: ESP32 device detection
- `find_best_port()`: Port selection logic
- `validate_port()`: Port validation
- `fix_port_permissions()`: Permission handling

### Environment Variables

- `ESPPORT`: Manually specify a port
- `IDF_TARGET`: ESP32 target (e.g., esp32c6)
- `IDF_CCACHE_ENABLE`: Enable ccache for faster builds

## Examples

### Basic Flash and Monitor

```bash
# Flash and monitor GPIO example
./examples/esp32/scripts/flash_app.sh gpio Release flash_monitor
```

### Flash Only

```bash
# Flash only (no monitor)
./examples/esp32/scripts/flash_app.sh gpio Release flash
```

### Monitor Only

```bash
# Monitor only (assumes already flashed)
./examples/esp32/scripts/flash_app.sh gpio Release monitor
```

### With Manual Port

```bash
# Use specific port
export ESPPORT="/dev/cu.usbmodem1201"
./examples/esp32/scripts/flash_app.sh gpio Release flash_monitor
```

## Support

If you continue to have issues:

1. Run `./examples/esp32/scripts/detect_ports.sh --verbose --test-connection`
2. Check the output for specific error messages
3. Follow the troubleshooting steps provided
4. Ensure your ESP32 device is properly connected and in download mode
5. Try different USB cables and ports

The improved port detection system should handle most common scenarios automatically, but these tools are available for troubleshooting when needed.

---

**Navigation**: [← Previous: Utility Scripts](README_UTILITY_SCRIPTS.md) | [Back to Scripts](../README.md) | [Next: Scripts Overview →](README_SCRIPTS_OVERVIEW.md)
