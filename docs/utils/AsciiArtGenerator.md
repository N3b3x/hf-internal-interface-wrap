# 🎨 AsciiArtGenerator API Reference

<div align="center">

**📋 Navigation**

[← Previous: DigitalOutputGuard](DigitalOutputGuard.md) | [Back to Utils Index](README.md) | [Next:
Utils Index](README.md)

</div>

---

## Overview

`AsciiArtGenerator` is a utility class that converts text strings into large ASCII art characters.
It provides a simple interface for generating stylized text that can be used in console output,
logging, and user interfaces to enhance visual presentation.

## Features

- **Text to ASCII Art** - Converts strings to large ASCII art characters
- **Custom Character Support** - Add custom character mappings
- **Built-in Character Set** - Supports letters, numbers, and common symbols
- **Uppercase Conversion** - Automatically converts input to uppercase
- **Memory Efficient** - Optimized for embedded systems
- **Thread Safe** - Safe for use in multi-threaded environments

## Header File

```cpp
#include "utils/AsciiArtGenerator.h"
```text

## Class Definition

```cpp
class AsciiArtGenerator {
public:
    // Constructor and destructor
    AsciiArtGenerator() noexcept;
    ~AsciiArtGenerator() noexcept = default;
    
    // Core ASCII art generation
    std::string Generate(const std::string& input) const noexcept;
    
    // Custom character management
    void AddCustomCharacter(char character, const std::vector<std::string>& art*lines) noexcept;
    void RemoveCustomCharacter(char character) noexcept;
    void ClearCustomCharacters() noexcept;
    
    // Character support validation
    bool IsCharacterSupported(char character) const noexcept;
    std::string GetSupportedCharacters() const noexcept;

private:
    std::map<char, std::vector<std::string>> custom*characters*;
    std::vector<std::string> GetCharacterArt(char character) const noexcept;
};
```bash

## Built-in Character Set

The `AsciiArtGenerator` includes a comprehensive set of ASCII art characters:

- **Letters**: A-Z (uppercase only)
- **Numbers**: 0-9
- **Symbols**: ! @ # $ % ^ & * ( ) [ ] { } | \ / ; : ' " < > ` ~
- **Punctuation**: . , ? - * = +

Each character is represented as a 6-line ASCII art pattern with consistent width and height.

## Usage Examples

### Basic ASCII Art Generation

```cpp
#include "utils/AsciiArtGenerator.h"

// Create generator instance
AsciiArtGenerator generator;

// Generate ASCII art for text
std::string hello*art = generator.Generate("HELLO");
printf("%s\n", hello*art.c*str());

// Generate ASCII art for numbers
std::string number*art = generator.Generate("123");
printf("%s\n", number*art.c*str());

// Generate ASCII art for mixed content
std::string mixed*art = generator.Generate("ESP32-C6");
printf("%s\n", mixed*art.c*str());
```text

### Custom Character Management

```cpp
AsciiArtGenerator generator;

// Add custom character
std::vector<std::string> custom*char = {
    "  ***  ",
    " /   \\ ",
    "|     |",
    "|     |",
    " \\***/ ",
    "       "
};

generator.AddCustomCharacter('@', custom*char);

// Generate text with custom character
std::string custom*art = generator.Generate("TEST@");
printf("%s\n", custom*art.c*str());

// Remove custom character
generator.RemoveCustomCharacter('@');

// Clear all custom characters
generator.ClearCustomCharacters();
```text

### Character Support Validation

```cpp
AsciiArtGenerator generator;

// Check if character is supported
if (generator.IsCharacterSupported('A')) {
    printf("Character 'A' is supported\n");
}

if (generator.IsCharacterSupported('€')) {
    printf("Character '€' is supported\n");
} else {
    printf("Character '€' is not supported\n");
}

// Get list of supported characters
std::string supported = generator.GetSupportedCharacters();
printf("Supported characters: %s\n", supported.c*str());
```text

### Integration with Logging

