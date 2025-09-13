---
layout: default
title: "Extensions"
nav_order: 2
parent: "Doxygen Extensions"
description: "Working copies of doxygen-awesome extension files used by the documentation"
has_children: false
has_toc: true
---

# Doxygen Awesome Extensions

The HardFOC Interface Wrapper documentation uses the doxygen-awesome-css theme directly from the submodule for modern styling and interactive features.

## Theme Files

The theme files are located in the `doxygen-awesome-css/` submodule:

### CSS Files
- `doxygen-awesome.css` - Main stylesheet with modern styling and dark mode support
- `doxygen-awesome-sidebar-only.css` - Sidebar-only theme for better navigation
- `doxygen-awesome-sidebar-only-darkmode-toggle.css` - Additional CSS for sidebar-only dark mode toggle

### JavaScript Extensions
- `doxygen-awesome-darkmode-toggle.js` - Dark/light mode toggle with automatic system preference detection
- `doxygen-awesome-fragment-copy-button.js` - Copy code fragments to clipboard on hover
- `doxygen-awesome-paragraph-link.js` - Link to specific paragraphs/sections
- `doxygen-awesome-interactive-toc.js` - Interactive table of contents with progress highlighting
- `doxygen-awesome-tabs.js` - Tabbed content organization

## Features

- **🌓 Automatic Dark Mode**: Detects system preference and switches automatically
- **📋 Copy Buttons**: Hover over code blocks to copy them
- **🔗 Paragraph Links**: Click the ¶ symbol to get direct links to sections
- **📑 Interactive TOC**: Dynamic table of contents that highlights current section
- **📑 Tabs**: Organize content in tabbed interfaces
- **📱 Responsive**: Works on desktop and mobile devices

## Source

These files come directly from the [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) submodule located at `doxygen-awesome-css/`.

## Configuration

The extensions are configured in the root `Doxyfile`:

- `HTML_EXTRA_FILES` includes all JavaScript files from the submodule
- `HTML_EXTRA_STYLESHEET` includes the main CSS files from the submodule
- `HTML_COPY_CLIPBOARD = NO` (required for fragment copy button)

## Directory Structure

```
docs/doxygen-extensions/
├── doxygen-awesome-css/           # Submodule with theme files only
│   ├── doxygen-awesome.css
│   ├── doxygen-awesome-sidebar-only.css
│   ├── doxygen-awesome-*.js
│   └── .git                       # Git submodule reference
├── README.md                      # Configuration documentation
├── index.md                       # Main documentation page
└── extensions.md                  # This file
```

This streamlined organization uses the submodule directly without unnecessary file duplication.
