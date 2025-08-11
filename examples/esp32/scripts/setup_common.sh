#!/bin/bash
# Common Setup Functions for ESP32 HardFOC Interface Wrapper
# This script contains shared functions used by both local and CI setup scripts

set -e  # Exit on any error

# Colors for output (only used in local setup)
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output (only used in local setup)
print_status() {
    if [[ "${SETUP_MODE}" == "local" ]]; then
        echo -e "${BLUE}[INFO]${NC} $1"
    else
        echo "[INFO] $1"
    fi
}

print_success() {
    if [[ "${SETUP_MODE}" == "local" ]]; then
        echo -e "${GREEN}[SUCCESS]${NC} $1"
    else
        echo "[SUCCESS] $1"
    fi
}

print_warning() {
    if [[ "${SETUP_MODE}" == "local" ]]; then
        echo -e "${YELLOW}[WARNING]${NC} $1"
    else
        echo "[WARNING] $1"
    fi
}

print_error() {
    if [[ "${SETUP_MODE}" == "local" ]]; then
        echo -e "${RED}[ERROR]${NC} $1"
    else
        echo "[ERROR] $1"
    fi
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command_exists apt-get; then
            echo "ubuntu"
        elif command_exists dnf; then
            echo "fedora"
        elif command_exists yum; then
            echo "centos"
        else
            echo "linux"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    else
        echo "unknown"
    fi
}

# Function to install system dependencies
install_system_deps() {
    print_status "Installing system dependencies..."
    
    local os=$(detect_os)
    
    case $os in
        "ubuntu")
            sudo apt-get update
            sudo apt-get install -y \
                build-essential \
                cmake \
                ninja-build \
                git \
                wget \
                curl \
                python3 \
                python3-pip \
                python3-venv \
                pkg-config \
                libssl-dev \
                libffi-dev \
                libncurses5-dev \
                libreadline-dev \
                libsqlite3-dev \
                libbz2-dev \
                libexpat1-dev \
                liblzma-dev \
                zlib1g-dev \
                libgdbm-dev \
                libnss3-dev
            ;;
        "fedora")
            sudo dnf install -y \
                gcc \
                gcc-c++ \
                cmake \
                ninja-build \
                git \
                wget \
                curl \
                python3 \
                python3-pip \
                python3-devel \
                pkg-config \
                openssl-devel \
                libffi-devel \
                ncurses-devel \
                readline-devel \
                sqlite-devel \
                bzip2-devel \
                expat-devel \
                xz-devel \
                zlib-devel \
                gdbm-devel \
                nss-devel
            ;;
        "centos")
            sudo yum install -y \
                gcc \
                gcc-c++ \
                cmake \
                ninja-build \
                git \
                wget \
                curl \
                python3 \
                python3-pip \
                python3-devel \
                pkg-config \
                openssl-devel \
                libffi-devel \
                ncurses-devel \
                readline-devel \
                sqlite-devel \
                bzip2-devel \
                expat-devel \
                xz-devel \
                zlib-devel \
                gdbm-devel \
                nss-devel
            ;;
        "macos")
            if command_exists brew; then
                brew install \
                    cmake \
                    ninja \
                    git \
                    wget \
                    curl \
                    python3 \
                    pkg-config \
                    openssl \
                    readline \
                    sqlite3 \
                    bzip2 \
                    expat \
                    xz \
                    zlib
            else
                print_error "Homebrew not found. Please install Homebrew first."
                return 1
            fi
            ;;
        *)
            print_error "Unsupported operating system: $os"
            return 1
            ;;
    esac
    
    print_success "System dependencies installed"
}

