# Robot Control Web Server

A comprehensive web server for robot control with demo mode support. This system provides a beautiful web interface for controlling robotic hardware with graceful fallback to demo mode when hardware is not available.

## Features

### 🤖 Hardware Integration
- **ArmPil_mini.py**: Robot control module with Board and Camera classes
- **Demo Mode**: Automatic fallback when hardware is unavailable
- **Graceful Error Handling**: Try-catch blocks for hardware initialization
- **Simulated Responses**: Realistic demo responses for all functions

### 🌐 Web Server
- **Beautiful Landing Page**: Modern, responsive UI with gradient background
- **Real-time Status**: Live system status updates with animated indicators
- **Interactive Controls**: Robot movement, gripper control, camera capture
- **Response Logging**: Real-time command response display
- **Loading Indicators**: Visual feedback during operations

### 🔌 API Endpoints
- **REST API**: `/api/status` - Get system status
- **JSON-RPC 2.0**: `/rpc` - All robot control functions
- **CORS Support**: Cross-origin requests enabled
- **Error Handling**: Proper HTTP status codes and error messages

## Quick Start

### Start the Web Server
```bash
# Demo mode (no hardware required)
python3 web_server.py --demo --port 8080

# With hardware detection
python3 web_server.py --port 8080

# Debug mode with verbose logging
python3 web_server.py --demo --debug --port 8080

# Using environment variables
export ROBOT_FORCE_DEMO=true
export ROBOT_SERVER_PORT=8080
export ROBOT_LOG_LEVEL=DEBUG
python3 web_server.py
```

### Access the Interface
- **Web Interface**: http://localhost:8080/
- **API Status**: http://localhost:8080/api/status
- **RPC Endpoint**: http://localhost:8080/rpc

## Available RPC Methods

### Robot Control
- `get_robot_status()` - Get current system status
- `move_robot(x, y, z)` - Move robot to position
- `get_robot_position()` - Get current robot position
- `control_gripper(open_state)` - Control gripper open/close
- `capture_image()` - Capture image from camera
- `emergency_stop()` - Emergency stop function
- `reset_emergency_stop()` - Reset emergency stop state
- `get_available_functions()` - List all available RPC methods

### Example RPC Request
```json
{
  "jsonrpc": "2.0",
  "method": "move_robot",
  "params": {"x": 150, "y": 75, "z": 30},
  "id": 1
}
```

### Example RPC Response
```json
{
  "jsonrpc": "2.0",
  "result": {
    "success": true,
    "message": "Demo: Simulated movement to (150, 75, 30)",
    "demo_mode": true,
    "timestamp": 1754952251.055
  },
  "id": 1
}
```

## Demo Mode Features

When hardware is not available, the system automatically enters demo mode:

- ✅ **Simulated Movement**: Realistic movement responses with delays
- ✅ **Virtual Gripper**: Open/close gripper simulation
- ✅ **Demo Images**: Base64-encoded placeholder images
- ✅ **Random Positions**: Simulated position data with variations
- ✅ **Status Indicators**: Clear demo mode indicators in UI
- ✅ **Error Simulation**: Proper error handling demonstrations

## File Structure

```
/workspace/
├── robot_config.py         # Centralized configuration management
├── robot_manager.py        # Thread-safe singleton robot manager
├── ArmPil_mini.py          # Robot hardware control classes
├── web_server.py           # Main web server with RPC endpoints
└── README_ROBOT_SERVER.md  # This documentation
```

## Testing

### Run Comprehensive Tests
```bash
# Install requests if needed
pip3 install requests

# Run demo tests
python3 test_demo.py
```

### Manual Testing
```bash
# Test status API
curl http://localhost:8080/api/status

# Test RPC endpoint
curl -X POST -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"get_robot_status","id":1}' \
  http://localhost:8080/rpc
```

## Architecture

### ⚡ Improved Architecture (v2.0)

**Major Architectural Improvements:**

1. **Centralized Configuration** (`robot_config.py`)
   - Environment variable support
   - Centralized settings management
   - Configurable logging and validation

2. **Thread-Safe Singleton Manager** (`robot_manager.py`)
   - Thread-safe robot operations
   - Proper resource management
   - Emergency stop state management
   - Input validation and error handling

