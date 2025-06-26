# 🌐 Publishing Documentation to GitHub Pages

This guide describes how to build the documentation and deploy it to **GitHub Pages** so your project has a polished website with navigation links.

## 🚀 Overview

1. **Build HTML with Doxygen** – Generate the API reference from your headers and source files.
2. **Create a GitHub Actions workflow** – Automate the build and deployment process.
3. **Enable GitHub Pages** – Serve the generated site from the `gh-pages` branch.

## 🔧 Prerequisites

- A GitHub repository with this project
- Doxygen installed locally (`sudo apt-get install doxygen`)
- Basic knowledge of GitHub Actions

## 🏗️ Building the Docs

Run the following from the repository root:

```bash
# Generate HTML documentation
$ doxygen Doxyfile
```

This creates an `html/` directory containing the rendered site. Verify `html/index.html` opens correctly in your browser.

## 🤖 GitHub Actions Workflow

Create `.github/workflows/deploy-docs.yml` in your repository with the following contents:

```yaml
name: Deploy Docs

on:
  push:
    branches: [main]

jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install Doxygen
        run: sudo apt-get update && sudo apt-get install -y doxygen
      - name: Build Docs
        run: doxygen Doxyfile
      - name: Deploy to GitHub Pages
        uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./html
```

The workflow builds the docs on every push to `main` and publishes them to the `gh-pages` branch.

## 🌍 Enabling GitHub Pages

1. Navigate to **Settings → Pages** in your GitHub repository.
2. Select **Deploy from a branch**.
3. Choose the `gh-pages` branch and `/` root folder.
4. Save the settings – your site will be available at `https://<user>.github.io/<repo>/`.

## 🔗 Navigation Helpers

To make your docs easier to browse, add navigation links to the bottom of each Markdown page:

```markdown
[⬅️ Prev](../examples/basic-gpio.md) | [⬆️ Index](../index.md) | [Next ➡️](../examples/basic-adc.md)
```

These links allow readers to move sequentially through the documentation and quickly return to the index page.

## ✅ Summary

With a small workflow file and GitHub Pages enabled, you can host a beautiful documentation site directly from your repository. Combine this with the existing Markdown guides and API reference to deliver a professional browsing experience.