# Function to install clang tools
install_clang_tools() {
    print_status "Installing clang tools..."
    
    local os=$(detect_os)
    
    case $os in
        "ubuntu")
            # Add LLVM APT repository for newer clang versions if needed
            if ! apt-cache search clang-18 | grep -q "clang-18"; then
                print_status "Adding LLVM APT repository for newer clang versions..."
                wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add - 2>/dev/null || true
                echo "deb http://apt.llvm.org/noble/ llvm-toolchain-noble-18 main" | sudo tee /etc/apt/sources.list.d/llvm.list >/dev/null || true
                sudo apt-get update || true
            fi
            
            # Try to install the latest available clang version
            # First try clang-18, fallback to default clang if not available
            if apt-cache search clang-18 | grep -q "clang-18"; then
                print_status "Installing clang-18 and tools..."
                sudo apt-get install -y \
                    clang-18 \
                    clang-format-18 \
                    clang-tidy-18 \
                    clang-tools-18 \
                    cppcheck \
                    valgrind \
                    gdb \
                    make
                
                # Set clang-18 as default
                if command_exists update-alternatives; then
                    print_status "Setting clang-18 as default..."
                    sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100
                    sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100
                    sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-18 100
                    sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-18 100
                fi
            else
                print_status "Installing default clang and tools..."
                sudo apt-get install -y \
                    clang \
                    clang-format \
                    clang-tidy \
                    clang-tools \
                    cppcheck \
                    valgrind \
                    gdb \
                    make
            fi
            ;;
        "fedora"|"centos")
            sudo dnf install -y \
                clang \
                clang-tools-extra \
                cppcheck \
                valgrind \
                gdb \
                make
            ;;
        "macos")
            if command_exists brew; then
                brew install \
                    llvm \
                    cppcheck \
                    valgrind \
                    gdb \
                    make
            fi
            ;;
    esac
    
    print_success "Clang tools installed"
}

# Function to install yq
install_yq() {
    print_status "Installing yq..."
    
    if command_exists yq; then
        local yq_version=$(yq --version 2>/dev/null | grep -oE '[0-9]+\.[0-9]+' | head -1)
        print_status "yq already installed: version $yq_version"
        return 0
    fi
    
    local os=$(detect_os)
    
    case $os in
        "ubuntu"|"fedora"|"centos")
            # Try package manager first
            if [[ "$os" == "ubuntu" ]]; then
                sudo apt-get install -y yq
            elif [[ "$os" == "fedora" ]]; then
                sudo dnf install -y yq
            elif [[ "$os" == "centos" ]]; then
                sudo yum install -y yq
            fi
            
            # If package manager failed, install manually
            if ! command_exists yq; then
                print_status "Installing yq manually..."
                local yq_version="4.40.5"
                local arch=$(uname -m)
                
                if [[ "$arch" == "x86_64" ]]; then
                    local arch="amd64"
                elif [[ "$arch" == "aarch64" ]]; then
                    local arch="arm64"
                fi
                
                wget -O yq "https://github.com/mikefarah/yq/releases/download/v${yq_version}/yq_linux_${arch}"
                chmod +x yq
                sudo mv yq /usr/local/bin/
            fi
            ;;
        "macos")
            if command_exists brew; then
                brew install yq
            else
                print_error "Homebrew not found. Please install Homebrew first."
                return 1
            fi
            ;;
    esac
    
    if command_exists yq; then
        print_success "yq installed successfully"
    else
        print_error "Failed to install yq"
        return 1
    fi
}

# Function to install ESP-IDF
install_esp_idf() {
    print_status "Installing ESP-IDF..."
    
    local esp_dir="$HOME/esp"
    local idf_dir="$esp_dir/esp-idf"
    
    # Create esp directory if it doesn't exist
    mkdir -p "$esp_dir"
    
    if [[ -d "$idf_dir" ]]; then
        print_status "ESP-IDF already exists, updating..."
        cd "$idf_dir"
        git fetch origin
        git checkout release/v5.5
        git pull origin release/v5.5
        ./install.sh esp32c6
        cd - > /dev/null
    else
        print_status "Cloning ESP-IDF..."
        cd "$esp_dir"
        git clone --recursive --branch release/v5.5 https://github.com/espressif/esp-idf.git
        cd esp-idf
        ./install.sh esp32c6
        cd - > /dev/null
    fi
    
    print_success "ESP-IDF installed/updated"
}

# Function to install Python dependencies
install_python_deps() {
    print_status "Installing Python dependencies..."
    
    # Upgrade pip
    python3 -m pip install --upgrade pip
    
    # Install required packages
    python3 -m pip install pyyaml
    
    print_success "Python dependencies installed"
}

# Function to setup environment variables
setup_environment_vars() {
    print_status "Setting up environment variables..."
    
    local bashrc="$HOME/.bashrc"
    local profile="$HOME/.profile"
    
    # Add ESP-IDF to PATH
    if ! grep -q "esp-idf" "$bashrc" 2>/dev/null; then
        echo "" >> "$bashrc"
        echo "# ESP-IDF Environment" >> "$bashrc"
        echo "export IDF_PATH=\"\$HOME/esp/esp-idf\"" >> "$bashrc"
        echo "alias get_idf='. \$HOME/esp/esp-idf/export.sh'" >> "$bashrc"
    fi
    
    # Add to profile for non-interactive shells
    if ! grep -q "esp-idf" "$profile" 2>/dev/null; then
        echo "" >> "$profile"
        echo "# ESP-IDF Environment" >> "$profile"
        echo "export IDF_PATH=\"\$HOME/esp/esp-idf\"" >> "$profile"
    fi
    
    print_success "Environment variables configured"
}

