/**
 * @file AsciiArtExample.cpp
 * @brief Example demonstrating the advanced ASCII art generator.
 *
 * This example shows how to use the new AsciiArtGenerator class with
 * various formatting options and features.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "utils/AsciiArtGenerator.h"
#include "utils/LoggerManager.h"

#include <cstdio>
#include <cstring>

//==============================================================================
// EXAMPLE FUNCTIONS
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

void DemonstrateColorFormatting() {
    printf("=== Color Formatting Demo ===\n");
    
    AsciiArtGenerator generator;
    
    // Red text
    AsciiArtConfig red_config;
    red_config.color = AsciiArtColor::RED;
    red_config.enable_colors = true;
    
    std::string red_art = generator.Generate("ERROR", red_config);
    printf("Red Text:\n%s\n", red_art.c_str());
    
    // Green text
    AsciiArtConfig green_config;
    green_config.color = AsciiArtColor::GREEN;
    green_config.enable_colors = true;
    
    std::string green_art = generator.Generate("SUCCESS", green_config);
    printf("Green Text:\n%s\n", green_art.c_str());
    
    // Blue text with bright background
    AsciiArtConfig blue_config;
    blue_config.color = AsciiArtColor::BRIGHT_BLUE;
    blue_config.background = AsciiArtBackground::YELLOW;
    blue_config.enable_colors = true;
    
    std::string blue_art = generator.Generate("INFO", blue_config);
    printf("Blue Text on Yellow Background:\n%s\n", blue_art.c_str());
    
    printf("Color demo completed\n\n");
}

void DemonstrateTextStyling() {
    printf("=== Text Styling Demo ===\n");
    
    AsciiArtGenerator generator;
    
    // Bold text
    AsciiArtConfig bold_config;
    bold_config.style = AsciiArtStyle::BOLD;
    bold_config.color = AsciiArtColor::CYAN;
    bold_config.enable_colors = true;
    
    std::string bold_art = generator.Generate("BOLD", bold_config);
    printf("Bold Text:\n%s\n", bold_art.c_str());
    
    // Underlined text
    AsciiArtConfig underline_config;
    underline_config.style = AsciiArtStyle::UNDERLINE;
    underline_config.color = AsciiArtColor::MAGENTA;
    underline_config.enable_colors = true;
    
    std::string underline_art = generator.Generate("UNDERLINE", underline_config);
    printf("Underlined Text:\n%s\n", underline_art.c_str());
    
    // Italic text
    AsciiArtConfig italic_config;
    italic_config.style = AsciiArtStyle::ITALIC;
    italic_config.color = AsciiArtColor::YELLOW;
    italic_config.enable_colors = true;
    
    std::string italic_art = generator.Generate("ITALIC", italic_config);
    printf("Italic Text:\n%s\n", italic_art.c_str());
    
    printf("Styling demo completed\n\n");
}

void DemonstrateBordersAndCentering() {
    printf("=== Borders and Centering Demo ===\n");
    
    AsciiArtGenerator generator;
    
    // Centered text
    AsciiArtConfig center_config;
    center_config.center_text = true;
    center_config.max_width = 80;
    center_config.color = AsciiArtColor::GREEN;
    center_config.enable_colors = true;
    
    std::string center_art = generator.Generate("CENTERED", center_config);
    printf("Centered Text:\n%s\n", center_art.c_str());
    
    // Bordered text
    AsciiArtConfig border_config;
    border_config.add_border = true;
    border_config.border_char = '*';
    border_config.border_padding = 2;
    border_config.color = AsciiArtColor::RED;
    border_config.enable_colors = true;
    
    std::string border_art = generator.Generate("BORDERED", border_config);
    printf("Bordered Text:\n%s\n", border_art.c_str());
    
    // Centered and bordered
    AsciiArtConfig full_config;
    full_config.center_text = true;
    full_config.add_border = true;
    full_config.border_char = '#';
    full_config.border_padding = 1;
    full_config.max_width = 80;
    full_config.color = AsciiArtColor::BRIGHT_CYAN;
    full_config.enable_colors = true;
    
    std::string full_art = generator.Generate("FULL", full_config);
    printf("Centered and Bordered Text:\n%s\n", full_art.c_str());
    
    printf("Borders and centering demo completed\n\n");
}

void DemonstrateInlineFormatting() {
    printf("=== Inline Formatting Demo ===\n");
    
    AsciiArtGenerator generator;
    
    // Inline formatting with color codes
    std::string formatted_input = "{red:bold}ERROR{/} {green}SUCCESS{/} {blue:underline}INFO{/}";
    std::string formatted_art = generator.GenerateFormatted(formatted_input);
    printf("Inline Formatted Text:\n%s\n", formatted_art.c_str());
    
    printf("Inline formatting demo completed\n\n");
}

void DemonstrateCustomCharacters() {
    printf("=== Custom Characters Demo ===\n");
    
    AsciiArtGenerator generator;
    
    // Add custom character
    std::vector<std::string> custom_char = {
        "  ___  ",
        " /   \\ ",
        "|     |",
        "|     |",
        " \\___/ ",
        "       "
    };
    
    generator.AddCustomCharacter('@', custom_char);
    
    // Test custom character
    std::string custom_art = generator.Generate("TEST@");
    printf("Custom Character (@):\n%s\n", custom_art.c_str());
    
    // Check if character is supported
    printf("Is '@' supported: %s\n", generator.IsCharacterSupported('@') ? "Yes" : "No");
    printf("Is '€' supported: %s\n", generator.IsCharacterSupported('€') ? "Yes" : "No");
    
    printf("Custom characters demo completed\n\n");
}

void DemonstrateUtilityFunctions() {
    printf("=== Utility Functions Demo ===\n");
    
    // Create different generator types
    auto simple_generator = CreateSimpleAsciiArtGenerator();
    auto colorful_generator = CreateColorfulAsciiArtGenerator();
    auto bordered_generator = CreateBorderedAsciiArtGenerator();
    
    printf("Simple Generator:\n%s\n", simple_generator.Generate("SIMPLE").c_str());
    printf("Colorful Generator:\n%s\n", colorful_generator.Generate("COLORFUL").c_str());
    printf("Bordered Generator:\n%s\n", bordered_generator.Generate("BORDERED").c_str());
    
    // Get supported characters
    printf("Supported characters: %s\n", simple_generator.GetSupportedCharacters().c_str());
    
    printf("Utility functions demo completed\n\n");
}

void DemonstratePerformanceComparison() {
    printf("=== Performance Comparison Demo ===\n");
    
    AsciiArtGenerator generator;
    
    // Test with different string lengths
    std::vector<std::string> test_strings = {
        "A",
        "HELLO",
        "HELLO WORLD",
        "HELLO WORLD 123!@#",
        "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG"
    };
    
    for (const auto& test_string : test_strings) {
        printf("Generating art for: '%s'\n", test_string.c_str());
        std::string art = generator.Generate(test_string);
        printf("Result length: %zu characters\n", art.length());
        printf("First few lines:\n");
        
        // Print first few lines
        size_t pos = 0;
        for (int i = 0; i < 3 && pos != std::string::npos; ++i) {
            size_t next_pos = art.find('\n', pos);
            if (next_pos != std::string::npos) {
                printf("  %s\n", art.substr(pos, next_pos - pos).c_str());
                pos = next_pos + 1;
            } else {
                printf("  %s\n", art.substr(pos).c_str());
                break;
            }
        }
        printf("\n");
    }
    
    printf("Performance comparison demo completed\n\n");
}

void DemonstrateIntegrationWithLogger() {
    printf("=== Logger Integration Demo ===\n");
    
    // Initialize logger
    LoggerManager::Initialize();
    
    AsciiArtGenerator generator;
    
    // Create a banner for the application
    AsciiArtConfig banner_config;
    banner_config.color = AsciiArtColor::BRIGHT_GREEN;
    banner_config.style = AsciiArtStyle::BOLD;
    banner_config.center_text = true;
    banner_config.add_border = true;
    banner_config.border_char = '=';
    banner_config.border_padding = 1;
    banner_config.max_width = 80;
    banner_config.enable_colors = true;
    
    std::string banner = generator.Generate("HARDFOC", banner_config);
    
    // Log the banner
    LoggerManager::Info("BANNER", "Application Banner:\n%s", banner.c_str());
    
    // Create error message
    AsciiArtConfig error_config;
    error_config.color = AsciiArtColor::BRIGHT_RED;
    error_config.style = AsciiArtStyle::BOLD;
    error_config.enable_colors = true;
    
    std::string error_art = generator.Generate("ERROR", error_config);
    LoggerManager::Error("ASCII", "Error Display:\n%s", error_art.c_str());
    
    printf("Logger integration demo completed\n\n");
}

//==============================================================================
// MAIN FUNCTION
//==============================================================================

extern "C" void app_main(void) {
    printf("=== Advanced ASCII Art Generator Demo ===\n\n");
    
    // Run all demonstrations
    DemonstrateBasicAsciiArt();
    DemonstrateColorFormatting();
    DemonstrateTextStyling();
    DemonstrateBordersAndCentering();
    DemonstrateInlineFormatting();
    DemonstrateCustomCharacters();
    DemonstrateUtilityFunctions();
    DemonstratePerformanceComparison();
    DemonstrateIntegrationWithLogger();
    
    printf("=== ASCII Art Demo Completed ===\n");
    printf("Check the output above for various ASCII art examples with formatting\n");
} 