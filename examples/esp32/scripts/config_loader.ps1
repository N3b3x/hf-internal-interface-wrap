# Configuration loader for ESP32 examples (PowerShell version)
# This script provides functions to load and parse the examples_config.yml file

param()

$ErrorActionPreference = "Stop"

# Get the directory where this script is located
$ScriptDir = $PSScriptRoot
$ProjectDir = Split-Path $ScriptDir -Parent
$ConfigFile = Join-Path $ProjectDir "examples_config.yml"

# Global configuration variables
$Global:Config = @{}
$Global:ConfigLoaded = $false

# Check if powershell-yaml module is available
function Test-YamlModule {
    try {
        Import-Module powershell-yaml -ErrorAction Stop
        return $true
    } catch {
        Write-Warning "powershell-yaml module not found. Falling back to basic parsing."
        return $false
    }
}

# Load configuration using powershell-yaml (preferred method)
function Import-ConfigYaml {
    if (-not (Test-YamlModule)) {
        return $false
    }
    
    try {
        $yamlContent = Get-Content $ConfigFile -Raw
        $Global:Config = ConvertFrom-Yaml $yamlContent
        $Global:ConfigLoaded = $true
        return $true
    } catch {
        Write-Warning "Failed to parse YAML with powershell-yaml: $_"
        return $false
    }
}

# Fallback: Basic parsing without powershell-yaml
function Import-ConfigBasic {
    try {
        $content = Get-Content $ConfigFile
        
        # Initialize basic config structure
        $Global:Config = @{
            metadata = @{
                default_example = "ascii_art"
                default_build_type = "Release"
                target = "esp32c6"
            }
            examples = @{}
            build_config = @{
                build_directory_pattern = "build_{example_type}_{build_type}"
                project_name_pattern = "esp32_iid_{example_type}_example"
                build_types = @{
                    Debug = @{ description = "Debug symbols, verbose logging, assertions enabled" }
                    Release = @{ description = "Optimized for performance and size" }
                }
            }
        }
        
        # Extract metadata
        $metadataSection = $false
        $examplesSection = $false
        $currentExample = $null
        
        foreach ($line in $content) {
            $line = $line.Trim()
            
            if ($line -eq "metadata:") {
                $metadataSection = $true
                $examplesSection = $false
                continue
            } elseif ($line -eq "examples:") {
                $metadataSection = $false
                $examplesSection = $true
                continue
            } elseif ($line -match "^[a-zA-Z].*:$") {
                $metadataSection = $false
                $examplesSection = $false
                continue
            }
            
            if ($metadataSection -and $line -match '^\s*(\w+):\s*"?([^"]*)"?$') {
                $key = $matches[1]
                $value = $matches[2]
                $Global:Config.metadata[$key] = $value
            } elseif ($examplesSection -and $line -match '^\s+(\w+):$') {
                $currentExample = $matches[1]
                $Global:Config.examples[$currentExample] = @{
                    description = ""
                    source_file = ""
                    category = "utility"
                    featured = $false
                    ci_enabled = $true
                }
            } elseif ($examplesSection -and $currentExample -and $line -match '^\s+(\w+):\s*"?([^"]*)"?$') {
                $key = $matches[1]
                $value = $matches[2]
                if ($key -eq "featured" -or $key -eq "ci_enabled") {
                    $Global:Config.examples[$currentExample][$key] = ($value -eq "true")
                } else {
                    $Global:Config.examples[$currentExample][$key] = $value
                }
            }
        }
        
        $Global:ConfigLoaded = $true
        return $true
    } catch {
        Write-Error "Failed to parse configuration file: $_"
        return $false
    }
}

# Initialize configuration
function Initialize-Config {
    if ($Global:ConfigLoaded) {
        return $true
    }
    
    if (-not (Test-Path $ConfigFile)) {
        Write-Error "Configuration file not found: $ConfigFile"
        return $false
    }
    
    if (Import-ConfigYaml) {
        Write-Verbose "Configuration loaded using powershell-yaml"
        return $true
    } elseif (Import-ConfigBasic) {
        Write-Verbose "Configuration loaded using basic parsing"
        return $true
    } else {
        Write-Error "Failed to load configuration"
        return $false
    }
}

# Get list of valid example types
function Get-ExampleTypes {
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    return $Global:Config.examples.Keys
}

# Get list of valid build types
function Get-BuildTypes {
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    return $Global:Config.build_config.build_types.Keys
}

# Get description for an example type
function Get-ExampleDescription {
    param([string]$ExampleType)
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    if ($Global:Config.examples.ContainsKey($ExampleType)) {
        return $Global:Config.examples[$ExampleType].description
    }
    return ""
}

# Get source file for an example type
function Get-ExampleSourceFile {
    param([string]$ExampleType)
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    if ($Global:Config.examples.ContainsKey($ExampleType)) {
        return $Global:Config.examples[$ExampleType].source_file
    }
    return ""
}

# Check if example type is valid
function Test-ValidExampleType {
    param([string]$ExampleType)
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    return $Global:Config.examples.ContainsKey($ExampleType)
}

# Check if build type is valid
function Test-ValidBuildType {
    param([string]$BuildType)
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    return $Global:Config.build_config.build_types.ContainsKey($BuildType)
}

# Get build directory pattern
function Get-BuildDirectory {
    param([string]$ExampleType, [string]$BuildType)
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    $pattern = $Global:Config.build_config.build_directory_pattern
    $result = $pattern -replace '\{example_type\}', $ExampleType
    $result = $result -replace '\{build_type\}', $BuildType
    return $result
}

# Get project name pattern
function Get-ProjectName {
    param([string]$ExampleType)
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    $pattern = $Global:Config.build_config.project_name_pattern
    $result = $pattern -replace '\{example_type\}', $ExampleType
    return $result
}

# Get CI-enabled example types
function Get-CiExampleTypes {
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    $ciTypes = @()
    foreach ($example in $Global:Config.examples.GetEnumerator()) {
        if ($example.Value.ci_enabled -eq $true) {
            $ciTypes += $example.Key
        }
    }
    return $ciTypes
}

# Get featured example types
function Get-FeaturedExampleTypes {
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    $featuredTypes = @()
    foreach ($example in $Global:Config.examples.GetEnumerator()) {
        if ($example.Value.featured -eq $true) {
            $featuredTypes += $example.Key
        }
    }
    return $featuredTypes
}

# Get default example type
function Get-DefaultExample {
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    return $Global:Config.metadata.default_example
}

# Get default build type
function Get-DefaultBuildType {
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    return $Global:Config.metadata.default_build_type
}

# Get target platform
function Get-TargetPlatform {
    if (-not $Global:ConfigLoaded) { Initialize-Config }
    return $Global:Config.metadata.target
}

# Initialize configuration when module is imported
Initialize-Config | Out-Null

# Export functions
Export-ModuleMember -Function @(
    'Initialize-Config',
    'Get-ExampleTypes',
    'Get-BuildTypes', 
    'Get-ExampleDescription',
    'Get-ExampleSourceFile',
    'Test-ValidExampleType',
    'Test-ValidBuildType',
    'Get-BuildDirectory',
    'Get-ProjectName',
    'Get-CiExampleTypes',
    'Get-FeaturedExampleTypes',
    'Get-DefaultExample',
    'Get-DefaultBuildType',
    'Get-TargetPlatform'
)