/**
 * @file AsciiArtComprehensiveTest.cpp
 * @brief ASCII Art Generator comprehensive example and test suite for ESP32-C6 DevKit-M-1
 * (noexcept)
 *
 * This file contains a comprehensive example and test suite for the AsciiArtGenerator
 * targeting ESP32-C6 with ESP-IDF v5.5+. It provides thorough testing and demonstration
 * of all ASCII art generation functionalities including basic text generation, custom
 * character support, character validation, and edge cases.
 *
 * All functions are noexcept - no exception handling used.
 *
 * NOTE: Linter errors for ESP-IDF headers (ESP_LOGI, ESP_LOGE, etc.) are expected
 * when ESP-IDF is not available in the linter environment. The code will compile
 * correctly when ESP-IDF is properly set up.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include "TestFramework.h"
#include "utils/AsciiArtGenerator.h"

static const char* TAG = "ASCII_ART_Test";

static TestResults g_test_results;

// Forward declarations
bool test_basic_ascii_art_generation() noexcept;
bool test_uppercase_conversion() noexcept;
bool test_special_characters() noexcept;
bool test_numbers_and_symbols() noexcept;
bool test_empty_and_edge_cases() noexcept;
bool test_custom_character_management() noexcept;
bool test_character_support_validation() noexcept;
bool test_supported_characters_list() noexcept;
bool test_complex_text_generation() noexcept;
bool test_performance_and_stability() noexcept;

bool test_basic_ascii_art_generation() noexcept {
  ESP_LOGI(TAG, "Testing basic ASCII art generation...");

  AsciiArtGenerator generator;

  // Test basic word generation
  std::string hello_art = generator.Generate("HELLO");
  if (hello_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for 'HELLO'");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for 'HELLO':\n%s", hello_art.c_str());

  // Test single character
  std::string a_art = generator.Generate("A");
  if (a_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for 'A'");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for 'A':\n%s", a_art.c_str());

  // Test space character
  std::string space_art = generator.Generate(" ");
  if (space_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for space");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for space:\n%s", space_art.c_str());

  // Test multiple spaces
  std::string spaces_art = generator.Generate("   ");
  if (spaces_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for multiple spaces");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for multiple spaces:\n%s", spaces_art.c_str());

  return true;
}

bool test_uppercase_conversion() noexcept {
  ESP_LOGI(TAG, "Testing uppercase conversion...");

  AsciiArtGenerator generator;

  // Test lowercase input
  std::string lowercase_art = generator.Generate("hello");
  if (lowercase_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for lowercase 'hello'");
    return false;
  }

  // Test mixed case input
  std::string mixed_art = generator.Generate("HeLlO");
  if (mixed_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for mixed case 'HeLlO'");
    return false;
  }

  // Both should produce the same result (uppercase)
  if (lowercase_art != mixed_art) {
    ESP_LOGE(TAG,
             "Uppercase conversion failed - lowercase and mixed case produced different results");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Uppercase conversion working correctly");
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for lowercase 'hello':\n%s", lowercase_art.c_str());

  return true;
}

bool test_special_characters() noexcept {
  ESP_LOGI(TAG, "Testing special characters...");

  AsciiArtGenerator generator;

  // Test punctuation
  std::string punctuation_art = generator.Generate("!@#$%");
  if (punctuation_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for punctuation");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for punctuation:\n%s", punctuation_art.c_str());

  // Test brackets
  std::string brackets_art = generator.Generate("()[]{}");
  if (brackets_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for brackets");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for brackets:\n%s", brackets_art.c_str());

  // Test operators
  std::string operators_art = generator.Generate("+-*/=");
  if (operators_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for operators");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for operators:\n%s", operators_art.c_str());

  return true;
}

bool test_numbers_and_symbols() noexcept {
  ESP_LOGI(TAG, "Testing numbers and symbols...");

  AsciiArtGenerator generator;

  // Test digits
  std::string digits_art = generator.Generate("0123456789");
  if (digits_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for digits");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for digits:\n%s", digits_art.c_str());

  // Test individual numbers
  for (char digit = '0'; digit <= '9'; ++digit) {
    std::string digit_art = generator.Generate(std::string(1, digit));
    if (digit_art.empty()) {
      ESP_LOGE(TAG, "Failed to generate ASCII art for digit '%c'", digit);
      return false;
    }
    ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for digit '%c':\n%s", digit, digit_art.c_str());
  }

  return true;
}