3. **Eliminated Global State**
   - No more global variables or import-time side effects
   - Clean module separation
   - No circular dependencies

4. **Enhanced Error Handling**
   - Proper exception propagation
   - Comprehensive input validation
   - Graceful failure modes

### Custom RPC Registry
- Replaced `jsonrpc2._rpc.register` with custom `@rpc_register` decorator
- Functions stored in `rpc_functions` dictionary
- Dynamic method dispatch in `/rpc` endpoint handler

### Hardware Abstraction
- Configuration-controlled demo mode
- Try-catch blocks for hardware initialization
- Graceful degradation to demo responses
- Thread-safe hardware access

### Error Handling
- JSON-RPC 2.0 compliant error responses
- HTTP status codes for different error types
- Detailed error logging and user feedback
- Input validation with configurable limits

## UI Components

### Status Card
- Animated status indicator (green/yellow)
- Real-time clock updates
- System status information

### Control Cards
- **Robot Movement**: X/Y/Z position controls
- **Gripper Control**: Open/close buttons
- **Camera System**: Image capture with display
- **System Control**: Emergency stop and diagnostics

### Response Log
- Real-time command/response logging
- JSON formatted output
- Auto-scrolling display

## Browser Compatibility

The web interface uses modern web standards:
- CSS Grid and Flexbox layouts
- CSS Animations and Transitions
- Fetch API for RPC calls
- ES6+ JavaScript features

## Security Considerations

- CORS headers for cross-origin requests
- Input validation on all RPC methods
- Error message sanitization
- No sensitive data exposure in demo mode

## Customization

### Adding New RPC Methods
```python
@rpc_register("my_custom_method")
def rpc_my_custom_method(param1, param2):
    if ROBOT_AVAILABLE:
        # Hardware implementation
        pass
    else:
        # Demo implementation
        return {"demo": True, "message": "Custom demo response"}
```

### Styling Modifications
Modify the CSS in `get_landing_page_html()` method to customize:
- Colors and gradients
- Layout and spacing
- Animations and transitions
- Responsive breakpoints

## Production Deployment

### Recommendations
- Use a proper WSGI server (gunicorn, uWSGI)
- Add HTTPS/SSL encryption
- Implement authentication/authorization
- Add rate limiting and request validation
- Use a reverse proxy (nginx, Apache)

### Environment Variables
```bash
export ROBOT_SERVER_PORT=8080          # Server port
export ROBOT_SERVER_HOST=0.0.0.0       # Server host
export ROBOT_DEBUG=true                # Debug mode
export ROBOT_FORCE_DEMO=true           # Force demo mode
export ROBOT_TIMEOUT=5.0               # Robot timeout
export ROBOT_CAMERA_TIMEOUT=3.0        # Camera timeout
export ROBOT_MAX_X=300.0               # Max X position
export ROBOT_MAX_Y=300.0               # Max Y position
export ROBOT_MAX_Z=100.0               # Max Z position
export ROBOT_LOG_LEVEL=INFO            # Logging level
```

## Troubleshooting

### Common Issues
1. **Port already in use**: Change port with `--port XXXX`
2. **Module import errors**: Ensure all dependencies are installed
3. **Hardware not detected**: Check hardware connections or use `--demo`
4. **Browser not loading**: Check firewall settings and port accessibility

### Debug Mode
Enable verbose logging by modifying the logging level:
```python
logging.basicConfig(level=logging.DEBUG)
```

## Contributing

When adding new features:
1. Maintain demo mode compatibility
2. Add proper error handling
3. Update documentation
4. Test both hardware and demo modes
5. Follow existing code style

---

**Status**: ✅ Fully Functional
**Architecture**: ✅ Production-Ready v2.0
**Demo Mode**: ✅ Complete
**Web Interface**: ✅ Beautiful & Responsive
**Thread Safety**: ✅ Implemented
**Error Handling**: ✅ Comprehensive
**Configuration**: ✅ Environment-Based
**Resource Management**: ✅ Proper Cleanup
**Input Validation**: ✅ Configurable Limits
**Documentation**: ✅ Complete