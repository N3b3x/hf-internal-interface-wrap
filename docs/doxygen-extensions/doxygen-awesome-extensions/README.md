# Doxygen Awesome Extensions

This directory contains the doxygen-awesome-css extensions for enhanced Doxygen documentation.

## Files

### CSS Files
- `doxygen-awesome.css` - Main stylesheet with modern styling and dark mode support
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

These files are copied from the [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) submodule located at `docs/assets/doxygen-awesome-css/`.

## Configuration

The extensions are configured in `header.html` and referenced in `Doxyfile`:

- `HTML_HEADER = header.html`
- `HTML_EXTRA_FILES` includes all JavaScript files
- `HTML_EXTRA_STYLESHEET` includes the main CSS file
- `HTML_COPY_CLIPBOARD = NO` (required for fragment copy button)

## Directory Structure

```
docs/assets/
├── doxygen-awesome-css/           # Submodule with full doxygen-awesome-css repository
└── doxygen-awesome-extensions/    # This directory - copies of extension files
    ├── doxygen-awesome.css
    ├── doxygen-awesome-*.js
    └── README.md
```

This organization keeps the submodule clean while providing easy access to the specific extension files needed for the documentation.
