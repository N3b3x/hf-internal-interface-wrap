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
    [ "🚀 HardFOC Internal Interface Wrapper", "index.html", "index" ],
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
    [ "ESP32-C6 GPIO Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__GPIO__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md45", [
        [ "Core Features", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md46", null ],
        [ "Advanced Features (ESP32-C6 Specific)", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md47", null ],
        [ "Performance & Robustness", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md48", null ]
      ] ],
      [ "Hardware Setup", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md49", [
        [ "ESP32-C6 DevKit-M-1 Pin Layout", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md50", null ],
        [ "Optional Physical Connections", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md51", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md52", [
        [ "Build Commands", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md53", null ],
        [ "CI Pipeline", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md54", null ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md55", [
        [ "1. Basic Functionality Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md56", null ],
        [ "2. Configuration Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md57", null ],
        [ "3. Advanced Feature Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md58", null ],
        [ "4. ESP32-C6 Specific Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md59", null ],
        [ "5. Robustness Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md60", null ],
        [ "6. Performance Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md61", null ]
      ] ],
      [ "Expected Output", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md62", null ],
      [ "Development Notes", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md63", [
        [ "Adding New Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md64", null ],
        [ "Test Patterns", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md65", null ],
        [ "Debugging", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md66", null ]
      ] ],
      [ "Integration with Main Project", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md67", null ],
      [ "Contributing", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md68", null ],
      [ "License", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md69", null ]
    ] ],
    [ "ESP32-C6 NVS Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__NVS__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md71", null ],
      [ "Test Coverage", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md72", null ],
      [ "Building the Test", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md73", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md74", null ],
        [ "Build Steps", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md75", null ]
      ] ],
      [ "Running the Test", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md76", [
        [ "Expected Output", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md77", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md78", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md79", null ]
      ] ],
      [ "Configuration", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md80", null ],
      [ "Customization", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md81", null ],
      [ "Notes", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md82", null ]
    ] ],
    [ "PIO Comprehensive Test Suite Documentation", "md_examples_2esp32_2docs_2README__PIO__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md84", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md85", [
        [ "Core Functionality", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md86", null ],
        [ "RMT-Specific Features", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md87", null ],
        [ "Protocol Testing", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md88", null ],
        [ "Diagnostics", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md89", null ]
      ] ],
      [ "Hardware Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md90", [
        [ "ESP32-C6-DevKitM-1 Pin Configuration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md91", null ],
        [ "Automated Testing Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md92", null ],
        [ "WS2812 LED Testing Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md93", null ],
        [ "External WS2812 LED Chain (Optional)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md94", null ],
        [ "Logic Analyzer Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md95", null ]
      ] ],
      [ "Running the Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md96", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md97", null ],
        [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md98", null ],
        [ "Direct ESP-IDF Build (Alternative)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md99", null ],
        [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md100", null ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md101", [
        [ "1. Constructor/Destructor Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md102", null ],
        [ "2. Lifecycle Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md103", null ],
        [ "3. Channel Configuration Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md104", null ],
        [ "4. Transmission Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md105", null ],
        [ "5. WS2812 LED Protocol Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md106", null ],
        [ "6. Automated Loopback Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md107", null ],
        [ "7. Advanced RMT Feature Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md108", null ],
        [ "8. System Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md109", null ]
      ] ],
      [ "WS2812 Protocol Specifications", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md110", [
        [ "Timing Requirements", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md111", null ],
        [ "Color Format", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md112", null ],
        [ "Test Colors", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md113", null ]
      ] ],
      [ "Logic Analyzer Test Patterns", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md114", [
        [ "Pattern 1: Basic Timing Test", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md115", null ],
        [ "Pattern 2: Frequency Sweep", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md116", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md117", [
        [ "Successful Test Output", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md118", null ],
        [ "Built-in RGB LED Verification", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md119", null ],
        [ "Automated Loopback Verification", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md120", null ],
        [ "Logic Analyzer Verification", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md121", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md122", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md123", [
          [ "Test Failures", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md124", null ],
          [ "Built-in RGB LED Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md125", null ],
          [ "Loopback Testing Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md126", null ],
          [ "Logic Analyzer Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md127", null ]
        ] ],
        [ "Debug Mode", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md128", null ]
      ] ],
      [ "Performance Metrics", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md129", [
        [ "Build Information", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md130", null ],
        [ "Typical Results (ESP32-C6 @ 160MHz)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md131", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md132", null ]
      ] ],
      [ "Integration with Development Workflow", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md133", [
        [ "Continuous Integration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md134", null ],
        [ "Hardware-in-the-Loop Testing", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md135", null ]
      ] ],
      [ "Advanced Configuration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md136", [
        [ "Custom GPIO Pins", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md137", null ],
        [ "Timing Resolution", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md138", null ],
        [ "Test Parameters", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md139", null ]
      ] ],
      [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md140", [
        [ "RMT Peripheral", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md141", null ],
        [ "Built-in RGB LED", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md142", null ],
        [ "Automated Testing Advantages", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md143", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md144", null ]
    ] ],
    [ "ESP32 Implementation Testing Infrastructure", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md146", null ],
      [ "✅ COMPLETED MIGRATION STATUS (August 2025)", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md147", [
        [ "Major Accomplishments", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md148", null ]
      ] ],
      [ "Architecture", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md149", [
        [ "Build System", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md150", null ]
      ] ],
      [ "Test Files - ALL COMPLETED", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md151", [
        [ "✅ Production-Ready Test Suites", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md152", null ]
      ] ],
      [ "CI Pipeline Integration", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md153", [
        [ "Build Matrix", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md154", null ]
      ] ],
      [ "Usage Instructions", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md155", [
        [ "1. GPIO Testing (Current Focus)", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md156", null ],
        [ "2. Build All Implementation Tests", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md157", null ],
        [ "3. Developing New Implementation Tests", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md158", null ]
      ] ],
      [ "Development Workflow", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md159", [
        [ "Current Status", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md160", null ],
        [ "Template Structure", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md161", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md162", null ],
      [ "Contributing", "md_examples_2esp32_2docs_2README__TESTING__INFRASTRUCTURE.html#autotoc_md163", null ]
    ] ],
    [ "ESP32-C6 EspPeriodicTimer Comprehensive Test Report", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html", [
      [ "Overview", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md166", null ],
      [ "ESP-IDF v5.5 ESP32-C6 Timer Capabilities Research", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md167", [
        [ "ESP Timer API Features (ESP-IDF v5.5)", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md168", null ],
        [ "ESP32-C6 Specific Capabilities", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md169", null ]
      ] ],
      [ "Implementation Analysis", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md170", [
        [ "Current EspPeriodicTimer Implementation Status", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md171", [
          [ "✅ <strong>Properly Implemented Features</strong>", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md172", null ],
          [ "⚠️ <strong>Implementation Gaps Identified</strong>", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md173", null ]
        ] ]
      ] ],
      [ "Comprehensive Test Suite", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md174", [
        [ "Test Architecture", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md175", [
          [ "<strong>Test 1: Basic Initialization and Deinitialization</strong>", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md176", null ],
          [ "<strong>Test 2: Basic Start/Stop Operations</strong>", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md177", null ],
          [ "<strong>Test 3: Period Validation and Edge Cases</strong>", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md178", null ],
          [ "<strong>Test 4: Callback Validation and User Data</strong>", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md179", null ],
          [ "<strong>Test 5: Statistics and Diagnostics</strong>", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md180", null ],
          [ "<strong>Test 6: Error Conditions and Edge Cases</strong>", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md181", null ],
          [ "<strong>Test 7: Stress Testing and Performance</strong>", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md182", null ],
          [ "<strong>Test 8: Timer Information and Capabilities</strong>", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md183", null ],
          [ "<strong>Test 9: Memory and Resource Management</strong>", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md184", null ]
        ] ],
        [ "Test Data Validation", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md185", null ],
        [ "Advanced Test Features", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md186", null ]
      ] ],
      [ "Test Results and Expectations", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md187", [
        [ "Expected Test Outcomes", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md188", [
          [ "<strong>Tests Expected to PASS</strong> ✅", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md189", null ],
          [ "<strong>Tests Expected to FAIL or Show Limitations</strong> ⚠️", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md190", null ],
          [ "<strong>Tests Providing Implementation Insights</strong> 📊", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md191", null ]
        ] ]
      ] ],
      [ "Recommendations for Implementation Improvement", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md192", [
        [ "1. Enhanced Statistics Implementation", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md193", null ],
        [ "2. Diagnostics Enhancement", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md194", null ],
        [ "3. ISR Dispatch Support", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md195", null ],
        [ "4. Sleep Mode Integration", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md196", null ]
      ] ],
      [ "Usage Instructions", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md197", [
        [ "Running the Comprehensive Test", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md198", null ],
        [ "Expected Output Format", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md199", null ]
      ] ],
      [ "Conclusion", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md200", [
        [ "Key Achievements", "md_examples_2esp32_2docs_2TIMER__TEST__REPORT.html#autotoc_md201", null ]
      ] ]
    ] ],
    [ "ESP32 HardFOC Interface Wrapper - Build Configuration Guide", "md_examples_2esp32_2README__BUILD__SYSTEM.html", [
      [ "Overview", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md203", null ],
      [ "Quick Start", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md204", [
        [ "Using the Build Scripts", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md205", [
          [ "Windows (PowerShell)", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md206", null ],
          [ "Linux/macOS (Bash)", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md207", null ]
        ] ]
      ] ],
      [ "Detailed Examples", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md208", [
        [ "1. ASCII Art Generator (<tt>ascii_art</tt>)", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md209", null ],
        [ "2. Peripheral Test Suites", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md210", null ],
        [ "3. Bluetooth Test Suite (<tt>bluetooth_test</tt>)", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md211", null ],
        [ "4. Utilities Test Suite (<tt>utils_test</tt>)", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md212", null ]
      ] ],
      [ "Build System Architecture", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md213", [
        [ "Flexible CMakeLists.txt", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md214", null ],
        [ "Command Line Usage", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md215", null ]
      ] ],
      [ "Build Types", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md216", null ],
      [ "Target Configuration", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md217", null ],
      [ "Troubleshooting", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md218", [
        [ "Common Issues", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md219", null ],
        [ "Getting Help", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md220", null ]
      ] ],
      [ "Advanced Usage", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md221", [
        [ "Custom Builds", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md222", null ],
        [ "CI/CD Integration", "md_examples_2esp32_2README__BUILD__SYSTEM.html#autotoc_md223", null ]
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
"BaseNvs_8h.html#a0a4f43b41760df506a65240a5ca415a4ac4acea1b622467a0e604b7a47f720ef1",
"BaseTemperature_8h.html#aee8dd042d1f1740b2a23650bbe6efe12a06110442d989e9e0d9e1b4c37dd9c82e",
"EspTypes__I2C_8h.html#a02c3031bc44ca85dff4ad7f483bc4dde",
"EspTypes__WiFi_8h.html#abd0498639fd9b5cc8ea3399376a1ae50a9e15acae6216318c114df902b556af21",
"NvsComprehensiveTest_8cpp.html#a98569769c995f605aa1ce223b447908a",
"classBaseAdc.html#a0b37c5f9bbeb2dfdd81a879570abe58d",
"classBaseNvs.html#a2784f0924a79ef398f6c42227197d87d",
"classBaseWifi.html#a6b722385f7ca418d40f17c853f1483b4",
"classEspI2cDevice.html#af8a169703da9511e1ee20957ff4062ab",
"classEspTemperature.html#a629744fc883c865e06a3f2369b577175",
"functions_func_r.html",
"group__gpio.html#ggaf02cdaf150fa829e4a871e58ed772c6dab2e7e26dbb35ac48971828008f86b356",
"md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md126",
"structesp__temp__state__t.html#a7ea6386f0990755a7e6569081a701bfa",
"structhf__i2c__custom__command__t.html#a67428d9b9c1eb4c41cc8d6a115983734",
"structhf__pwm__capabilities__t.html#a391016f7b9de8439103097b0775a50ea",
"structhf__uart__config__t.html#afdc6f2316977e5a7a400ce2031e49c1c"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';