```cpp
#include "utils/AsciiArtGenerator.h"
#include "mcu/esp32/EspLogger.h"

// Create generator and logger
AsciiArtGenerator generator;
EspLogger logger(config);
logger.EnsureInitialized();

// Generate ASCII art banner
std::string banner = generator.Generate("SYSTEM STARTUP");
logger.Log(hf*log*level*t::LOG*INFO, "APP", "ASCII Art Banner:\n%s", banner.c*str());

// Generate test results
std::string result = generator.Generate("SUCCESS");
logger.Log(hf*log*level*t::LOG*INFO, "TEST", "Test Result:\n%s", result.c*str());
```text

### Complete Example

```cpp
#include "utils/AsciiArtGenerator.h"

void print*test*header() {
    AsciiArtGenerator generator;
    
    // Print main header
    std::string header = generator.Generate("ESP32-C6 ADC TEST");
    printf("%s\n", header.c*str());
    
    // Print section header
    std::string section = generator.Generate("HARDWARE VALIDATION");
    printf("%s\n", section.c*str());
    
    // Print info
    std::string info = generator.Generate("CONNECTING TO ESP32-C6");
    printf("%s\n", info.c*str());
}

void print*test*results(bool success) {
    AsciiArtGenerator generator;
    
    if (success) {
        std::string success*msg = generator.Generate("SUCCESS");
        printf("%s\n", success*msg.c*str());
        
        std::string passed = generator.Generate("ALL TESTS PASSED");
        printf("%s\n", passed.c*str());
    } else {
        std::string error*msg = generator.Generate("ERROR");
        printf("%s\n", error*msg.c*str());
        
        std::string failed = generator.Generate("TESTS FAILED");
        printf("%s\n", failed.c*str());
    }
}
```text

## ASCII Art Examples

### Letter Example (A)
```text
  __*   
 / * \  
/ /*\ \ 
|  *  | 
| | | | 
\*| |*/ 
```text

### Number Example (1)
```text
 _*   
/  |  
`| |  
 | |  
*| |* 
\*_*/ 
```text

### Symbol Example (!)
```text
 *  
| | 
| | 
| | 
|*| 
(*) 
```text

### Word Example (HELLO)
```text
 *   *  *      *      *        *  
| | | || |    | |    | |      | | 
| |*| || |    | |    | |      | | 
|  *  || |    | |    | |      | | 
| | | || |_***| |****| |****  | | 
\*| |*/\*_***/\*****/\**__*/  \*/ 
```text

## Performance Characteristics

- **Memory Usage**: ~2KB for built-in character set + custom characters
- **Execution Time**: <1ms per character generation
- **Thread Safety**: Fully thread-safe
- **Flash Usage**: ~2KB for all built-in patterns
- **Character Height**: 6 lines per character
- **Character Width**: Variable (typically 6-8 characters wide)

## Customization

The `AsciiArtGenerator` can be extended with custom characters by adding them at runtime:

```cpp
AsciiArtGenerator generator;

// Add custom character with 6-line ASCII art
std::vector<std::string> custom*char = {
    "  ***  ",  // Line 1
    " /   \\ ",  // Line 2
    "|     |",  // Line 3
    "|     |",  // Line 4
    " \\***/ ",  // Line 5
    "       "   // Line 6
};

generator.AddCustomCharacter('@', custom*char);

// Now '@' can be used in text generation
std::string email*art = generator.Generate("TEST@EXAMPLE");
printf("%s\n", email*art.c*str());
```text

### Custom Character Requirements

- **Height**: Must be exactly 6 lines
- **Width**: Should be consistent (typically 6-8 characters)
- **Format**: Each line should be a string with consistent padding
- **Characters**: Use standard ASCII characters for best compatibility

## Related Documentation

- **[EspLogger API Reference](../esp_api/EspLogger.md)** - Logging integration
- **[DigitalOutputGuard Documentation](DigitalOutputGuard.md)** - Other utility classes
- **[Utils Overview](README.md)** - Complete utilities documentation
## # <<<<<<< Current (Your changes)
<div align="center">

**📋 Navigation**

[← Previous: DigitalOutputGuard](DigitalOutputGuard.md) | [Back to Utils Index](README.md) | [Next:
Utils Index](README.md)

</div>
>>>>>>> Incoming (Background Agent changes)
