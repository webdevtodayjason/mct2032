"""
BLE Controller for MCT2032
Handles Bluetooth LE communication with the ESP32 device
"""

import asyncio
import logging
from typing import Optional, Callable, Any, Dict
from queue import Queue
from bleak import BleakClient, BleakScanner
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

from .protocol import (
    Protocol, Commands, ResponseStatus,
    SERVICE_UUID, CMD_CHAR_UUID, DATA_CHAR_UUID, STATUS_CHAR_UUID
)


logger = logging.getLogger(__name__)


class BLEController:
    """Manages BLE connection and communication with MCT2032 device"""
    
    def __init__(self, response_queue: Optional[Queue] = None):
        self.client: Optional[BleakClient] = None
        self.device: Optional[BLEDevice] = None
        self.connected = False
        self.response_queue = response_queue or Queue()
        self.notification_handlers = {}
        self._response_event = asyncio.Event()
        self._last_response: Optional[Dict[str, Any]] = None
        
        # For handling chunked responses
        self._chunked_data: Dict[str, Any] = {}
        self._chunk_buffer: List[Any] = []
        
    async def scan_for_device(self, timeout: float = 10.0) -> Optional[BLEDevice]:
        """Scan for MCT2032 device"""
        logger.info("Scanning for CyberTool device...")
        
        def detection_callback(device: BLEDevice, advertisement_data: AdvertisementData):
            if device.name and "CyberTool" in device.name:
                logger.info(f"Found device: {device.name} [{device.address}]")
                return True
            return False
        
        try:
            device = await BleakScanner.find_device_by_filter(
                detection_callback,
                timeout=timeout
            )
            
            if device:
                self.device = device
                return device
            else:
                logger.warning("No CyberTool device found")
                return None
                
        except Exception as e:
            logger.error(f"Error during scanning: {e}")
            return None
    
    async def connect(self, device: Optional[BLEDevice] = None) -> bool:
        """Connect to MCT2032 device"""
        if not device and not self.device:
            # Try to find device first
            device = await self.scan_for_device()
            if not device:
                return False
        
        target_device = device or self.device
        
        try:
            logger.info(f"Connecting to {target_device.address}...")
            self.client = BleakClient(target_device)
            await self.client.connect()
            
            if self.client.is_connected:
                self.connected = True
                self.device = target_device
                
                # Subscribe to notifications
                await self._setup_notifications()
                
                logger.info("Connected successfully")
                return True
            else:
                logger.error("Failed to connect")
                return False
                
        except Exception as e:
            logger.error(f"Connection error: {e}")
            self.connected = False
            return False
    
    async def disconnect(self):
        """Disconnect from device"""
        if self.client and self.client.is_connected:
            try:
                await self.client.disconnect()
                logger.info("Disconnected from device")
            except Exception as e:
                logger.error(f"Error during disconnect: {e}")
        
        self.connected = False
        self.client = None
    
    async def _setup_notifications(self):
        """Setup notification handlers for characteristics"""
        try:
            # Data characteristic notifications
            await self.client.start_notify(
                DATA_CHAR_UUID,
                self._handle_data_notification
            )
            
            # Status characteristic notifications
            await self.client.start_notify(
                STATUS_CHAR_UUID,
                self._handle_status_notification
            )
            
            logger.info("Notification handlers setup complete")
            
        except Exception as e:
            logger.error(f"Error setting up notifications: {e}")
            raise
    
    def _handle_data_notification(self, sender: int, data: bytearray):
        """Handle data notifications from device"""
        try:
            response = Protocol.parse_response(bytes(data))
            logger.info(f"Data notification received: {response}")
            
            # Handle chunked responses
            if response.get("type") == "chunk":
                cmd = response.get("cmd", "UNKNOWN")
                chunk_index = response.get("chunkIndex", 0)
                total_chunks = response.get("totalChunks", 0)
                
                logger.info(f"Received chunk {chunk_index + 1}/{total_chunks} for {cmd}")
                
                # Store network data from chunk
                if "network" in response:
                    self._chunk_buffer.append(response["network"])
                
                # Don't trigger response event yet for chunks
                return
            
            # Check if this is a response to a command
            if response.get("type") == "response":
                # Log the command this response is for
                cmd = response.get("cmd", "UNKNOWN")
                status = response.get("status", "")
                logger.info(f"Response is for command: {cmd}, status: {status}")
                
                # Handle chunked response start
                if status == "chunked":
                    logger.info(f"Starting chunked response for {cmd}")
                    self._chunk_buffer = []
                    self._chunked_data = {"cmd": cmd}
                    return
                
                # Handle chunked response completion
                if cmd in ["SCAN_WIFI", "SCAN_BLE"] and self._chunk_buffer:
                    logger.info(f"Completing chunked response with {len(self._chunk_buffer)} networks")
                    response["data"] = {"networks": self._chunk_buffer}
                    self._chunk_buffer = []
                
                # Only store if we're waiting for a response
                if not self._response_event.is_set():
                    # Store for command/response pattern
                    self._last_response = response
                    self._response_event.set()
                    logger.info(f"Stored response for command: {cmd}")
                else:
                    logger.warning(f"Received unexpected response for {cmd}, ignoring")
            
            # Always queue for GUI updates (for real-time data)
            if self.response_queue:
                self.response_queue.put(("data", response))
                
        except Exception as e:
            logger.error(f"Error handling data notification: {e}")
            logger.error(f"Raw data: {data.hex()}")
    
    def _handle_status_notification(self, sender: int, data: bytearray):
        """Handle status notifications from device"""
        try:
            response = Protocol.parse_response(bytes(data))
            logger.debug(f"Status notification: {response}")
            
            # Queue for GUI updates
            if self.response_queue:
                self.response_queue.put(("status", response))
                
        except Exception as e:
            logger.error(f"Error handling status notification: {e}")
    
    async def send_command(self, command: Commands, params: Optional[Dict[str, Any]] = None,
                          timeout: float = 5.0) -> Optional[Dict[str, Any]]:
        """Send command and wait for response"""
        if not self.connected or not self.client:
            logger.error("Not connected to device")
            return None
        
        try:
            # Clear previous response
            self._last_response = None
            self._response_event.clear()
            logger.info(f"Cleared previous response, waiting for {command.value}")
            
            # Create and send command
            cmd_bytes = Protocol.create_command(command, params)
            logger.info(f"Sending command: {command.value}")
            logger.info(f"Command bytes: {cmd_bytes.hex()}")
            await self.client.write_gatt_char(CMD_CHAR_UUID, cmd_bytes)
            
            # Wait for response
            try:
                await asyncio.wait_for(self._response_event.wait(), timeout=timeout)
                logger.info(f"Response received for {command.value}")
                return self._last_response
            except asyncio.TimeoutError:
                logger.error(f"Command timeout after {timeout}s: {command.value}")
                return {
                    "status": ResponseStatus.TIMEOUT.value,
                    "error": f"Command timeout after {timeout}s"
                }
                
        except Exception as e:
            logger.error(f"Error sending command: {e}")
            return {
                "status": ResponseStatus.ERROR.value,
                "error": str(e)
            }
    
    async def scan_wifi(self, duration: int = 3000, channel: int = 0) -> Optional[Dict[str, Any]]:
        """Perform WiFi scan"""
        # Increase timeout to account for scan time + processing
        return await self.send_command(
            Commands.SCAN_WIFI,
            {"duration": duration, "channel": channel},
            timeout=10.0  # 10 seconds timeout
        )
    
    async def scan_ble(self, duration: int = 5000) -> Optional[Dict[str, Any]]:
        """Perform BLE scan"""
        return await self.send_command(
            Commands.SCAN_BLE,
            {"duration": duration}
        )
    
    async def get_status(self) -> Optional[Dict[str, Any]]:
        """Get device status"""
        return await self.send_command(Commands.GET_STATUS)
    
    async def start_monitor(self, channel: int = 0) -> Optional[Dict[str, Any]]:
        """Start packet monitoring"""
        return await self.send_command(
            Commands.MONITOR_START,
            {"channel": channel}
        )
    
    async def stop_monitor(self) -> Optional[Dict[str, Any]]:
        """Stop packet monitoring"""
        return await self.send_command(Commands.MONITOR_STOP)
    
    async def set_channel(self, channel: int) -> Optional[Dict[str, Any]]:
        """Set WiFi channel"""
        return await self.send_command(
            Commands.SET_CHANNEL,
            {"channel": channel}
        )
    
    async def export_data(self) -> Optional[Dict[str, Any]]:
        """Export data to SD card"""
        return await self.send_command(Commands.EXPORT_DATA)
    
    async def clear_data(self) -> Optional[Dict[str, Any]]:
        """Clear all stored data"""
        return await self.send_command(Commands.CLEAR_DATA)