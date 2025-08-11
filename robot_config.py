#!/usr/bin/env python3
"""
Robot Configuration Management
Centralized configuration for the robot control system
"""

import os
import logging
from dataclasses import dataclass
from typing import Optional


@dataclass
class RobotConfig:
    """Configuration settings for the robot system"""
    
    # Server settings
    server_port: int = 8080
    server_host: str = "0.0.0.0"
    debug_mode: bool = False
    
    # Robot settings
    force_demo_mode: bool = False
    robot_timeout: float = 5.0
    camera_timeout: float = 3.0
    
    # Movement settings
    max_position_x: float = 300.0
    max_position_y: float = 300.0
    max_position_z: float = 100.0
    movement_speed: float = 1.0
    
    # Logging settings
    log_level: str = "INFO"
    log_format: str = "%(asctime)s - %(name)s - %(levelname)s - %(message)s"
    
    @classmethod
    def from_environment(cls) -> 'RobotConfig':
        """Create configuration from environment variables"""
        return cls(
            server_port=int(os.getenv('ROBOT_SERVER_PORT', 8080)),
            server_host=os.getenv('ROBOT_SERVER_HOST', '0.0.0.0'),
            debug_mode=os.getenv('ROBOT_DEBUG', 'false').lower() == 'true',
            force_demo_mode=os.getenv('ROBOT_FORCE_DEMO', 'false').lower() == 'true',
            robot_timeout=float(os.getenv('ROBOT_TIMEOUT', 5.0)),
            camera_timeout=float(os.getenv('CAMERA_TIMEOUT', 3.0)),
            max_position_x=float(os.getenv('ROBOT_MAX_X', 300.0)),
            max_position_y=float(os.getenv('ROBOT_MAX_Y', 300.0)),
            max_position_z=float(os.getenv('ROBOT_MAX_Z', 100.0)),
            movement_speed=float(os.getenv('ROBOT_SPEED', 1.0)),
            log_level=os.getenv('ROBOT_LOG_LEVEL', 'INFO'),
        )
    
    def setup_logging(self) -> None:
        """Configure logging based on configuration"""
        logging.basicConfig(
            level=getattr(logging, self.log_level.upper()),
            format=self.log_format,
            force=True  # Reconfigure existing loggers
        )


# Global configuration instance
config = RobotConfig.from_environment()