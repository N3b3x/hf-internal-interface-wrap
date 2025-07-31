# Build script for different ESP32 examples (PowerShell version)
# Usage: .\build_example.ps1 [example_type] [build_type]
# 
# Example types:
#   comprehensive  - Main integration test (default)
#   ascii_art      - ASCII art generator example
#
# Build types: Debug, Release (default: Release)

param(
    [string]$ExampleType = "comprehensive",
    [string]$BuildType = "Release"
)

$ErrorActionPreference = "Stop"

# Configuration
$ProjectDir = $PSScriptRoot
$ValidExampleTypes = @("comprehensive", "ascii_art")
$ValidBuildTypes = @("Debug", "Release")

Write-Host "=== ESP32 HardFOC Interface Wrapper Build System ===" -ForegroundColor Green
Write-Host "Project Directory: $ProjectDir"
Write-Host "Example Type: $ExampleType"
Write-Host "Build Type: $BuildType"
Write-Host "======================================================" -ForegroundColor Green

# Validate example type
if ($ExampleType -notin $ValidExampleTypes) {
    Write-Host "ERROR: Invalid example type: $ExampleType" -ForegroundColor Red
    Write-Host "Available types: $($ValidExampleTypes -join ', ')" -ForegroundColor Yellow
    exit 1
}
Write-Host "Valid example type: $ExampleType" -ForegroundColor Green

# Validate build type
if ($BuildType -notin $ValidBuildTypes) {
    Write-Host "ERROR: Invalid build type: $BuildType" -ForegroundColor Red
    Write-Host "Available types: $($ValidBuildTypes -join ', ')" -ForegroundColor Yellow
    exit 1
}
Write-Host "Valid build type: $BuildType" -ForegroundColor Green

# Switch to project directory
Set-Location $ProjectDir

# Set build directory
$BuildDir = "build_${ExampleType}_${BuildType}"
Write-Host "Build directory: $BuildDir" -ForegroundColor Blue

# Clean previous build if it exists
if (Test-Path $BuildDir) {
    Write-Host "Cleaning previous build..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

# Configure and build
try {
    Write-Host "Configuring project..." -ForegroundColor Blue
    & idf.py -B $BuildDir -D CMAKE_BUILD_TYPE=$BuildType -D EXAMPLE_TYPE=$ExampleType reconfigure
    
    if ($LASTEXITCODE -ne 0) {
        throw "Configuration failed"
    }

    Write-Host "Building project..." -ForegroundColor Blue
    & idf.py -B $BuildDir build
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }

    # Get actual binary name from build output
    $ProjectName = "esp32_iid_${ExampleType}_example"
    $BinFile = "$BuildDir\$ProjectName.bin"
    
    Write-Host "======================================================" -ForegroundColor Green
    Write-Host "BUILD COMPLETED SUCCESSFULLY" -ForegroundColor Green
    Write-Host "======================================================" -ForegroundColor Green
    Write-Host "Example Type: $ExampleType" -ForegroundColor Cyan
    Write-Host "Build Type: $BuildType" -ForegroundColor Cyan
    Write-Host "Build Directory: $BuildDir" -ForegroundColor Cyan
    Write-Host "Project Name: $ProjectName" -ForegroundColor Cyan
    if (Test-Path $BinFile) {
        Write-Host "Binary: $BinFile" -ForegroundColor Cyan
    } else {
        Write-Host "Binary: Check $BuildDir for output files" -ForegroundColor Cyan
    }
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Yellow
    Write-Host "  Flash and monitor: idf.py -B $BuildDir flash monitor" -ForegroundColor White
    Write-Host "  Monitor only:      idf.py -B $BuildDir monitor" -ForegroundColor White
    Write-Host "  Size analysis:     idf.py -B $BuildDir size" -ForegroundColor White
    Write-Host "======================================================" -ForegroundColor Green

} catch {
    Write-Host "BUILD FAILED: $_" -ForegroundColor Red
    Write-Host "Check the error messages above for details." -ForegroundColor Yellow
    exit 1
}
