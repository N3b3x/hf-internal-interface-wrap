---
layout: default
title: "Configuration"
nav_order: 1
parent: "Doxygen Extensions"
description: "Doxygen configuration and setup files for the HardFOC Interface Wrapper documentation"
has_children: false
has_toc: true
---

# Doxygen Extensions Configuration

This directory contains all Doxygen-related content for generating the API documentation.

## Contents

- **`doxygen-awesome-css/`** - Submodule containing the doxygen-awesome-css theme files

## Features

The API documentation includes:

- **🌓 Automatic Dark Mode** - Detects system preference and switches automatically
- **📋 Copy Buttons** - Hover over code blocks to copy them
- **🔗 Paragraph Links** - Click the ¶ symbol to get direct links to sections
- **📑 Interactive TOC** - Dynamic table of contents with progress highlighting
- **📑 Tabs** - Organize content in tabbed interfaces
- **📱 Responsive** - Works on desktop and mobile devices

## Configuration

The Doxygen configuration is set up in the root `Doxyfile`:

- `HTML_EXTRA_FILES` - References JavaScript extension files
- `HTML_EXTRA_STYLESHEET` - References the main CSS files
- `HTML_COPY_CLIPBOARD = NO` - Required for fragment copy button

## Directory Structure

```
docs/doxygen-extensions/
├── README.md                           # This file
├── index.md                            # Main documentation page
├── extensions.md                       # Extensions documentation
└── doxygen-awesome-css/                # Submodule (theme files only)
    ├── doxygen-awesome.css
    ├── doxygen-awesome-sidebar-only.css
    ├── doxygen-awesome-*.js
    └── .git                            # Git submodule reference
```

## Usage

To generate the API documentation:

```bash
doxygen Doxyfile
```

The generated documentation will be output to `docs/doxygen/` (as configured in the Doxyfile).

## Updating Extensions

When the doxygen-awesome-css submodule is updated:

1. Update the submodule: `git submodule update --remote`
2. Check if new JavaScript files need to be added to `HTML_EXTRA_FILES` in `Doxyfile`
3. Test the documentation generation to ensure all features work correctly
