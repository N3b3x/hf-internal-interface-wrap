#!/usr/bin/env python3
"""
Web Server for Robot Control with Demo Mode Support
"""

import json
import logging
import time
import signal
import sys
from datetime import datetime
from typing import Dict, Any, Callable
from functools import wraps
import threading
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
import mimetypes
import os

# Import configuration and robot manager
from robot_config import config
from robot_manager import robot_manager

# Configure logging
config.setup_logging()
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
    try:
        return robot_manager.get_status()
    except Exception as e:
        logger.error(f"Error getting robot status: {e}")
        return {
            "robot_available": False,
            "board_connected": False,
            "camera_connected": False,
            "demo_mode": True,
            "error": str(e),
            "timestamp": time.time()
        }


@rpc_register("move_robot")
def rpc_move_robot(x: float, y: float, z: float):
    """Move robot to specified position"""
    try:
        return robot_manager.move_to_position(x, y, z)
    except ValueError as e:
        return {
            "success": False,
            "error": f"Invalid parameters: {e}",
            "timestamp": time.time()
        }
    except RuntimeError as e:
        return {
            "success": False,
            "error": f"Robot error: {e}",
            "timestamp": time.time()
        }
    except Exception as e:
        logger.error(f"Unexpected error in move_robot: {e}")
        return {
            "success": False,
            "error": f"Unexpected error: {e}",
            "timestamp": time.time()
        }


@rpc_register("get_robot_position")
def rpc_get_robot_position():
    """Get current robot position"""
    try:
        return robot_manager.get_position()
    except Exception as e:
        logger.error(f"Error getting robot position: {e}")
        return {
            "error": f"Position error: {e}",
            "timestamp": time.time()
        }


@rpc_register("control_gripper")
def rpc_control_gripper(open_state: bool):
    """Control robot gripper"""
    try:
        return robot_manager.control_gripper(open_state)
    except Exception as e:
        logger.error(f"Error controlling gripper: {e}")
        return {
            "success": False,
            "error": f"Gripper error: {e}",
            "timestamp": time.time()
        }


@rpc_register("capture_image")
def rpc_capture_image():
    """Capture image from robot camera"""
    try:
        return robot_manager.capture_image()
    except Exception as e:
        logger.error(f"Error capturing image: {e}")
        return {
            "success": False,
            "error": f"Image capture error: {e}",
            "timestamp": time.time()
        }


@rpc_register("emergency_stop")
def rpc_emergency_stop():
    """Emergency stop function"""
    try:
        return robot_manager.emergency_stop()
    except Exception as e:
        logger.error(f"Error during emergency stop: {e}")
        return {
            "success": False,
            "error": f"Emergency stop error: {e}",
            "timestamp": time.time()
        }


@rpc_register("get_available_functions")
def rpc_get_available_functions():
    """Get list of available RPC functions"""
    try:
        status = robot_manager.get_status()
        return {
            "functions": list(rpc_functions.keys()),
            "count": len(rpc_functions),
            "demo_mode": status.get("demo_mode", True),
            "robot_available": status.get("robot_available", False),
            "timestamp": time.time()
        }
    except Exception as e:
        logger.error(f"Error getting available functions: {e}")
        return {
            "functions": list(rpc_functions.keys()),
            "count": len(rpc_functions),
            "demo_mode": True,
            "error": str(e),
            "timestamp": time.time()
        }


@rpc_register("reset_emergency_stop")
def rpc_reset_emergency_stop():
    """Reset emergency stop state"""
    try:
        return robot_manager.reset_emergency_stop()
    except Exception as e:
        logger.error(f"Error resetting emergency stop: {e}")
        return {
            "success": False,
            "error": f"Reset error: {e}",
            "timestamp": time.time()
        }


