# 3D Robot Models Directory

This directory contains 3D models for robot visualization in the web interface.

## Supported Formats

- **GLTF/GLB** (.gltf, .glb) - Recommended for best performance and features
- **OBJ** (.obj) - Common 3D model format
- **STL** (.stl) - CAD and 3D printing format  
- **PLY** (.ply) - Point cloud and mesh format

## Usage

1. Place your CAD files in this directory
2. Use the "List Models" button in the web interface to refresh available models
3. Select a model from the dropdown to load it
4. Or use the "Upload 3D Model" button to load files directly from your computer

## CAD Workflow

To use your existing CAD files:

1. **Export from CAD software** (SolidWorks, Fusion 360, etc.) as:
   - STEP/STP → Convert to GLTF using online tools
   - STL → Can be used directly
   - OBJ → Can be used directly

2. **Convert to GLTF** (recommended):
   - Use online converters like `gltf.report` 
   - Or use Blender to import and export as GLTF

3. **Optimize for web**:
   - Keep file sizes under 10MB for good performance
   - Use texture compression if available
   - Simplify geometry if needed

## Demo Robot

The system includes a procedural demo robot that shows:
- Multi-link arm structure
- Animated joints with inverse kinematics
- Gripper with open/close animation
- Target position markers
- Work envelope visualization

## Features

- **Real-time Animation**: Robot moves in sync with control commands
- **Interactive Controls**: Mouse/touch controls for camera
- **Visual Effects**: Shadows, lighting, and materials
- **Target Visualization**: Shows movement targets with crosshairs
- **Grid and Axes**: Reference helpers for positioning

Place your robot's CAD files here to see them in beautiful 3D!