/**
 * @file AsciiArtGenerator.cpp
 * @brief Implementation of the simple ASCII art generator.
 *
 * This file provides the implementation for the AsciiArtGenerator class,
 * which creates large, stylized text from input strings. It focuses only
 * on generating plain ASCII art without formatting - formatting is handled
 * by the Logger system.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "utils/AsciiArtGenerator.h"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>

//==============================================================================
// CONSTANTS
//==============================================================================

static constexpr size_t ART_HEIGHT = 6; ///< Height of ASCII art characters
static constexpr size_t ART_WIDTH = 8;  ///< Width of ASCII art characters

//==============================================================================
// CHARACTER ART DATA
//==============================================================================

// This is a much more efficient approach - store all character art in a single data structure
static const std::map<char, std::vector<std::string>> CHARACTER_ART = {
    {'A', {"  ___   ", " / _ \\  ", "/ /_\\ \\ ", "|  _  | ", "| | | | ", "\\_| |_/ "}},
    {'B', {" _____  ", "| ___ \\ ", "| |_/ / ", "| ___ \\ ", "| |_/ / ", "\\____/  "}},
    {'C', {" _____  ", "/  __ \\ ", "| /  \\/ ", "| |     ", "| \\__/\\ ", " \\____/ "}},
    {'D', {" _____  ", "|  _  \\ ", "| | | | ", "| | | | ", "| |/ /  ", "|___/   "}},
    {'E', {" _____  ", "|  ___| ", "| |__   ", "|  __|  ", "| |___  ", "\\____/  "}},
    {'F', {" _____  ", "|  ___| ", "| |_    ", "|  _|   ", "| |     ", "\\_|     "}},
    {'G', {" _____  ", "|  __ \\ ", "| |  \\/ ", "| | __  ", "| |_\\ \\ ", " \\____/ "}},
    {'H', {" _   _  ", "| | | | ", "| |_| | ", "|  _  | ", "| | | | ", "\\_| |_/ "}},
    {'I', {" _____  ", "|_   _| ", "  | |   ", "  | |   ", " _| |_  ", " \\___/  "}},
    {'J', {"   ___  ", "  |_  | ", "    | | ", "    | | ", "/\\__/ / ", "\\____/  "}},
    {'K', {" _   __ ", "| | / / ", "| |/ /  ", "|    \\  ", "| |\\  \\ ", "\\_| \\_/ "}},
    {'L', {" _      ", "| |     ", "| |     ", "| |     ", "| |____ ", "\\_____/ "}},
    {'M', {" __  __  ", "|  \\/  | ", "| .  . | ", "| |\\/| | ", "| |  | | ", "\\_|  |_/ "}},
    {'N', {" _   _  ", "| \\ | | ", "|  \\| | ", "| . ` | ", "| |\\  | ", "\\_| \\_/ "}},
    {'O', {" _____  ", "|  _  | ", "| | | | ", "| | | | ", "\\ \\_/ / ", " \\___/  "}},
    {'P', {" _____  ", "| ___ \\ ", "| |_/ / ", "|  __/  ", "| |     ", "\\_|     "}},
    {'Q', {" _____  ", "|  _  | ", "| | | | ", "| | | | ", "\\ \\/' / ", " \\_/\\_\\ "}},
    {'R', {" _____  ", "| ___ \\ ", "| |_/ / ", "|    /  ", "| |\\ \\  ", "\\_| \\_| "}},
    {'S', {" _____  ", "|  ___| ", "\\ `--.  ", " `--. \\ ", "/\\__/ / ", "\\____/  "}},
    {'T', {" _____  ", "|_   _| ", "  | |   ", "  | |   ", "  | |   ", "  \\_/   "}},
    {'U', {" _   _  ", "| | | | ", "| | | | ", "| | | | ", "| |_| | ", " \\___/  "}},
    {'V', {" _   _  ", "| | | | ", "| | | | ", "| | | | ", "\\ \\_/ / ", " \\___/  "}},
    {'W', {" _    _  ", "| |  | | ", "| |  | | ", "| |/\\| | ", "\\  /\\  / ", " \\/  \\/  "}},
    {'X', {"__   __ ", "\\ \\ / / ", " \\ V /  ", " / ^ \\  ", "/ / \\ \\ ", "\\/   \\/ "}},
    {'Y', {"__   __ ", "\\ \\ / / ", " \\ V /  ", "  \\ /   ", "  | |   ", "  \\_/   "}},
    {'Z', {" ______ ", "|___  / ", "   / /  ", "  / /   ", "./ /___ ", "\\_____/ "}},
    {' ', {"  ", "  ", "  ", "  ", "  ", "  "}},
    {'0', {" _____  ", "|  _  | ", "| |/' | ", "|  /| | ", "\\ |_/ / ", " \\___/  "}},
    {'1', {" __   ", "/  |  ", "`| |  ", " | |  ", "_| |_ ", "\\___/ "}},
    {'2', {" _____  ", "/ __  \\ ", "`' / /' ", "  / /   ", "./ /___ ", "\\_____/ "}},
    {'3', {" _____  ", "|____ | ", "    / / ", "    \\ \\ ", ".___/ / ", "\\____/  "}},
    {'4', {"   ___  ", "  /   | ", " / /| | ", "/ /_| | ", "\\___  | ", "    |_/ "}},
    {'5', {" _____  ", "|  ___| ", "|___ \\  ", "    \\ \\ ", "/\\__/ / ", "\\____/  "}},
    {'6', {"  ____  ", " / ___| ", "/ /___  ", "| ___ \\ ", "| \\_/ | ", "\\_____/ "}},
    {'7', {" ______ ", "|___  / ", "   / /  ", "  / /   ", "./ /    ", "\\_/     "}},
    {'8', {" _____  ", "|  _  | ", " \\ V /  ", " / _ \\  ", "| |_| | ", "\\_____/ "}},
    {'9', {" _____  ", "|  _  | ", "| |_| | ", "\\____ | ", ".___/ / ", "\\____/  "}},
    {'.', {"    ", "    ", "    ", "    ", " _  ", "(_) "}},
    {',', {"    ", "    ", "    ", " _  ", "( ) ", "|/  "}},
    {'!', {" _  ", "| | ", "| | ", "| | ", "|_| ", "(_) "}},
    {'?', {" ___   ", "|__ \\  ", "   ) | ", "  / /  ", " |_|   ", " (_)   "}},
    {'@', {"   ____   ", "  / __ \\  ", " / / _` | ", "| | (_| | ", " \\ \\__,_| ", "  \\____/  "}},
    {'#',
     {"   _  _    ", " _| || |_  ", "|_  __  _| ", " _| || |_  ", "|_  __  _| ", "  |_||_|   "}},
    {'$', {"  _   ", " | |  ", "/ __) ", "\\__ \\ ", "(   / ", " |_|  "}},
    {'%', {" _   __ ", "(_) / / ", "   / /  ", "  / /   ", " / / _  ", "/_/ (_) "}},
    {'^', {" /\\  ", "|/\\| ", "     ", "     ", "     ", "     "}},
    {'&', {"         ", "  ___    ", " ( _ )   ", " / _ \\/\\ ", "| (_>  < ", " \\___/\\/ "}},
    {'*',
     {"    _     ", " /\\| |/\\  ", " \\ ` ' /  ", "|_     _| ", " / , . \\  ", " \\/|_|\\/  "}},
    {'(', {"  __ ", " / / ", "| |  ", "| |  ", "| |  ", " \\_\\ "}},
    {')', {"__   ", "\\ \\  ", " | | ", " | | ", " | | ", "/_/  "}},
    {'-', {"         ", "         ", " ______  ", "|______| ", "         ", "         "}},
    {'_', {"         ", "         ", "         ", "         ", "         ", "|______| "}},
    {'=', {"         ", " ______  ", "|______| ", " ______  ", "|______| ", "         "}},
    {'+', {"        ", "   _    ", " _| |_  ", "|_   _| ", "  |_|   ", "        "}},
    {'[', {" ___  ", "|  _| ", "| |   ", "| |   ", "| |_  ", "|___| "}},
    {']', {" ___  ", "|_  | ", "  | | ", "  | | ", " _| | ", "|___| "}},
    {'{', {"   __ ", "  / / ", " | |  ", "< <   ", " | |  ", "  \\_\\ "}},
    {'}', {"__    ", "\\ \\   ", " | |  ", "  > > ", " | |  ", "/_/   "}},
    {'|', {" _  ", "| | ", "| | ", "| | ", "| | ", "|_| "}},
    {'\\', {"__      ", "\\ \\     ", " \\ \\    ", "  \\ \\   ", "   \\ \\  ", "    \\_\\ "}},
    {'/', {"     __ ", "    / / ", "   / /  ", "  / /   ", " / /    ", "/_/     "}},
    {';', {" _  ", "(_) ", "    ", " _  ", "( ) ", "|/  "}},
    {':', {"    ", " _  ", "(_) ", "    ", " _  ", "(_) "}},
    {'\'', {" _  ", "( ) ", " \\| ", "    ", "    ", "    "}},
    {'"', {" _ _  ", "( | ) ", " V V  ", "      ", "      ", "      "}},
    {'<', {"   __ ", "  / / ", " / /  ", "< <   ", " \\ \\  ", "  \\_\\ "}},
    {'>', {"__    ", "\\ \\   ", " \\ \\  ", "  > > ", " / /  ", "/_/   "}},
    {'`', {" _  ", "( ) ", " \\| ", "    ", "    ", "    "}},
    {'~', {"      ", " /\\/| ", "|/\\/  ", "      ", "      ", "      "}}};

//==============================================================================
// CONSTRUCTOR
//==============================================================================

AsciiArtGenerator::AsciiArtGenerator() noexcept {
  // Simple constructor - no configuration needed
}

//==============================================================================
// PUBLIC METHODS
//==============================================================================

std::string AsciiArtGenerator::Generate(const std::string& input) const noexcept {
  if (input.empty()) {
    return "";
  }

  // Convert input to uppercase for consistent processing
  std::string upper_input = input;
  std::transform(upper_input.begin(), upper_input.end(), upper_input.begin(), ::toupper);

  // Generate art lines for each character
  std::vector<std::string> art_lines(ART_HEIGHT);

  for (char c : upper_input) {
    auto char_art = GetCharacterArt(c);
    if (char_art.size() == ART_HEIGHT) {
      for (size_t i = 0; i < ART_HEIGHT; ++i) {
        art_lines[i] += char_art[i];
      }
    }
  }

  // Convert to string
  std::string result;
  for (const auto& line : art_lines) {
    result += line + "\n";
  }

  return result;
}

void AsciiArtGenerator::AddCustomCharacter(char character,
                                           const std::vector<std::string>& art_lines) noexcept {
  if (art_lines.size() == ART_HEIGHT) {
    custom_characters_[character] = art_lines;
  }
}

void AsciiArtGenerator::RemoveCustomCharacter(char character) noexcept {
  custom_characters_.erase(character);
}

void AsciiArtGenerator::ClearCustomCharacters() noexcept {
  custom_characters_.clear();
}

bool AsciiArtGenerator::IsCharacterSupported(char character) const noexcept {
  return CHARACTER_ART.find(character) != CHARACTER_ART.end() ||
         custom_characters_.find(character) != custom_characters_.end();
}

std::string AsciiArtGenerator::GetSupportedCharacters() const noexcept {
  std::string result;
  for (const auto& pair : CHARACTER_ART) {
    result += pair.first;
  }
  for (const auto& pair : custom_characters_) {
    result += pair.first;
  }
  return result;
}
