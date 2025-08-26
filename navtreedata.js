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
    [ "ESP32-C6 ADC Comprehensive Testing Suite", "md_examples_2esp32_2docs_2README__ADC__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md19", null ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md20", [
        [ "Target Hardware", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md21", null ],
        [ "Required Components", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md22", null ],
        [ "Test Hardware Setup", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md23", [
          [ "Channel Configuration", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md24", null ],
          [ "Detailed Circuit Configuration", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md25", null ],
          [ "Alternative Simple Setup", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md26", null ]
        ] ]
      ] ],
      [ "Test Suite Description", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md27", [
        [ "1. Hardware Validation Test ✨ <strong>NEW</strong>", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md28", null ],
        [ "2. ADC Initialization Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md29", null ],
        [ "3. Channel Configuration Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md30", null ],
        [ "4. Basic Conversion Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md31", null ],
        [ "5. Calibration Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md32", null ],
        [ "6. Multiple Channels Test ✨ <strong>ENHANCED</strong>", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md33", null ],
        [ "7. Averaging Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md34", null ],
        [ "8. Continuous Mode Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md35", null ],
        [ "9. Monitor Threshold Test ✨ <strong>COMPREHENSIVE INTERACTIVE TESTING</strong>", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md36", null ],
        [ "10. Error Handling Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md37", null ],
        [ "11. Statistics and Diagnostics Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md38", null ],
        [ "12. Performance Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md39", null ]
      ] ],
      [ "Building and Running Tests", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md40", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md41", null ],
        [ "Build Commands", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md42", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md43", null ]
      ] ],
      [ "Expected Test Output", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md44", [
        [ "Successful Test Run", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md45", null ],
        [ "Hardware Setup Issues", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md46", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md47", [
        [ "Common Hardware Issues", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md48", [
          [ "GPIO3 Reading Too Low (<2800mV)", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md49", null ],
          [ "GPIO1 Reading Too High (>300mV)", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md50", null ],
          [ "GPIO0 Potentiometer Issues", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md51", null ],
          [ "Monitor Test No Events", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md52", null ]
        ] ],
        [ "Test Failure Analysis", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md53", [
          [ "Performance Issues", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md54", null ],
          [ "Monitor Test Troubleshooting", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md55", null ]
        ] ]
      ] ],
      [ "Test Configuration Details", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md56", [
        [ "ADC Configuration Used", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md57", null ],
        [ "Expected Performance Characteristics", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md58", null ]
      ] ],
      [ "Architecture Notes", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md59", null ]
    ] ],
    [ "ESP32-C6 ASCII Art Generator Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md63", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md64", [
        [ "Core ASCII Art Generation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md65", null ],
        [ "Advanced Features", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md66", null ],
        [ "Output Quality", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md67", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md68", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md69", null ],
        [ "Connections", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md70", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md71", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md72", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md73", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md74", [
          [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md75", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md76", null ]
        ] ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md77", [
        [ "1. Basic ASCII Art Generation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md78", null ],
        [ "2. Uppercase Conversion", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md79", null ],
        [ "3. Special Characters", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md80", null ],
        [ "4. Numbers and Symbols", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md81", null ],
        [ "5. Empty and Edge Cases", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md82", null ],
        [ "6. Custom Character Management", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md83", null ],
        [ "7. Character Support Validation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md84", null ],
        [ "8. Supported Characters List", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md85", null ],
        [ "9. Complex Text Generation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md86", null ],
        [ "10. Performance and Stability", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md87", null ]
      ] ],
      [ "ASCII Art Character Set", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md88", [
        [ "Supported Characters", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md89", [
          [ "Letters (A-Z)", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md90", null ],
          [ "Numbers (0-9)", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md91", null ],
          [ "Special Characters", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md92", null ]
        ] ],
        [ "Character Height and Width", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md93", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md94", [
        [ "Successful Execution Output", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md95", null ],
        [ "Performance Metrics", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md96", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md97", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md98", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md99", [
          [ "Build Failures", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md100", null ],
          [ "Runtime Issues", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md101", null ],
          [ "Serial Monitor Issues", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md102", null ]
        ] ],
        [ "Debug Mode Configuration", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md103", null ]
      ] ],
      [ "Integration Examples", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md104", [
        [ "Basic ASCII Art Generation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md105", null ],
        [ "Advanced Usage with Custom Characters", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md106", null ],
        [ "Performance-Optimized Usage", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md107", null ]
      ] ],
      [ "API Reference", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md108", [
        [ "Core Functions", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md109", null ],
        [ "Advanced Functions", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md110", null ]
      ] ],
      [ "Character Pattern Format", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md111", [
        [ "Standard Pattern Structure", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md112", null ],
        [ "Design Guidelines", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md113", null ],
        [ "Custom Pattern Requirements", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md114", null ]
      ] ],
      [ "Embedded Development Best Practices", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md115", [
        [ "Memory Optimization", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md116", null ],
        [ "Performance Considerations", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md117", null ],
        [ "Real-time Constraints", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md118", null ]
      ] ],
      [ "Applications and Use Cases", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md119", [
        [ "System Status Display", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md120", null ],
        [ "User Interface Elements", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md121", null ],
        [ "Debug and Development", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md122", null ]
      ] ],
      [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md123", [
        [ "Automated Testing", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md124", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md125", null ]
    ] ],
    [ "ESP32-C6 GPIO Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__GPIO__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md127", [
        [ "Core Features", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md128", null ],
        [ "Advanced Features (ESP32-C6 Specific)", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md129", null ],
        [ "Performance & Robustness", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md130", null ]
      ] ],
      [ "Hardware Setup", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md131", [
        [ "ESP32-C6 DevKit-M-1 Pin Layout", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md132", null ],
        [ "Optional Physical Connections", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md133", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md134", [
        [ "Build Commands", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md135", null ],
        [ "CI Pipeline", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md136", null ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md137", [
        [ "1. Basic Functionality Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md138", null ],
        [ "2. Configuration Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md139", null ],
        [ "3. Advanced Feature Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md140", null ],
        [ "4. ESP32-C6 Specific Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md141", null ],
        [ "5. Robustness Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md142", null ],
        [ "6. Performance Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md143", null ]
      ] ],
      [ "Expected Output", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md144", null ],
      [ "Development Notes", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md145", [
        [ "Adding New Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md146", null ],
        [ "Test Patterns", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md147", null ],
        [ "Debugging", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md148", null ]
      ] ],
      [ "Integration with Main Project", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md149", null ],
      [ "Contributing", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md150", null ],
      [ "License", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md151", null ]
    ] ],
    [ "ESP32 I2C Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__I2C__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md153", null ],
      [ "Quick Test Information", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md154", [
        [ "Test File Location", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md155", null ],
        [ "Running the Tests", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md156", null ],
        [ "Test Categories (24 Total)", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md157", [
          [ "Core Functionality (10 tests)", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md158", null ],
          [ "Advanced Features (8 tests)", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md159", null ],
          [ "New Features (6 tests)", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md160", null ]
        ] ],
        [ "Expected Output", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md161", null ],
        [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md162", null ],
        [ "Test Configuration", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md163", null ]
      ] ],
      [ "For Complete Documentation", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md164", null ]
    ] ],
    [ "ESP32-C6 Logger Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md166", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md167", [
        [ "Core Logging Functionality", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md168", null ],
        [ "Advanced Features", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md169", null ],
        [ "Monitoring & Diagnostics", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md170", null ],
        [ "Configuration & Management", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md171", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md172", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md173", null ],
        [ "Connections", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md174", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md175", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md176", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md177", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md178", [
          [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md179", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md180", null ]
        ] ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md181", [
        [ "1. Construction and Initialization Tests", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md182", null ],
        [ "2. Basic Logging Operations", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md183", null ],
        [ "3. Level Management", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md184", null ],
        [ "4. Formatted Logging", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md185", null ],
        [ "5. ESP-IDF Log V2 Features", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md186", null ],
        [ "6. Buffer Logging", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md187", null ],
        [ "7. Location Logging", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md188", null ],
        [ "8. Statistics and Diagnostics", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md189", null ],
        [ "9. Error Handling", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md190", null ],
        [ "10. Performance Testing", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md191", null ],
        [ "11. Utility Functions", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md192", null ],
        [ "12. Cleanup Operations", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md193", null ]
      ] ],
      [ "Configuration Options", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md194", [
        [ "Logger Configuration Structure", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md195", null ],
        [ "Key Configuration Parameters", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md196", null ],
        [ "Log Levels", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md197", null ],
        [ "Output Destinations", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md198", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md199", [
        [ "Successful Execution Output", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md200", null ],
        [ "Performance Metrics", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md201", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md202", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md203", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md204", [
          [ "Build Failures", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md205", null ],
          [ "Runtime Issues", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md206", null ],
          [ "Serial Monitor Issues", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md207", null ]
        ] ],
        [ "Debug Mode Configuration", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md208", null ]
      ] ],
      [ "Integration Examples", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md209", [
        [ "Basic Logger Usage", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md210", null ],
        [ "Advanced Features", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md211", null ]
      ] ],
      [ "API Reference", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md212", [
        [ "Core Functions", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md213", null ],
        [ "Advanced Functions", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md214", null ]
      ] ],
      [ "Embedded Development Best Practices", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md215", [
        [ "Performance Optimization", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md216", null ],
        [ "Memory Management", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md217", null ],
        [ "Real-time Considerations", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md218", null ]
      ] ],
      [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md219", [
        [ "Automated Testing", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md220", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md221", null ]
    ] ],
    [ "ESP32-C6 NVS Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__NVS__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md223", null ],
      [ "Test Coverage", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md224", null ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md225", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md226", null ],
        [ "Connections", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md227", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md228", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md229", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md230", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md231", [
          [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md232", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md233", null ]
        ] ]
      ] ],
      [ "Running the Test", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md234", [
        [ "Expected Output", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md235", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md236", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md237", null ]
      ] ],
      [ "Configuration", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md238", null ],
      [ "Customization", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md239", null ],
      [ "Notes", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md240", null ]
    ] ],
    [ "ESP32-C6 PIO Comprehensive Test Suite Documentation", "md_examples_2esp32_2docs_2README__PIO__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md242", [
        [ "Supported ESP32 Variants", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md243", null ]
      ] ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md244", [
        [ "Core Functionality", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md245", null ],
        [ "ESP32 Variant-Specific Features", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md246", null ],
        [ "Advanced RMT Features", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md247", null ],
        [ "WS2812 LED Protocol Testing", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md248", null ],
        [ "Automated Testing & Diagnostics", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md249", null ],
        [ "Performance & Stress Testing", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md250", null ]
      ] ],
      [ "Hardware Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md251", [
        [ "ESP32-C6-DevKitM-1 Pin Configuration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md252", null ],
        [ "Automated Testing Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md253", null ],
        [ "WS2812 LED Testing Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md254", null ],
        [ "Test Progression Indicator", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md255", null ],
        [ "External WS2812 LED Chain (Optional)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md256", null ],
        [ "Logic Analyzer Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md257", null ]
      ] ],
      [ "Running the Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md258", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md259", null ],
        [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md260", null ],
        [ "Direct ESP-IDF Build (Alternative)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md261", null ],
        [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md262", null ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md263", [
        [ "1. ESP32 Variant Information Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md264", null ],
        [ "2. Constructor/Destructor Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md265", null ],
        [ "3. Lifecycle Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md266", null ],
        [ "4. Channel Configuration Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md267", null ],
        [ "5. Transmission Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md268", null ],
        [ "6. WS2812 LED Protocol Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md269", null ],
        [ "7. Logic Analyzer Test Scenarios", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md270", null ],
        [ "8. Advanced RMT Feature Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md271", null ],
        [ "9. Loopback and Reception Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md272", null ],
        [ "10. Callback Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md273", null ],
        [ "11. Statistics and Diagnostics Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md274", null ],
        [ "12. Stress and Performance Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md275", null ],
        [ "13. System Validation Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md276", null ]
      ] ],
      [ "WS2812 Protocol Specifications", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md277", [
        [ "Timing Requirements", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md278", null ],
        [ "Color Format", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md279", null ],
        [ "Comprehensive Test Colors", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md280", [
          [ "Primary Colors (Maximum Brightness)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md281", null ],
          [ "Secondary Colors", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md282", null ],
          [ "White Variations", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md283", null ],
          [ "Bit Pattern Test Values", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md284", null ],
          [ "Rainbow Transition", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md285", null ]
        ] ]
      ] ],
      [ "Logic Analyzer Test Patterns", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md286", [
        [ "Pattern 1: Basic Timing Test", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md287", null ],
        [ "Pattern 2: Frequency Sweep", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md288", null ],
        [ "Pattern 3: Test Progression Monitoring", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md289", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md290", [
        [ "Successful Test Output", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md291", null ],
        [ "Built-in RGB LED Verification", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md292", null ],
        [ "Automated Loopback Verification", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md293", null ],
        [ "Test Progression Monitoring", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md294", null ],
        [ "Logic Analyzer Verification", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md295", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md296", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md297", [
          [ "Test Failures", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md298", null ],
          [ "Built-in RGB LED Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md299", null ],
          [ "Test Progression Indicator Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md300", null ],
          [ "Loopback Testing Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md301", null ],
          [ "WS2812 Testing Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md302", null ],
          [ "Logic Analyzer Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md303", null ]
        ] ],
        [ "Debug Mode", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md304", null ]
      ] ],
      [ "Performance Metrics", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md305", [
        [ "Build Information", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md306", null ],
        [ "Typical Results (ESP32-C6 @ 160MHz)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md307", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md308", null ]
      ] ],
      [ "Integration with Development Workflow", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md309", [
        [ "Continuous Integration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md310", null ],
        [ "Hardware-in-the-Loop Testing", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md311", null ]
      ] ],
      [ "Advanced Configuration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md312", [
        [ "Custom GPIO Pins", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md313", null ],
        [ "Timing Resolution", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md314", null ],
        [ "Test Parameters", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md315", null ]
      ] ],
      [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md316", [
        [ "RMT Peripheral", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md317", null ],
        [ "Built-in RGB LED", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md318", null ],
        [ "Testing Advantages", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md319", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md320", null ]
    ] ],
    [ "ESP32 Family PWM Comprehensive Test Suite Documentation", "md_examples_2esp32_2docs_2README__PWM__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md322", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md323", [
        [ "Core PWM Functionality", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md324", null ],
        [ "Advanced PWM Features", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md325", null ],
        [ "LEDC Peripheral Validation (ESP32 Family)", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md326", null ],
        [ "ESP32 Variant-Specific Features", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md327", null ],
        [ "System Integration & Diagnostics", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md328", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md329", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md330", null ],
        [ "PWM Output Pins", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md331", null ],
        [ "Logic Analyzer Setup", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md332", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md333", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md334", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md335", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md336", [
          [ "Using ESP-IDF directly", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md337", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md338", null ],
          [ "Available Example Script Options", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md339", null ]
        ] ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md340", [
        [ "1. Constructor/Destructor Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md341", [
          [ "<tt>test_constructor_default()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md342", null ],
          [ "<tt>test_destructor_cleanup()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md343", null ]
        ] ],
        [ "2. Lifecycle Management Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md344", [
          [ "<tt>test_initialization_states()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md345", null ],
          [ "<tt>test_lazy_initialization()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md346", null ]
        ] ],
        [ "3. Configuration Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md347", [
          [ "<tt>test_mode_configuration()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md348", null ],
          [ "<tt>test_clock_source_configuration()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md349", null ]
        ] ],
        [ "4. Channel Management Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md350", [
          [ "<tt>test_channel_configuration()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md351", null ],
          [ "<tt>test_channel_enable_disable()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md352", null ]
        ] ],
        [ "5. PWM Control Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md353", [
          [ "<tt>test_duty_cycle_control()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md354", null ],
          [ "<tt>test_frequency_control()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md355", null ],
          [ "<tt>test_phase_shift_control()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md356", null ]
        ] ],
        [ "6. Advanced Features Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md357", [
          [ "<tt>test_synchronized_operations()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md358", null ],
          [ "<tt>test_complementary_outputs()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md359", null ]
        ] ],
        [ "7. ESP32-Specific Features Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md360", [
          [ "<tt>test_hardware_fade()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md361", null ],
          [ "<tt>test_idle_level_control()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md362", null ],
          [ "<tt>test_timer_management()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md363", null ]
        ] ],
        [ "8. Status and Diagnostics Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md364", [
          [ "<tt>test_status_reporting()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md365", null ],
          [ "<tt>test_statistics_and_diagnostics()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md366", null ]
        ] ],
        [ "9. Callback Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md367", [
          [ "<tt>test_callbacks()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md368", null ]
        ] ],
        [ "10. Edge Cases and Stress Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md369", [
          [ "<tt>test_edge_cases()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md370", null ],
          [ "<tt>test_stress_scenarios()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md371", null ]
        ] ]
      ] ],
      [ "Logic Analyzer Analysis Guide", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md372", [
        [ "Key Measurements", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md373", null ],
        [ "Logic Analyzer Measurement Guidelines", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md374", [
          [ "Key Measurement Techniques", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md375", null ]
        ] ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md376", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md377", null ],
        [ "Performance Optimization", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md378", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md379", [
        [ "Success Criteria", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md380", null ],
        [ "Typical Test Sequence Timing", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md381", null ],
        [ "Hardware Validation", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md382", null ]
      ] ],
      [ "Conclusion", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md383", null ]
    ] ],
    [ "ESP32-C6 Temperature Sensor Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md385", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md386", [
        [ "Core Temperature Functionality", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md387", null ],
        [ "Advanced Monitoring Features", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md388", null ],
        [ "System Integration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md389", null ],
        [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md390", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md391", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md392", null ],
        [ "Temperature Sensor", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md393", null ],
        [ "Connections", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md394", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md395", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md396", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md397", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md398", [
          [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md399", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md400", null ]
        ] ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md401", [
        [ "1. Sensor Initialization Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md402", null ],
        [ "2. Basic Temperature Reading Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md403", null ],
        [ "3. Sensor Information Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md404", null ],
        [ "4. Range Management Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md405", null ],
        [ "5. Threshold Monitoring Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md406", null ],
        [ "6. Continuous Monitoring Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md407", null ],
        [ "7. Calibration Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md408", null ],
        [ "8. Power Management Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md409", null ],
        [ "9. Self-Test and Health Monitoring", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md410", null ],
        [ "10. Statistics and Diagnostics", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md411", null ],
        [ "11. ESP32-Specific Features", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md412", null ],
        [ "12. Error Handling Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md413", null ],
        [ "13. Performance and Stress Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md414", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md415", [
        [ "Successful Execution Output", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md416", null ],
        [ "Performance Metrics", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md417", null ],
        [ "Accuracy Specifications", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md418", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md419", null ]
      ] ],
      [ "Configuration Options", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md420", [
        [ "Temperature Sensor Configuration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md421", null ],
        [ "Monitoring Configuration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md422", null ],
        [ "Power Management", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md423", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md424", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md425", [
          [ "Build Failures", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md426", null ],
          [ "Runtime Issues", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md427", null ],
          [ "Calibration Issues", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md428", null ]
        ] ],
        [ "Debug Mode Configuration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md429", null ]
      ] ],
      [ "Integration Examples", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md430", [
        [ "Basic Temperature Monitoring", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md431", null ],
        [ "Advanced Monitoring with Callbacks", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md432", null ],
        [ "Calibration and Accuracy Improvement", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md433", null ]
      ] ],
      [ "API Reference", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md434", [
        [ "Core Functions", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md435", null ],
        [ "Advanced Functions", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md436", null ]
      ] ],
      [ "Embedded Development Best Practices", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md437", [
        [ "Performance Optimization", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md438", null ],
        [ "Memory Management", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md439", null ],
        [ "Real-time Considerations", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md440", null ]
      ] ],
      [ "Applications and Use Cases", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md441", [
        [ "Environmental Monitoring", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md442", null ],
        [ "Thermal Protection", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md443", null ],
        [ "Data Logging", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md444", null ]
      ] ],
      [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md445", [
        [ "Automated Testing", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md446", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md447", null ]
    ] ],
    [ "ESP32-C6 UART Comprehensive Test Suite Documentation", "md_examples_2esp32_2docs_2README__UART__TESTING.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md449", [
        [ "Supported ESP32 Variants", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md450", null ]
      ] ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md451", [
        [ "Core Functionality", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md452", null ],
        [ "Advanced UART Features", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md453", null ],
        [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md454", null ],
        [ "Pattern Detection Testing", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md455", null ],
        [ "Testing Infrastructure", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md456", null ]
      ] ],
      [ "Hardware Setup", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md457", [
        [ "ESP32-C6-DevKitM-1 Pin Configuration", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md458", null ],
        [ "External Loopback Testing Setup", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md459", null ],
        [ "Test Progression Indicator", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md460", null ],
        [ "Logic Analyzer Setup", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md461", null ]
      ] ],
      [ "Running the Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md462", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md463", null ],
        [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md464", null ],
        [ "Direct ESP-IDF Build (Alternative)", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md465", null ],
        [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md466", null ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md467", [
        [ "1. Constructor/Destructor Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md468", null ],
        [ "2. Basic Communication Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md469", null ],
        [ "3. Advanced Features Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md470", null ],
        [ "4. Async Operations Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md471", null ],
        [ "5. Statistics and Diagnostics Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md472", null ],
        [ "6. ESP32-C6 Specific Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md473", null ],
        [ "7. Event-Driven Pattern Detection Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md474", null ]
      ] ],
      [ "Pattern Detection Specifications", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md475", [
        [ "ESP-IDF v5.5 Pattern Detection", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md476", null ],
        [ "Pattern Detection Parameters", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md477", null ],
        [ "Timing Optimization", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md478", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md479", [
        [ "Successful Test Output", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md480", null ],
        [ "Pattern Detection Test Results", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md481", null ],
        [ "Event-Driven Pattern Detection Results", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md482", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md483", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md484", [
          [ "Test Failures", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md485", null ],
          [ "Pattern Detection Issues", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md486", null ],
          [ "External Loopback Issues", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md487", null ],
          [ "Test Progression Indicator Issues", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md488", null ],
          [ "UART Configuration Issues", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md489", null ]
        ] ],
        [ "Debug Mode", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md490", null ],
        [ "Pattern Detection Debugging", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md491", null ]
      ] ],
      [ "Performance Metrics", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md492", [
        [ "Build Information", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md493", null ],
        [ "Typical Results (ESP32-C6 @ 160MHz)", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md494", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md495", null ]
      ] ],
      [ "Integration with Development Workflow", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md496", [
        [ "Continuous Integration", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md497", null ],
        [ "Hardware-in-the-Loop Testing", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md498", null ]
      ] ],
      [ "Advanced Configuration", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md499", [
        [ "Custom UART Ports", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md500", null ],
        [ "Pattern Detection Configuration", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md501", null ],
        [ "Event Queue Configuration", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md502", null ],
        [ "Test Parameters", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md503", null ]
      ] ],
      [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md504", [
        [ "UART Peripheral", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md505", null ],
        [ "Pattern Detection Capabilities", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md506", null ],
        [ "Testing Advantages", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md507", null ],
        [ "ESP32-C6 Specific Observations", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md508", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md509", null ]
    ] ],
    [ "ESP32 HardFOC Interface Wrapper - Build System Guide", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html", [
      [ "📋 <strong>Table of Contents</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md619", null ],
      [ "📋 <strong>Overview</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md620", [
        [ "<strong>Core Features</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md621", null ],
        [ "<strong>Key Capabilities</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md622", null ]
      ] ],
      [ "🏗️ <strong>Architecture and Design</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md623", [
        [ "<strong>System Architecture</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md624", null ],
        [ "<strong>Component Interaction</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md625", null ],
        [ "<strong>Design Principles</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md626", null ]
      ] ],
      [ "⚙️ <strong>Configuration System</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md627", [
        [ "<strong>Configuration File Structure</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md628", null ],
        [ "<strong>Configuration Loading Process</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md629", null ],
        [ "<strong>Configuration Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md630", null ]
      ] ],
      [ "🔧 <strong>Build Process and Workflow</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md631", [
        [ "<strong>Build Execution Flow</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md632", [
          [ "<strong>1. Configuration Loading</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md633", null ],
          [ "<strong>2. Parameter Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md634", null ],
          [ "<strong>3. Environment Setup</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md635", null ],
          [ "<strong>4. Build Execution</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md636", null ],
          [ "<strong>5. Output Generation</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md637", null ]
        ] ],
        [ "<strong>Build Commands and Operations</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md638", null ],
        [ "<strong>Build Type Configurations</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md639", [
          [ "<strong>Debug Build</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md640", null ],
          [ "<strong>Release Build</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md641", null ]
        ] ]
      ] ],
      [ "🚀 <strong>Usage Examples and Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md642", [
        [ "<strong>Basic Build Workflows</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md643", [
          [ "<strong>1. Development Build</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md644", null ],
          [ "<strong>2. Production Build</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md645", null ],
          [ "<strong>3. Multi-Version Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md646", null ]
        ] ],
        [ "<strong>Advanced Build Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md647", [
          [ "<strong>1. Clean Build Workflow</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md648", null ],
          [ "<strong>2. Cache-Optimized Build</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md649", null ],
          [ "<strong>3. Configuration Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md650", null ]
        ] ],
        [ "<strong>Build Output and Artifacts</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md651", [
          [ "<strong>Build Directory Structure</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md652", null ],
          [ "<strong>Firmware Files</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md653", null ]
        ] ]
      ] ],
      [ "⚡ <strong>Performance and Optimization</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md654", [
        [ "<strong>Build Acceleration Features</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md655", [
          [ "<strong>ccache Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md656", null ],
          [ "<strong>Incremental Builds</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md657", null ]
        ] ],
        [ "<strong>Build Optimization Strategies</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md658", [
          [ "<strong>1. Parallel Compilation</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md659", null ],
          [ "<strong>2. Build Cache Optimization</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md660", null ],
          [ "<strong>3. Dependency Optimization</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md661", null ]
        ] ],
        [ "<strong>Performance Monitoring</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md662", [
          [ "<strong>Build Time Metrics</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md663", null ],
          [ "<strong>Resource Utilization</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md664", null ]
        ] ]
      ] ],
      [ "🔍 <strong>Troubleshooting and Debugging</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md665", [
        [ "<strong>Common Build Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md666", [
          [ "<strong>1. Configuration Errors</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md667", null ],
          [ "<strong>2. ESP-IDF Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md668", null ],
          [ "<strong>3. Build Failures</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md669", null ],
          [ "<strong>4. Cache Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md670", null ]
        ] ],
        [ "<strong>Debug and Verbose Mode</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md671", [
          [ "<strong>Enabling Debug Output</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md672", null ],
          [ "<strong>Debug Information Available</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md673", null ]
        ] ],
        [ "<strong>Build Log Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md674", [
          [ "<strong>Log File Locations</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md675", null ],
          [ "<strong>Common Log Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md676", null ]
        ] ]
      ] ],
      [ "📚 <strong>Reference and Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md677", [
        [ "<strong>Command Reference</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md678", [
          [ "<strong>Build Script Parameters</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md679", null ],
          [ "<strong>Build Options</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md680", null ],
          [ "<strong>Environment Variables</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md681", null ]
        ] ],
        [ "<strong>Configuration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md682", [
          [ "<strong>Minimal Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md683", null ],
          [ "<strong>Advanced Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md684", null ]
        ] ],
        [ "<strong>Integration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md685", [
          [ "<strong>CMake Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md686", null ],
          [ "<strong>CI/CD Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md687", null ]
        ] ],
        [ "<strong>Best Practices</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md688", [
          [ "<strong>1. Configuration Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md689", null ],
          [ "<strong>2. Build Optimization</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md690", null ],
          [ "<strong>3. Error Handling</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md691", null ],
          [ "<strong>4. Performance Monitoring</strong>", "md_examples_2esp32_2scripts_2docs_2README__BUILD__SYSTEM.html#autotoc_md692", null ]
        ] ]
      ] ]
    ] ],
    [ "ESP32 HardFOC Interface Wrapper - Centralized Configuration Guide", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html", [
      [ "📋 <strong>Table of Contents</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md697", null ],
      [ "📋 <strong>Overview</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md698", [
        [ "<strong>Core Features</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md699", null ],
        [ "<strong>Key Capabilities</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md700", null ]
      ] ],
      [ "🏗️ <strong>Architecture and Design</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md701", [
        [ "<strong>System Architecture</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md702", null ],
        [ "<strong>Component Interaction</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md703", null ],
        [ "<strong>Design Principles</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md704", null ]
      ] ],
      [ "⚙️ <strong>Configuration File Structure</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md705", [
        [ "<strong>File Location and Naming</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md706", null ],
        [ "<strong>Configuration Schema</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md707", [
          [ "<strong>Metadata Section</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md708", null ],
          [ "<strong>Applications Section</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md709", null ],
          [ "<strong>Build Configuration Section</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md710", null ],
          [ "<strong>Flash Configuration Section</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md711", null ],
          [ "<strong>System Configuration Section</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md712", null ]
        ] ]
      ] ],
      [ "🔧 <strong>Configuration Loading and Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md713", [
        [ "<strong>Configuration Loading Process</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md714", [
          [ "<strong>Primary Loading Method (yq)</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md715", null ],
          [ "<strong>Fallback Loading Method (grep/sed)</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md716", null ],
          [ "<strong>Configuration Loading Priority</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md717", null ]
        ] ],
        [ "<strong>Configuration Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md718", [
          [ "<strong>Schema Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md719", null ],
          [ "<strong>Application Configuration Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md720", null ],
          [ "<strong>Configuration Integrity Checking</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md721", null ]
        ] ]
      ] ],
      [ "🌍 <strong>Environment Variable Overrides</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md722", [
        [ "<strong>Override Priority System</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md723", [
          [ "<strong>Priority Order</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md724", null ],
          [ "<strong>Environment Variable Naming Convention</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md725", null ]
        ] ],
        [ "<strong>Supported Environment Variables</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md726", [
          [ "<strong>Build Configuration Overrides</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md727", null ],
          [ "<strong>Application Configuration Overrides</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md728", null ],
          [ "<strong>Flash Configuration Overrides</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md729", null ],
          [ "<strong>System Configuration Overrides</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md730", null ]
        ] ],
        [ "<strong>Dynamic Configuration Updates</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md731", [
          [ "<strong>Runtime Configuration Modification</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md732", null ],
          [ "<strong>Configuration Reloading</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md733", null ]
        ] ]
      ] ],
      [ "🚀 <strong>Usage Examples and Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md734", [
        [ "<strong>Basic Configuration Usage</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md735", [
          [ "<strong>1. Loading Configuration Values</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md736", null ],
          [ "<strong>2. Configuration Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md737", null ],
          [ "<strong>3. Environment Variable Overrides</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md738", null ]
        ] ],
        [ "<strong>Advanced Configuration Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md739", [
          [ "<strong>1. Configuration-Driven Scripts</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md740", null ],
          [ "<strong>2. Dynamic Configuration Updates</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md741", null ],
          [ "<strong>3. Configuration Testing and Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md742", null ]
        ] ],
        [ "<strong>Integration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md743", [
          [ "<strong>1. CMake Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md744", null ],
          [ "<strong>2. CI/CD Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md745", null ],
          [ "<strong>3. Development Environment Setup</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md746", null ]
        ] ]
      ] ],
      [ "🔍 <strong>Troubleshooting and Debugging</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md747", [
        [ "<strong>Common Configuration Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md748", [
          [ "<strong>1. Configuration File Not Found</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md749", null ],
          [ "<strong>2. YAML Syntax Errors</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md750", null ],
          [ "<strong>3. Configuration Validation Failures</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md751", null ],
          [ "<strong>4. Environment Variable Conflicts</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md752", null ]
        ] ],
        [ "<strong>Debug and Verbose Mode</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md753", [
          [ "<strong>Enabling Debug Output</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md754", null ],
          [ "<strong>Configuration Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md755", null ]
        ] ]
      ] ],
      [ "📚 <strong>Reference and Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md756", [
        [ "<strong>Configuration Function Reference</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md757", [
          [ "<strong>Core Loading Functions</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md758", null ],
          [ "<strong>Environment Variable Functions</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md759", null ],
          [ "<strong>Utility Functions</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md760", null ]
        ] ],
        [ "<strong>Configuration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md761", [
          [ "<strong>Minimal Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md762", null ],
          [ "<strong>Standard Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md763", null ],
          [ "<strong>Advanced Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md764", null ]
        ] ],
        [ "<strong>Best Practices</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md765", [
          [ "<strong>1. Configuration Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md766", null ],
          [ "<strong>2. Environment Variable Usage</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md767", null ],
          [ "<strong>3. Configuration Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md768", null ],
          [ "<strong>4. Integration and Automation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CENTRALIZED__CONFIG.html#autotoc_md769", null ]
        ] ]
      ] ]
    ] ],
    [ "ESP32 HardFOC Interface Wrapper - Configuration System Guide", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html", [
      [ "📋 <strong>Table of Contents</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md774", null ],
      [ "📋 <strong>Overview</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md775", [
        [ "<strong>Core Features</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md776", null ],
        [ "<strong>Key Capabilities</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md777", null ]
      ] ],
      [ "🏗️ <strong>Architecture and Design</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md778", [
        [ "<strong>System Architecture</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md779", null ],
        [ "<strong>Component Interaction</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md780", null ],
        [ "<strong>Design Principles</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md781", null ]
      ] ],
      [ "⚙️ <strong>Configuration File Structure</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md782", [
        [ "<strong>Configuration File Location</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md783", null ],
        [ "<strong>Configuration Schema</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md784", [
          [ "<strong>Metadata Section</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md785", null ],
          [ "<strong>Applications Section</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md786", null ],
          [ "<strong>Build Configuration Section</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md787", null ],
          [ "<strong>Flash Configuration Section</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md788", null ],
          [ "<strong>System Configuration Section</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md789", null ]
        ] ],
        [ "<strong>Configuration File Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md790", [
          [ "<strong>YAML Schema Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md791", null ],
          [ "<strong>Configuration Integrity Checks</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md792", null ]
        ] ]
      ] ],
      [ "🔧 <strong>Configuration Loading and Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md793", [
        [ "<strong>Configuration Loading Process</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md794", [
          [ "<strong>1. Primary Loading Method (yq)</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md795", null ],
          [ "<strong>2. Fallback Loading Method (grep/sed)</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md796", null ]
        ] ],
        [ "<strong>Configuration Validation Functions</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md797", [
          [ "<strong>Application Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md798", null ],
          [ "<strong>Build Type Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md799", null ],
          [ "<strong>ESP-IDF Version Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md800", null ]
        ] ],
        [ "<strong>Configuration Access Functions</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md801", [
          [ "<strong>Application Information</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md802", null ],
          [ "<strong>Build Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md803", null ]
        ] ]
      ] ],
      [ "🔄 <strong>Environment Variable Overrides</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md804", [
        [ "<strong>Configuration Override System</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md805", [
          [ "<strong>Environment Variable Priority</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md806", null ],
          [ "<strong>Supported Environment Variables</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md807", null ]
        ] ],
        [ "<strong>Dynamic Configuration Updates</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md808", [
          [ "<strong>Runtime Configuration Changes</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md809", null ],
          [ "<strong>Configuration Validation at Runtime</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md810", null ]
        ] ]
      ] ],
      [ "🚀 <strong>Usage Examples and Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md811", [
        [ "<strong>Basic Configuration Usage</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md812", [
          [ "<strong>1. Load and Validate Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md813", null ],
          [ "<strong>2. Application Configuration Access</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md814", null ],
          [ "<strong>3. Build Configuration Access</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md815", null ]
        ] ],
        [ "<strong>Advanced Configuration Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md816", [
          [ "<strong>1. Configuration-Driven Scripts</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md817", null ],
          [ "<strong>2. Dynamic Configuration Updates</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md818", null ],
          [ "<strong>3. Configuration Validation Scripts</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md819", null ]
        ] ],
        [ "<strong>Integration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md820", [
          [ "<strong>1. CMake Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md821", null ],
          [ "<strong>2. CI/CD Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md822", null ]
        ] ]
      ] ],
      [ "🔍 <strong>Troubleshooting and Debugging</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md823", [
        [ "<strong>Common Configuration Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md824", [
          [ "<strong>1. Configuration File Not Found</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md825", null ],
          [ "<strong>2. YAML Syntax Errors</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md826", null ],
          [ "<strong>3. Configuration Validation Failures</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md827", null ],
          [ "<strong>4. Environment Variable Conflicts</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md828", null ]
        ] ],
        [ "<strong>Debug and Verbose Mode</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md829", [
          [ "<strong>Enabling Configuration Debug Output</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md830", null ],
          [ "<strong>Configuration Debugging Functions</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md831", null ]
        ] ],
        [ "<strong>Configuration Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md832", [
          [ "<strong>Configuration Test Scripts</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md833", null ]
        ] ]
      ] ],
      [ "📚 <strong>Reference and Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md834", [
        [ "<strong>Configuration Function Reference</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md835", [
          [ "<strong>Core Functions</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md836", null ],
          [ "<strong>Helper Functions</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md837", null ]
        ] ],
        [ "<strong>Configuration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md838", [
          [ "<strong>Minimal Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md839", null ],
          [ "<strong>Standard Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md840", null ],
          [ "<strong>Advanced Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md841", null ]
        ] ],
        [ "<strong>Best Practices</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md842", [
          [ "<strong>1. Configuration Structure</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md843", null ],
          [ "<strong>2. Validation and Error Handling</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md844", null ],
          [ "<strong>3. Documentation and Maintenance</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md845", null ],
          [ "<strong>4. Performance and Optimization</strong>", "md_examples_2esp32_2scripts_2docs_2README__CONFIG__SYSTEM.html#autotoc_md846", null ]
        ] ]
      ] ]
    ] ],
    [ "ESP32 HardFOC Interface Wrapper - Flash System Guide", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html", [
      [ "📋 <strong>Table of Contents</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md851", null ],
      [ "📋 <strong>Overview</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md852", [
        [ "<strong>Core Features</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md853", null ],
        [ "<strong>Key Capabilities</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md854", null ]
      ] ],
      [ "🏗️ <strong>Architecture and Design</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md855", [
        [ "<strong>System Architecture</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md856", null ],
        [ "<strong>Component Interaction</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md857", null ],
        [ "<strong>Design Principles</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md858", null ]
      ] ],
      [ "🔌 <strong>Port Detection and Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md859", [
        [ "<strong>Automatic Device Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md860", [
          [ "<strong>Cross-Platform Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md861", null ],
          [ "<strong>Device Identification Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md862", null ]
        ] ],
        [ "<strong>Port Validation and Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md863", [
          [ "<strong>Connectivity Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md864", null ],
          [ "<strong>Permission Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md865", null ]
        ] ],
        [ "<strong>Port Selection Logic</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md866", [
          [ "<strong>Automatic Port Selection</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md867", null ],
          [ "<strong>Manual Port Override</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md868", null ]
        ] ]
      ] ],
      [ "⚡ <strong>Flash Operations and Workflows</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md869", [
        [ "<strong>Operation Types</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md870", [
          [ "<strong>1. Flash Operations</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md871", null ],
          [ "<strong>2. Operation Syntax</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md872", null ]
        ] ],
        [ "<strong>Flash Process Workflow</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md873", [
          [ "<strong>1. Pre-Flash Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md874", null ],
          [ "<strong>2. Port Detection and Selection</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md875", null ],
          [ "<strong>3. Flash Execution</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md876", null ],
          [ "<strong>4. Post-Flash Operations</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md877", null ]
        ] ],
        [ "<strong>Flash Configuration Options</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md878", [
          [ "<strong>Build Type Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md879", null ],
          [ "<strong>ESP-IDF Version Support</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md880", null ]
        ] ]
      ] ],
      [ "📺 <strong>Monitoring and Logging</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md881", [
        [ "<strong>Integrated Logging System</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md882", [
          [ "<strong>Log Generation</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md883", null ],
          [ "<strong>Log Content and Structure</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md884", null ]
        ] ],
        [ "<strong>Monitoring Capabilities</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md885", [
          [ "<strong>Real-Time Monitoring</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md886", null ],
          [ "<strong>Monitor Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md887", null ]
        ] ],
        [ "<strong>Log Management Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md888", [
          [ "<strong>Automatic Log Rotation</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md889", null ],
          [ "<strong>Log Analysis Tools</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md890", null ]
        ] ]
      ] ],
      [ "🚀 <strong>Usage Examples and Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md891", [
        [ "<strong>Basic Flash Workflows</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md892", [
          [ "<strong>1. Development Flash Workflow</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md893", null ],
          [ "<strong>2. Production Flash Workflow</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md894", null ],
          [ "<strong>3. Debugging Workflow</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md895", null ]
        ] ],
        [ "<strong>Advanced Flash Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md896", [
          [ "<strong>1. Multi-Device Deployment</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md897", null ],
          [ "<strong>2. Conditional Flash Operations</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md898", null ],
          [ "<strong>3. Automated Testing Flash</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md899", null ]
        ] ],
        [ "<strong>Integration with Build System</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md900", [
          [ "<strong>Build-Flash Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md901", null ],
          [ "<strong>CI/CD Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md902", null ]
        ] ]
      ] ],
      [ "🔍 <strong>Troubleshooting and Debugging</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md903", [
        [ "<strong>Common Flash Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md904", [
          [ "<strong>1. Port Detection Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md905", null ],
          [ "<strong>2. Permission Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md906", null ],
          [ "<strong>3. Flash Failures</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md907", null ],
          [ "<strong>4. Monitor Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md908", null ]
        ] ],
        [ "<strong>Debug and Verbose Mode</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md909", [
          [ "<strong>Enabling Debug Output</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md910", null ],
          [ "<strong>Debug Information Available</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md911", null ]
        ] ],
        [ "<strong>Log Analysis for Troubleshooting</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md912", [
          [ "<strong>Flash Log Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md913", null ],
          [ "<strong>Monitor Log Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md914", null ]
        ] ]
      ] ],
      [ "📚 <strong>Reference and Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md915", [
        [ "<strong>Command Reference</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md916", [
          [ "<strong>Flash Script Parameters</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md917", null ],
          [ "<strong>Flash Options</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md918", null ],
          [ "<strong>Environment Variables</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md919", null ]
        ] ],
        [ "<strong>Configuration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md920", [
          [ "<strong>Minimal Flash Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md921", null ],
          [ "<strong>Advanced Flash Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md922", null ]
        ] ],
        [ "<strong>Integration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md923", [
          [ "<strong>CMake Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md924", null ],
          [ "<strong>CI/CD Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md925", null ]
        ] ],
        [ "<strong>Best Practices</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md926", [
          [ "<strong>1. Flash Operations</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md927", null ],
          [ "<strong>2. Port Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md928", null ],
          [ "<strong>3. Error Handling</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md929", null ],
          [ "<strong>4. Performance Optimization</strong>", "md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md930", null ]
        ] ]
      ] ]
    ] ],
    [ "ESP32 HardFOC Interface Wrapper - Logging System Guide", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html", [
      [ "📋 <strong>Table of Contents</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md935", null ],
      [ "📋 <strong>Overview</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md936", [
        [ "<strong>Core Features</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md937", null ],
        [ "<strong>Key Capabilities</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md938", null ]
      ] ],
      [ "🏗️ <strong>Architecture and Design</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md939", [
        [ "<strong>System Architecture</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md940", null ],
        [ "<strong>Component Interaction</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md941", null ],
        [ "<strong>Design Principles</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md942", null ]
      ] ],
      [ "📝 <strong>Log Generation and Capture</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md943", [
        [ "<strong>Automatic Log Generation</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md944", [
          [ "<strong>Built-in Logging</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md945", null ],
          [ "<strong>Log File Naming Convention</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md946", null ],
          [ "<strong>Log Content Structure</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md947", null ]
        ] ],
        [ "<strong>Log Capture Mechanisms</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md948", [
          [ "<strong>Output Capture</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md949", null ],
          [ "<strong>Integration Points</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md950", null ]
        ] ]
      ] ],
      [ "🗂️ <strong>Log Management and Organization</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md951", [
        [ "<strong>Log Directory Structure</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md952", [
          [ "<strong>Standard Organization</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md953", null ],
          [ "<strong>Automatic Organization</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md954", null ]
        ] ],
        [ "<strong>Log Rotation and Cleanup</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md955", [
          [ "<strong>Automatic Rotation</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md956", null ],
          [ "<strong>Cleanup Operations</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md957", null ]
        ] ],
        [ "<strong>Log File Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md958", [
          [ "<strong>File Operations</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md959", null ],
          [ "<strong>Storage Optimization</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md960", null ]
        ] ]
      ] ],
      [ "🔍 <strong>Log Analysis and Search</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md961", [
        [ "<strong>Cross-Log Search</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md962", [
          [ "<strong>Pattern Search</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md963", null ],
          [ "<strong>Search Capabilities</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md964", null ]
        ] ],
        [ "<strong>Log Statistics and Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md965", [
          [ "<strong>Statistical Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md966", null ],
          [ "<strong>Trend Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md967", null ]
        ] ],
        [ "<strong>Log Comparison and Diff</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md968", [
          [ "<strong>Cross-Log Comparison</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md969", null ]
        ] ]
      ] ],
      [ "🚀 <strong>Usage Examples and Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md970", [
        [ "<strong>Basic Log Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md971", [
          [ "<strong>1. View Logs</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md972", null ],
          [ "<strong>2. Search Logs</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md973", null ],
          [ "<strong>3. Log Maintenance</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md974", null ]
        ] ],
        [ "<strong>Advanced Log Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md975", [
          [ "<strong>1. Development Workflow Logging</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md976", null ],
          [ "<strong>2. Production Deployment Logging</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md977", null ],
          [ "<strong>3. Debugging and Troubleshooting</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md978", null ]
        ] ],
        [ "<strong>Log Analysis Workflows</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md979", [
          [ "<strong>1. Error Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md980", null ],
          [ "<strong>2. Performance Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md981", null ],
          [ "<strong>3. Success Rate Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md982", null ]
        ] ]
      ] ],
      [ "🔧 <strong>Integration and Automation</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md983", [
        [ "<strong>Script Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md984", [
          [ "<strong>Automatic Logging</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md985", null ],
          [ "<strong>Log Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md986", null ]
        ] ],
        [ "<strong>CI/CD Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md987", [
          [ "<strong>Automated Logging</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md988", null ],
          [ "<strong>Log Artifacts</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md989", null ]
        ] ],
        [ "<strong>Automation Scripts</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md990", [
          [ "<strong>Log Analysis Automation</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md991", null ],
          [ "<strong>Log Cleanup Automation</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md992", null ]
        ] ]
      ] ],
      [ "🔍 <strong>Troubleshooting and Debugging</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md993", [
        [ "<strong>Common Log Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md994", [
          [ "<strong>1. Log Files Not Created</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md995", null ],
          [ "<strong>2. Log Directory Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md996", null ],
          [ "<strong>3. Log File Corruption</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md997", null ]
        ] ],
        [ "<strong>Debug and Verbose Mode</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md998", [
          [ "<strong>Enabling Log Debug Output</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md999", null ],
          [ "<strong>Log System Debugging</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1000", null ]
        ] ],
        [ "<strong>Log Performance Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1001", [
          [ "<strong>Large Log Files</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1002", null ],
          [ "<strong>Search Performance</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1003", null ]
        ] ]
      ] ],
      [ "📚 <strong>Reference and Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1004", [
        [ "<strong>Command Reference</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1005", [
          [ "<strong>Log Management Commands</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1006", null ],
          [ "<strong>Log Options</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1007", null ],
          [ "<strong>Environment Variables</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1008", null ]
        ] ],
        [ "<strong>Configuration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1009", [
          [ "<strong>Minimal Log Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1010", null ],
          [ "<strong>Standard Log Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1011", null ],
          [ "<strong>Advanced Log Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1012", null ]
        ] ],
        [ "<strong>Integration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1013", [
          [ "<strong>CMake Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1014", null ],
          [ "<strong>CI/CD Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1015", null ]
        ] ],
        [ "<strong>Best Practices</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1016", [
          [ "<strong>1. Log Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1017", null ],
          [ "<strong>2. Log Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1018", null ],
          [ "<strong>3. Log Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1019", null ],
          [ "<strong>4. Log Performance</strong>", "md_examples_2esp32_2scripts_2docs_2README__LOGGING__SYSTEM.html#autotoc_md1020", null ]
        ] ]
      ] ]
    ] ],
    [ "ESP32 HardFOC Interface Wrapper - Port Detection Guide", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html", [
      [ "📋 <strong>Table of Contents</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1025", null ],
      [ "📋 <strong>Overview</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1026", [
        [ "<strong>Core Features</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1027", null ],
        [ "<strong>Key Capabilities</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1028", null ]
      ] ],
      [ "🏗️ <strong>Architecture and Design</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1029", [
        [ "<strong>System Architecture</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1030", null ],
        [ "<strong>Component Interaction</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1031", null ],
        [ "<strong>Design Principles</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1032", null ]
      ] ],
      [ "🔌 <strong>Cross-Platform Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1033", [
        [ "<strong>Linux Port Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1034", [
          [ "<strong>Device Detection Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1035", null ],
          [ "<strong>Linux-Specific Features</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1036", null ],
          [ "<strong>Permission Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1037", null ]
        ] ],
        [ "<strong>macOS Port Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1038", [
          [ "<strong>Device Detection Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1039", null ],
          [ "<strong>macOS-Specific Features</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1040", null ],
          [ "<strong>macOS Troubleshooting</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1041", null ]
        ] ],
        [ "<strong>Windows (WSL2) Port Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1042", [
          [ "<strong>WSL2 Compatibility</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1043", null ],
          [ "<strong>WSL2-Specific Features</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1044", null ],
          [ "<strong>WSL2 Troubleshooting</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1045", null ]
        ] ]
      ] ],
      [ "🔍 <strong>Port Validation and Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1046", [
        [ "<strong>Connectivity Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1047", [
          [ "<strong>Basic Port Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1048", null ],
          [ "<strong>Advanced Port Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1049", null ],
          [ "<strong>Test Output Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1050", null ]
        ] ],
        [ "<strong>Permission Verification</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1051", [
          [ "<strong>Permission Level Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1052", null ],
          [ "<strong>Permission Resolution</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1053", null ]
        ] ],
        [ "<strong>Device Identification</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1054", [
          [ "<strong>USB Device Information</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1055", null ],
          [ "<strong>Device Type Recognition</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1056", null ]
        ] ]
      ] ],
      [ "⚙️ <strong>Configuration and Customization</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1057", [
        [ "<strong>Environment Variables</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1058", [
          [ "<strong>Detection Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1059", null ],
          [ "<strong>Platform-Specific Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1060", null ],
          [ "<strong>Troubleshooting Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1061", null ]
        ] ],
        [ "<strong>Custom Detection Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1062", [
          [ "<strong>User-Defined Device Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1063", null ],
          [ "<strong>Custom Validation Rules</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1064", null ]
        ] ],
        [ "<strong>Integration Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1065", [
          [ "<strong>Build System Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1066", null ],
          [ "<strong>CI/CD Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1067", null ]
        ] ]
      ] ],
      [ "🚀 <strong>Usage Examples and Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1068", [
        [ "<strong>Basic Port Detection Workflows</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1069", [
          [ "<strong>1. Quick Port Check</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1070", null ],
          [ "<strong>2. Detailed Port Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1071", null ],
          [ "<strong>3. Port Connectivity Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1072", null ]
        ] ],
        [ "<strong>Advanced Port Detection Workflows</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1073", [
          [ "<strong>1. Troubleshooting Workflow</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1074", null ],
          [ "<strong>2. Development Environment Setup</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1075", null ],
          [ "<strong>3. CI/CD Port Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1076", null ]
        ] ],
        [ "<strong>Integration Workflows</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1077", [
          [ "<strong>1. Build System Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1078", null ],
          [ "<strong>2. Flash System Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1079", null ],
          [ "<strong>3. Development Workflow Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1080", null ]
        ] ]
      ] ],
      [ "🔧 <strong>Troubleshooting and Debugging</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1081", [
        [ "<strong>Common Port Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1082", [
          [ "<strong>1. No ESP32 Devices Detected</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1083", null ],
          [ "<strong>2. Port Permission Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1084", null ],
          [ "<strong>3. Port Connectivity Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1085", null ]
        ] ],
        [ "<strong>Platform-Specific Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1086", [
          [ "<strong>Linux Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1087", null ],
          [ "<strong>macOS Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1088", null ],
          [ "<strong>WSL2 Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1089", null ]
        ] ],
        [ "<strong>Debug and Verbose Mode</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1090", [
          [ "<strong>Enabling Debug Output</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1091", null ],
          [ "<strong>Debug Information Available</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1092", null ]
        ] ]
      ] ],
      [ "📚 <strong>Reference and Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1093", [
        [ "<strong>Command Reference</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1094", [
          [ "<strong>Port Detection Commands</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1095", null ],
          [ "<strong>Environment Variables</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1096", null ]
        ] ],
        [ "<strong>Configuration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1097", [
          [ "<strong>Minimal Port Detection Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1098", null ],
          [ "<strong>Advanced Port Detection Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1099", null ],
          [ "<strong>CI/CD Port Detection Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1100", null ]
        ] ],
        [ "<strong>Integration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1101", [
          [ "<strong>CMake Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1102", null ],
          [ "<strong>CI/CD Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1103", null ],
          [ "<strong>Automation Scripts</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1104", null ]
        ] ],
        [ "<strong>Best Practices</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1105", [
          [ "<strong>1. Port Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1106", null ],
          [ "<strong>2. Troubleshooting</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1107", null ],
          [ "<strong>3. Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1108", null ],
          [ "<strong>4. Performance</strong>", "md_examples_2esp32_2scripts_2docs_2README__PORT__DETECTION.html#autotoc_md1109", null ]
        ] ]
      ] ]
    ] ],
    [ "ESP32 HardFOC Interface Wrapper - Scripts Overview", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html", [
      [ "📋 <strong>Table of Contents</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1114", null ],
      [ "📋 <strong>Overview</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1115", [
        [ "<strong>Core Design Principles</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1116", null ],
        [ "<strong>Key Capabilities</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1117", null ]
      ] ],
      [ "🏗️ <strong>Architecture and Design</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1118", [
        [ "<strong>Script Organization Structure</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1119", null ],
        [ "<strong>Architecture Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1120", null ],
        [ "<strong>Data Flow Architecture</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1121", null ]
      ] ],
      [ "📁 <strong>Script Categories and Capabilities</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1122", [
        [ "<strong>1. Core Development Scripts</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1123", [
          [ "**<tt>build_app.sh</tt> - Build System Management**", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1124", null ],
          [ "**<tt>flash_app.sh</tt> - Flash and Monitor Operations**", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1125", null ],
          [ "**<tt>manage_logs.sh</tt> - Log Management System**", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1126", null ]
        ] ],
        [ "<strong>2. Configuration and Setup Scripts</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1127", [
          [ "**<tt>config_loader.sh</tt> - Configuration Management**", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1128", null ],
          [ "**<tt>setup_common.sh</tt> - Environment Setup Functions**", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1129", null ],
          [ "**<tt>setup_repo.sh</tt> - Local Development Setup**", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1130", null ],
          [ "**<tt>setup_ci.sh</tt> - CI/CD Environment Setup**", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1131", null ]
        ] ],
        [ "<strong>3. Utility and Helper Scripts</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1132", [
          [ "**<tt>detect_ports.sh</tt> - Port Detection and Troubleshooting**", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1133", null ],
          [ "**<tt>get_app_info.py</tt> - App Information Extraction**", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1134", null ]
        ] ]
      ] ],
      [ "🔗 <strong>Dependencies and Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1135", [
        [ "<strong>Core Dependencies</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1136", [
          [ "<strong>System-Level Dependencies</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1137", null ],
          [ "<strong>Development Dependencies</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1138", null ],
          [ "<strong>Optional Dependencies</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1139", null ]
        ] ],
        [ "<strong>Integration Points</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1140", [
          [ "<strong>ESP-IDF Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1141", null ],
          [ "<strong>Configuration Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1142", null ],
          [ "<strong>Logging Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1143", null ]
        ] ]
      ] ],
      [ "⚙️ <strong>Configuration Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1144", [
        [ "<strong>Configuration File Structure</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1145", null ],
        [ "<strong>Configuration Loading Process</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1146", null ],
        [ "<strong>Configuration Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1147", null ]
      ] ],
      [ "🚀 <strong>Usage Patterns and Workflows</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1148", [
        [ "<strong>Development Workflow</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1149", null ],
        [ "<strong>Debugging Workflow</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1150", null ],
        [ "<strong>CI/CD Workflow</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1151", null ],
        [ "<strong>Production Deployment Workflow</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1152", null ]
      ] ],
      [ "🛠️ <strong>Development and Maintenance</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1153", [
        [ "<strong>Script Development Guidelines</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1154", [
          [ "<strong>Code Quality Standards</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1155", null ],
          [ "<strong>Adding New Scripts</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1156", null ],
          [ "<strong>Modifying Existing Scripts</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1157", null ]
        ] ],
        [ "<strong>Testing and Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1158", [
          [ "<strong>Testing Requirements</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1159", null ],
          [ "<strong>Validation Procedures</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1160", null ]
        ] ]
      ] ],
      [ "🔍 <strong>Troubleshooting and Support</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1161", [
        [ "<strong>Common Issues and Solutions</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1162", [
          [ "<strong>1. Configuration Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1163", null ],
          [ "<strong>2. ESP-IDF Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1164", null ],
          [ "<strong>3. Port Detection Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1165", null ],
          [ "<strong>4. Permission Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1166", null ]
        ] ],
        [ "<strong>Debug and Verbose Mode</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1167", [
          [ "<strong>Enabling Debug Output</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1168", null ],
          [ "<strong>Debug Information Available</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1169", null ]
        ] ],
        [ "<strong>Getting Help and Support</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1170", [
          [ "<strong>Script Help System</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1171", null ],
          [ "<strong>Documentation Resources</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1172", null ],
          [ "<strong>Issue Reporting Guidelines</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1173", null ]
        ] ]
      ] ],
      [ "📊 <strong>Performance and Optimization</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1174", [
        [ "<strong>Build Performance</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1175", null ],
        [ "<strong>Execution Performance</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1176", null ],
        [ "<strong>Cache Optimization</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1177", null ]
      ] ],
      [ "🔄 <strong>Version Information and Compatibility</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1178", [
        [ "<strong>Current Version</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1179", null ],
        [ "<strong>Version Compatibility Matrix</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1180", null ],
        [ "<strong>Backward Compatibility</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1181", null ]
      ] ],
      [ "🚀 <strong>Future Development and Roadmap</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1182", [
        [ "<strong>Planned Enhancements</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1183", null ],
        [ "<strong>Community Contributions</strong>", "md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1184", null ]
      ] ]
    ] ],
    [ "ESP32 HardFOC Interface Wrapper - Utility Scripts Guide", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html", [
      [ "📋 <strong>Table of Contents</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1189", null ],
      [ "📋 <strong>Overview</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1190", [
        [ "<strong>Core Features</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1191", null ],
        [ "<strong>Key Capabilities</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1192", null ]
      ] ],
      [ "🏗️ <strong>Architecture and Design</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1193", [
        [ "<strong>System Architecture</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1194", null ],
        [ "<strong>Component Interaction</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1195", null ],
        [ "<strong>Design Principles</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1196", null ]
      ] ],
      [ "🔌 <strong>Port Detection and Troubleshooting</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1197", [
        [ "<strong>Cross-Platform Port Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1198", [
          [ "<strong>Linux Port Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1199", null ],
          [ "<strong>macOS Port Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1200", null ],
          [ "<strong>Windows (WSL2) Port Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1201", null ]
        ] ],
        [ "<strong>Port Validation and Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1202", [
          [ "<strong>Connectivity Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1203", null ],
          [ "<strong>Permission Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1204", null ]
        ] ],
        [ "<strong>Troubleshooting Capabilities</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1205", [
          [ "<strong>Automatic Problem Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1206", null ],
          [ "<strong>Problem Resolution</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1207", null ]
        ] ]
      ] ],
      [ "⚙️ <strong>Environment Setup and Automation</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1208", [
        [ "<strong>Local Development Setup</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1209", [
          [ "<strong>Complete Environment Initialization</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1210", null ],
          [ "<strong>Interactive Setup Process</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1211", null ]
        ] ],
        [ "<strong>CI/CD Environment Setup</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1212", [
          [ "<strong>Optimized CI Environment</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1213", null ],
          [ "<strong>Cache Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1214", null ]
        ] ],
        [ "<strong>Cross-Platform Compatibility</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1215", [
          [ "<strong>Operating System Support</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1216", null ],
          [ "<strong>Dependency Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1217", null ]
        ] ]
      ] ],
      [ "🔧 <strong>Configuration and Information Tools</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1218", [
        [ "<strong>Configuration Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1219", [
          [ "<strong>Centralized Configuration Access</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1220", null ],
          [ "<strong>Configuration Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1221", null ]
        ] ],
        [ "<strong>Information Extraction</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1222", [
          [ "<strong>Application Information</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1223", null ],
          [ "<strong>System Information</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1224", null ]
        ] ]
      ] ],
      [ "🚀 <strong>Usage Examples and Patterns</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1225", [
        [ "<strong>Port Detection Workflows</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1226", [
          [ "<strong>1. Basic Port Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1227", null ],
          [ "<strong>2. Detailed Port Analysis</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1228", null ],
          [ "<strong>3. Port Connectivity Testing</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1229", null ]
        ] ],
        [ "<strong>Environment Setup Workflows</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1230", [
          [ "<strong>1. Local Development Setup</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1231", null ],
          [ "<strong>2. CI/CD Environment Setup</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1232", null ],
          [ "<strong>3. Environment Verification</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1233", null ]
        ] ],
        [ "<strong>Configuration Management Workflows</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1234", [
          [ "<strong>1. Application Information Access</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1235", null ],
          [ "<strong>2. Configuration Validation</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1236", null ]
        ] ],
        [ "<strong>Troubleshooting Workflows</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1237", [
          [ "<strong>1. Port Problem Resolution</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1238", null ],
          [ "<strong>2. Environment Problem Resolution</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1239", null ]
        ] ]
      ] ],
      [ "🔍 <strong>Troubleshooting and Debugging</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1240", [
        [ "<strong>Common Port Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1241", [
          [ "<strong>1. No ESP32 Devices Detected</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1242", null ],
          [ "<strong>2. Port Permission Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1243", null ],
          [ "<strong>3. Port Connectivity Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1244", null ]
        ] ],
        [ "<strong>Environment Setup Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1245", [
          [ "<strong>1. Dependency Installation Failures</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1246", null ],
          [ "<strong>2. ESP-IDF Installation Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1247", null ],
          [ "<strong>3. Permission and Path Issues</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1248", null ]
        ] ],
        [ "<strong>Debug and Verbose Mode</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1249", [
          [ "<strong>Enabling Debug Output</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1250", null ],
          [ "<strong>Debug Information Available</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1251", null ]
        ] ]
      ] ],
      [ "📚 <strong>Reference and Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1252", [
        [ "<strong>Command Reference</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1253", [
          [ "<strong>Port Detection Commands</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1254", null ],
          [ "<strong>Setup Commands</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1255", null ],
          [ "<strong>Configuration Commands</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1256", null ]
        ] ],
        [ "<strong>Environment Variables</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1257", [
          [ "<strong>Port Detection Variables</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1258", null ],
          [ "<strong>Setup Configuration Variables</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1259", null ],
          [ "<strong>Debug Configuration Variables</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1260", null ]
        ] ],
        [ "<strong>Configuration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1261", [
          [ "<strong>Minimal Port Detection Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1262", null ],
          [ "<strong>Advanced Port Detection Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1263", null ],
          [ "<strong>Environment Setup Configuration</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1264", null ]
        ] ],
        [ "<strong>Integration Examples</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1265", [
          [ "<strong>CMake Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1266", null ],
          [ "<strong>CI/CD Integration</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1267", null ],
          [ "<strong>Automation Scripts</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1268", null ]
        ] ],
        [ "<strong>Best Practices</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1269", [
          [ "<strong>1. Port Detection</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1270", null ],
          [ "<strong>2. Environment Setup</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1271", null ],
          [ "<strong>3. Configuration Management</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1272", null ],
          [ "<strong>4. Troubleshooting</strong>", "md_examples_2esp32_2scripts_2docs_2README__UTILITY__SCRIPTS.html#autotoc_md1273", null ]
        ] ]
      ] ]
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
"BaseNvs_8h.html#a0a4f43b41760df506a65240a5ca415a4ac3598e620dfd937da04ca8a1b4fc197d",
"BaseTemperature_8h.html#aee8dd042d1f1740b2a23650bbe6efe12",
"EspTypes__CAN_8h.html#ab7f2de3bbd0be41bdd4749610fd787f6a218ec6a4ce230d831e99e3c9b5963581",
"EspTypes__UART_8h.html#abde746b6c6a9bf11878d665ef32b70c8a481e16e3243e7966bda5596c47e47794",
"McuSelect_8h.html#a404f6fafd0a07c9ba5bb5e344a941294",
"TemperatureComprehensiveTest_8cpp.html#a978a6b3ea32c84350a5ae5808f192133",
"classBaseCan.html#a6de6c09dd8b68a3d07ab804967e34782",
"classBasePwm.html",
"classEspBluetooth.html#aaf3338ba2f2d3fd8c5d07607cccfc56c",
"classEspPeriodicTimer.html#a24851bbc3359b261076ac7ce63175bc4",
"classEspTemperature.html#a629744fc883c865e06a3f2369b577175",
"classRtosMutex.html#aa994fd43b2c205c871c86b4d95ff7c8d",
"group__bluetooth.html#ggaf4d7f320ef7ffa2ef26966adc8facb67a5c82831a3aaeb50cb8b58dc8e992f771",
"index.html#autotoc_md594",
"md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md248",
"md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md498",
"md_examples_2esp32_2scripts_2docs_2README__FLASH__SYSTEM.html#autotoc_md865",
"md_examples_2esp32_2scripts_2docs_2README__SCRIPTS__OVERVIEW.html#autotoc_md1124",
"structEspPwm_1_1ValidationContext.html",
"structhf__can__diagnostics__t.html#aefc0cac14ebba0fc64e72d99a6f72f19",
"structhf__nvs__capabilities__t.html#aade4d0e7880b1d69f0691b2188410522",
"structhf__spi__device__config__t.html#aef5e3c25eff3b4ab346f894ec3c1556b"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';