#!/usr/bin/env python3
"""
Web Server for Robot Control with Demo Mode Support
"""

import json
import logging
import time
from datetime import datetime
from typing import Dict, Any, Callable
from functools import wraps
import threading
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
import mimetypes
import os

# Import our robot module
try:
    from ArmPil_mini import robot_board, robot_camera, ROBOT_AVAILABLE, get_robot_status
except ImportError:
    logging.error("Failed to import ArmPil_mini. Creating mock objects.")
    ROBOT_AVAILABLE = False
    robot_board = None
    robot_camera = None
    def get_robot_status():
        return {"robot_available": False, "demo_mode": True, "error": "ArmPil_mini not available"}

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Custom RPC registry to replace jsonrpc2._rpc.register
rpc_functions: Dict[str, Callable] = {}

def rpc_register(func_name: str = None):
    """Custom decorator to register RPC functions"""
    def decorator(func):
        name = func_name or func.__name__
        rpc_functions[name] = func
        
        @wraps(func)
        def wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        return wrapper
    return decorator


# RPC Function Definitions
@rpc_register("get_robot_status")
def rpc_get_robot_status():
    """Get current robot status"""
    if ROBOT_AVAILABLE:
        return get_robot_status()
    else:
        return {
            "robot_available": False,
            "board_connected": False,
            "camera_connected": False,
            "demo_mode": True,
            "message": "Running in demo mode - no hardware detected",
            "timestamp": time.time()
        }


@rpc_register("move_robot")
def rpc_move_robot(x: float, y: float, z: float):
    """Move robot to specified position"""
    if ROBOT_AVAILABLE and robot_board:
        success = robot_board.move_to_position(x, y, z)
        return {
            "success": success,
            "message": f"Moved to position ({x}, {y}, {z})",
            "timestamp": time.time()
        }
    else:
        # Demo response
        time.sleep(1)  # Simulate movement time
        return {
            "success": True,
            "message": f"Demo: Simulated movement to ({x}, {y}, {z})",
            "demo_mode": True,
            "timestamp": time.time()
        }


@rpc_register("get_robot_position")
def rpc_get_robot_position():
    """Get current robot position"""
    if ROBOT_AVAILABLE and robot_board:
        position = robot_board.get_position()
        return {
            "position": {"x": position[0], "y": position[1], "z": position[2]},
            "timestamp": time.time()
        }
    else:
        # Demo response with simulated position
        import random
        return {
            "position": {
                "x": round(100 + random.uniform(-10, 10), 2),
                "y": round(50 + random.uniform(-10, 10), 2),
                "z": round(25 + random.uniform(-5, 5), 2)
            },
            "demo_mode": True,
            "timestamp": time.time()
        }


@rpc_register("control_gripper")
def rpc_control_gripper(open_state: bool):
    """Control robot gripper"""
    if ROBOT_AVAILABLE and robot_board:
        success = robot_board.set_gripper(open_state)
        action = "opened" if open_state else "closed"
        return {
            "success": success,
            "message": f"Gripper {action}",
            "gripper_open": open_state,
            "timestamp": time.time()
        }
    else:
        # Demo response
        action = "opened" if open_state else "closed"
        return {
            "success": True,
            "message": f"Demo: Gripper {action}",
            "gripper_open": open_state,
            "demo_mode": True,
            "timestamp": time.time()
        }


@rpc_register("capture_image")
def rpc_capture_image():
    """Capture image from robot camera"""
    if ROBOT_AVAILABLE and robot_camera:
        image_data = robot_camera.capture_image()
        return {
            "success": True,
            "image_data": image_data,
            "timestamp": time.time()
        }
    else:
        # Demo response with a simple base64 image
        demo_image = "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg=="
        return {
            "success": True,
            "image_data": demo_image,
            "demo_mode": True,
            "message": "Demo image captured",
            "timestamp": time.time()
        }


@rpc_register("emergency_stop")
def rpc_emergency_stop():
    """Emergency stop function"""
    if ROBOT_AVAILABLE and robot_board:
        success = robot_board.emergency_stop()
        return {
            "success": success,
            "message": "Emergency stop activated",
            "timestamp": time.time()
        }
    else:
        return {
            "success": True,
            "message": "Demo: Emergency stop activated",
            "demo_mode": True,
            "timestamp": time.time()
        }


