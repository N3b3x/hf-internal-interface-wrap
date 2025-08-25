#!/usr/bin/env python3
"""
Get app information from centralized configuration for CMake.
This script reads app_config.yml and outputs app information for CMake consumption.
"""

import sys
import yaml
import json
from pathlib import Path

def load_config():
    """Load the apps configuration file."""
    config_file = Path(__file__).parent.parent / "app_config.yml"
    
    if not config_file.exists():
        print(f"Error: Configuration file not found: {config_file}", file=sys.stderr)
        sys.exit(1)
    
    try:
        with open(config_file, 'r') as f:
            return yaml.safe_load(f)
    except Exception as e:
        print(f"Error loading configuration: {e}", file=sys.stderr)
        sys.exit(1)

def get_app_source_file(app_type):
    """Get source file for an app type."""
    config = load_config()
    
    if app_type not in config['apps']:
        print(f"Error: Unknown app type: {app_type}", file=sys.stderr)
        sys.exit(1)
    
    return config['apps'][app_type]['source_file']

def list_apps():
    """List all available apps."""
    config = load_config()
    apps = list(config['apps'].keys())
    return apps

def validate_app(app_type):
    """Validate if app type exists."""
    config = load_config()
    return app_type in config['apps']

def main():
    """Main function."""
    if len(sys.argv) < 2 or sys.argv[1] in ["--help", "-h"]:
        print("ESP32 App Information Script", file=sys.stderr)
        print("", file=sys.stderr)
        print("Usage: get_app_info.py <command> [args...]", file=sys.stderr)
        print("", file=sys.stderr)
        print("Purpose: Get information about ESP32 apps from configuration", file=sys.stderr)
        print("", file=sys.stderr)
        print("Commands:", file=sys.stderr)
        print("  source_file <app_type>      - Get source file path for app", file=sys.stderr)
        print("  list                        - List all available apps", file=sys.stderr)
        print("  validate <app_type>         - Check if app type is valid", file=sys.stderr)
        print("  --help, -h                  - Show this help message", file=sys.stderr)
        print("", file=sys.stderr)
        print("Examples:", file=sys.stderr)
        print("  python3 get_app_info.py list", file=sys.stderr)
        print("  python3 get_app_info.py source_file gpio_test", file=sys.stderr)
        print("  python3 get_app_info.py validate adc_test", file=sys.stderr)
        print("", file=sys.stderr)
        print("This script reads app_config.yml and provides app information", file=sys.stderr)
        print("for CMake and other build tools.", file=sys.stderr)
        print("", file=sys.stderr)
        print("For detailed information, see: docs/README_UTILITY_SCRIPTS.md", file=sys.stderr)
        sys.exit(1)
    
    command = sys.argv[1]
    
    if command == "source_file":
        if len(sys.argv) != 3:
            print("Usage: get_app_info.py source_file <app_type>", file=sys.stderr)
            sys.exit(1)
        source_file = get_app_source_file(sys.argv[2])
        print(source_file)
    
    elif command == "list":
        apps = list_apps()
        print(" ".join(apps))
    
    elif command == "validate":
        if len(sys.argv) != 3:
            print("Usage: get_app_info.py validate <app_type>", file=sys.stderr)
            sys.exit(1)
        is_valid = validate_app(sys.argv[2])
        print("true" if is_valid else "false")
        if not is_valid:
            sys.exit(1)
    
    else:
        print(f"Error: Unknown command: {command}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()

