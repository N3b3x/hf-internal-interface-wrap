#!/usr/bin/env python3
"""
Script to fix compound words in README.md that use asterisks (*) instead of underscores (_).

This script identifies patterns like 'idf*version' and converts them to 'idf_version'.
"""

import re
import sys
from pathlib import Path

def fix_underscores_in_file(file_path):
    """Fix compound words with asterisks to use underscores instead."""
    
    # Read the file
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except FileNotFoundError:
        print(f"Error: File {file_path} not found")
        return False
    except Exception as e:
        print(f"Error reading file {file_path}: {e}")
        return False
    
    # Pattern to match word*word (compound words with asterisks)
    # This matches word boundaries, then word characters, asterisk, word characters, then word boundary
    pattern = r'\b(\w+)\*(\w+)\b'
    
    # Find all matches for reporting
    matches = re.findall(pattern, content)
    if not matches:
        print("No compound words with asterisks found.")
        return True
    
    # Count unique patterns
    unique_patterns = set(matches)
    print(f"Found {len(matches)} total matches of {len(unique_patterns)} unique patterns:")
    for pattern in sorted(unique_patterns):
        count = matches.count(pattern)
        print(f"  {pattern[0]}*{pattern[1]} -> {pattern[0]}_{pattern[1]} ({count} times)")
    
    # Replace all occurrences
    new_content = re.sub(r'\b(\w+)\*(\w+)\b', r'\1_\2', content)
    
    # Check if any changes were made
    if new_content == content:
        print("No changes needed.")
        return True
    
    # Write the updated content
    try:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"Successfully updated {file_path}")
        return True
    except Exception as e:
        print(f"Error writing file {file_path}: {e}")
        return False

def main():
    """Main function to process the README file."""
    script_dir = Path(__file__).parent
    readme_path = script_dir / "README.md"
    
    if not readme_path.exists():
        print(f"Error: README.md not found at {readme_path}")
        sys.exit(1)
    
    print(f"Processing {readme_path}...")
    print("=" * 50)
    
    success = fix_underscores_in_file(readme_path)
    
    if success:
        print("=" * 50)
        print("✅ All compound words have been converted from asterisks to underscores!")
    else:
        print("=" * 50)
        print("❌ Error occurred during processing")
        sys.exit(1)

if __name__ == "__main__":
    main()