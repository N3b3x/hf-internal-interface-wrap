# Flash and monitor script for different ESP32 examples (PowerShell version)
# Usage: .\flash_example.ps1 [example_type] [build_type] [operation]
#
# Example types and build types are loaded from examples_config.yml
# Use '.\flash_example.ps1 list' to see all available examples
# Operations: flash, monitor, flash_monitor (default: flash_monitor)

param(
    [string]$ExampleType = "",
    [string]$BuildType = "",
    [string]$Operation = "flash_monitor"
)

$ErrorActionPreference = "Stop"

# Load configuration
$ProjectDir = $PSScriptRoot
. "$ProjectDir\scripts\config_loader.ps1"
$ValidOperations = @("flash", "monitor", "flash_monitor")

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
    Write-Host "Operations: flash, monitor, flash_monitor" -ForegroundColor Yellow
    exit 0
}

# Ensure ESP32 target is set from config
$env:IDF_TARGET = Get-TargetPlatform

Write-Host "=== ESP32 HardFOC Interface Wrapper Flash System ===" -ForegroundColor Green
Write-Host "Project Directory: $ProjectDir"
Write-Host "Example Type: $ExampleType"
Write-Host "Build Type: $BuildType"
Write-Host "Operation: $Operation"
Write-Host "Target: $($env:IDF_TARGET)"
Write-Host "======================================================" -ForegroundColor Green

