#!/usr/bin/env python3
"""
ArmPil Mini - Robot control module with demo mode support
"""

import logging
import time
from typing import Optional, Tuple, Any
from robot_config import config

# Configure logging
config.setup_logging()
logger = logging.getLogger(__name__)

class Board:
    """Board class for robot hardware control with demo mode support"""
    
    def __init__(self):
        self.connected = False
        self.demo_mode = True
        self.hardware = None
        
        if config.force_demo_mode:
            logger.info("Demo mode forced by configuration")
            return
            
        try:
            # Attempt to initialize real hardware
            logger.info("Attempting to initialize robot board...")
            # This would normally import and initialize real hardware
            # import board_hardware_lib
            # self.hardware = board_hardware_lib.Board()
            
            # For now, simulate failure to force demo mode
            # Comment out the next line when real hardware is available
            raise ImportError("Hardware not available")
            
            # If hardware initialization succeeds:
            # self.connected = True
            # self.demo_mode = False
            
        except (ImportError, ConnectionError, OSError) as e:
            logger.warning(f"Hardware initialization failed: {e}")
            logger.info("Board running in demo mode")
            self.demo_mode = True
            self.connected = False
    
    def move_to_position(self, x: float, y: float, z: float) -> bool:
        """Move robot arm to specified position"""
        if self.demo_mode:
            logger.info(f"Demo: Moving to position ({x:.2f}, {y:.2f}, {z:.2f})")
            movement_time = max(0.3, abs(x) * 0.01 + abs(y) * 0.01 + abs(z) * 0.01)
            time.sleep(movement_time)  # Simulate realistic movement time
            return True
        else:
            # Real hardware movement would go here
            if self.hardware:
                try:
                    # return self.hardware.move_to(x, y, z)
                    pass
                except Exception as e:
                    logger.error(f"Hardware movement error: {e}")
                    return False
            return False
    
    def get_position(self) -> Tuple[float, float, float]:
        """Get current robot arm position"""
        if self.demo_mode:
            # Return demo position with slight variation
            import random
            base_x, base_y, base_z = 100.0, 50.0, 25.0
            return (
                base_x + random.uniform(-5, 5),
                base_y + random.uniform(-5, 5),
                base_z + random.uniform(-2, 2)
            )
        else:
            # Real hardware position reading would go here
            pass
    
    def set_gripper(self, open_state: bool) -> bool:
        """Control gripper open/close"""
        if self.demo_mode:
            state_str = "open" if open_state else "closed"
            logger.info(f"Demo: Setting gripper to {state_str}")
            return True
        else:
            # Real gripper control would go here
            pass
    
    def emergency_stop(self) -> bool:
        """Emergency stop function"""
        if self.demo_mode:
            logger.warning("Demo: Emergency stop activated")
            return True
        else:
            # Real emergency stop would go here
            pass


class Camera:
    """Camera class for robot vision with demo mode support"""
    
    def __init__(self):
        self.connected = False
        self.demo_mode = True
        self.camera = None
        
        if config.force_demo_mode:
            logger.info("Camera demo mode forced by configuration")
            return
            
        try:
            # Attempt to initialize real camera hardware
            logger.info("Attempting to initialize camera...")
            # This would normally import and initialize real camera
            # import camera_hardware_lib
            # self.camera = camera_hardware_lib.Camera()
            
            # For now, simulate failure to force demo mode
            # Comment out the next line when real hardware is available
            raise ImportError("Camera hardware not available")
            
            # If camera initialization succeeds:
            # self.connected = True
            # self.demo_mode = False
            
        except (ImportError, ConnectionError, OSError) as e:
            logger.warning(f"Camera initialization failed: {e}")
            logger.info("Camera running in demo mode")
            self.demo_mode = True
            self.connected = False
    
    def capture_image(self) -> Optional[str]:
        """Capture image and return base64 encoded data"""
        if self.demo_mode:
            logger.info("Demo: Capturing simulated image")
            # Return a simple demo image (1x1 pixel base64 encoded)
            demo_image_b64 = "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg=="
            return demo_image_b64
        else:
            # Real camera capture would go here
            pass
    
    def get_camera_info(self) -> dict:
        """Get camera information and status"""
        if self.demo_mode:
            return {
                "status": "demo_mode",
                "resolution": "640x480",
                "fps": 30,
                "connected": False,
                "demo": True
            }
        else:
            # Real camera info would go here
            pass


def initialize_robot() -> Tuple[Optional[Board], Optional[Camera]]:
    """Initialize robot board and camera with error handling"""
    board = None
    camera = None
    
    try:
        board = Board()
        logger.info("Board initialized successfully")
    except Exception as e:
        logger.error(f"Failed to initialize board: {e}")
    
    try:
        camera = Camera()
        logger.info("Camera initialized successfully")
    except Exception as e:
        logger.error(f"Failed to initialize camera: {e}")
    
    return board, camera


# Module-level testing
if __name__ == "__main__":
    # For testing the hardware classes directly
    print("Testing ArmPil Mini hardware classes...")
    
    board, camera = initialize_robot()
    
    if board:
        print(f"Board demo mode: {board.demo_mode}")
        test_result = board.move_to_position(50, 25, 10)
        print(f"Movement test result: {test_result}")
        position = board.get_position()
        print(f"Current position: {position}")
    
    if camera:
        print(f"Camera demo mode: {camera.demo_mode}")
        image_data = camera.capture_image()
        print(f"Image capture successful: {image_data is not None}")
    
    print("ArmPil Mini testing complete.")