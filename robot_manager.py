#!/usr/bin/env python3
"""
Robot Manager - Thread-safe singleton for robot control
"""

import logging
import threading
import time
from typing import Optional, Tuple, Dict, Any
from contextlib import contextmanager
from robot_config import config

logger = logging.getLogger(__name__)


class RobotManager:
    """Thread-safe singleton manager for robot hardware"""
    
    _instance: Optional['RobotManager'] = None
    _lock = threading.Lock()
    
    def __new__(cls) -> 'RobotManager':
        """Singleton pattern implementation"""
        if cls._instance is None:
            with cls._lock:
                if cls._instance is None:
                    cls._instance = super().__new__(cls)
                    cls._instance._initialized = False
        return cls._instance
    
    def __init__(self):
        """Initialize the robot manager (only once)"""
        if hasattr(self, '_initialized') and self._initialized:
            return
            
        self._initialized = True
        self._robot_lock = threading.RLock()
        self._board = None
        self._camera = None
        self._robot_available = False
        self._last_position = {"x": 100.0, "y": 50.0, "z": 25.0}
        self._gripper_open = False
        self._emergency_stopped = False
        
        # Initialize hardware
        self._initialize_hardware()
    
    def _initialize_hardware(self) -> None:
        """Initialize robot hardware with proper error handling"""
        if config.force_demo_mode:
            logger.info("Demo mode forced by configuration")
            self._robot_available = False
            return
            
        try:
            # Import here to avoid circular dependencies
            from ArmPil_mini import Board, Camera
            
            logger.info("Attempting to initialize robot hardware...")
            
            # Initialize board
            try:
                self._board = Board()
                logger.info("Robot board initialized successfully")
            except Exception as e:
                logger.warning(f"Board initialization failed: {e}")
                self._board = None
            
            # Initialize camera
            try:
                self._camera = Camera()
                logger.info("Robot camera initialized successfully")
            except Exception as e:
                logger.warning(f"Camera initialization failed: {e}")
                self._camera = None
            
            # Check if any hardware is available
            self._robot_available = (self._board is not None and 
                                   hasattr(self._board, 'connected') and 
                                   self._board.connected)
            
            if not self._robot_available:
                logger.info("No robot hardware detected, running in demo mode")
            
        except ImportError as e:
            logger.warning(f"Hardware modules not available: {e}")
            self._robot_available = False
        except Exception as e:
            logger.error(f"Unexpected error during hardware initialization: {e}")
            self._robot_available = False
    
    @contextmanager
    def _robot_context(self):
        """Context manager for thread-safe robot operations"""
        with self._robot_lock:
            if self._emergency_stopped:
                raise RuntimeError("Robot is in emergency stop state")
            yield
    
    def get_status(self) -> Dict[str, Any]:
        """Get current robot status (thread-safe)"""
        with self._robot_lock:
            return {
                "robot_available": self._robot_available,
                "board_connected": self._board.connected if self._board else False,
                "camera_connected": self._camera.connected if self._camera else False,
                "demo_mode": not self._robot_available,
                "emergency_stopped": self._emergency_stopped,
                "gripper_open": self._gripper_open,
                "timestamp": time.time()
            }
    
    def move_to_position(self, x: float, y: float, z: float) -> Dict[str, Any]:
        """Move robot to specified position (thread-safe)"""
        # Validate input parameters
        if not (-config.max_position_x <= x <= config.max_position_x):
            raise ValueError(f"X position {x} out of range [-{config.max_position_x}, {config.max_position_x}]")
        if not (-config.max_position_y <= y <= config.max_position_y):
            raise ValueError(f"Y position {y} out of range [-{config.max_position_y}, {config.max_position_y}]")
        if not (0 <= z <= config.max_position_z):
            raise ValueError(f"Z position {z} out of range [0, {config.max_position_z}]")
        
        with self._robot_context():
            if self._robot_available and self._board:
                try:
                    success = self._board.move_to_position(x, y, z)
                    if success:
                        self._last_position = {"x": x, "y": y, "z": z}
                    return {
                        "success": success,
                        "message": f"Moved to position ({x:.2f}, {y:.2f}, {z:.2f})",
                        "position": self._last_position,
                        "timestamp": time.time()
                    }
                except Exception as e:
                    logger.error(f"Hardware movement error: {e}")
                    raise RuntimeError(f"Movement failed: {e}")
            else:
                # Demo mode simulation
                movement_time = max(0.5, abs(x - self._last_position["x"]) * 0.01)
                time.sleep(movement_time)  # Simulate realistic movement time
                self._last_position = {"x": x, "y": y, "z": z}
                return {
                    "success": True,
                    "message": f"Demo: Simulated movement to ({x:.2f}, {y:.2f}, {z:.2f})",
                    "position": self._last_position,
                    "demo_mode": True,
                    "timestamp": time.time()
                }
    
    def get_position(self) -> Dict[str, Any]:
        """Get current robot position (thread-safe)"""
        with self._robot_context():
            if self._robot_available and self._board:
                try:
                    position = self._board.get_position()
                    self._last_position = {"x": position[0], "y": position[1], "z": position[2]}
                    return {
                        "position": self._last_position,
                        "timestamp": time.time()
                    }
                except Exception as e:
                    logger.error(f"Position reading error: {e}")
                    # Fall back to last known position
                    return {
                        "position": self._last_position,
                        "error": f"Hardware error: {e}",
                        "timestamp": time.time()
                    }
            else:
                # Demo mode with slight random variation
                import random
                demo_position = {
                    "x": self._last_position["x"] + random.uniform(-1, 1),
                    "y": self._last_position["y"] + random.uniform(-1, 1),
                    "z": self._last_position["z"] + random.uniform(-0.5, 0.5)
                }
                return {
                    "position": demo_position,
                    "demo_mode": True,
                    "timestamp": time.time()
                }
    
    def control_gripper(self, open_state: bool) -> Dict[str, Any]:
        """Control robot gripper (thread-safe)"""
        with self._robot_context():
            if self._robot_available and self._board:
                try:
                    success = self._board.set_gripper(open_state)
                    if success:
                        self._gripper_open = open_state
                    action = "opened" if open_state else "closed"
                    return {
                        "success": success,
                        "message": f"Gripper {action}",
                        "gripper_open": self._gripper_open,
                        "timestamp": time.time()
                    }
                except Exception as e:
                    logger.error(f"Gripper control error: {e}")
                    raise RuntimeError(f"Gripper control failed: {e}")
            else:
                # Demo mode
                self._gripper_open = open_state
                action = "opened" if open_state else "closed"
                time.sleep(0.5)  # Simulate gripper action time
                return {
                    "success": True,
                    "message": f"Demo: Gripper {action}",
                    "gripper_open": self._gripper_open,
                    "demo_mode": True,
                    "timestamp": time.time()
                }
    
    def capture_image(self) -> Dict[str, Any]:
        """Capture image from robot camera (thread-safe)"""
        with self._robot_context():
            if self._robot_available and self._camera:
                try:
                    image_data = self._camera.capture_image()
                    return {
                        "success": True,
                        "image_data": image_data,
                        "timestamp": time.time()
                    }
                except Exception as e:
                    logger.error(f"Image capture error: {e}")
                    raise RuntimeError(f"Image capture failed: {e}")
            else:
                # Demo mode with a simple base64 image
                time.sleep(0.3)  # Simulate capture time
                demo_image = "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg=="
                return {
                    "success": True,
                    "image_data": demo_image,
                    "demo_mode": True,
                    "message": "Demo image captured",
                    "timestamp": time.time()
                }
    
    def emergency_stop(self) -> Dict[str, Any]:
        """Emergency stop function (thread-safe)"""
        with self._robot_lock:
            self._emergency_stopped = True
            
            if self._robot_available and self._board:
                try:
                    success = self._board.emergency_stop()
                    return {
                        "success": success,
                        "message": "Emergency stop activated",
                        "emergency_stopped": True,
                        "timestamp": time.time()
                    }
                except Exception as e:
                    logger.error(f"Emergency stop error: {e}")
                    # Still mark as emergency stopped even if hardware fails
                    return {
                        "success": False,
                        "message": f"Emergency stop attempted but hardware error: {e}",
                        "emergency_stopped": True,
                        "timestamp": time.time()
                    }
            else:
                return {
                    "success": True,
                    "message": "Demo: Emergency stop activated",
                    "emergency_stopped": True,
                    "demo_mode": True,
                    "timestamp": time.time()
                }
    
    def reset_emergency_stop(self) -> Dict[str, Any]:
        """Reset emergency stop state (thread-safe)"""
        with self._robot_lock:
            self._emergency_stopped = False
            return {
                "success": True,
                "message": "Emergency stop reset",
                "emergency_stopped": False,
                "timestamp": time.time()
            }
    
    def shutdown(self) -> None:
        """Properly shutdown robot manager and cleanup resources"""
        with self._robot_lock:
            logger.info("Shutting down robot manager...")
            
            if self._board:
                try:
                    # Perform safe shutdown of board
                    if hasattr(self._board, 'shutdown'):
                        self._board.shutdown()
                except Exception as e:
                    logger.error(f"Board shutdown error: {e}")
            
            if self._camera:
                try:
                    # Perform safe shutdown of camera
                    if hasattr(self._camera, 'shutdown'):
                        self._camera.shutdown()
                except Exception as e:
                    logger.error(f"Camera shutdown error: {e}")
            
            self._board = None
            self._camera = None
            self._robot_available = False
            logger.info("Robot manager shutdown complete")


# Global robot manager instance (singleton)
robot_manager = RobotManager()