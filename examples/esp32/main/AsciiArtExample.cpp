/**
 * @file AsciiArtExample.cpp
 * @brief Example demonstrating the simple ASCII art generator.
 */

#include "utils/AsciiArtGenerator.h"
#include <cstdio>
#include <string>
#include <vector>

//==============================================================================
// SIMPLE ASCII ART EXAMPLE (matches current AsciiArtGenerator.h)
//==============================================================================

void DemonstrateBasicAsciiArt() {
  printf("=== Basic ASCII Art Demo ===\n");

  AsciiArtGenerator generator;

  // Basic usage
  std::string art = generator.Generate("HELLO");
  printf("Basic ASCII Art:\n%s\n", art.c_str());

  // With numbers and symbols
  std::string art2 = generator.Generate("123!@#");
  printf("Numbers and Symbols:\n%s\n", art2.c_str());

  printf("Basic demo completed\n\n");
}

void DemonstrateCustomCharacters() {
  printf("=== Custom Characters Demo ===\n");

  AsciiArtGenerator generator;

  // Add custom character
  std::vector<std::string> custom_char = {"  ___  ", " /   \\",  "|     |",
                                          "|     |", " \\___/ ", "       "};

  generator.AddCustomCharacter('@', custom_char);

  // Test custom character
  std::string custom_art = generator.Generate("TEST@");
  printf("Custom Character (@):\n%s\n", custom_art.c_str());

  // Check if character is supported
  printf("Is '@' supported: %s\n", generator.IsCharacterSupported('@') ? "Yes" : "No");
  // Only check ASCII characters, avoid non-ASCII like '€'
  printf("Is 'A' supported: %s\n", generator.IsCharacterSupported('A') ? "Yes" : "No");

  printf("Custom characters demo completed\n\n");
}

//==============================================================================
// MAIN FUNCTION
//==============================================================================

extern "C" void app_main(void) {
  printf("=== Simple ASCII Art Generator Demo ===\n\n");

  DemonstrateBasicAsciiArt();
  DemonstrateCustomCharacters();

  printf("=== ASCII Art Demo Completed ===\n");
  printf("Check the output above for basic ASCII art examples.\n");
}