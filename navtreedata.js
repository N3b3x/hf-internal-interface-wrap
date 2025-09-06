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
    [ "🧪 ESP32 Test Documentation", "index.html", "index" ],
    [ "ESP32-C6 ADC Comprehensive Testing Suite", "md_examples_2esp32_2docs_2README__ADC__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md45", null ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md46", [
        [ "Target Hardware", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md47", null ],
        [ "Required Components", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md48", null ],
        [ "Hardware Setup", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md49", [
          [ "Channel Configuration", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md50", null ],
          [ "Circuit Connections", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md51", null ]
        ] ]
      ] ],
      [ "Test Suite Structure", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md52", [
        [ "1. Hardware Validation Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md53", null ],
        [ "2. ADC Initialization Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md54", null ],
        [ "3. Channel Configuration Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md55", null ],
        [ "4. Basic Conversion Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md56", null ],
        [ "5. Calibration Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md57", null ],
        [ "6. Multiple Channels Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md58", null ],
        [ "7. Averaging Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md59", null ],
        [ "8. Continuous Mode Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md60", null ],
        [ "9. Monitor Threshold Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md61", null ],
        [ "10. Error Handling Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md62", null ],
        [ "11. Statistics and Diagnostics Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md63", null ],
        [ "12. Performance Test", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md64", null ]
      ] ],
      [ "Building and Running Tests", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md65", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md66", null ],
        [ "Build Commands", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md67", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md68", [
        [ "Successful Test Run", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md69", null ],
        [ "Monitor Threshold Test Details", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md70", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md71", [
        [ "Hardware Connection Issues", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md72", [
          [ "GPIO3 Reading Incorrect", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md73", null ],
          [ "GPIO1 Reading Too High", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md74", null ],
          [ "GPIO0 Potentiometer Issues", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md75", null ]
        ] ],
        [ "Test Failure Analysis", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md76", [
          [ "Monitor Test No Events", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md77", null ],
          [ "Performance Issues", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md78", null ],
          [ "Calibration Failures", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md79", null ]
        ] ]
      ] ],
      [ "Test Configuration", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md80", [
        [ "ADC Configuration Used", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md81", null ],
        [ "Expected Performance", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md82", null ]
      ] ],
      [ "Architecture Notes", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md83", null ],
      [ "Related Documentation", "md_examples_2esp32_2docs_2README__ADC__TEST.html#autotoc_md84", null ]
    ] ],
    [ "ESP32-C6 ASCII Art Generator Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md86", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md87", [
        [ "Core ASCII Art Generation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md88", null ],
        [ "Advanced Features", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md89", null ],
        [ "Output Quality", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md90", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md91", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md92", null ],
        [ "Connections", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md93", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md94", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md95", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md96", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md97", [
          [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md98", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md99", null ]
        ] ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md100", [
        [ "1. Basic ASCII Art Generation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md101", null ],
        [ "2. Uppercase Conversion", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md102", null ],
        [ "3. Special Characters", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md103", null ],
        [ "4. Numbers and Symbols", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md104", null ],
        [ "5. Empty and Edge Cases", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md105", null ],
        [ "6. Custom Character Management", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md106", null ],
        [ "7. Character Support Validation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md107", null ],
        [ "8. Supported Characters List", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md108", null ],
        [ "9. Complex Text Generation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md109", null ],
        [ "10. Performance and Stability", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md110", null ]
      ] ],
      [ "ASCII Art Character Set", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md111", [
        [ "Supported Characters", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md112", [
          [ "Letters (A-Z)", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md113", null ],
          [ "Numbers (0-9)", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md114", null ],
          [ "Special Characters", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md115", null ]
        ] ],
        [ "Character Height and Width", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md116", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md117", [
        [ "Successful Execution Output", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md118", null ],
        [ "Performance Metrics", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md119", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md120", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md121", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md122", [
          [ "Build Failures", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md123", null ],
          [ "Runtime Issues", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md124", null ],
          [ "Serial Monitor Issues", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md125", null ]
        ] ],
        [ "Debug Mode Configuration", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md126", null ]
      ] ],
      [ "Integration Examples", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md127", [
        [ "Basic ASCII Art Generation", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md128", null ],
        [ "Advanced Usage with Custom Characters", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md129", null ],
        [ "Performance-Optimized Usage", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md130", null ]
      ] ],
      [ "API Reference", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md131", [
        [ "Core Functions", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md132", null ],
        [ "Advanced Functions", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md133", null ]
      ] ],
      [ "Character Pattern Format", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md134", [
        [ "Standard Pattern Structure", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md135", null ],
        [ "Design Guidelines", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md136", null ],
        [ "Custom Pattern Requirements", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md137", null ]
      ] ],
      [ "Embedded Development Best Practices", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md138", [
        [ "Memory Optimization", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md139", null ],
        [ "Performance Considerations", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md140", null ],
        [ "Real-time Constraints", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md141", null ]
      ] ],
      [ "Applications and Use Cases", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md142", [
        [ "System Status Display", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md143", null ],
        [ "User Interface Elements", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md144", null ],
        [ "Debug and Development", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md145", null ]
      ] ],
      [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md146", [
        [ "Automated Testing", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md147", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__ASCII__ART__TEST.html#autotoc_md148", null ]
    ] ],
    [ "CAN Comprehensive Test Documentation", "md_examples_2esp32_2docs_2README__CAN__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md150", null ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md151", [
        [ "Required Components", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md152", null ],
        [ "Optional Components", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md153", null ]
      ] ],
      [ "Wiring Configuration", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md154", [
        [ "ESP32-C6 + SN65 Transceiver", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md155", null ],
        [ "CAN Bus Connections", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md156", null ],
        [ "External Loopback Testing", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md157", null ]
      ] ],
      [ "Test Configuration", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md158", [
        [ "Pin Configuration", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md159", null ],
        [ "Test Sections", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md160", null ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md161", [
        [ "1. Core Tests", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md162", null ],
        [ "2. Advanced Tests", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md163", null ],
        [ "3. Error Tests", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md164", null ],
        [ "4. Performance Tests", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md165", null ],
        [ "5. Transceiver Tests", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md166", null ]
      ] ],
      [ "Loopback Modes", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md167", [
        [ "Internal Loopback", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md168", null ],
        [ "External Loopback", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md169", null ]
      ] ],
      [ "Test Execution", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md170", [
        [ "Running Tests", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md171", null ],
        [ "Monitoring Progress", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md172", null ],
        [ "Expected Results", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md173", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md174", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md175", [
          [ "Initialization Failures", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md176", null ],
          [ "Message Transmission Errors", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md177", null ],
          [ "Signal Quality Issues", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md178", null ]
        ] ],
        [ "Error Codes", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md179", null ]
      ] ],
      [ "Performance Metrics", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md180", [
        [ "Expected Performance", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md181", null ],
        [ "Monitoring", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md182", null ]
      ] ],
      [ "Integration Notes", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md183", [
        [ "ESP-IDF v5.5 Compatibility", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md184", null ],
        [ "SN65 Transceiver Features", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md185", null ]
      ] ],
      [ "Related Documentation", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md186", null ],
      [ "Test Results Interpretation", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md187", [
        [ "Success Criteria", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md188", null ],
        [ "Failure Analysis", "md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md189", null ]
      ] ]
    ] ],
    [ "DigitalOutputGuard (DOG) Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__DOG__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md192", null ],
      [ "Test Configuration", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md193", [
        [ "App Type", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md194", null ],
        [ "Test GPIO Pins", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md195", null ]
      ] ],
      [ "Test Sections", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md196", [
        [ "1. Basic Tests (<tt>ENABLE_BASIC_TESTS</tt>)", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md197", null ],
        [ "2. Constructor Tests (<tt>ENABLE_CONSTRUCTOR_TESTS</tt>)", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md198", null ],
        [ "3. State Tests (<tt>ENABLE_STATE_TESTS</tt>)", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md199", null ],
        [ "4. Move Semantics Tests (<tt>ENABLE_MOVE_SEMANTICS_TESTS</tt>)", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md200", null ],
        [ "5. Edge Case Tests (<tt>ENABLE_EDGE_CASE_TESTS</tt>)", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md201", null ],
        [ "6. Concurrent Tests (<tt>ENABLE_CONCURRENT_TESTS</tt>)", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md202", null ],
        [ "7. Performance Tests (<tt>ENABLE_PERFORMANCE_TESTS</tt>)", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md203", null ]
      ] ],
      [ "Performance Metrics", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md204", [
        [ "Expected Performance Thresholds", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md205", null ],
        [ "Typical Performance (ESP32-C6)", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md206", null ]
      ] ],
      [ "Progress Indicators", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md207", [
        [ "GPIO14 Test Progress Indicator", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md208", null ],
        [ "Test Output Format", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md209", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md210", [
        [ "Build the Test", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md211", null ],
        [ "Flash and Monitor", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md212", null ],
        [ "Flash and Monitor (Combined)", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md213", null ]
      ] ],
      [ "Test Output Example", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md214", null ],
      [ "Test Configuration", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md215", [
        [ "Enabling/Disabling Test Sections", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md216", null ]
      ] ],
      [ "Performance Interpretation", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md217", [
        [ "Excellent Performance Indicators", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md218", null ],
        [ "Performance Degradation Warnings", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md219", null ]
      ] ],
      [ "Test Coverage", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md220", [
        [ "RAII Pattern Validation", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md221", null ],
        [ "GPIO State Management", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md222", null ],
        [ "Constructor Variants", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md223", null ],
        [ "Move Semantics", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md224", null ],
        [ "Edge Cases", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md225", null ],
        [ "Concurrent Access", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md226", null ],
        [ "Performance Testing", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md227", null ]
      ] ],
      [ "Integration with Test Framework", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md228", null ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md229", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md230", null ],
        [ "Debug Information", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md231", null ]
      ] ],
      [ "Related Documentation", "md_examples_2esp32_2docs_2README__DOG__TEST.html#autotoc_md232", null ]
    ] ],
    [ "ESP32-C6 GPIO Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__GPIO__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md234", [
        [ "Core Features", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md235", null ],
        [ "Advanced Features (ESP32-C6 Specific)", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md236", null ],
        [ "Performance & Robustness", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md237", null ]
      ] ],
      [ "Hardware Setup", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md238", [
        [ "ESP32-C6 DevKit-M-1 Pin Layout", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md239", null ],
        [ "Optional Physical Connections", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md240", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md241", [
        [ "Build Commands", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md242", null ],
        [ "CI Pipeline", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md243", null ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md244", [
        [ "1. Basic Functionality Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md245", null ],
        [ "2. Configuration Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md246", null ],
        [ "3. Advanced Feature Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md247", null ],
        [ "4. ESP32-C6 Specific Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md248", null ],
        [ "5. Robustness Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md249", null ],
        [ "6. Performance Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md250", null ]
      ] ],
      [ "Expected Output", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md251", null ],
      [ "Development Notes", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md252", [
        [ "Adding New Tests", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md253", null ],
        [ "Test Patterns", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md254", null ],
        [ "Debugging", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md255", null ]
      ] ],
      [ "Integration with Main Project", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md256", null ],
      [ "Contributing", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md257", null ],
      [ "License", "md_examples_2esp32_2docs_2README__GPIO__TEST.html#autotoc_md258", null ]
    ] ],
    [ "ESP32 I2C Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__I2C__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md260", null ],
      [ "Quick Test Information", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md261", [
        [ "Test File Location", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md262", null ],
        [ "Running the Tests", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md263", null ],
        [ "Test Categories (24 Total)", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md264", [
          [ "Core Functionality (10 tests)", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md265", null ],
          [ "Advanced Features (8 tests)", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md266", null ],
          [ "New Features (6 tests)", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md267", null ]
        ] ],
        [ "Expected Output", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md268", null ],
        [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md269", null ],
        [ "Test Configuration", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md270", null ]
      ] ],
      [ "For Complete Documentation", "md_examples_2esp32_2docs_2README__I2C__TEST.html#autotoc_md271", null ]
    ] ],
    [ "ESP32-C6 Logger Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md273", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md274", [
        [ "Core Logging Functionality", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md275", null ],
        [ "Advanced Features", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md276", null ],
        [ "Monitoring & Diagnostics", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md277", null ],
        [ "Configuration & Management", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md278", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md279", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md280", null ],
        [ "Connections", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md281", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md282", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md283", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md284", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md285", [
          [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md286", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md287", null ]
        ] ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md288", [
        [ "1. Construction and Initialization Tests", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md289", null ],
        [ "2. Basic Logging Operations", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md290", null ],
        [ "3. Level Management", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md291", null ],
        [ "4. Formatted Logging", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md292", null ],
        [ "5. ESP-IDF Log V2 Features", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md293", null ],
        [ "6. Buffer Logging", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md294", null ],
        [ "7. Location Logging", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md295", null ],
        [ "8. Statistics and Diagnostics", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md296", null ],
        [ "9. Error Handling", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md297", null ],
        [ "10. Performance Testing", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md298", null ],
        [ "11. Utility Functions", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md299", null ],
        [ "12. Cleanup Operations", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md300", null ]
      ] ],
      [ "Configuration Options", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md301", [
        [ "Logger Configuration Structure", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md302", null ],
        [ "Key Configuration Parameters", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md303", null ],
        [ "Log Levels", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md304", null ],
        [ "Output Destinations", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md305", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md306", [
        [ "Successful Execution Output", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md307", null ],
        [ "Performance Metrics", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md308", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md309", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md310", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md311", [
          [ "Build Failures", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md312", null ],
          [ "Runtime Issues", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md313", null ],
          [ "Serial Monitor Issues", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md314", null ]
        ] ],
        [ "Debug Mode Configuration", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md315", null ]
      ] ],
      [ "Integration Examples", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md316", [
        [ "Basic Logger Usage", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md317", null ],
        [ "Advanced Features", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md318", null ]
      ] ],
      [ "API Reference", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md319", [
        [ "Core Functions", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md320", null ],
        [ "Advanced Functions", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md321", null ]
      ] ],
      [ "Embedded Development Best Practices", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md322", [
        [ "Performance Optimization", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md323", null ],
        [ "Memory Management", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md324", null ],
        [ "Real-time Considerations", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md325", null ]
      ] ],
      [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md326", [
        [ "Automated Testing", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md327", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__LOGGER__TEST.html#autotoc_md328", null ]
    ] ],
    [ "ESP32-C6 NVS Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__NVS__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md330", null ],
      [ "Test Coverage", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md331", null ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md332", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md333", null ],
        [ "Connections", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md334", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md335", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md336", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md337", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md338", [
          [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md339", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md340", null ]
        ] ]
      ] ],
      [ "Running the Test", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md341", [
        [ "Expected Output", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md342", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md343", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md344", null ]
      ] ],
      [ "Configuration", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md345", null ],
      [ "Customization", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md346", null ],
      [ "Notes", "md_examples_2esp32_2docs_2README__NVS__TEST.html#autotoc_md347", null ]
    ] ],
    [ "ESP32-C6 PIO Comprehensive Test Suite Documentation", "md_examples_2esp32_2docs_2README__PIO__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md349", [
        [ "Supported ESP32 Variants", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md350", null ]
      ] ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md351", [
        [ "Core Functionality", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md352", null ],
        [ "ESP32 Variant-Specific Features", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md353", null ],
        [ "Advanced RMT Features", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md354", null ],
        [ "WS2812 LED Protocol Testing", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md355", null ],
        [ "Automated Testing & Diagnostics", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md356", null ],
        [ "Performance & Stress Testing", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md357", null ]
      ] ],
      [ "Hardware Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md358", [
        [ "ESP32-C6-DevKitM-1 Pin Configuration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md359", null ],
        [ "Automated Testing Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md360", null ],
        [ "WS2812 LED Testing Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md361", null ],
        [ "Test Progression Indicator", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md362", null ],
        [ "External WS2812 LED Chain (Optional)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md363", null ],
        [ "Logic Analyzer Setup", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md364", null ]
      ] ],
      [ "Running the Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md365", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md366", null ],
        [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md367", null ],
        [ "Direct ESP-IDF Build (Alternative)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md368", null ],
        [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md369", null ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md370", [
        [ "1. ESP32 Variant Information Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md371", null ],
        [ "2. Constructor/Destructor Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md372", null ],
        [ "3. Lifecycle Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md373", null ],
        [ "4. Channel Configuration Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md374", null ],
        [ "5. Transmission Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md375", null ],
        [ "6. WS2812 LED Protocol Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md376", null ],
        [ "7. Logic Analyzer Test Scenarios", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md377", null ],
        [ "8. Advanced RMT Feature Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md378", null ],
        [ "9. Loopback and Reception Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md379", null ],
        [ "10. Callback Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md380", null ],
        [ "11. Statistics and Diagnostics Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md381", null ],
        [ "12. Stress and Performance Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md382", null ],
        [ "13. System Validation Tests", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md383", null ]
      ] ],
      [ "WS2812 Protocol Specifications", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md384", [
        [ "Timing Requirements", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md385", null ],
        [ "Color Format", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md386", null ],
        [ "Comprehensive Test Colors", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md387", [
          [ "Primary Colors (Maximum Brightness)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md388", null ],
          [ "Secondary Colors", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md389", null ],
          [ "White Variations", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md390", null ],
          [ "Bit Pattern Test Values", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md391", null ],
          [ "Rainbow Transition", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md392", null ]
        ] ]
      ] ],
      [ "Logic Analyzer Test Patterns", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md393", [
        [ "Pattern 1: Basic Timing Test", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md394", null ],
        [ "Pattern 2: Frequency Sweep", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md395", null ],
        [ "Pattern 3: Test Progression Monitoring", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md396", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md397", [
        [ "Successful Test Output", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md398", null ],
        [ "Built-in RGB LED Verification", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md399", null ],
        [ "Automated Loopback Verification", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md400", null ],
        [ "Test Progression Monitoring", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md401", null ],
        [ "Logic Analyzer Verification", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md402", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md403", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md404", [
          [ "Test Failures", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md405", null ],
          [ "Built-in RGB LED Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md406", null ],
          [ "Test Progression Indicator Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md407", null ],
          [ "Loopback Testing Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md408", null ],
          [ "WS2812 Testing Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md409", null ],
          [ "Logic Analyzer Issues", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md410", null ]
        ] ],
        [ "Debug Mode", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md411", null ]
      ] ],
      [ "Performance Metrics", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md412", [
        [ "Build Information", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md413", null ],
        [ "Typical Results (ESP32-C6 @ 160MHz)", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md414", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md415", null ]
      ] ],
      [ "Integration with Development Workflow", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md416", [
        [ "Continuous Integration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md417", null ],
        [ "Hardware-in-the-Loop Testing", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md418", null ]
      ] ],
      [ "Advanced Configuration", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md419", [
        [ "Custom GPIO Pins", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md420", null ],
        [ "Timing Resolution", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md421", null ],
        [ "Test Parameters", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md422", null ]
      ] ],
      [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md423", [
        [ "RMT Peripheral", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md424", null ],
        [ "Built-in RGB LED", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md425", null ],
        [ "Testing Advantages", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md426", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md427", null ]
    ] ],
    [ "ESP32 Family PWM Comprehensive Test Suite Documentation", "md_examples_2esp32_2docs_2README__PWM__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md429", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md430", [
        [ "Core PWM Functionality", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md431", null ],
        [ "Advanced PWM Features", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md432", null ],
        [ "LEDC Peripheral Validation (ESP32 Family)", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md433", null ],
        [ "ESP32 Variant-Specific Features", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md434", null ],
        [ "System Integration & Diagnostics", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md435", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md436", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md437", null ],
        [ "PWM Output Pins", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md438", null ],
        [ "Logic Analyzer Setup", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md439", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md440", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md441", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md442", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md443", [
          [ "Using ESP-IDF directly", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md444", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md445", null ],
          [ "Available Example Script Options", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md446", null ]
        ] ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md447", [
        [ "1. Constructor/Destructor Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md448", [
          [ "<tt>test_constructor_default()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md449", null ],
          [ "<tt>test_destructor_cleanup()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md450", null ]
        ] ],
        [ "2. Lifecycle Management Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md451", [
          [ "<tt>test_initialization_states()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md452", null ],
          [ "<tt>test_lazy_initialization()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md453", null ]
        ] ],
        [ "3. Configuration Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md454", [
          [ "<tt>test_mode_configuration()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md455", null ],
          [ "<tt>test_clock_source_configuration()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md456", null ]
        ] ],
        [ "4. Channel Management Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md457", [
          [ "<tt>test_channel_configuration()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md458", null ],
          [ "<tt>test_channel_enable_disable()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md459", null ]
        ] ],
        [ "5. PWM Control Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md460", [
          [ "<tt>test_duty_cycle_control()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md461", null ],
          [ "<tt>test_frequency_control()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md462", null ],
          [ "<tt>test_phase_shift_control()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md463", null ]
        ] ],
        [ "6. Advanced Features Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md464", [
          [ "<tt>test_synchronized_operations()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md465", null ],
          [ "<tt>test_complementary_outputs()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md466", null ]
        ] ],
        [ "7. ESP32-Specific Features Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md467", [
          [ "<tt>test_hardware_fade()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md468", null ],
          [ "<tt>test_idle_level_control()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md469", null ],
          [ "<tt>test_timer_management()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md470", null ]
        ] ],
        [ "8. Status and Diagnostics Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md471", [
          [ "<tt>test_status_reporting()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md472", null ],
          [ "<tt>test_statistics_and_diagnostics()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md473", null ]
        ] ],
        [ "9. Callback Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md474", [
          [ "<tt>test_callbacks()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md475", null ]
        ] ],
        [ "10. Edge Cases and Stress Tests", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md476", [
          [ "<tt>test_edge_cases()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md477", null ],
          [ "<tt>test_stress_scenarios()</tt>", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md478", null ]
        ] ]
      ] ],
      [ "Logic Analyzer Analysis Guide", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md479", [
        [ "Key Measurements", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md480", null ],
        [ "Logic Analyzer Measurement Guidelines", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md481", [
          [ "Key Measurement Techniques", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md482", null ]
        ] ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md483", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md484", null ],
        [ "Performance Optimization", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md485", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md486", [
        [ "Success Criteria", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md487", null ],
        [ "Typical Test Sequence Timing", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md488", null ],
        [ "Hardware Validation", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md489", null ]
      ] ],
      [ "Conclusion", "md_examples_2esp32_2docs_2README__PWM__TEST.html#autotoc_md490", null ]
    ] ],
    [ "ESP32-C6 SPI Comprehensive Test Suite Documentation", "md_examples_2esp32_2docs_2README__SPI__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md492", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md493", [
        [ "Core SPI Functionality", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md494", null ],
        [ "Advanced SPI Features", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md495", null ],
        [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md496", null ],
        [ "Direct API Comparison", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md497", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md498", [
        [ "ESP32-C6 DevKit-M-1 Pin Layout", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md499", null ],
        [ "Optional Hardware", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md500", null ]
      ] ],
      [ "Test Configuration", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md501", [
        [ "Default Test Settings", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md502", null ],
        [ "Test Section Configuration", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md503", null ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md504", [
        [ "Core Functionality (8 tests)", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md505", null ],
        [ "Advanced Features (8 tests)", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md506", null ],
        [ "Transfer Size Testing (4 tests)", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md507", null ],
        [ "Direct API Comparison (2 tests)", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md508", null ]
      ] ],
      [ "Expected Output", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md509", [
        [ "Test Suite Header", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md510", null ]
      ] ],
      [ "Logic Analyzer Requirements", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md511", [
        [ "Sampling Rate Requirements", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md512", [
          [ "Minimum Requirements", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md513", null ],
          [ "Recommended Requirements", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md514", null ]
        ] ],
        [ "Mathematical Justification", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md515", [
          [ "Nyquist Rate", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md516", null ],
          [ "Oversampling Benefits", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md517", null ]
        ] ],
        [ "Key Considerations", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md518", [
          [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md519", null ],
          [ "Signal Analysis", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md520", null ]
        ] ]
      ] ],
      [ "Running the Tests", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md521", [
        [ "Build and Flash", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md522", null ],
        [ "Test Execution", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md523", null ],
        [ "Monitoring Output", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md524", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md525", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md526", [
          [ "Build Errors", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md527", null ],
          [ "Runtime Errors", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md528", null ],
          [ "Logic Analyzer Issues", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md529", null ]
        ] ],
        [ "Debug Information", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md530", null ]
      ] ],
      [ "Performance Characteristics", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md531", [
        [ "Transfer Performance", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md532", null ],
        [ "Clock Performance", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md533", null ]
      ] ],
      [ "For Complete Documentation", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md534", null ],
      [ "Navigation", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md535", [
        [ "<strong>Documentation Structure</strong>", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md536", null ],
        [ "<strong>Related Documentation</strong>", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md537", null ],
        [ "<strong>Navigation Links</strong>", "md_examples_2esp32_2docs_2README__SPI__TEST.html#autotoc_md538", null ]
      ] ]
    ] ],
    [ "ESP32-C6 Temperature Sensor Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md540", null ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md541", [
        [ "Core Temperature Functionality", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md542", null ],
        [ "Advanced Monitoring Features", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md543", null ],
        [ "System Integration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md544", null ],
        [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md545", null ]
      ] ],
      [ "Hardware Requirements", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md546", [
        [ "Supported Platforms", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md547", null ],
        [ "Temperature Sensor", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md548", null ],
        [ "Connections", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md549", null ]
      ] ],
      [ "Building and Running", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md550", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md551", null ],
        [ "Quick Start", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md552", null ],
        [ "Alternative Build Methods", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md553", [
          [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md554", null ],
          [ "Debug Build for Development", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md555", null ]
        ] ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md556", [
        [ "1. Sensor Initialization Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md557", null ],
        [ "2. Basic Temperature Reading Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md558", null ],
        [ "3. Sensor Information Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md559", null ],
        [ "4. Range Management Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md560", null ],
        [ "5. Threshold Monitoring Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md561", null ],
        [ "6. Continuous Monitoring Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md562", null ],
        [ "7. Calibration Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md563", null ],
        [ "8. Power Management Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md564", null ],
        [ "9. Self-Test and Health Monitoring", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md565", null ],
        [ "10. Statistics and Diagnostics", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md566", null ],
        [ "11. ESP32-Specific Features", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md567", null ],
        [ "12. Error Handling Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md568", null ],
        [ "13. Performance and Stress Tests", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md569", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md570", [
        [ "Successful Execution Output", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md571", null ],
        [ "Performance Metrics", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md572", null ],
        [ "Accuracy Specifications", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md573", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md574", null ]
      ] ],
      [ "Configuration Options", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md575", [
        [ "Temperature Sensor Configuration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md576", null ],
        [ "Monitoring Configuration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md577", null ],
        [ "Power Management", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md578", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md579", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md580", [
          [ "Build Failures", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md581", null ],
          [ "Runtime Issues", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md582", null ],
          [ "Calibration Issues", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md583", null ]
        ] ],
        [ "Debug Mode Configuration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md584", null ]
      ] ],
      [ "Integration Examples", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md585", [
        [ "Basic Temperature Monitoring", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md586", null ],
        [ "Advanced Monitoring with Callbacks", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md587", null ],
        [ "Calibration and Accuracy Improvement", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md588", null ]
      ] ],
      [ "API Reference", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md589", [
        [ "Core Functions", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md590", null ],
        [ "Advanced Functions", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md591", null ]
      ] ],
      [ "Embedded Development Best Practices", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md592", [
        [ "Performance Optimization", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md593", null ],
        [ "Memory Management", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md594", null ],
        [ "Real-time Considerations", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md595", null ]
      ] ],
      [ "Applications and Use Cases", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md596", [
        [ "Environmental Monitoring", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md597", null ],
        [ "Thermal Protection", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md598", null ],
        [ "Data Logging", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md599", null ]
      ] ],
      [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md600", [
        [ "Automated Testing", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md601", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__TEMPERATURE__TEST.html#autotoc_md602", null ]
    ] ],
    [ "ESP32-C6 UART Comprehensive Test Suite Documentation", "md_examples_2esp32_2docs_2README__UART__TESTING.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md604", [
        [ "Supported ESP32 Variants", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md605", null ]
      ] ],
      [ "Features Tested", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md606", [
        [ "Core Functionality", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md607", null ],
        [ "Advanced UART Features", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md608", null ],
        [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md609", null ],
        [ "Pattern Detection Testing", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md610", null ],
        [ "Testing Infrastructure", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md611", null ]
      ] ],
      [ "Hardware Setup", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md612", [
        [ "ESP32-C6-DevKitM-1 Pin Configuration", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md613", null ],
        [ "External Loopback Testing Setup", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md614", null ],
        [ "Test Progression Indicator", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md615", null ],
        [ "Logic Analyzer Setup", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md616", null ]
      ] ],
      [ "Running the Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md617", [
        [ "Prerequisites", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md618", null ],
        [ "Using Build Scripts (Recommended)", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md619", null ],
        [ "Direct ESP-IDF Build (Alternative)", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md620", null ],
        [ "CI/CD Integration", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md621", null ]
      ] ],
      [ "Test Categories", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md622", [
        [ "1. Constructor/Destructor Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md623", null ],
        [ "2. Basic Communication Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md624", null ],
        [ "3. Advanced Features Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md625", null ],
        [ "4. Async Operations Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md626", null ],
        [ "5. Statistics and Diagnostics Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md627", null ],
        [ "6. ESP32-C6 Specific Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md628", null ],
        [ "7. Event-Driven Pattern Detection Tests", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md629", null ]
      ] ],
      [ "Pattern Detection Specifications", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md630", [
        [ "ESP-IDF v5.5 Pattern Detection", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md631", null ],
        [ "Pattern Detection Parameters", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md632", null ],
        [ "Timing Optimization", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md633", null ]
      ] ],
      [ "Expected Test Results", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md634", [
        [ "Successful Test Output", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md635", null ],
        [ "Pattern Detection Test Results", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md636", null ],
        [ "Event-Driven Pattern Detection Results", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md637", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md638", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md639", [
          [ "Test Failures", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md640", null ],
          [ "Pattern Detection Issues", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md641", null ],
          [ "External Loopback Issues", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md642", null ],
          [ "Test Progression Indicator Issues", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md643", null ],
          [ "UART Configuration Issues", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md644", null ]
        ] ],
        [ "Debug Mode", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md645", null ],
        [ "Pattern Detection Debugging", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md646", null ]
      ] ],
      [ "Performance Metrics", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md647", [
        [ "Build Information", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md648", null ],
        [ "Typical Results (ESP32-C6 @ 160MHz)", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md649", null ],
        [ "Memory Usage", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md650", null ]
      ] ],
      [ "Integration with Development Workflow", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md651", [
        [ "Continuous Integration", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md652", null ],
        [ "Hardware-in-the-Loop Testing", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md653", null ]
      ] ],
      [ "Advanced Configuration", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md654", [
        [ "Custom UART Ports", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md655", null ],
        [ "Pattern Detection Configuration", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md656", null ],
        [ "Event Queue Configuration", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md657", null ],
        [ "Test Parameters", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md658", null ]
      ] ],
      [ "ESP32-C6 Specific Features", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md659", [
        [ "UART Peripheral", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md660", null ],
        [ "Pattern Detection Capabilities", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md661", null ],
        [ "Testing Advantages", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md662", null ],
        [ "ESP32-C6 Specific Observations", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md663", null ]
      ] ],
      [ "References", "md_examples_2esp32_2docs_2README__UART__TESTING.html#autotoc_md664", null ]
    ] ],
    [ "WiFi Comprehensive Test Suite", "md_examples_2esp32_2docs_2README__WIFI__TEST.html", [
      [ "Overview", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md666", null ],
      [ "Test Architecture", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md667", [
        [ "1. Core Tests (Interface-Only)", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md668", null ],
        [ "2. Interface Tests (Interface-Only)", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md669", null ],
        [ "3. Performance Tests (Interface-Only)", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md670", null ],
        [ "4. Functional Tests (Real Hardware)", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md671", null ]
      ] ],
      [ "Test Configuration", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md672", null ],
      [ "Running the Tests", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md673", [
        [ "Build and Flash", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md674", null ],
        [ "Expected Output", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md675", null ]
      ] ],
      [ "Test Results Summary", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md676", [
        [ "Latest Test Run Results", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md677", null ],
        [ "Individual Test Results", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md678", [
          [ "Core Tests", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md679", null ],
          [ "Interface Tests", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md680", null ],
          [ "Performance Tests", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md681", null ],
          [ "Functional Tests", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md682", null ]
        ] ]
      ] ],
      [ "Real Hardware Validation", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md683", [
        [ "Access Point Test", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md684", null ],
        [ "Network Scanning Test", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md685", null ],
        [ "Performance Metrics", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md686", null ]
      ] ],
      [ "Test Framework Features", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md687", [
        [ "Task-Based Testing", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md688", null ],
        [ "Timeout Protection", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md689", null ],
        [ "Comprehensive Logging", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md690", null ],
        [ "Error Handling", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md691", null ],
        [ "Memory Management", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md692", null ]
      ] ],
      [ "Network Interface Management", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md693", [
        [ "Problem Resolution", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md694", null ],
        [ "Technical Implementation", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md695", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md696", [
        [ "Common Issues", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md697", null ],
        [ "Debug Information", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md698", null ]
      ] ],
      [ "Test Customization", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md699", [
        [ "Adding New Tests", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md700", null ],
        [ "Modifying Test Parameters", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md701", null ],
        [ "Test Environment", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md702", null ]
      ] ],
      [ "Integration with CI/CD", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md703", null ],
      [ "Related Documentation", "md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md704", null ]
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
"BaseI2c_8h_source.html",
"BaseTemperature_8h.html#a9107d93f48c1ae86146f7d60e6226a20aa5531387558432b3177e98383f8a2166",
"EspTypes_8h_source.html",
"EspTypes__PWM_8h.html#ad8fbde8152ecafefd714f3a7eb8344c1",
"I2cComprehensiveTest_8cpp.html#a286364be1181eb6afc457dd0c224b79f",
"PioComprehensiveTest_8cpp.html#af850c4a522d3a608f115ea60fb8f049c",
"WifiComprehensiveTest_8cpp.html#a94ce41073e051577d040ad57caf0081d",
"classBaseLogger.html#a960f90f79007791d7129bd22be151a57",
"classBaseUart.html#aacddae116924093dc2d45a894772a9a6",
"classEspGpio.html#a9661ef14697ca21f3a580add2f3a4623",
"classEspPio.html#adcdcad3a021a42f84cc978c5ac8b81aa",
"classEspUart.html#a42cee776787269a8b0ec70ddd83c0550",
"functions_f.html",
"group__gpio.html#gga2632aac2351807c35e790ec20bda305dae656a8d5b2e8d547c0d976cc883cb601",
"md_examples_2esp32_2docs_2README__CAN__TEST.html#autotoc_md173",
"md_examples_2esp32_2docs_2README__PIO__TEST.html#autotoc_md424",
"md_examples_2esp32_2docs_2README__WIFI__TEST.html#autotoc_md674",
"structesp__temp__state__t.html#aa27ed92ce9a8ac860496978b0a41f614",
"structhf__esp__can__filter__config__t.html#abe6a19952410570b8be6ccdb2e2daf39",
"structhf__pio__channel__statistics__t.html#a6f810ce1ffeaa8e51fb018e6abcdb438",
"structhf__temp__sensor__info__t.html#a6de08587fd4c08f24b025ab985966fe9"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';