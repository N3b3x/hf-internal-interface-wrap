/**
 * @file AsciiArtGenerator.h
 * @brief Simple ASCII art generator.
 *
 * This file provides a simple ASCII art generator that creates
 * large, stylized text from input strings. It focuses only on
 * generating plain ASCII art without formatting - formatting
 * is handled by the Logger system.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * @class AsciiArtGenerator
 * @brief Simple ASCII art generator.
 *
 * This class provides a simple ASCII art generator that creates
 * large, stylized text from input strings. It focuses only on
 * generating plain ASCII art without formatting - formatting
 * is handled by the Logger system.
 *
 * Key Features:
 * - **Simple Interface**: Just generate ASCII art
 * - **Performance Optimized**: Efficient string handling
 * - **Extensible**: Easy to add new fonts and characters
 * - **No Formatting**: Pure ASCII art generation
 *
 * Usage Example:
 * @code
 * AsciiArtGenerator generator;
 *
 * // Basic usage
 * std::string art = generator.Generate("Hello World");
 *
 * // Use with Logger for formatting
 * Logger& logger = Logger::GetInstance();
 * logger.LogAsciiArt("BANNER", art, AsciiArtFormat{});
 * @endcode
 */
class AsciiArtGenerator {
public:
  /**
   * @brief Default constructor
   */
  AsciiArtGenerator() noexcept;

  /**
   * @brief Destructor
   */
  ~AsciiArtGenerator() noexcept = default;

  /**
   * @brief Generate ASCII art from string
   * @param input Input string to convert
   * @return Generated ASCII art
   */
  std::string Generate(const std::string& input) const noexcept;

  /**
   * @brief Add custom character mapping
   * @param character Character to map
   * @param art_lines Vector of art lines for the character
   */
  void AddCustomCharacter(char character, const std::vector<std::string>& art_lines) noexcept;

  /**
   * @brief Remove custom character mapping
   * @param character Character to remove
   */
  void RemoveCustomCharacter(char character) noexcept;

  /**
   * @brief Clear all custom character mappings
   */
  void ClearCustomCharacters() noexcept;

  /**
   * @brief Check if character is supported
   * @param character Character to check
   * @return true if supported, false otherwise
   */
  bool IsCharacterSupported(char character) const noexcept;

  /**
   * @brief Get supported characters
   * @return String of supported characters
   */
  std::string GetSupportedCharacters() const noexcept;

private:
  //==============================================================================
  // PRIVATE MEMBERS
  //==============================================================================

  std::map<char, std::vector<std::string>> custom_characters_; ///< Custom character mappings

  //==============================================================================
  // PRIVATE METHODS
  //==============================================================================

  /**
   * @brief Get art lines for a character
   * @param character Character to get art for
   * @return Vector of art lines
   */
  std::vector<std::string> GetCharacterArt(char character) const noexcept;
};
