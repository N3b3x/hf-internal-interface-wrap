#!/usr/bin/env python3
"""Simple documentation link checker.

Scan markdown files for local links and verify the referenced files exist.
"""
import os
import re
import sys

RE_LINK = re.compile(r"\[.+?\]\(([^)]+)\)")


def check_file(md_path: str) -> int:
    root = os.path.dirname(md_path)
    missing = 0
    with open(md_path, 'r', encoding='utf-8') as f:
        for line_num, line in enumerate(f, 1):
            for match in RE_LINK.findall(line):
                # Ignore external links and function signatures
                if (
                    match.startswith('http')
                    or match.startswith('#')
                    or '://' in match
                    or match.startswith('mailto:')
                    or '&' in match  # Function signature with reference
                    or match.startswith('const ')  # Function signature with const
                    or match.startswith('../../')  # External references outside docs
                    or 'void*' in match  # Function signature with void pointer
                    or match.startswith('hf_')  # Function signature with hf_ prefix
                    or match.startswith('../.github/')  # External GitHub references
                    or match.startswith('../CONTRIBUTING.md')  # External contributing guide
                ):
                    continue
                
                # Handle cross-directory references within docs/
                if match.startswith('api/') or match.startswith('esp_api/'):
                    # This is a cross-directory reference, check from docs root
                    docs_root = os.path.dirname(root) if os.path.basename(root) in ['api', 'esp_api'] else root
                    path = os.path.normpath(os.path.join(docs_root, match))
                elif match.startswith('..'):
                    # Relative path going up directories
                    path = os.path.normpath(os.path.join(root, match))
                else:
                    # Local file reference
                    path = os.path.normpath(os.path.join(root, match))
                
                if not os.path.exists(path):
                    print(f"{md_path}:{line_num} missing file {match}")
                    missing += 1
    return missing


def main(paths):
    errors = 0
    for path in paths:
        if os.path.isdir(path):
            for dirpath, _dirs, files in os.walk(path):
                for f in files:
                    if f.endswith('.md'):
                        errors += check_file(os.path.join(dirpath, f))
        else:
            errors += check_file(path)
    if errors:
        print(f"Found {errors} broken links")
    return 0 if errors == 0 else 1

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
