/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "HF Internal Interface Wrapper", "index.html", [
    [ "ESP32 HardFOC Interface Wrapper — Comprehensive Test Suite Documentation", "index.html", "index" ],
    [ "ESP32-C6 ADC Comprehensive Testing Suite", "md_examples_2esp32_2docs_2README__ADC__TESTING.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md1", null ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md2", [
        [ "Target Hardware", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md3", null ],
        [ "Required Components", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md4", null ],
        [ "Test Hardware Setup", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md5", [
          [ "Channel Configuration", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md6", null ],
          [ "Detailed Circuit Configuration", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md7", null ],
          [ "Alternative Simple Setup", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md8", null ]
        ] ]
      ] ],
      [ "Test Suite Description", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md9", [
        [ "1. Hardware Validation Test ✨ <strong>NEW</strong>", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md10", null ],
        [ "2. ADC Initialization Test", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md11", null ],
        [ "3. Channel Configuration Test", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md12", null ],
        [ "4. Basic Conversion Test", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md13", null ],
        [ "5. Calibration Test", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md14", null ],
        [ "6. Multiple Channels Test ✨ <strong>ENHANCED</strong>", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md15", null ],
        [ "7. Averaging Test", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md16", null ],
        [ "8. Continuous Mode Test", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md17", null ],
        [ "9. Monitor Threshold Test ✨ <strong>COMPREHENSIVE INTERACTIVE TESTING</strong>", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md18", null ],
        [ "10. Error Handling Test", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md19", null ],
        [ "11. Statistics and Diagnostics Test", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md20", null ],
        [ "12. Performance Test", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md21", null ]
      ] ],
      [ "Building and Running Tests", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md22", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md23", null ],
        [ "Build Commands", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md24", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md25", null ]
      ] ],
      [ "Expected Test Output", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md26", [
        [ "Successful Test Run", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md27", null ],
        [ "Hardware Setup Issues", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md28", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md29", [
        [ "Common Hardware Issues", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md30", [
          [ "GPIO3 Reading Too Low (<2800mV)", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md31", null ],
          [ "GPIO1 Reading Too High (>300mV)", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md32", null ],
          [ "GPIO0 Potentiometer Issues", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md33", null ],
          [ "Monitor Test No Events", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md34", null ]
        ] ],
        [ "Test Failure Analysis", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md35", [
          [ "Performance Issues", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md36", null ],
          [ "Monitor Test Troubleshooting", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md37", null ]
        ] ]
      ] ],
      [ "Test Configuration Details", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md38", [
        [ "ADC Configuration Used", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md39", null ],
        [ "Expected Performance Characteristics", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md40", null ]
      ] ],
      [ "Architecture Notes", "md_examples_2esp32_2docs_2README__ADC__TESTING.html#autotoc_md41", null ]
    ] ],
    [ "ESP32-C6 ASCII Art Generator Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md45", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md46", [
        [ "Core ASCII Art Generation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md47", null ],
        [ "Advanced Features", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md48", null ],
        [ "Output Quality", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md49", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md50", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md51", null ],
        [ "Connections", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md52", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md53", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md54", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md55", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md56", [
          [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md57", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md58", null ]
        ] ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md59", [
        [ "1. Basic ASCII Art Generation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md60", null ],
        [ "2. Uppercase Conversion", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md61", null ],
        [ "3. Special Characters", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md62", null ],
        [ "4. Numbers and Symbols", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md63", null ],
        [ "5. Empty and Edge Cases", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md64", null ],
        [ "6. Custom Character Management", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md65", null ],
        [ "7. Character Support Validation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md66", null ],
        [ "8. Supported Characters List", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md67", null ],
        [ "9. Complex Text Generation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md68", null ],
        [ "10. Performance and Stability", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md69", null ]
      ] ],
      [ "ASCII Art Character Set", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md70", [
        [ "Supported Characters", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md71", [
          [ "Letters (A-Z)", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md72", null ],
          [ "Numbers (0-9)", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md73", null ],
          [ "Special Characters", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md74", null ]
        ] ],
        [ "Character Height and Width", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md75", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md76", [
        [ "Successful Execution Output", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md77", null ],
        [ "Performance Metrics", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md78", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md79", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md80", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md81", [
          [ "Build Failures", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md82", null ],
          [ "Runtime Issues", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md83", null ],
          [ "Serial Monitor Issues", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md84", null ]
        ] ],
        [ "Debug Mode Configuration", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md85", null ]
      ] ],
      [ "Integration Examples", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md86", [
        [ "Basic ASCII Art Generation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md87", null ],
        [ "Advanced Usage with Custom Characters", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md88", null ],
        [ "Performance-Optimized Usage", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md89", null ]
      ] ],
      [ "API Reference", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md90", [
        [ "Core Functions", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md91", null ],
        [ "Advanced Functions", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md92", null ]
      ] ],
      [ "Character Pattern Format", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md93", [
        [ "Standard Pattern Structure", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md94", null ],
        [ "Design Guidelines", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md95", null ],
        [ "Custom Pattern Requirements", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md96", null ]
      ] ],
      [ "Embedded Development Best Practices", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md97", [
        [ "Memory Optimization", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md98", null ],
        [ "Performance Considerations", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md99", null ],
        [ "Real-time Constraints", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md100", null ]
      ] ],
      [ "Applications and Use Cases", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md101", [
        [ "System Status Display", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md102", null ],
        [ "User Interface Elements", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md103", null ],
        [ "Debug and Development", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md104", null ]
      ] ],
      [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md105", [
        [ "Automated Testing", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md106", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md107", null ]
    ] ],
    [ "ESP32 HardFOC Interface Wrapper - Build Configuration Guide", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md109", null ],
      [ "Quick Start", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md110", [
        [ "Using the Build Scripts", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md111", [
          [ "Linux/macOS (Bash)", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md112", null ]
        ] ],
        [ "Using the Flash and Monitor Scripts", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md113", [
          [ "Linux/macOS (Bash)", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md114", null ],
          [ "Flash Script Features", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md115", null ]
        ] ]
      ] ],
      [ "Detailed Examples", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md116", [
        [ "1. ASCII Art Generator (<tt>ascii_art</tt>)", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md117", null ],
        [ "2. Peripheral Test Suites", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md118", null ],
        [ "3. Bluetooth Test Suite (<tt>bluetooth_test</tt>)", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md119", null ],
        [ "4. Utilities Test Suite (<tt>utils_test</tt>)", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md120", null ]
      ] ],
      [ "Build System Architecture", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md121", [
        [ "Flexible CMakeLists.txt", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md122", null ],
        [ "Command Line Usage", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md123", null ]
      ] ],
      [ "Build Types", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md124", null ],
      [ "Target Configuration", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md125", null ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md126", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md127", null ],
        [ "Getting Help", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md128", null ]
      ] ],
      [ "Advanced Usage", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md129", [
        [ "Custom Builds", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md130", null ],
        [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__BUILD__SYSTEM.html#autotoc_md131", null ]
      ] ]
    ] ],
    [ "ESP32-C6 GPIO Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__GPIO__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md133", [
        [ "Core Features", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md134", null ],
        [ "Advanced Features (ESP32-C6 Specific)", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md135", null ],
        [ "Performance & Robustness", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md136", null ]
      ] ],
      [ "Hardware Setup", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md137", [
        [ "ESP32-C6 DevKit-M-1 Pin Layout", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md138", null ],
        [ "Optional Physical Connections", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md139", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md140", [
        [ "Build Commands", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md141", null ],
        [ "CI Pipeline", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md142", null ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md143", [
        [ "1. Basic Functionality Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md144", null ],
        [ "2. Configuration Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md145", null ],
        [ "3. Advanced Feature Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md146", null ],
        [ "4. ESP32-C6 Specific Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md147", null ],
        [ "5. Robustness Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md148", null ],
        [ "6. Performance Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md149", null ]
      ] ],
      [ "Expected Output", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md150", null ],
      [ "Development Notes", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md151", [
        [ "Adding New Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md152", null ],
        [ "Test Patterns", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md153", null ],
        [ "Debugging", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md154", null ]
      ] ],
      [ "Integration with Main Project", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md155", null ],
      [ "Contributing", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md156", null ],
      [ "License", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md157", null ]
    ] ],
    [ "ESP32-C6 Logger Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md159", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md160", [
        [ "Core Logging Functionality", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md161", null ],
        [ "Advanced Features", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md162", null ],
        [ "Monitoring & Diagnostics", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md163", null ],
        [ "Configuration & Management", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md164", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md165", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md166", null ],
        [ "Connections", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md167", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md168", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md169", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md170", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md171", [
          [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md172", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md173", null ]
        ] ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md174", [
        [ "1. Construction and Initialization Tests", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md175", null ],
        [ "2. Basic Logging Operations", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md176", null ],
        [ "3. Level Management", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md177", null ],
        [ "4. Formatted Logging", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md178", null ],
        [ "5. ESP-IDF Log V2 Features", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md179", null ],
        [ "6. Buffer Logging", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md180", null ],
        [ "7. Location Logging", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md181", null ],
        [ "8. Statistics and Diagnostics", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md182", null ],
        [ "9. Error Handling", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md183", null ],
        [ "10. Performance Testing", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md184", null ],
        [ "11. Utility Functions", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md185", null ],
        [ "12. Cleanup Operations", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md186", null ]
      ] ],
      [ "Configuration Options", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md187", [
        [ "Logger Configuration Structure", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md188", null ],
        [ "Key Configuration Parameters", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md189", null ],
        [ "Log Levels", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md190", null ],
        [ "Output Destinations", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md191", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md192", [
        [ "Successful Execution Output", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md193", null ],
        [ "Performance Metrics", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md194", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md195", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md196", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md197", [
          [ "Build Failures", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md198", null ],
          [ "Runtime Issues", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md199", null ],
          [ "Serial Monitor Issues", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md200", null ]
        ] ],
        [ "Debug Mode Configuration", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md201", null ]
      ] ],
      [ "Integration Examples", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md202", [
        [ "Basic Logger Usage", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md203", null ],
        [ "Advanced Features", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md204", null ]
      ] ],
      [ "API Reference", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md205", [
        [ "Core Functions", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md206", null ],
        [ "Advanced Functions", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md207", null ]
      ] ],
      [ "Embedded Development Best Practices", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md208", [
        [ "Performance Optimization", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md209", null ],
        [ "Memory Management", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md210", null ],
        [ "Real-time Considerations", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md211", null ]
      ] ],
      [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md212", [
        [ "Automated Testing", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md213", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md214", null ]
    ] ],
    [ "ESP32-C6 NVS Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__NVS__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md216", null ],
      [ "Test Coverage", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md217", null ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md218", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md219", null ],
        [ "Connections", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md220", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md221", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md222", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md223", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md224", [
          [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md225", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md226", null ]
        ] ]
      ] ],
      [ "Running the Test", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md227", [
        [ "Expected Output", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md228", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md229", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md230", null ]
      ] ],
      [ "Configuration", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md231", null ],
      [ "Customization", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md232", null ],
      [ "Notes", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md233", null ]
    ] ],
    [ "ESP32-C6 PIO Comprehensive Test Suite Documentation", "md_examples_2esp32_2docs_2README__PIO__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md235", [
        [ "Supported ESP32 Variants", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md236", null ]
      ] ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md237", [
        [ "Core Functionality", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md238", null ],
        [ "ESP32 Variant-Specific Features", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md239", null ],
        [ "Advanced RMT Features", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md240", null ],
        [ "WS2812 LED Protocol Testing", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md241", null ],
        [ "Automated Testing & Diagnostics", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md242", null ],
        [ "Performance & Stress Testing", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md243", null ]
      ] ],
      [ "Hardware Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md244", [
        [ "ESP32-C6-DevKitM-1 Pin Configuration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md245", null ],
        [ "Automated Testing Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md246", null ],
        [ "WS2812 LED Testing Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md247", null ],
        [ "Test Progression Indicator", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md248", null ],
        [ "External WS2812 LED Chain (Optional)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md249", null ],
        [ "Logic Analyzer Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md250", null ]
      ] ],
      [ "Running the Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md251", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md252", null ],
        [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md253", null ],
        [ "Direct ESP-IDF Build (Alternative)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md254", null ],
        [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md255", null ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md256", [
        [ "1. ESP32 Variant Information Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md257", null ],
        [ "2. Constructor/Destructor Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md258", null ],
        [ "3. Lifecycle Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md259", null ],
        [ "4. Channel Configuration Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md260", null ],
        [ "5. Transmission Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md261", null ],
        [ "6. WS2812 LED Protocol Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md262", null ],
        [ "7. Logic Analyzer Test Scenarios", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md263", null ],
        [ "8. Advanced RMT Feature Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md264", null ],
        [ "9. Loopback and Reception Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md265", null ],
        [ "10. Callback Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md266", null ],
        [ "11. Statistics and Diagnostics Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md267", null ],
        [ "12. Stress and Performance Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md268", null ],
        [ "13. System Validation Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md269", null ]
      ] ],
      [ "WS2812 Protocol Specifications", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md270", [
        [ "Timing Requirements", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md271", null ],
        [ "Color Format", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md272", null ],
        [ "Comprehensive Test Colors", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md273", [
          [ "Primary Colors (Maximum Brightness)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md274", null ],
          [ "Secondary Colors", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md275", null ],
          [ "White Variations", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md276", null ],
          [ "Bit Pattern Test Values", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md277", null ],
          [ "Rainbow Transition", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md278", null ]
        ] ]
      ] ],
      [ "Logic Analyzer Test Patterns", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md279", [
        [ "Pattern 1: Basic Timing Test", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md280", null ],
        [ "Pattern 2: Frequency Sweep", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md281", null ],
        [ "Pattern 3: Test Progression Monitoring", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md282", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md283", [
        [ "Successful Test Output", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md284", null ],
        [ "Built-in RGB LED Verification", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md285", null ],
        [ "Automated Loopback Verification", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md286", null ],
        [ "Test Progression Monitoring", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md287", null ],
        [ "Logic Analyzer Verification", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md288", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md289", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md290", [
          [ "Test Failures", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md291", null ],
          [ "Built-in RGB LED Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md292", null ],
          [ "Test Progression Indicator Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md293", null ],
          [ "Loopback Testing Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md294", null ],
          [ "WS2812 Testing Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md295", null ],
          [ "Logic Analyzer Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md296", null ]
        ] ],
        [ "Debug Mode", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md297", null ]
      ] ],
      [ "Performance Metrics", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md298", [
        [ "Build Information", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md299", null ],
        [ "Typical Results (ESP32-C6 @ 160MHz)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md300", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md301", null ]
      ] ],
      [ "Integration with Development Workflow", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md302", [
        [ "Continuous Integration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md303", null ],
        [ "Hardware-in-the-Loop Testing", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md304", null ]
      ] ],
      [ "Advanced Configuration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md305", [
        [ "Custom GPIO Pins", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md306", null ],
        [ "Timing Resolution", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md307", null ],
        [ "Test Parameters", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md308", null ]
      ] ],
      [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md309", [
        [ "RMT Peripheral", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md310", null ],
        [ "Built-in RGB LED", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md311", null ],
        [ "Testing Advantages", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md312", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md313", null ]
    ] ],
    [ "ESP32-C6 PWM Comprehensive Test Suite Documentation", "md_examples_2esp32_2docs_2README__PWM__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md315", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md316", [
        [ "Core PWM Functionality", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md317", null ],
        [ "Advanced PWM Features", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md318", null ],
        [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md319", null ],
        [ "System Integration & Diagnostics", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md320", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md321", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md322", null ],
        [ "PWM Output Pins", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md323", null ],
        [ "Logic Analyzer Setup", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md324", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md325", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md326", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md327", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md328", [
          [ "Using ESP-IDF directly", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md329", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md330", null ],
          [ "Available Example Script Options", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md331", null ]
        ] ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md332", [
        [ "1. Constructor/Destructor Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md333", [
          [ "<tt>test_constructor_default()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md334", null ],
          [ "<tt>test_destructor_cleanup()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md335", null ]
        ] ],
        [ "2. Lifecycle Management Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md336", [
          [ "<tt>test_initialization_states()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md337", null ],
          [ "<tt>test_lazy_initialization()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md338", null ]
        ] ],
        [ "3. Configuration Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md339", [
          [ "<tt>test_mode_configuration()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md340", null ],
          [ "<tt>test_clock_source_configuration()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md341", null ]
        ] ],
        [ "4. Channel Management Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md342", [
          [ "<tt>test_channel_configuration()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md343", null ],
          [ "<tt>test_channel_enable_disable()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md344", null ]
        ] ],
        [ "5. PWM Control Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md345", [
          [ "<tt>test_duty_cycle_control()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md346", null ],
          [ "<tt>test_frequency_control()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md347", null ],
          [ "<tt>test_phase_shift_control()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md348", null ]
        ] ],
        [ "6. Advanced Features Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md349", [
          [ "<tt>test_synchronized_operations()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md350", null ],
          [ "<tt>test_complementary_outputs()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md351", null ]
        ] ],
        [ "7. ESP32-Specific Features Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md352", [
          [ "<tt>test_hardware_fade()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md353", null ],
          [ "<tt>test_idle_level_control()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md354", null ],
          [ "<tt>test_timer_management()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md355", null ]
        ] ],
        [ "8. Status and Diagnostics Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md356", [
          [ "<tt>test_status_reporting()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md357", null ],
          [ "<tt>test_statistics_and_diagnostics()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md358", null ]
        ] ],
        [ "9. Callback Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md359", [
          [ "<tt>test_callbacks()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md360", null ]
        ] ],
        [ "10. Edge Cases and Stress Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md361", [
          [ "<tt>test_edge_cases()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md362", null ],
          [ "<tt>test_stress_scenarios()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md363", null ]
        ] ]
      ] ],
      [ "Logic Analyzer Analysis Guide", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md364", [
        [ "Key Measurements", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md365", null ],
        [ "Logic Analyzer Measurement Guidelines", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md366", [
          [ "Key Measurement Techniques", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md367", null ]
        ] ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md368", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md369", null ],
        [ "Performance Optimization", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md370", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md371", [
        [ "Success Criteria", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md372", null ],
        [ "Typical Test Sequence Timing", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md373", null ],
        [ "Hardware Validation", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md374", null ]
      ] ],
      [ "Conclusion", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md375", null ]
    ] ],
    [ "ESP32-C6 Temperature Sensor Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md377", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md378", [
        [ "Core Temperature Functionality", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md379", null ],
        [ "Advanced Monitoring Features", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md380", null ],
        [ "System Integration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md381", null ],
        [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md382", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md383", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md384", null ],
        [ "Temperature Sensor", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md385", null ],
        [ "Connections", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md386", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md387", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md388", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md389", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md390", [
          [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md391", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md392", null ]
        ] ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md393", [
        [ "1. Sensor Initialization Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md394", null ],
        [ "2. Basic Temperature Reading Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md395", null ],
        [ "3. Sensor Information Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md396", null ],
        [ "4. Range Management Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md397", null ],
        [ "5. Threshold Monitoring Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md398", null ],
        [ "6. Continuous Monitoring Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md399", null ],
        [ "7. Calibration Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md400", null ],
        [ "8. Power Management Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md401", null ],
        [ "9. Self-Test and Health Monitoring", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md402", null ],
        [ "10. Statistics and Diagnostics", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md403", null ],
        [ "11. ESP32-Specific Features", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md404", null ],
        [ "12. Error Handling Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md405", null ],
        [ "13. Performance and Stress Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md406", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md407", [
        [ "Successful Execution Output", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md408", null ],
        [ "Performance Metrics", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md409", null ],
        [ "Accuracy Specifications", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md410", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md411", null ]
      ] ],
      [ "Configuration Options", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md412", [
        [ "Temperature Sensor Configuration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md413", null ],
        [ "Monitoring Configuration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md414", null ],
        [ "Power Management", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md415", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md416", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md417", [
          [ "Build Failures", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md418", null ],
          [ "Runtime Issues", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md419", null ],
          [ "Calibration Issues", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md420", null ]
        ] ],
        [ "Debug Mode Configuration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md421", null ]
      ] ],
      [ "Integration Examples", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md422", [
        [ "Basic Temperature Monitoring", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md423", null ],
        [ "Advanced Monitoring with Callbacks", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md424", null ],
        [ "Calibration and Accuracy Improvement", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md425", null ]
      ] ],
      [ "API Reference", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md426", [
        [ "Core Functions", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md427", null ],
        [ "Advanced Functions", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md428", null ]
      ] ],
      [ "Embedded Development Best Practices", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md429", [
        [ "Performance Optimization", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md430", null ],
        [ "Memory Management", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md431", null ],
        [ "Real-time Considerations", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md432", null ]
      ] ],
      [ "Applications and Use Cases", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md433", [
        [ "Environmental Monitoring", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md434", null ],
        [ "Thermal Protection", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md435", null ],
        [ "Data Logging", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md436", null ]
      ] ],
      [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md437", [
        [ "Automated Testing", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md438", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md439", null ]
    ] ],
    [ "ESP32 Examples - Centralized Configuration System", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html", [
      [ "Overview", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md544", null ],
      [ "Configuration File Structure", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md545", [
        [ "<tt>examples_config.yml</tt>", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md546", null ]
      ] ],
      [ "Helper Scripts and Libraries", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md547", [
        [ "Bash Configuration Loader (<tt>scripts/config_loader.sh</tt>)", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md548", null ],
        [ "Python Configuration Scripts", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md549", [
          [ "<tt>scripts/get_example_info.py</tt>", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md550", null ],
          [ "<tt>.github/workflows/generate_matrix.py</tt>", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md551", null ]
        ] ]
      ] ],
      [ "Usage Examples", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md552", [
        [ "Adding a New Example", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md553", null ],
        [ "Building Examples", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md554", null ],
        [ "Disabling CI for an Example", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md555", null ],
        [ "Making an Example Featured", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md556", null ]
      ] ],
      [ "Integration Points", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md557", [
        [ "Build Scripts", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md558", null ],
        [ "Flash Scripts", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md559", null ],
        [ "CI Workflows", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md560", null ],
        [ "CMake", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md561", null ]
      ] ],
      [ "Available Examples", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md562", [
        [ "Featured Examples (displayed first in listings)", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md563", null ],
        [ "Additional Examples", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md564", null ]
      ] ],
      [ "Benefits", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md565", null ],
      [ "Troubleshooting", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md566", [
        [ "Missing <tt>yq</tt> Tool", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md567", null ],
        [ "Python Dependencies", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md568", null ]
      ] ],
      [ "Migration Guide", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md569", null ],
      [ "File Structure", "md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md570", null ]
    ] ],
    [ "Topics", "topics.html", "topics" ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", "globals_func" ],
        [ "Variables", "globals_vars.html", "globals_vars" ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ],
    [ "Examples", "examples.html", "examples" ]
  ] ]
];

var NAVTREEINDEX =
[
"AdcComprehensiveTest_8cpp.html",
"BaseNvs_8h.html#a0a4f43b41760df506a65240a5ca415a4ac4acea1b622467a0e604b7a47f720ef1",
"BaseTemperature_8h.html#aee8dd042d1f1740b2a23650bbe6efe12a06110442d989e9e0d9e1b4c37dd9c82e",
"EspTypes__I2C_8h.html#a02c3031bc44ca85dff4ad7f483bc4ddea46e00b456caa639f7fabd2edb44a72e7",
"EspTypes__WiFi_8h.html#aaa9828097e7aa3aaa2539792201d75e5ac2940ff56796bd1895634e4eaeb8c90d",
"NvsComprehensiveTest_8cpp.html#a52c7240eb996dc640b022a0f8e6bb0eb",
"UtilsComprehensiveTest_8cpp.html#a4b9a60fc625088181bf855b25a0c082b",
"classBaseI2c.html#aa0015d0aff3de7f30f7444935a50c094",
"classBaseTemperature.html#ac3e2177be073e991acc4f03da8c2b07d",
"classEspI2cBus.html#a4eb323a3531218e2623d8126d31d255d",
"classEspSpiBus.html#a61699b15fb8f109b10084e824ec6f3df",
"classRtosSharedMutex.html#ae1d3068d9207b5ef477e1ec1ae93c325",
"group__gpio.html#ga7d27555a7050f5d9d9006c96b841e335",
"md_examples_2esp32_2README__CENTRALIZED__CONFIG.html#autotoc_md555",
"md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md236",
"structCallbackTestData.html#aa09dd0d4c9d9749c308bd5e31220f87d",
"structhf__bluetooth__gatt__characteristic__t.html#a15cc0566fe8b0cffa81d200120cc27cf",
"structhf__logger__diagnostics__t.html#a05c5d1234a118c2ecfcf0a97d28bbefa",
"structhf__spi__device__config__t.html#a14b1dd2e7f8dfd85c5c95207bdd0f5bd"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';