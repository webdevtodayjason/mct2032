"""
MCT2032 Communication Protocol
Python implementation for admin console
"""

import json
from typing import Dict, Any, Optional, List
from enum import Enum
from dataclasses import dataclass, asdict


# BLE Service and Characteristic UUIDs
SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CMD_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
DATA_CHAR_UUID = "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"
STATUS_CHAR_UUID = "8d7e5d2e-bf3d-413a-d8f7-e3f95c9c3319"

# USB HID and SD Card Commands
CMD_USB_TYPE_STRING = "USB_TYPE_STRING"
CMD_DUCKY_SCRIPT = "DUCKY_SCRIPT"
CMD_DUCKY_STOP = "DUCKY_STOP"
CMD_DUCKY_CHUNK = "DUCKY_CHUNK"
CMD_SD_LIST_PAYLOADS = "SD_LIST_PAYLOADS"
CMD_SD_LOAD_PAYLOAD = "SD_LOAD_PAYLOAD"
CMD_SD_TEST = "SD_TEST"


class Commands(Enum):
    """Command types"""
    SCAN_WIFI = "SCAN_WIFI"
    SCAN_BLE = "SCAN_BLE"
    MONITOR_START = "MONITOR_START"
    MONITOR_STOP = "MONITOR_STOP"
    GET_STATUS = "GET_STATUS"
    SET_CHANNEL = "SET_CHANNEL"
    EXPORT_DATA = "EXPORT_DATA"
    SET_MODE = "SET_MODE"
    CLEAR_DATA = "CLEAR_DATA"
    
    # Advanced commands
    DEAUTH_ATTACK = "DEAUTH_ATTACK"
    BEACON_SPAM = "BEACON_SPAM"
    PROBE_FLOOD = "PROBE_FLOOD"
    EVIL_PORTAL = "EVIL_PORTAL"
    RICKROLL = "RICKROLL"
    PCAP_START = "PCAP_START"
    PCAP_STOP = "PCAP_STOP"
    PACKET_INJECT = "PACKET_INJECT"


class ResponseStatus(Enum):
    """Response status codes"""
    SUCCESS = "success"
    ERROR = "error"
    BUSY = "busy"
    INVALID_CMD = "invalid_command"
    TIMEOUT = "timeout"


class ErrorCode(Enum):
    """Error codes"""
    NONE = 0
    INVALID_JSON = 1
    UNKNOWN_CMD = 2
    INVALID_PARAMS = 3
    WIFI_SCAN_FAIL = 4
    BLE_SCAN_FAIL = 5
    SD_CARD_ERROR = 6
    OUT_OF_MEMORY = 7
    MONITOR_FAIL = 8


class DeviceMode(Enum):
    """Device operating modes"""
    IDLE = 0
    SCANNING = 1
    MONITORING = 2
    EXPORTING = 3
    ATTACKING = 4
    BEACON_SPAM = 5
    EVIL_PORTAL = 6
    PCAP_CAPTURE = 7


class SecurityType(Enum):
    """WiFi security types"""
    OPEN = "OPEN"
    WEP = "WEP"
    WPA = "WPA"
    WPA2 = "WPA2"
    WPA3 = "WPA3"
    WPA_WPA2 = "WPA/WPA2"
    ENTERPRISE = "ENTERPRISE"


@dataclass
class NetworkInfo:
    """WiFi network information"""
    ssid: str
    bssid: str
    channel: int
    rssi: int
    security: str
    hidden: bool = False


@dataclass
class BLEDevice:
    """BLE device information"""
    name: Optional[str]
    address: str
    rssi: int
    device_type: str
    services: List[str] = None


@dataclass
class DeviceStatus:
    """Device status information"""
    uptime: int
    free_heap: int
    battery_level: int
    mode: DeviceMode
    sd_card: bool
    wifi_connected: bool
    ble_connected: bool


@dataclass
class PacketStats:
    """Packet monitoring statistics"""
    packets_total: int
    packets_per_sec: int
    beacon_count: int
    probe_count: int
    data_count: int
    mgmt_count: int