bool test_empty_and_edge_cases() noexcept {
  ESP_LOGI(TAG, "Testing empty and edge cases...");

  AsciiArtGenerator generator;

  // Test empty string
  std::string empty_art = generator.Generate("");
  if (!empty_art.empty()) {
    ESP_LOGE(TAG, "Empty string should return empty result, got: '%s'", empty_art.c_str());
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Empty string correctly returns empty result");

  // Test unsupported characters (should be replaced with spaces)
  std::string unsupported_art = generator.Generate("ABC€XYZ");
  if (unsupported_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art with unsupported characters");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art with unsupported characters:\n%s",
           unsupported_art.c_str());

  // Test very long string
  std::string long_string(100, 'A');
  std::string long_art = generator.Generate(long_string);
  if (long_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for long string");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for long string (100 characters)");

  return true;
}

bool test_custom_character_management() noexcept {
  ESP_LOGI(TAG, "Testing custom character management...");

  AsciiArtGenerator generator;

  // Test adding custom character
  std::vector<std::string> custom_char = {"  ___  ", " /   \\ ", "|     |",
                                          "|     |", " \\___/ ", "       "};

  generator.AddCustomCharacter('@', custom_char);

  // Test custom character generation
  std::string custom_art = generator.Generate("TEST@");
  if (custom_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art with custom character");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art with custom character:\n%s", custom_art.c_str());

  // Test multiple custom characters (use characters not in built-in set)
  std::vector<std::string> custom_char2 = {" _____ ", "|     |", "|     |",
                                           "|     |", "|_____|", "       "};

  generator.AddCustomCharacter('\x01', custom_char2); // Use \x01 which is not in our built-in set
  std::string multi_custom_art = generator.Generate("@\x01"); // @ is built-in, \x01 is custom
  if (multi_custom_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art with multiple custom characters");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art with multiple custom characters:\n%s",
           multi_custom_art.c_str());

  // Test removing custom character
  generator.RemoveCustomCharacter('\x01');
  std::string after_remove_art = generator.Generate("TEST\x01");
  if (after_remove_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art after removing custom character");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art after removing custom character:\n%s",
           after_remove_art.c_str());

  // Test clearing all custom characters
  generator.ClearCustomCharacters();
  std::string after_clear_art = generator.Generate("\x01");
  if (after_clear_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art after clearing custom characters");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art after clearing custom characters:\n%s",
           after_clear_art.c_str());

  return true;
}

bool test_character_support_validation() noexcept {
  ESP_LOGI(TAG, "Testing character support validation...");

  AsciiArtGenerator generator;

  // Test supported characters
  if (!generator.IsCharacterSupported('A')) {
    ESP_LOGE(TAG, "Character 'A' should be supported");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Character 'A' is supported");

  if (!generator.IsCharacterSupported('0')) {
    ESP_LOGE(TAG, "Character '0' should be supported");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Character '0' is supported");

  if (!generator.IsCharacterSupported('!')) {
    ESP_LOGE(TAG, "Character '!' should be supported");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Character '!' is supported");

  // Test unsupported characters
  if (generator.IsCharacterSupported('\x01')) {
    ESP_LOGE(TAG, "Character '\\x01' should not be supported");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Character '\\x01' is not supported");

  // Test custom character support (use a character not in built-in set)
  std::vector<std::string> custom_char = {"  ___  ", " /   \\ ", "|     |",
                                          "|     |", " \\___/ ", "       "};

  generator.AddCustomCharacter('\x01', custom_char); // Use \x01 which is not built-in
  if (!generator.IsCharacterSupported('\x01')) {
    ESP_LOGE(TAG, "Custom character '\\x01' should be supported after adding");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Custom character '\\x01' is supported after adding");

  // Test after removal
  generator.RemoveCustomCharacter('\x01');
  if (generator.IsCharacterSupported('\x01')) {
    ESP_LOGE(TAG, "Custom character '\\x01' should not be supported after removal");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Custom character '\\x01' is not supported after removal");

  return true;
}

bool test_supported_characters_list() noexcept {
  ESP_LOGI(TAG, "Testing supported characters list...");

  AsciiArtGenerator generator;

  std::string supported = generator.GetSupportedCharacters();
  if (supported.empty()) {
    ESP_LOGE(TAG, "Supported characters list should not be empty");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Supported characters: %s", supported.c_str());

  // Test that common characters are in the list
  if (supported.find('A') == std::string::npos) {
    ESP_LOGE(TAG, "Character 'A' should be in supported characters list");
    return false;
  }

  if (supported.find('0') == std::string::npos) {
    ESP_LOGE(TAG, "Character '0' should be in supported characters list");
    return false;
  }

  if (supported.find('!') == std::string::npos) {
    ESP_LOGE(TAG, "Character '!' should be in supported characters list");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Common characters found in supported list");

  // Test custom character addition to list (use a character not in built-in set)
  std::vector<std::string> custom_char = {"  ___  ", " /   \\ ", "|     |",
                                          "|     |", " \\___/ ", "       "};

  generator.AddCustomCharacter('\x01', custom_char); // Use \x01 which is not built-in
  std::string supported_after = generator.GetSupportedCharacters();

  if (supported_after.find('\x01') == std::string::npos) {
    ESP_LOGE(TAG, "Custom character '\\x01' should be in supported characters list after adding");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Custom character '\\x01' found in supported list after adding");

  return true;
}

bool test_complex_text_generation() noexcept {
  ESP_LOGI(TAG, "Testing complex text generation...");

  AsciiArtGenerator generator;

  // Test complex text with mixed content
  std::string complex_art = generator.Generate("ESP32-C6 TEST v1.0!");
  if (complex_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for complex text");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for complex text:\n%s", complex_art.c_str());

  // Test text with spaces and punctuation
  std::string spaced_art = generator.Generate("HELLO, WORLD!");
  if (spaced_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for spaced text");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for spaced text:\n%s", spaced_art.c_str());

  // Test text with numbers and symbols
  std::string numeric_art = generator.Generate("TEST 123 @#$%");
  if (numeric_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for numeric text");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for numeric text:\n%s", numeric_art.c_str());

  return true;
}

bool test_performance_and_stability() noexcept {
  ESP_LOGI(TAG, "Testing performance and stability...");

  AsciiArtGenerator generator;

  // Test multiple rapid generations
  for (int i = 0; i < 10; ++i) {
    std::string test_art = generator.Generate("PERFORMANCE TEST");
    if (test_art.empty()) {
      ESP_LOGE(TAG, "Failed to generate ASCII art in performance test iteration %d", i);
      return false;
    }
  }
  ESP_LOGI(TAG, "[SUCCESS] Completed 10 rapid generation tests");

  // Test with different text lengths
  std::vector<std::string> test_strings = {"A",         "AB",        "ABC",     "ABCD",
                                           "ABCDE",     "ABCDEF",    "ABCDEFG", "ABCDEFGH",
                                           "ABCDEFGHI", "ABCDEFGHIJ"};

  for (const auto& test_str : test_strings) {
    std::string art = generator.Generate(test_str);
    if (art.empty()) {
      ESP_LOGE(TAG, "Failed to generate ASCII art for string '%s'", test_str.c_str());
      return false;
    }
  }
  ESP_LOGI(TAG, "[SUCCESS] Completed variable length generation tests");

  // Test memory stability with custom characters
  for (int i = 0; i < 5; ++i) {
    std::vector<std::string> custom_char = {"  ___  ", " /   \\ ", "|     |",
                                            "|     |", " \\___/ ", "       "};

    char custom_char_code = 'A' + i;
    generator.AddCustomCharacter(custom_char_code, custom_char);

    std::string art = generator.Generate(std::string(1, custom_char_code));
    if (art.empty()) {
      ESP_LOGE(TAG, "Failed to generate ASCII art for custom character '%c'", custom_char_code);
      return false;
    }

    generator.RemoveCustomCharacter(custom_char_code);
  }
  ESP_LOGI(TAG, "[SUCCESS] Completed custom character add/remove cycle tests");

  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 ASCII ART GENERATOR EXAMPLE                    ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                          ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  RUN_TEST(test_basic_ascii_art_generation);
  RUN_TEST(test_uppercase_conversion);
  RUN_TEST(test_special_characters);
  RUN_TEST(test_numbers_and_symbols);
  RUN_TEST(test_empty_and_edge_cases);
  RUN_TEST(test_custom_character_management);
  RUN_TEST(test_character_support_validation);
  RUN_TEST(test_supported_characters_list);
  RUN_TEST(test_complex_text_generation);
  RUN_TEST(test_performance_and_stability);

  print_test_summary(g_test_results, "ASCII ART GENERATOR", TAG);

  if (g_test_results.failed_tests == 0) {
    ESP_LOGI(TAG, "[SUCCESS] ALL ASCII ART GENERATOR TESTS PASSED!");

    // Generate a final success banner
    AsciiArtGenerator final_generator;
    std::string success_banner = final_generator.Generate("ASCII ART EXAMPLE COMPLETE!");
    ESP_LOGI(TAG, "\n%s", success_banner.c_str());
  } else {
    ESP_LOGE(TAG, "[FAILED] Some ASCII art generator tests failed.");
  }

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