# Validate example type
if (Test-ValidExampleType $ExampleType) {
    Write-Host "Valid example type: $ExampleType" -ForegroundColor Green
    $description = Get-ExampleDescription $ExampleType
    if ($description) { Write-Host "Description: $description" -ForegroundColor Cyan }
} else {
    Write-Host "ERROR: Invalid example type: $ExampleType" -ForegroundColor Red
    Write-Host "Available types: $((Get-ExampleTypes) -join ', ')" -ForegroundColor Yellow
    Write-Host "Use '.\flash_example.ps1 list' to see all examples with descriptions" -ForegroundColor Yellow
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

# Validate operation
if ($Operation -notin $ValidOperations) {
    Write-Host "ERROR: Invalid operation: $Operation" -ForegroundColor Red
    Write-Host "Available operations: $($ValidOperations -join ', ')" -ForegroundColor Yellow
    exit 1
}
Write-Host "Valid operation: $Operation" -ForegroundColor Green

# Switch to project directory
Set-Location $ProjectDir

# Set build directory using configuration
$BuildDir = Get-BuildDirectory $ExampleType $BuildType
Write-Host "Build directory: $BuildDir" -ForegroundColor Blue

# Get project information using configuration
$ProjectName = Get-ProjectName $ExampleType
$BinFile = "$BuildDir\$ProjectName.bin"

# Check if build exists and is valid
$BuildExists = $false
if (Test-Path $BuildDir) {
    if ((Test-Path $BinFile) -or (Test-Path "$BuildDir\bootloader\bootloader.bin")) {
        Write-Host "Found existing build in $BuildDir" -ForegroundColor Green
        $BuildExists = $true
    } else {
        Write-Host "Build directory exists but no valid binary found" -ForegroundColor Yellow
    }
} else {
    Write-Host "No build directory found" -ForegroundColor Yellow
}

# Auto-build if necessary
if (-not $BuildExists) {
    Write-Host ""
    Write-Host "======================================================" -ForegroundColor Red
    Write-Host "NO VALID BUILD FOUND - STARTING AUTO-BUILD" -ForegroundColor Red
    Write-Host "======================================================" -ForegroundColor Red
    Write-Host "Building $ExampleType ($BuildType) before flashing..." -ForegroundColor Yellow
    
    # Clean any existing incomplete build
    if (Test-Path $BuildDir) {
        Write-Host "Cleaning incomplete build..." -ForegroundColor Yellow
        Remove-Item -Recurse -Force $BuildDir
    }
    
    try {
        # Configure and build
        Write-Host "Configuring project for $($env:IDF_TARGET)..." -ForegroundColor Blue
        & idf.py -B $BuildDir -D CMAKE_BUILD_TYPE=$BuildType -D EXAMPLE_TYPE=$ExampleType reconfigure
        
        if ($LASTEXITCODE -ne 0) {
            throw "Configuration failed"
        }
        
        Write-Host "Building project..." -ForegroundColor Blue
        & idf.py -B $BuildDir build
        
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed"
        }
        
        Write-Host "Build completed successfully!" -ForegroundColor Green
        Write-Host "======================================================" -ForegroundColor Green
    } catch {
        Write-Host "ERROR: Auto-build failed: $_" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "Using existing build in $BuildDir" -ForegroundColor Green
}

# Verify binary exists after build
if (-not (Test-Path $BinFile) -and -not (Test-Path "$BuildDir\bootloader\bootloader.bin")) {
    Write-Host "ERROR: No valid binary found after build attempt" -ForegroundColor Red
    Write-Host "Expected: $BinFile" -ForegroundColor Yellow
    Write-Host "Build directory contents:" -ForegroundColor Yellow
    try {
        Get-ChildItem $BuildDir | Format-Table Name, Length, LastWriteTime
    } catch {
        Write-Host "Build directory not accessible" -ForegroundColor Red
    }
    exit 1
}

# Execute the requested operation
Write-Host ""
Write-Host "======================================================" -ForegroundColor Cyan
Write-Host "EXECUTING OPERATION: $Operation" -ForegroundColor Cyan
Write-Host "======================================================" -ForegroundColor Cyan

try {
    switch ($Operation) {
        "flash" {
            Write-Host "Flashing $ExampleType example..." -ForegroundColor Blue
            & idf.py -B $BuildDir flash
            if ($LASTEXITCODE -ne 0) {
                throw "Flash operation failed"
            }
            Write-Host "Flash completed successfully!" -ForegroundColor Green
        }
        "monitor" {
            Write-Host "Starting monitor for $ExampleType example..." -ForegroundColor Blue
            Write-Host "Press Ctrl+] to exit monitor" -ForegroundColor Yellow
            & idf.py -B $BuildDir monitor
            if ($LASTEXITCODE -ne 0) {
                throw "Monitor operation failed"
            }
        }
        "flash_monitor" {
            Write-Host "Flashing and monitoring $ExampleType example..." -ForegroundColor Blue
            Write-Host "Press Ctrl+] to exit monitor after flashing" -ForegroundColor Yellow
            & idf.py -B $BuildDir flash monitor
            if ($LASTEXITCODE -ne 0) {
                throw "Flash and monitor operation failed"
            }
        }
    }

    Write-Host ""
    Write-Host "======================================================" -ForegroundColor Green
    Write-Host "OPERATION COMPLETED SUCCESSFULLY" -ForegroundColor Green
    Write-Host "======================================================" -ForegroundColor Green
    Write-Host "Example Type: $ExampleType" -ForegroundColor Cyan
    Write-Host "Build Type: $BuildType" -ForegroundColor Cyan
    Write-Host "Operation: $Operation" -ForegroundColor Cyan
    Write-Host "Target: $($env:IDF_TARGET)" -ForegroundColor Cyan
    Write-Host "Build Directory: $BuildDir" -ForegroundColor Cyan
    Write-Host "Project Name: $ProjectName" -ForegroundColor Cyan
    if (Test-Path $BinFile) {
        Write-Host "Binary: $BinFile" -ForegroundColor Cyan
    }
    Write-Host ""
    Write-Host "Available operations:" -ForegroundColor Yellow
    Write-Host "  Flash only:        .\flash_example.ps1 $ExampleType $BuildType flash" -ForegroundColor White
    Write-Host "  Monitor only:      .\flash_example.ps1 $ExampleType $BuildType monitor" -ForegroundColor White
    Write-Host "  Flash & monitor:   .\flash_example.ps1 $ExampleType $BuildType flash_monitor" -ForegroundColor White
    Write-Host "  Build only:        .\build_example.ps1 $ExampleType $BuildType" -ForegroundColor White
    Write-Host "======================================================" -ForegroundColor Green

} catch {
    Write-Host "OPERATION FAILED: $_" -ForegroundColor Red
    Write-Host "Check the error messages above for details." -ForegroundColor Yellow
    exit 1
}