# Function to verify installation
verify_installation() {
    print_status "Verifying installation..."
    
    local errors=0
    
    # Check required commands
    local required_commands=("cmake" "ninja" "git" "python3" "yq")
    for cmd in "${required_commands[@]}"; do
        if command_exists "$cmd"; then
            print_success "$cmd: $(command -v "$cmd")"
        else
            print_error "$cmd: NOT FOUND"
            ((errors++))
        fi
    done
    
    # Check clang tools
    print_status "Checking clang tools..."
    if command_exists clang; then
        local clang_version=$(clang --version | head -1)
        print_success "clang: $clang_version"
    else
        print_error "clang: NOT FOUND"
        ((errors++))
    fi
    
    if command_exists clang-format; then
        local format_version=$(clang-format --version | head -1)
        print_success "clang-format: $format_version"
    else
        print_error "clang-format: NOT FOUND"
        ((errors++))
    fi
    
    if command_exists clang-tidy; then
        local tidy_version=$(clang-tidy --version | head -1)
        print_success "clang-tidy: $tidy_version"
    else
        print_error "clang-tidy: NOT FOUND"
        ((errors++))
    fi
    
    # Check ESP-IDF
    if [[ -d "$HOME/esp/esp-idf" ]]; then
        print_success "ESP-IDF: $HOME/esp/esp-idf"
    else
        print_error "ESP-IDF: NOT FOUND"
        ((errors++))
    fi
    
    # Check Python packages
    if python3 -c "import yaml" 2>/dev/null; then
        print_success "PyYAML: installed"
    else
        print_error "PyYAML: NOT FOUND"
        ((errors++))
    fi
    
    if [[ $errors -eq 0 ]]; then
        print_success "All dependencies verified successfully!"
        if [[ "${SETUP_MODE}" == "local" ]]; then
            print_status "Next steps:"
            print_status "1. Restart your terminal or run: source ~/.bashrc"
            print_status "2. Navigate to examples/esp32 directory"
            print_status "3. Run: get_idf"
            print_status "4. Build examples with: ./scripts/build_example.sh <example_type> <build_type>"
        fi
    else
        print_error "Installation verification failed with $errors errors"
        return 1
    fi
}

# Function to setup CI-specific environment
setup_ci_environment() {
    print_status "Setting up CI environment..."
    
    # Set clang as default for CI (use whatever version was installed)
    if command_exists clang-18 && command_exists update-alternatives; then
        print_status "Setting clang-18 as default for CI..."
        sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100
        sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100
        sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-18 100
        sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-18 100
    elif command_exists clang && command_exists update-alternatives; then
        print_status "Setting default clang as default for CI..."
        # Find the actual clang version and set alternatives
        local clang_path=$(which clang)
        local clang_pp_path=$(which clang++)
        local clang_format_path=$(which clang-format)
        local clang_tidy_path=$(which clang-tidy)
        
        if [[ -n "$clang_path" ]]; then
            sudo update-alternatives --install /usr/bin/clang clang "$clang_path" 100
        fi
        if [[ -n "$clang_pp_path" ]]; then
            sudo update-alternatives --install /usr/bin/clang++ clang++ "$clang_pp_path" 100
        fi
        if [[ -n "$clang_format_path" ]]; then
            sudo update-alternatives --install /usr/bin/clang-format clang-format "$clang_format_path" 100
        fi
        if [[ -n "$clang_tidy_path" ]]; then
            sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy "$clang_tidy_path" 100
        fi
    fi
    
    print_success "CI environment configured"
}

# Function to setup local-specific environment
setup_local_environment() {
    print_status "Setting up local development environment..."
    
    # Setup environment variables
    setup_environment_vars
    
    # Create useful aliases
    local bashrc="$HOME/.bashrc"
    if ! grep -q "alias build_example" "$bashrc" 2>/dev/null; then
        echo "" >> "$bashrc"
        echo "# ESP32 Development Aliases" >> "$bashrc"
        echo "alias build_example='./scripts/build_example.sh'" >> "$bashrc"
        echo "alias flash_example='./scripts/flash_example.sh'" >> "$bashrc"
        echo "alias list_examples='./scripts/build_example.sh list'" >> "$bashrc"
    fi
    
    print_success "Local development environment configured"
}
