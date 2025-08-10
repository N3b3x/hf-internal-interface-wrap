#!/usr/bin/env python3
"""
Get example information from centralized configuration for CMake.
This script reads examples_config.yml and outputs example information for CMake consumption.
"""

import sys
import yaml
import json
from pathlib import Path

def load_config():
    """Load the examples configuration file."""
    config_file = Path(__file__).parent.parent / "examples_config.yml"
    
    if not config_file.exists():
        print(f"Error: Configuration file not found: {config_file}", file=sys.stderr)
        sys.exit(1)
    
    try:
        with open(config_file, 'r') as f:
            return yaml.safe_load(f)
    except Exception as e:
        print(f"Error loading configuration: {e}", file=sys.stderr)
        sys.exit(1)

def get_example_source_file(example_type):
    """Get source file for an example type."""
    config = load_config()
    
    if example_type not in config['examples']:
        print(f"Error: Unknown example type: {example_type}", file=sys.stderr)
        sys.exit(1)
    
    return config['examples'][example_type]['source_file']

def list_examples():
    """List all available examples."""
    config = load_config()
    examples = list(config['examples'].keys())
    return examples

def validate_example(example_type):
    """Validate if example type exists."""
    config = load_config()
    return example_type in config['examples']

def main():
    """Main function."""
    if len(sys.argv) < 2:
        print("Usage: get_example_info.py <command> [args...]", file=sys.stderr)
        print("Commands:", file=sys.stderr)
        print("  source_file <example_type>  - Get source file for example", file=sys.stderr)
        print("  list                        - List all examples", file=sys.stderr)
        print("  validate <example_type>     - Validate example type", file=sys.stderr)
        sys.exit(1)
    
    command = sys.argv[1]
    
    if command == "source_file":
        if len(sys.argv) != 3:
            print("Usage: get_example_info.py source_file <example_type>", file=sys.stderr)
            sys.exit(1)
        source_file = get_example_source_file(sys.argv[2])
        print(source_file)
    
    elif command == "list":
        examples = list_examples()
        print(" ".join(examples))
    
    elif command == "validate":
        if len(sys.argv) != 3:
            print("Usage: get_example_info.py validate <example_type>", file=sys.stderr)
            sys.exit(1)
        is_valid = validate_example(sys.argv[2])
        print("true" if is_valid else "false")
        if not is_valid:
            sys.exit(1)
    
    else:
        print(f"Error: Unknown command: {command}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()