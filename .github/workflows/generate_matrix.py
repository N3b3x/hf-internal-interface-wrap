#!/usr/bin/env python3
"""
Generate CI matrix from centralized configuration.
This script reads examples_config.yml and outputs GitHub Actions matrix configuration.
"""

import sys
import yaml
import json
from pathlib import Path

def load_config():
    """Load the examples configuration file."""
    # Try multiple possible paths for the configuration file
    possible_paths = [
        # When run from workspace root
        Path("examples/esp32/examples_config.yml"),
        # When run from .github/workflows directory  
        Path("../../examples/esp32/examples_config.yml"),
        # Absolute path calculation from script location
        Path(__file__).resolve().parent.parent / "examples" / "esp32" / "examples_config.yml"
    ]
    
    config_file = None
    for path in possible_paths:
        if path.exists():
            config_file = path
            break
    
    if not config_file:
        print(f"Error: Configuration file not found in any of these locations:", file=sys.stderr)
        for path in possible_paths:
            print(f"  {path.resolve()}", file=sys.stderr)
        sys.exit(1)
    
    try:
        with open(config_file, 'r') as f:
            return yaml.safe_load(f)
    except Exception as e:
        print(f"Error loading configuration: {e}", file=sys.stderr)
        sys.exit(1)

def generate_matrix():
    """Generate CI matrix from configuration."""
    config = load_config()
    
    # Get CI-enabled examples
    ci_examples = []
    for example_name, example_config in config['examples'].items():
        if example_config.get('ci_enabled', True):
            ci_examples.append(example_name)
    
    # Get build types
    build_types = list(config['build_config']['build_types'].keys())
    
    # Get IDF versions
    idf_versions = config['metadata'].get('idf_versions', ['release-v5.5'])
    
    # Generate matrix
    matrix = {
        'idf_version': idf_versions,
        'build_type': build_types,
        'example_type': ci_examples
    }
    
    # Get exclude combinations if any
    exclude_combinations = config.get('ci_config', {}).get('exclude_combinations', [])
    
    result = {
        'matrix': matrix
    }
    
    if exclude_combinations:
        result['exclude'] = exclude_combinations
    
    return result

def main():
    """Main function."""
    if len(sys.argv) > 1:
        if sys.argv[1] == '--examples-only':
            # Output only the example types for simpler usage
            config = load_config()
            ci_examples = []
            for example_name, example_config in config['examples'].items():
                if example_config.get('ci_enabled', True):
                    ci_examples.append(example_name)
            print(json.dumps(ci_examples))
            return
        elif sys.argv[1] == '--build-types-only':
            # Output only the build types
            config = load_config()
            build_types = list(config['build_config']['build_types'].keys())
            print(json.dumps(build_types))
            return
        elif sys.argv[1] == '--metadata':
            # Output metadata
            config = load_config()
            metadata = config['metadata']
            print(json.dumps(metadata))
            return
    
    # Generate full matrix
    matrix_config = generate_matrix()
    print(json.dumps(matrix_config, indent=2))

if __name__ == '__main__':
    main()