@rpc_register("get_available_functions")
def rpc_get_available_functions():
    """Get list of available RPC functions"""
    return {
        "functions": list(rpc_functions.keys()),
        "count": len(rpc_functions),
        "demo_mode": not ROBOT_AVAILABLE,
        "timestamp": time.time()
    }


class RobotWebHandler(BaseHTTPRequestHandler):
    """HTTP request handler for the robot web server"""
    
    def log_message(self, format, *args):
        """Override to use our logger"""
        logger.info(f"{self.address_string()} - {format % args}")
    
    def do_GET(self):
        """Handle GET requests"""
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        
        if path == "/" or path == "/index.html":
            self.serve_landing_page()
        elif path == "/api/status":
            self.serve_status_api()
        elif path.startswith("/static/"):
            self.serve_static_file(path)
        else:
            self.send_error(404, "Not Found")
    
    def do_POST(self):
        """Handle POST requests"""
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        
        if path == "/rpc":
            self.handle_rpc_request()
        else:
            self.send_error(404, "Not Found")
    
    def do_HEAD(self):
        """Handle HEAD requests"""
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        
        if path == "/" or path == "/index.html":
            html_content = self.get_landing_page_html()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", len(html_content.encode('utf-8')))
            self.end_headers()
        elif path == "/api/status":
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
        else:
            self.send_error(404, "Not Found")
    
    def do_OPTIONS(self):
        """Handle CORS preflight requests"""
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS, HEAD")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()
    
    def serve_landing_page(self):
        """Serve the beautiful landing page"""
        html_content = self.get_landing_page_html()
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", len(html_content.encode('utf-8')))
        self.end_headers()
        self.wfile.write(html_content.encode('utf-8'))
    
    def serve_status_api(self):
        """Serve robot status API"""
        try:
            status = rpc_get_robot_status()
            response = json.dumps(status, indent=2)
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", len(response.encode('utf-8')))
            self.end_headers()
            self.wfile.write(response.encode('utf-8'))
        except Exception as e:
            error_response = json.dumps({"error": str(e), "timestamp": time.time()})
            self.send_response(500)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", len(error_response.encode('utf-8')))
            self.end_headers()
            self.wfile.write(error_response.encode('utf-8'))
    
    def handle_rpc_request(self):
        """Handle RPC requests using custom registry"""
        try:
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length)
            
            if not post_data:
                raise ValueError("Empty request body")
            
            request_data = json.loads(post_data.decode('utf-8'))
            
            # Validate JSON-RPC 2.0 format
            if "method" not in request_data:
                raise ValueError("Missing 'method' in request")
            
            method_name = request_data["method"]
            params = request_data.get("params", {})
            request_id = request_data.get("id", None)
            
            # Check if method exists in our registry
            if method_name not in rpc_functions:
                response = {
                    "jsonrpc": "2.0",
                    "error": {
                        "code": -32601,
                        "message": f"Method '{method_name}' not found",
                        "data": {"available_methods": list(rpc_functions.keys())}
                    },
                    "id": request_id
                }
            else:
                # Call the registered function
                try:
                    if isinstance(params, dict):
                        result = rpc_functions[method_name](**params)
                    elif isinstance(params, list):
                        result = rpc_functions[method_name](*params)
                    else:
                        result = rpc_functions[method_name]()
                    
                    response = {
                        "jsonrpc": "2.0",
                        "result": result,
                        "id": request_id
                    }
                except Exception as func_error:
                    logger.error(f"Error executing {method_name}: {func_error}")
                    response = {
                        "jsonrpc": "2.0",
                        "error": {
                            "code": -32603,
                            "message": f"Internal error in {method_name}",
                            "data": {"error_details": str(func_error)}
                        },
                        "id": request_id
                    }
            
            # Send response
            response_json = json.dumps(response, indent=2)
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS")
            self.send_header("Access-Control-Allow-Headers", "Content-Type")
            self.send_header("Content-Length", len(response_json.encode('utf-8')))
            self.end_headers()
            self.wfile.write(response_json.encode('utf-8'))
            
        except Exception as e:
            logger.error(f"RPC request error: {e}")
            error_response = {
                "jsonrpc": "2.0",
                "error": {
                    "code": -32700,
                    "message": "Parse error",
                    "data": {"error_details": str(e)}
                },
                "id": None
            }
            response_json = json.dumps(error_response)
            self.send_response(400)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", len(response_json.encode('utf-8')))
            self.end_headers()
            self.wfile.write(response_json.encode('utf-8'))
    
    def get_landing_page_html(self):
        """Generate beautiful landing page HTML"""
        try:
            robot_status = rpc_get_robot_status()
            status_color = "#28a745" if robot_status.get("robot_available") else "#ffc107"
            status_text = "Online" if robot_status.get("robot_available") else "Demo Mode"
            current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            demo_mode_text = 'demo mode' if not robot_status.get('robot_available') else 'live mode'
            
            # Build HTML using string concatenation to avoid f-string issues
            html = '<!DOCTYPE html>\n'
            html += '<html lang="en">\n'
            html += '<head>\n'
            html += '    <meta charset="UTF-8">\n'
            html += '    <meta name="viewport" content="width=device-width, initial-scale=1.0">\n'
            html += '    <title>Robot Control Center</title>\n'
            html += '    <style>\n'
            html += '        * { margin: 0; padding: 0; box-sizing: border-box; }\n'
            html += '        body {\n'
            html += '            font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;\n'
            html += '            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n'
            html += '            min-height: 100vh;\n'
            html += '            color: #333;\n'
            html += '        }\n'
            html += '        .container { max-width: 1200px; margin: 0 auto; padding: 20px; }\n'
            html += '        .header { text-align: center; margin-bottom: 40px; color: white; }\n'
            html += '        .header h1 { font-size: 3em; margin-bottom: 10px; text-shadow: 2px 2px 4px rgba(0,0,0,0.3); }\n'
            html += '        .status-card {\n'
            html += '            background: white; border-radius: 15px; padding: 30px;\n'
            html += '            margin-bottom: 30px; box-shadow: 0 10px 30px rgba(0,0,0,0.2);\n'
            html += '            text-align: center;\n'
            html += '        }\n'
            html += '        .status-indicator {\n'
            html += '            display: inline-flex; align-items: center; gap: 10px;\n'
            html += '            font-size: 1.5em; font-weight: bold; margin-bottom: 20px;\n'
            html += '        }\n'
            html += '        .status-dot {\n'
            html += '            width: 20px; height: 20px; border-radius: 50%;\n'
            html += '            background-color: ' + status_color + ';\n'
            html += '            animation: pulse 2s infinite;\n'
            html += '        }\n'
            html += '        @keyframes pulse {\n'
            html += '            0% { opacity: 1; }\n'
            html += '            50% { opacity: 0.5; }\n'
            html += '            100% { opacity: 1; }\n'
            html += '        }\n'
            html += '        .controls-grid {\n'
            html += '            display: grid;\n'
            html += '            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));\n'
            html += '            gap: 20px; margin-bottom: 30px;\n'
            html += '        }\n'
            html += '        .control-card {\n'
            html += '            background: white; border-radius: 15px; padding: 25px;\n'
            html += '            box-shadow: 0 5px 15px rgba(0,0,0,0.1);\n'
            html += '            transition: transform 0.3s ease;\n'
            html += '        }\n'
            html += '        .control-card:hover { transform: translateY(-5px); }\n'
            html += '        .control-card h3 { color: #667eea; margin-bottom: 15px; font-size: 1.3em; }\n'
            html += '        .btn {\n'
            html += '            background: linear-gradient(45deg, #667eea, #764ba2);\n'
            html += '            color: white; border: none; padding: 12px 24px;\n'
            html += '            border-radius: 8px; font-size: 1em; cursor: pointer;\n'
            html += '            transition: all 0.3s ease; margin: 5px; min-width: 120px;\n'
            html += '        }\n'
            html += '        .btn:hover {\n'
            html += '            transform: translateY(-2px);\n'
            html += '            box-shadow: 0 5px 15px rgba(0,0,0,0.2);\n'
            html += '        }\n'
            html += '        .btn.danger { background: linear-gradient(45deg, #ff6b6b, #ee5a24); }\n'
            html += '        .input-group { margin: 10px 0; }\n'
            html += '        .input-group label { display: block; margin-bottom: 5px; font-weight: 500; }\n'
            html += '        .input-group input {\n'
            html += '            width: 100%; padding: 10px; border: 2px solid #ddd;\n'
            html += '            border-radius: 5px; font-size: 1em;\n'
            html += '        }\n'
            html += '        .response-area {\n'
            html += '            background: #f8f9fa; border-radius: 10px; padding: 20px;\n'
            html += '            margin-top: 20px; min-height: 100px;\n'
            html += '            font-family: "Courier New", monospace;\n'
            html += '            white-space: pre-wrap; border: 2px solid #dee2e6;\n'
            html += '        }\n'
            html += '        .loading {\n'
            html += '            display: inline-block; width: 20px; height: 20px;\n'
            html += '            border: 3px solid #f3f3f3; border-top: 3px solid #667eea;\n'
            html += '            border-radius: 50%; animation: spin 1s linear infinite;\n'
            html += '        }\n'
            html += '        @keyframes spin {\n'
            html += '            0% { transform: rotate(0deg); }\n'
            html += '            100% { transform: rotate(360deg); }\n'
            html += '        }\n'
            html += '    </style>\n'
            html += '</head>\n'
            html += '<body>\n'
            html += '    <div class="container">\n'
            html += '        <div class="header">\n'
            html += '            <h1>Robot Control Center</h1>\n'
            html += '            <p>Advanced robotics control interface with demo mode support</p>\n'
            html += '        </div>\n'
            html += '        <div class="status-card">\n'
            html += '            <div class="status-indicator">\n'
            html += '                <div class="status-dot"></div>\n'
            html += '                <span>Status: ' + status_text + '</span>\n'
            html += '            </div>\n'
            html += '            <p>System Time: <span id="current-time">' + current_time + '</span></p>\n'
            html += '            <button class="btn" onclick="refreshStatus()">Refresh Status</button>\n'
            html += '        </div>\n'
            html += '        <div class="controls-grid">\n'
            html += '            <div class="control-card">\n'
            html += '                <h3>Robot Movement</h3>\n'
            html += '                <div class="input-group">\n'
            html += '                    <label>X Position:</label>\n'
            html += '                    <input type="number" id="pos-x" value="100" step="0.1">\n'
            html += '                </div>\n'
            html += '                <div class="input-group">\n'
            html += '                    <label>Y Position:</label>\n'
            html += '                    <input type="number" id="pos-y" value="50" step="0.1">\n'
            html += '                </div>\n'
            html += '                <div class="input-group">\n'
            html += '                    <label>Z Position:</label>\n'
            html += '                    <input type="number" id="pos-z" value="25" step="0.1">\n'
            html += '                </div>\n'
            html += '                <button class="btn" onclick="moveRobot()">Move Robot</button>\n'
            html += '                <button class="btn" onclick="getPosition()">Get Position</button>\n'
            html += '            </div>\n'
            html += '            <div class="control-card">\n'
            html += '                <h3>Gripper Control</h3>\n'
            html += '                <p>Control the robot gripper mechanism</p>\n'
            html += '                <button class="btn" onclick="controlGripper(true)">Open Gripper</button>\n'
            html += '                <button class="btn" onclick="controlGripper(false)">Close Gripper</button>\n'
            html += '            </div>\n'
            html += '            <div class="control-card">\n'
            html += '                <h3>Camera System</h3>\n'
            html += '                <p>Capture images from robot camera</p>\n'
            html += '                <button class="btn" onclick="captureImage()">Capture Image</button>\n'
            html += '                <div id="image-display" style="margin-top: 15px;"></div>\n'
            html += '            </div>\n'
            html += '            <div class="control-card">\n'
            html += '                <h3>System Control</h3>\n'
            html += '                <p>Emergency and system functions</p>\n'
            html += '                <button class="btn danger" onclick="emergencyStop()">Emergency Stop</button>\n'
            html += '                <button class="btn" onclick="getAvailableFunctions()">Available Functions</button>\n'
            html += '            </div>\n'
            html += '        </div>\n'
            html += '        <div class="control-card">\n'
            html += '            <h3>Response Log</h3>\n'
            html += '            <div id="response-area" class="response-area">Welcome to Robot Control Center!\n'
            html += 'System initialized in ' + demo_mode_text + '.\n'
            html += 'Ready for commands...</div>\n'
            html += '        </div>\n'
            html += '    </div>\n'
            html += '    <script>\n'
            html += '        let isLoading = false;\n'
            html += '        function updateTime() {\n'
            html += '            document.getElementById("current-time").textContent = new Date().toLocaleString();\n'
            html += '        }\n'
            html += '        setInterval(updateTime, 1000);\n'
            html += '        function showLoading() {\n'
            html += '            if (isLoading) return;\n'
            html += '            isLoading = true;\n'
            html += '            const responseArea = document.getElementById("response-area");\n'
            html += '            responseArea.innerHTML += "\\n\\n<span class=\\"loading\\"></span> Processing request...";\n'
            html += '            responseArea.scrollTop = responseArea.scrollHeight;\n'
            html += '        }\n'
            html += '        function hideLoading() { isLoading = false; }\n'
            html += '        function logResponse(response) {\n'
            html += '            const responseArea = document.getElementById("response-area");\n'
            html += '            const timestamp = new Date().toLocaleTimeString();\n'
            html += '            responseArea.innerHTML += "\\n\\n[" + timestamp + "] " + JSON.stringify(response, null, 2);\n'
            html += '            responseArea.scrollTop = responseArea.scrollHeight;\n'
            html += '            hideLoading();\n'
            html += '        }\n'
            html += '        async function callRPC(method, params = {}) {\n'
            html += '            showLoading();\n'
            html += '            try {\n'
            html += '                const response = await fetch("/rpc", {\n'
            html += '                    method: "POST",\n'
            html += '                    headers: { "Content-Type": "application/json" },\n'
            html += '                    body: JSON.stringify({ jsonrpc: "2.0", method: method, params: params, id: Date.now() })\n'
            html += '                });\n'
            html += '                const result = await response.json();\n'
            html += '                logResponse(result);\n'
            html += '                return result;\n'
            html += '            } catch (error) {\n'
            html += '                logResponse({"error": "Network error: " + error.message});\n'
            html += '            }\n'
            html += '        }\n'
            html += '        async function refreshStatus() { await callRPC("get_robot_status"); }\n'
            html += '        async function moveRobot() {\n'
            html += '            const x = parseFloat(document.getElementById("pos-x").value);\n'
            html += '            const y = parseFloat(document.getElementById("pos-y").value);\n'
            html += '            const z = parseFloat(document.getElementById("pos-z").value);\n'
            html += '            await callRPC("move_robot", {x, y, z});\n'
            html += '        }\n'
            html += '        async function getPosition() { await callRPC("get_robot_position"); }\n'
            html += '        async function controlGripper(open) { await callRPC("control_gripper", {open_state: open}); }\n'
            html += '        async function captureImage() {\n'
            html += '            const result = await callRPC("capture_image");\n'
            html += '            if (result && result.result && result.result.image_data) {\n'
            html += '                const imageDisplay = document.getElementById("image-display");\n'
            html += '                imageDisplay.innerHTML = "<img src=\\"data:image/png;base64," + result.result.image_data + "\\" alt=\\"Captured Image\\" style=\\"max-width: 100%; border-radius: 5px;\\">";\n'
            html += '            }\n'
            html += '        }\n'
            html += '        async function emergencyStop() { await callRPC("emergency_stop"); }\n'
            html += '        async function getAvailableFunctions() { await callRPC("get_available_functions"); }\n'
            html += '        refreshStatus();\n'
            html += '    </script>\n'
            html += '</body>\n'
            html += '</html>'
            
            return html
            
        except Exception as e:
            logger.error(f"Error generating landing page: {e}")
            return """<!DOCTYPE html>
<html>
<head><title>Robot Control Center</title></head>
<body>
<h1>Robot Control Center</h1>
<p>Demo Mode Active</p>
<p>Error loading full interface. Basic functionality available.</p>
</body>
</html>"""


def startWebServer(port: int = 8080, robot_available: bool = None):
    """Start the web server with demo mode support"""
    global ROBOT_AVAILABLE
    
    if robot_available is not None:
        ROBOT_AVAILABLE = robot_available
    
    logger.info(f"Starting Robot Web Server on port {port}")
    logger.info(f"Robot hardware available: {ROBOT_AVAILABLE}")
    logger.info(f"Demo mode: {not ROBOT_AVAILABLE}")
    
    server_address = ('', port)
    httpd = HTTPServer(server_address, RobotWebHandler)
    
    try:
        logger.info(f"Server running at http://localhost:{port}")
        logger.info("Press Ctrl+C to stop the server")
        httpd.serve_forever()
    except KeyboardInterrupt:
        logger.info("Server stopped by user")
    finally:
        httpd.server_close()


if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="Robot Control Web Server")
    parser.add_argument("--port", type=int, default=8080, help="Server port (default: 8080)")
    parser.add_argument("--demo", action="store_true", help="Force demo mode")
    
    args = parser.parse_args()
    
    if args.demo:
        ROBOT_AVAILABLE = False
    
    startWebServer(args.port, ROBOT_AVAILABLE)