@rpc_register("list_3d_models")
def rpc_list_3d_models():
    """List available 3D models"""
    try:
        models_dir = "/workspace/static/models"
        if not os.path.exists(models_dir):
            return {
                "models": [],
                "message": "No models directory found",
                "timestamp": time.time()
            }
        
        supported_formats = ['.gltf', '.glb', '.obj', '.stl', '.ply']
        models = []
        
        for filename in os.listdir(models_dir):
            ext = os.path.splitext(filename)[1].lower()
            if ext in supported_formats:
                file_path = os.path.join(models_dir, filename)
                file_size = os.path.getsize(file_path)
                models.append({
                    "name": filename,
                    "format": ext[1:],  # Remove the dot
                    "size": file_size,
                    "url": f"/static/models/{filename}"
                })
        
        return {
            "models": models,
            "count": len(models),
            "supported_formats": supported_formats,
            "timestamp": time.time()
        }
        
    except Exception as e:
        logger.error(f"Error listing 3D models: {e}")
        return {
            "models": [],
            "error": str(e),
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
    
    def serve_static_file(self, path):
        """Serve static files (CSS, JS, images, 3D models)"""
        try:
            # Remove /static/ prefix and get actual file path
            file_path = path[8:]  # Remove "/static/"
            full_path = os.path.join("/workspace/static", file_path)
            
            if not os.path.exists(full_path):
                self.send_error(404, "File not found")
                return
            
            # Get MIME type
            mime_type, _ = mimetypes.guess_type(full_path)
            if mime_type is None:
                # Default MIME types for 3D model formats
                ext = os.path.splitext(full_path)[1].lower()
                if ext == '.gltf':
                    mime_type = 'model/gltf+json'
                elif ext == '.glb':
                    mime_type = 'model/gltf-binary'
                elif ext == '.stl':
                    mime_type = 'application/sla'
                elif ext == '.obj':
                    mime_type = 'application/object'
                elif ext == '.ply':
                    mime_type = 'application/ply'
                else:
                    mime_type = 'application/octet-stream'
            
            # Read and serve file
            with open(full_path, 'rb') as f:
                content = f.read()
            
            self.send_response(200)
            self.send_header("Content-Type", mime_type)
            self.send_header("Content-Length", len(content))
            self.send_header("Cache-Control", "public, max-age=3600")  # Cache for 1 hour
            self.end_headers()
            self.wfile.write(content)
            
        except Exception as e:
            logger.error(f"Error serving static file {path}: {e}")
            self.send_error(500, "Internal Server Error")
    
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
            html += '    <!-- Three.js Libraries -->\n'
            html += '    <script src="https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js"></script>\n'
            html += '    <script src="https://cdn.jsdelivr.net/npm/three@0.128.0/examples/js/controls/OrbitControls.js"></script>\n'
            html += '    <script src="https://cdn.jsdelivr.net/npm/three@0.128.0/examples/js/loaders/GLTFLoader.js"></script>\n'
            html += '    <script src="https://cdn.jsdelivr.net/npm/three@0.128.0/examples/js/loaders/OBJLoader.js"></script>\n'
            html += '    <script src="https://cdn.jsdelivr.net/npm/three@0.128.0/examples/js/loaders/STLLoader.js"></script>\n'
            html += '    <script src="https://cdn.jsdelivr.net/npm/three@0.128.0/examples/js/loaders/PLYLoader.js"></script>\n'
            html += '    <script src="/static/js/robot3d.js"></script>\n'
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
            html += '        .robot-3d-viewer {\n'
            html += '            width: 100%; height: 400px;\n'
            html += '            border: 2px solid #dee2e6;\n'
            html += '            border-radius: 10px;\n'
            html += '            background: #1a1a2e;\n'
            html += '            position: relative;\n'
            html += '            overflow: hidden;\n'
            html += '        }\n'
            html += '        .viewer-controls {\n'
            html += '            position: absolute;\n'
            html += '            top: 10px; right: 10px;\n'
            html += '            z-index: 100;\n'
            html += '            background: rgba(0,0,0,0.7);\n'
            html += '            padding: 10px;\n'
            html += '            border-radius: 5px;\n'
            html += '        }\n'
            html += '        .viewer-controls button {\n'
            html += '            background: rgba(255,255,255,0.2);\n'
            html += '            color: white;\n'
            html += '            border: none;\n'
            html += '            padding: 5px 10px;\n'
            html += '            margin: 2px;\n'
            html += '            border-radius: 3px;\n'
            html += '            cursor: pointer;\n'
            html += '            font-size: 0.8em;\n'
            html += '        }\n'
            html += '        .viewer-controls button:hover {\n'
            html += '            background: rgba(255,255,255,0.3);\n'
            html += '        }\n'
            html += '        .model-upload {\n'
            html += '            margin-top: 10px;\n'
            html += '        }\n'
            html += '        .model-upload input[type="file"] {\n'
            html += '            display: none;\n'
            html += '        }\n'
            html += '        .model-upload label {\n'
            html += '            display: inline-block;\n'
            html += '            padding: 8px 16px;\n'
            html += '            background: linear-gradient(45deg, #9b59b6, #8e44ad);\n'
            html += '            color: white;\n'
            html += '            border-radius: 5px;\n'
            html += '            cursor: pointer;\n'
            html += '            transition: transform 0.2s;\n'
            html += '        }\n'
            html += '        .model-upload label:hover {\n'
            html += '            transform: translateY(-2px);\n'
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
            html += '                <button class="btn" onclick="resetEmergencyStop()">Reset E-Stop</button>\n'
            html += '                <button class="btn" onclick="getAvailableFunctions()">Available Functions</button>\n'
            html += '            </div>\n'
            html += '        </div>\n'
            html += '        \n'
            html += '        <div class="control-card">\n'
            html += '            <h3>3D Robot Visualization</h3>\n'
            html += '            <p>Real-time 3D robot model with movement visualization</p>\n'
            html += '            <div id="robot-3d-container" class="robot-3d-viewer">\n'
            html += '                <div class="viewer-controls">\n'
            html += '                    <button onclick="robot3d.resetView()">Reset View</button>\n'
            html += '                    <button onclick="toggleGrid()">Grid</button>\n'
            html += '                    <button onclick="toggleAxes()">Axes</button>\n'
            html += '                </div>\n'
            html += '            </div>\n'
            html += '            <div class="model-upload">\n'
            html += '                <label for="model-file">Upload 3D Model</label>\n'
            html += '                <input type="file" id="model-file" accept=".gltf,.glb,.obj,.stl,.ply" onchange="loadCustomModel(this)">\n'
            html += '                <button class="btn" onclick="loadDemoModel()">Load Demo Robot</button>\n'
            html += '                <button class="btn" onclick="listAvailableModels()">List Models</button>\n'
            html += '                <select id="model-selector" onchange="loadSelectedModel()" style="margin-left: 10px; padding: 5px;">\n'
            html += '                    <option value="">Select Model...</option>\n'
            html += '                </select>\n'
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
            html += '        async function resetEmergencyStop() { await callRPC("reset_emergency_stop"); }\n'
            html += '        async function getAvailableFunctions() { await callRPC("get_available_functions"); }\n'
            html += '        \n'
            html += '        // 3D Viewer Integration\n'
            html += '        let robot3d = null;\n'
            html += '        let gridVisible = true;\n'
            html += '        let axesVisible = true;\n'
            html += '        \n'
            html += '        function init3DViewer() {\n'
            html += '            try {\n'
            html += '                robot3d = new Robot3DViewer("robot-3d-container", {\n'
            html += '                    backgroundColor: 0x1a1a2e,\n'
            html += '                    cameraPosition: { x: 4, y: 4, z: 4 },\n'
            html += '                    enableControls: true,\n'
            html += '                    showGrid: true,\n'
            html += '                    showAxes: true,\n'
            html += '                    modelScale: 1.0,\n'
            html += '                    animationSpeed: 2.0\n'
            html += '                });\n'
            html += '                console.log("3D Viewer initialized successfully");\n'
            html += '            } catch (error) {\n'
            html += '                console.error("Failed to initialize 3D viewer:", error);\n'
            html += '            }\n'
            html += '        }\n'
            html += '        \n'
            html += '        function toggleGrid() {\n'
            html += '            gridVisible = !gridVisible;\n'
            html += '            if (robot3d && robot3d.scene) {\n'
            html += '                const grid = robot3d.scene.children.find(child => child.type === "GridHelper");\n'
            html += '                if (grid) grid.visible = gridVisible;\n'
            html += '            }\n'
            html += '        }\n'
            html += '        \n'
            html += '        function toggleAxes() {\n'
            html += '            axesVisible = !axesVisible;\n'
            html += '            if (robot3d && robot3d.scene) {\n'
            html += '                const axes = robot3d.scene.children.find(child => child.type === "AxesHelper");\n'
            html += '                if (axes) axes.visible = axesVisible;\n'
            html += '            }\n'
            html += '        }\n'
            html += '        \n'
            html += '        function loadCustomModel(input) {\n'
            html += '            const file = input.files[0];\n'
            html += '            if (!file) return;\n'
            html += '            \n'
            html += '            const url = URL.createObjectURL(file);\n'
            html += '            if (robot3d) {\n'
            html += '                robot3d.loadModel(\n'
            html += '                    url,\n'
            html += '                    (model) => {\n'
            html += '                        logResponse({"message": "Custom 3D model loaded successfully", "model": file.name});\n'
            html += '                        URL.revokeObjectURL(url);\n'
            html += '                    },\n'
            html += '                    (progress) => {\n'
            html += '                        console.log("Loading progress:", progress);\n'
            html += '                    },\n'
            html += '                    (error) => {\n'
            html += '                        logResponse({"error": "Failed to load 3D model: " + error.message});\n'
            html += '                        URL.revokeObjectURL(url);\n'
            html += '                    }\n'
            html += '                );\n'
            html += '            }\n'
            html += '        }\n'
            html += '        \n'
            html += '        function loadDemoModel() {\n'
            html += '            if (robot3d) {\n'
            html += '                robot3d.createDemoRobot();\n'
            html += '                logResponse({"message": "Demo robot model loaded"});\n'
            html += '            }\n'
            html += '        }\n'
            html += '        \n'
            html += '        async function listAvailableModels() {\n'
            html += '            const result = await callRPC("list_3d_models");\n'
            html += '            if (result && result.result && result.result.models) {\n'
            html += '                const selector = document.getElementById("model-selector");\n'
            html += '                selector.innerHTML = "<option value=\\"\\">Select Model...</option>";\n'
            html += '                \n'
            html += '                result.result.models.forEach(model => {\n'
            html += '                    const option = document.createElement("option");\n'
            html += '                    option.value = model.url;\n'
            html += '                    option.textContent = `${model.name} (${model.format.toUpperCase()}, ${(model.size/1024).toFixed(1)}KB)`;\n'
            html += '                    selector.appendChild(option);\n'
            html += '                });\n'
            html += '                \n'
            html += '                logResponse({"message": `Found ${result.result.count} 3D models`});\n'
            html += '            }\n'
            html += '        }\n'
            html += '        \n'
            html += '        function loadSelectedModel() {\n'
            html += '            const selector = document.getElementById("model-selector");\n'
            html += '            const url = selector.value;\n'
            html += '            if (url && robot3d) {\n'
            html += '                robot3d.loadModel(\n'
            html += '                    url,\n'
            html += '                    (model) => {\n'
            html += '                        logResponse({"message": "3D model loaded successfully", "url": url});\n'
            html += '                    },\n'
            html += '                    (progress) => {\n'
            html += '                        console.log("Loading progress:", progress);\n'
            html += '                    },\n'
            html += '                    (error) => {\n'
            html += '                        logResponse({"error": "Failed to load 3D model: " + error.message});\n'
            html += '                    }\n'
            html += '                );\n'
            html += '            }\n'
            html += '        }\n'
            html += '        \n'
            html += '        // Override movement function to update 3D view\n'
            html += '        const originalMoveRobot = moveRobot;\n'
            html += '        async function moveRobot() {\n'
            html += '            const x = parseFloat(document.getElementById("pos-x").value);\n'
            html += '            const y = parseFloat(document.getElementById("pos-y").value);\n'
            html += '            const z = parseFloat(document.getElementById("pos-z").value);\n'
            html += '            \n'
            html += '            // Update 3D visualization\n'
            html += '            if (robot3d) {\n'
            html += '                robot3d.moveToPosition(x/100, y/100, z/100, true);\n'
            html += '            }\n'
            html += '            \n'
            html += '            // Call original function\n'
            html += '            await callRPC("move_robot", {x, y, z});\n'
            html += '        }\n'
            html += '        \n'
            html += '        // Override gripper function to update 3D view\n'
            html += '        const originalControlGripper = controlGripper;\n'
            html += '        async function controlGripper(open) {\n'
            html += '            // Update 3D visualization\n'
            html += '            if (robot3d) {\n'
            html += '                robot3d.setGripperState(open);\n'
            html += '            }\n'
            html += '            \n'
            html += '            // Call original function\n'
            html += '            await callRPC("control_gripper", {open_state: open});\n'
            html += '        }\n'
            html += '        \n'
            html += '        // Initialize everything\n'
            html += '        window.addEventListener("load", () => {\n'
            html += '            setTimeout(init3DViewer, 1000); // Wait for Three.js to load\n'
            html += '        });\n'
            html += '        \n'
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


def signal_handler(signum, frame):
    """Handle shutdown signals gracefully"""
    logger.info(f"Received signal {signum}, shutting down gracefully...")
    robot_manager.shutdown()
    sys.exit(0)


def startWebServer(port: int = None, host: str = None):
    """Start the web server with demo mode support"""
    # Use configuration values if not provided
    port = port or config.server_port
    host = host or config.server_host
    
    # Register signal handlers for graceful shutdown
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    logger.info(f"Starting Robot Web Server on {host}:{port}")
    
    # Get initial robot status
    try:
        status = robot_manager.get_status()
        logger.info(f"Robot hardware available: {status.get('robot_available', False)}")
        logger.info(f"Demo mode: {status.get('demo_mode', True)}")
        if status.get('emergency_stopped'):
            logger.warning("Robot is in emergency stop state")
    except Exception as e:
        logger.error(f"Error getting initial robot status: {e}")
    
    server_address = (host, port)
    httpd = HTTPServer(server_address, RobotWebHandler)
    
    try:
        logger.info(f"Server running at http://{host}:{port}")
        logger.info("Press Ctrl+C to stop the server")
        httpd.serve_forever()
    except KeyboardInterrupt:
        logger.info("Server stopped by user (Ctrl+C)")
    except Exception as e:
        logger.error(f"Server error: {e}")
    finally:
        logger.info("Shutting down server...")
        httpd.server_close()
        robot_manager.shutdown()
        logger.info("Shutdown complete")


if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="Robot Control Web Server")
    parser.add_argument("--port", type=int, help=f"Server port (default: {config.server_port})")
    parser.add_argument("--host", type=str, help=f"Server host (default: {config.server_host})")
    parser.add_argument("--demo", action="store_true", help="Force demo mode")
    parser.add_argument("--debug", action="store_true", help="Enable debug mode")
    
    args = parser.parse_args()
    
    # Update configuration based on arguments
    if args.demo:
        config.force_demo_mode = True
        logger.info("Demo mode forced by command line argument")
    
    if args.debug:
        config.debug_mode = True
        config.log_level = "DEBUG"
        config.setup_logging()
        logger.info("Debug mode enabled")
    
    startWebServer(args.port, args.host)