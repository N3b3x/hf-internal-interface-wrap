# Doxygen Extensions

This directory contains all Doxygen-related content for generating the API documentation.

## Contents

- **`header.html`** - Custom Doxygen header template with doxygen-awesome extensions
- **`doxygen-awesome-css/`** - Submodule containing the full doxygen-awesome-css repository
- **`doxygen-awesome-extensions/`** - Working copies of extension files used by Doxygen

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

- `HTML_HEADER = docs/doxygen-extensions/header.html`
- `HTML_EXTRA_FILES` - References extension files
- `HTML_EXTRA_STYLESHEET` - References the main CSS file
- `HTML_COPY_CLIPBOARD = NO` - Required for fragment copy button

## Directory Structure

```
docs/doxygen-extensions/
├── README.md                           # This file
├── header.html                         # Custom Doxygen header template
├── doxygen-awesome-css/                # Submodule (full repository)
│   ├── doxygen-awesome.css
│   ├── doxygen-awesome-*.js
│   └── ... (full submodule contents)
└── doxygen-awesome-extensions/         # Working copies
    ├── README.md
    ├── doxygen-awesome.css
    ├── doxygen-awesome-*.js
    └── doxygen-awesome-sidebar-only-darkmode-toggle.css
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
2. Copy new extension files to `doxygen-awesome-extensions/` if needed
3. Update the `header.html` if new features are added
