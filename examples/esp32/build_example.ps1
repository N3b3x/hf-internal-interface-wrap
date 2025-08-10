# Build script for different ESP32 examples (PowerShell version)
# Usage: .\build_example.ps1 [example_type] [build_type]
# 
# Example types and build types are loaded from examples_config.yml
# Use '.\build_example.ps1 list' to see all available examples

param(
    [string]$ExampleType = "",
    [string]$BuildType = ""
)

$ErrorActionPreference = "Stop"

# Load configuration
$ProjectDir = $PSScriptRoot
. "$ProjectDir\scripts\config_loader.ps1"

# Set defaults from configuration
if (-not $ExampleType) { $ExampleType = Get-DefaultExample }
if (-not $BuildType) { $BuildType = Get-DefaultBuildType }

# Handle special commands
if ($ExampleType -eq "list") {
    Write-Host "=== Available Example Types ===" -ForegroundColor Green
    Write-Host "Featured examples:" -ForegroundColor Yellow
    foreach ($example in Get-FeaturedExampleTypes) {
        $description = Get-ExampleDescription $example
        Write-Host "  $example - $description" -ForegroundColor White
    }
    Write-Host ""
    Write-Host "All examples:" -ForegroundColor Yellow
    foreach ($example in Get-ExampleTypes) {
        $description = Get-ExampleDescription $example
        Write-Host "  $example - $description" -ForegroundColor White
    }
    Write-Host ""
    Write-Host "Build types: $((Get-BuildTypes) -join ', ')" -ForegroundColor Yellow
    exit 0
}

Write-Host "=== ESP32 HardFOC Interface Wrapper Build System ===" -ForegroundColor Green
Write-Host "Project Directory: $ProjectDir"
Write-Host "Example Type: $ExampleType"
Write-Host "Build Type: $BuildType"
Write-Host "======================================================" -ForegroundColor Green

# Validate example type
if (Test-ValidExampleType $ExampleType) {
    Write-Host "Valid example type: $ExampleType" -ForegroundColor Green
    $description = Get-ExampleDescription $ExampleType
    Write-Host "Description: $description" -ForegroundColor Cyan
} else {
    Write-Host "ERROR: Invalid example type: $ExampleType" -ForegroundColor Red
    Write-Host "Available types: $((Get-ExampleTypes) -join ', ')" -ForegroundColor Yellow
    Write-Host "Use '.\build_example.ps1 list' to see all examples with descriptions" -ForegroundColor Yellow
    exit 1
}

# Validate build type
if (Test-ValidBuildType $BuildType) {
    Write-Host "Valid build type: $BuildType" -ForegroundColor Green
} else {
    Write-Host "ERROR: Invalid build type: $BuildType" -ForegroundColor Red
    Write-Host "Available types: $((Get-BuildTypes) -join ', ')" -ForegroundColor Yellow
    exit 1
}

# Switch to project directory
Set-Location $ProjectDir

# Set build directory using configuration
$BuildDir = Get-BuildDirectory $ExampleType $BuildType
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

    # Get actual binary name from configuration
    $ProjectName = Get-ProjectName $ExampleType
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
