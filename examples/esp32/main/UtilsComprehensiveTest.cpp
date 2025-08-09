/**
 * @file UtilsComprehensiveTest.cpp
 * @brief Comprehensive utilities testing suite for ESP32-C6 DevKit-M-1
 *
 * This test suite provides comprehensive testing of utility classes including
 * AsciiArtGenerator and DigitalOutputGuard with thorough coverage of all features,
 * edge cases, and error conditions.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "TestFramework.h"
#include "utils/AsciiArtGenerator.h"

static const char* TAG = "UTILS_Test";

static TestResults g_test_results;

//==============================================================================
// ASCII ART GENERATOR TESTS
//==============================================================================

bool test_ascii_art_generator_creation() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art generator creation...");

  AsciiArtGenerator generator;

  // Test basic creation by generating some text
  auto test_result = generator.Generate("TEST");
  if (test_result.empty()) {
    ESP_LOGE(TAG, "ASCII art generator failed to generate basic text");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art generator creation successful");
  return true;
}

bool test_ascii_art_basic_text() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art basic text generation...");

  AsciiArtGenerator generator;

  // Test simple text
  std::string test_text = "HELLO";
  auto result = generator.Generate(test_text);

  if (result.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for basic text");
    return false;
  }

  // Verify the result has multiple lines (ASCII art should be multi-line)
  size_t newline_count = 0;
  for (char c : result) {
    if (c == '\n')
      newline_count++;
  }
  if (newline_count < 5) { // ASCII art should have at least 5 lines
    ESP_LOGE(TAG, "Generated ASCII art has insufficient lines: %zu", newline_count);
    return false;
  }

  ESP_LOGI(TAG, "Generated ASCII art for 'HELLO':\n%s", result.c_str());
  ESP_LOGI(TAG, "[SUCCESS] ASCII art basic text generation successful");
  return true;
}

bool test_ascii_art_supported_characters() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art supported characters...");

  AsciiArtGenerator generator;

  // Test all supported character types
  std::string test_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?@#$%^&*()[]{}|\\/;:'\"-_=+";

  for (char c : test_chars) {
    if (!generator.IsCharacterSupported(c)) {
      ESP_LOGE(TAG, "Character '%c' not supported but should be", c);
      return false;
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art supported characters test successful");
  return true;
}

bool test_ascii_art_unsupported_characters() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art unsupported characters...");

  AsciiArtGenerator generator;

  // Test some unsupported characters
  std::string unsupported_chars = "áéíóúñçßäöü";

  for (char c : unsupported_chars) {
    if (generator.IsCharacterSupported(c)) {
      ESP_LOGE(TAG, "Character '%c' supported but should not be", c);
      return false;
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art unsupported characters test successful");
  return true;
}

bool test_ascii_art_empty_string() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art empty string handling...");

  AsciiArtGenerator generator;

  // Test empty string
  auto result = generator.Generate("");

  if (!result.empty()) {
    ESP_LOGE(TAG, "Empty string should generate empty result");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art empty string handling successful");
  return true;
}

bool test_ascii_art_mixed_case() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art mixed case handling...");

  AsciiArtGenerator generator;

  // Test mixed case text
  std::string test_text = "Hello World 123!";
  auto result = generator.Generate(test_text);

  if (result.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for mixed case text");
    return false;
  }

  ESP_LOGI(TAG, "Generated ASCII art for mixed case:\n%s", result.c_str());
  ESP_LOGI(TAG, "[SUCCESS] ASCII art mixed case handling successful");
  return true;
}

bool test_ascii_art_special_characters() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art special characters...");

  AsciiArtGenerator generator;

  // Test special characters
  std::string test_text = "!@#$%^&*()";
  auto result = generator.Generate(test_text);

  if (result.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for special characters");
    return false;
  }

  ESP_LOGI(TAG, "Generated ASCII art for special characters:\n%s", result.c_str());
  ESP_LOGI(TAG, "[SUCCESS] ASCII art special characters test successful");
  return true;
}

bool test_ascii_art_long_text() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art long text handling...");

  AsciiArtGenerator generator;

  // Test long text
  std::string test_text = "VERY LONG TEXT THAT SHOULD BE HANDLED PROPERLY";
  auto result = generator.Generate(test_text);

  if (result.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for long text");
    return false;
  }

  // Verify the result is substantial
  if (result.length() < 100) {
    ESP_LOGE(TAG, "Long text generated insufficient output: %zu chars", result.length());
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art long text handling successful");
  return true;
}

bool test_ascii_art_custom_character_management() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art custom character management...");

  AsciiArtGenerator generator;

  // Test adding custom character
  std::vector<std::string> custom_art = {"  ___  ", " /   \\ ", "|     |",
                                         "|     |", " \\___/ ", "       "};

  generator.AddCustomCharacter('X', custom_art);

  if (!generator.IsCharacterSupported('X')) {
    ESP_LOGE(TAG, "Custom character 'X' not supported after addition");
    return false;
  }

  // Test removing custom character
  generator.RemoveCustomCharacter('X');

  if (generator.IsCharacterSupported('X')) {
    ESP_LOGE(TAG, "Custom character 'X' still supported after removal");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art custom character management successful");
  return true;
}

bool test_ascii_art_supported_characters_list() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art supported characters list...");

  AsciiArtGenerator generator;

  // Test getting supported characters
  std::string supported = generator.GetSupportedCharacters();

  if (supported.empty()) {
    ESP_LOGE(TAG, "Supported characters list is empty");
    return false;
  }

  // Verify it contains expected characters
  if (supported.find('A') == std::string::npos) {
    ESP_LOGE(TAG, "Supported characters list missing 'A'");
    return false;
  }

  if (supported.find('0') == std::string::npos) {
    ESP_LOGE(TAG, "Supported characters list missing '0'");
    return false;
  }

  ESP_LOGI(TAG, "Supported characters: %s", supported.c_str());
  ESP_LOGI(TAG, "[SUCCESS] ASCII art supported characters list test successful");
  return true;
}

//==============================================================================
// PERFORMANCE AND STRESS TESTS
//==============================================================================

bool test_ascii_art_performance() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art performance...");

  AsciiArtGenerator generator;

  // Test performance with repeated generation
  const int iterations = 100;
  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < iterations; i++) {
    auto result = generator.Generate("PERFORMANCE TEST");
    if (result.empty()) {
      ESP_LOGE(TAG, "ASCII art generation failed in performance test iteration %d", i);
      return false;
    }
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time = end_time - start_time;
  double avg_time = static_cast<double>(total_time) / iterations;

  ESP_LOGI(TAG, "Performance test: %d iterations in %.2f ms (avg: %.2f us per iteration)",
           iterations, total_time / 1000.0, avg_time);

  if (avg_time > 1000.0) { // Should be less than 1ms per iteration
    ESP_LOGE(TAG, "ASCII art generation too slow: %.2f us per iteration", avg_time);
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art performance test successful");
  return true;
}

bool test_ascii_art_stress() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art stress...");

  AsciiArtGenerator generator;

  // Test stress with many different texts
  const int iterations = 1000;
  std::string test_strings[] = {"A",     "AB",         "ABC",   "ABCD", "ABCDE", "123",  "456",
                                "789",   "0123456789", "!@#",   "$%^",  "&*()",  "[]{}", "|\\/",
                                "HELLO", "WORLD",      "ESP32", "C6",   "TEST"};

  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < iterations; i++) {
    std::string test_text = test_strings[i % (sizeof(test_strings) / sizeof(test_strings[0]))];
    auto result = generator.Generate(test_text);
    if (result.empty()) {
      ESP_LOGE(TAG, "ASCII art generation failed in stress test iteration %d", i);
      return false;
    }
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time = end_time - start_time;
  double avg_time = static_cast<double>(total_time) / iterations;

  ESP_LOGI(TAG, "Stress test: %d iterations in %.2f ms (avg: %.2f us per iteration)", iterations,
           total_time / 1000.0, avg_time);

  if (avg_time > 500.0) { // Should be less than 500us per iteration
    ESP_LOGE(TAG, "ASCII art generation too slow in stress test: %.2f us per iteration", avg_time);
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art stress test successful");
  return true;
}

//==============================================================================
// MAIN TEST EXECUTION
//==============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                ESP32-C6 UTILS COMPREHENSIVE TEST SUITE v1.0                ║");
  ESP_LOGI(TAG, "║                           AsciiArtGenerator Focused                        ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // ASCII Art Generator Tests
  ESP_LOGI(TAG, "\n=== ASCII ART GENERATOR TESTS ===");
  RUN_TEST(test_ascii_art_generator_creation);
  RUN_TEST(test_ascii_art_basic_text);
  RUN_TEST(test_ascii_art_supported_characters);
  RUN_TEST(test_ascii_art_unsupported_characters);
  RUN_TEST(test_ascii_art_empty_string);
  RUN_TEST(test_ascii_art_mixed_case);
  RUN_TEST(test_ascii_art_special_characters);
  RUN_TEST(test_ascii_art_long_text);
  RUN_TEST(test_ascii_art_custom_character_management);
  RUN_TEST(test_ascii_art_supported_characters_list);

  // Performance and Stress Tests
  ESP_LOGI(TAG, "\n=== PERFORMANCE AND STRESS TESTS ===");
  RUN_TEST(test_ascii_art_performance);
  RUN_TEST(test_ascii_art_stress);

  // Print final summary
  print_test_summary(g_test_results, "UTILS", TAG);

  ESP_LOGI(TAG,
           "\n╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG,
           "║                    UTILS COMPREHENSIVE TEST SUITE COMPLETE                    ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  // Keep the system running
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
