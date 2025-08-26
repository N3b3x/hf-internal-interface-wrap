#!/usr/bin/env python3
"""
Generate CI matrix from centralized configuration.
This script reads app_config.yml and outputs GitHub Actions matrix configuration.
Supports hierarchical configuration where apps can override global settings.
"""

import sys
import yaml
import json
from pathlib import Path

def load_config():
    """Load the apps configuration file."""
    # Try multiple possible paths for the configuration file
    possible_paths = [
        # When run from workspace root
        Path("examples/esp32/app_config.yml"),
        # When run from .github/workflows directory  
        Path("../../examples/esp32/app_config.yml"),
        # Absolute path calculation from script location
        Path(__file__).resolve().parent.parent / "examples" / "esp32" / "app_config.yml"
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
    """Generate CI matrix from configuration with hierarchical overrides."""
    config = load_config()
    
    # Global defaults from metadata
    global_idf_versions = config['metadata'].get('idf_versions', ['release/v5.5'])
    global_build_types_per_idf = config['metadata'].get('default_build_types', [['Debug', 'Release']])
    
    # Create a mapping from IDF version to its allowed build types
    idf_to_build_types = {}
    for i, idf_version in enumerate(global_idf_versions):
        if i < len(global_build_types_per_idf):
            idf_to_build_types[idf_version] = global_build_types_per_idf[i]
        else:
            # Fallback if no build types specified for this IDF version
            idf_to_build_types[idf_version] = ['Debug', 'Release']
    
    # Optional excludes for special cases - handle None case
    exclude_combinations = config.get('ci_config', {}).get('exclude_combinations', []) or []

    def is_excluded(entry: dict) -> bool:
        for exc in exclude_combinations:
            # If all keys in exc match the entry, then exclude
            if all(k in entry and entry[k] == v for k, v in exc.items()):
                return True
        return False

    # Build an explicit include list honoring per-app overrides
    include: list[dict] = []
    for app_name, app_config in config['apps'].items():
        if not app_config.get('ci_enabled', True):
            continue
        
        # Per-app overrides (app-specific settings take precedence)
        per_app_idf_versions = app_config.get('idf_versions', global_idf_versions)
        
        # For each IDF version, determine the allowed build types
        effective_combinations = []
        for i, idf_version in enumerate(per_app_idf_versions):
            if 'build_types' in app_config:
                # App has specific build type requirements
                app_build_types = app_config['build_types']
                
                # Check if build_types is nested (array of arrays) or flat (single array)
                if isinstance(app_build_types[0], list):
                    # Nested format: build_types: [["Debug", "Release"], ["Debug"]]
                    if i < len(app_build_types):
                        # Use the build types for this specific IDF version index
                        for build_type in app_build_types[i]:
                            effective_combinations.append((idf_version, build_type))
                    else:
                        # Fallback if no build types specified for this IDF version index
                        for build_type in ['Debug', 'Release']:
                            effective_combinations.append((idf_version, build_type))
                else:
                    # Flat format: build_types: ["Debug", "Release"] (same for all IDF versions)
                    for build_type in app_build_types:
                        effective_combinations.append((idf_version, build_type))
            else:
                # Use global IDF-specific build types
                allowed_build_types = idf_to_build_types.get(idf_version, ['Debug', 'Release'])
                for build_type in allowed_build_types:
                    effective_combinations.append((idf_version, build_type))
        
        # Generate matrix entries for this app
        for idf_version, build_type in effective_combinations:
            # Create Docker-safe version for artifact naming (replace / with -)
            docker_safe_version = idf_version.replace('/', '-')
            candidate = {
                'idf_version': idf_version,  # Git format for ESP-IDF cloning
                'idf_version_docker': docker_safe_version,  # Docker-safe format for artifacts
                'build_type': build_type,
                'app_name': app_name,  # Use app_name for consistency
                'config_source': 'app' if ('build_types' in app_config or 'idf_versions' in app_config) else 'global'
            }
            if not is_excluded(candidate):
                include.append(candidate)

    return { 'include': include }

def main():
    """Main function."""
    if len(sys.argv) > 1:
        if sys.argv[1] == '--apps-only':
            # Output only the app types for simpler usage
            config = load_config()
            ci_apps = []
            for app_name, app_config in config['apps'].items():
                if app_config.get('ci_enabled', True):
                    ci_apps.append(app_name)
            print(json.dumps(ci_apps))
            return
        elif sys.argv[1] == '--build-types-only':
            # Output only the build types (flattened from nested arrays)
            config = load_config()
            build_types_per_idf = config['metadata'].get('default_build_types', [['Debug', 'Release']])
            # Flatten the nested arrays to get all unique build types
            all_build_types = []
            for build_types in build_types_per_idf:
                all_build_types.extend(build_types)
            # Remove duplicates while preserving order
            unique_build_types = list(dict.fromkeys(all_build_types))
            print(json.dumps(unique_build_types))
            return
        elif sys.argv[1] == '--metadata':
            # Output metadata
            config = load_config()
            metadata = config['metadata']
            print(json.dumps(metadata))
            return
        elif sys.argv[1] == '--pretty':
            # Generate full matrix with pretty printing (for debugging)
            matrix_config = generate_matrix()
            print(json.dumps(matrix_config, indent=2))
            return
        elif sys.argv[1] == '--hierarchical-info':
            # Show hierarchical configuration information
            config = load_config()
            print("=== Hierarchical Configuration Analysis ===")
            print(f"Global IDF versions: {config['metadata'].get('idf_versions', ['release/v5.5'])}")
            print(f"Global build types per IDF: {config['metadata'].get('default_build_types', [['Debug', 'Release']])}")
            
            # Show IDF version to build types mapping
            global_idf_versions = config['metadata'].get('idf_versions', ['release/v5.5'])
            global_build_types_per_idf = config['metadata'].get('default_build_types', [['Debug', 'Release']])
            print("\nIDF Version to Build Types Mapping:")
            for i, idf_version in enumerate(global_idf_versions):
                if i < len(global_build_types_per_idf):
                    print(f"  {idf_version}: {global_build_types_per_idf[i]}")
                else:
                    print(f"  {idf_version}: ['Debug', 'Release'] (fallback)")
            
            print("\nPer-app overrides:")
            for app_name, app_config in config['apps'].items():
                if app_config.get('ci_enabled', True):
                    app_idf = app_config.get('idf_versions', 'uses global')
                    app_build = app_config.get('build_types', 'uses global')
                    print(f"  {app_name}:")
                    print(f"    IDF versions: {app_idf}")
                    print(f"    Build types: {app_build}")
            return
    
    # Generate full matrix in compact format for GitHub Actions
    matrix_config = generate_matrix()
    print(json.dumps(matrix_config))

if __name__ == '__main__':
    main()