class Protocol:
    """MCT2032 communication protocol handler"""
    
    @staticmethod
    def create_command(cmd: Commands, params: Optional[Dict[str, Any]] = None) -> bytes:
        """Create a command message"""
        command = {"cmd": cmd.value}
        if params:
            command["params"] = params
        return json.dumps(command).encode('utf-8')
    
    @staticmethod
    def parse_response(data: bytes) -> Dict[str, Any]:
        """Parse response from device"""
        try:
            return json.loads(data.decode('utf-8'))
        except (json.JSONDecodeError, UnicodeDecodeError) as e:
            return {
                "status": ResponseStatus.ERROR.value,
                "error": f"Failed to parse response: {str(e)}"
            }
    
    @staticmethod
    def parse_network_list(data: Dict[str, Any]) -> List[NetworkInfo]:
        """Parse WiFi network list from response"""
        networks = []
        if "networks" in data:
            for net in data["networks"]:
                networks.append(NetworkInfo(
                    ssid=net.get("ssid", ""),
                    bssid=net.get("bssid", ""),
                    channel=net.get("channel", 0),
                    rssi=net.get("rssi", -100),
                    security=net.get("security", "UNKNOWN"),
                    hidden=net.get("hidden", False)
                ))
        return networks
    
    @staticmethod
    def parse_ble_devices(data: Dict[str, Any]) -> List[BLEDevice]:
        """Parse BLE device list from response"""
        devices = []
        if "devices" in data:
            for dev in data["devices"]:
                devices.append(BLEDevice(
                    name=dev.get("name"),
                    address=dev.get("address", ""),
                    rssi=dev.get("rssi", -100),
                    device_type=dev.get("type", "Unknown"),
                    services=dev.get("services", [])
                ))
        return devices
    
    @staticmethod
    def parse_status(data: Dict[str, Any]) -> Optional[DeviceStatus]:
        """Parse device status from response"""
        if "data" in data:
            status_data = data["data"]
            return DeviceStatus(
                uptime=status_data.get("uptime", 0),
                free_heap=status_data.get("free_heap", 0),
                battery_level=status_data.get("battery_level", 0),
                mode=DeviceMode(status_data.get("mode", 0)),
                sd_card=status_data.get("sd_card", False),
                wifi_connected=status_data.get("wifi_connected", False),
                ble_connected=status_data.get("ble_connected", True)
            )
        return None
    
    @staticmethod
    def parse_packet_stats(data: Dict[str, Any]) -> Optional[PacketStats]:
        """Parse packet statistics from response"""
        if "data" in data:
            stats_data = data["data"]
            return PacketStats(
                packets_total=stats_data.get("packets_total", 0),
                packets_per_sec=stats_data.get("packets_per_sec", 0),
                beacon_count=stats_data.get("beacon_count", 0),
                probe_count=stats_data.get("probe_count", 0),
                data_count=stats_data.get("data_count", 0),
                mgmt_count=stats_data.get("mgmt_count", 0)
            )
        return None
    
    @staticmethod
    def create_scan_wifi_cmd(duration: int = 3000, channel: int = 0) -> bytes:
        """Create WiFi scan command"""
        return Protocol.create_command(
            Commands.SCAN_WIFI,
            {"duration": duration, "channel": channel}
        )
    
    @staticmethod
    def create_scan_ble_cmd(duration: int = 5000) -> bytes:
        """Create BLE scan command"""
        return Protocol.create_command(
            Commands.SCAN_BLE,
            {"duration": duration}
        )
    
    @staticmethod
    def create_monitor_start_cmd(channel: int = 0) -> bytes:
        """Create monitor start command"""
        return Protocol.create_command(
            Commands.MONITOR_START,
            {"channel": channel}
        )
    
    @staticmethod
    def create_set_channel_cmd(channel: int) -> bytes:
        """Create set channel command"""
        return Protocol.create_command(
            Commands.SET_CHANNEL,
            {"channel": channel}
        )