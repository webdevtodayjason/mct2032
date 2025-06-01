/**
 * MCT2032 Communication Protocol
 * Shared definitions for ESP32 and Python admin console
 */

#ifndef MCT2032_PROTOCOL_H
#define MCT2032_PROTOCOL_H

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CMD_CHAR_UUID       "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define DATA_CHAR_UUID      "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"
#define STATUS_CHAR_UUID    "8d7e5d2e-bf3d-413a-d8f7-e3f95c9c3319"

// Command Types
#define CMD_SCAN_WIFI       "SCAN_WIFI"
#define CMD_SCAN_BLE        "SCAN_BLE"
#define CMD_MONITOR_START   "MONITOR_START"
#define CMD_MONITOR_STOP    "MONITOR_STOP"
#define CMD_GET_STATUS      "GET_STATUS"
#define CMD_SET_CHANNEL     "SET_CHANNEL"
#define CMD_EXPORT_DATA     "EXPORT_DATA"
#define CMD_SET_MODE        "SET_MODE"
#define CMD_CLEAR_DATA      "CLEAR_DATA"

// Advanced Commands (Marauder-inspired)
#define CMD_DEAUTH_ATTACK   "DEAUTH_ATTACK"
#define CMD_BEACON_SPAM     "BEACON_SPAM"
#define CMD_PROBE_FLOOD     "PROBE_FLOOD"
#define CMD_EVIL_PORTAL     "EVIL_PORTAL"
#define CMD_RICKROLL        "RICKROLL"
#define CMD_PCAP_START      "PCAP_START"
#define CMD_PCAP_STOP       "PCAP_STOP"

// USB HID Commands
#define CMD_USB_SET_MODE    "USB_SET_MODE"
#define CMD_USB_HID_TYPE    "USB_HID_TYPE"
#define CMD_USB_TYPE_STRING "USB_TYPE_STRING"
#define CMD_USB_HID_EXECUTE "USB_HID_EXECUTE"
#define CMD_DUCKY_SCRIPT    "DUCKY_SCRIPT"
#define CMD_DUCKY_STOP      "DUCKY_STOP"
#define CMD_DUCKY_CHUNK     "DUCKY_CHUNK"
#define CMD_HOST_NET_SCAN   "HOST_NET_SCAN"
#define CMD_HARVEST_CREDS   "HARVEST_CREDS"
#define CMD_DEPLOY_PAYLOAD  "DEPLOY_PAYLOAD"

// SD Card Commands
#define CMD_SD_LIST_DIR     "SD_LIST_DIR"
#define CMD_SD_LIST_PAYLOADS "SD_LIST_PAYLOADS"
#define CMD_SD_READ_FILE    "SD_READ_FILE"
#define CMD_SD_WRITE_FILE   "SD_WRITE_FILE"
#define CMD_SD_DELETE_FILE  "SD_DELETE_FILE"
#define CMD_SD_GET_INFO     "SD_GET_INFO"
#define CMD_SD_LOAD_PAYLOAD "SD_LOAD_PAYLOAD"
#define CMD_SD_SAVE_PAYLOAD "SD_SAVE_PAYLOAD"
#define CMD_SD_LIST_LOGS    "SD_LIST_LOGS"
#define CMD_HOST_INFO       "HOST_INFO"
#define CMD_PACKET_INJECT   "PACKET_INJECT"

// Response Status Codes
#define STATUS_SUCCESS      "success"
#define STATUS_ERROR        "error"
#define STATUS_BUSY         "busy"
#define STATUS_INVALID_CMD  "invalid_command"
#define STATUS_TIMEOUT      "timeout"

// Error Codes
#define ERR_NONE            0
#define ERR_INVALID_JSON    1
#define ERR_UNKNOWN_CMD     2
#define ERR_INVALID_PARAMS  3
#define ERR_WIFI_SCAN_FAIL  4
#define ERR_BLE_SCAN_FAIL   5
#define ERR_SD_CARD_ERROR   6
#define ERR_OUT_OF_MEMORY   7
#define ERR_MONITOR_FAIL    8

// Device Modes - commented out to use enum in CommandProcessor.h
// #define MODE_IDLE           0
// #define MODE_SCANNING       1
// #define MODE_MONITORING     2
// #define MODE_EXPORTING      3
// #define MODE_ATTACKING      4
// #define MODE_BEACON_SPAM    5
// #define MODE_EVIL_PORTAL    6
// #define MODE_PCAP_CAPTURE   7

// Maximum sizes
#define MAX_COMMAND_SIZE    512
#define MAX_RESPONSE_SIZE   4096
#define MAX_SSID_LENGTH     32
#define MAX_BSSID_LENGTH    18
#define MAX_NETWORKS        50

// JSON Keys
#define JSON_CMD            "cmd"
#define JSON_PARAMS         "params"
#define JSON_STATUS         "status"
#define JSON_DATA           "data"
#define JSON_ERROR          "error"
#define JSON_TYPE           "type"
#define JSON_TIMESTAMP      "timestamp"

// WiFi Scan JSON Keys
#define JSON_NETWORKS       "networks"
#define JSON_SSID           "ssid"
#define JSON_BSSID          "bssid"
#define JSON_CHANNEL        "channel"
#define JSON_RSSI           "rssi"
#define JSON_SECURITY       "security"
#define JSON_HIDDEN         "hidden"

// BLE Scan JSON Keys
#define JSON_DEVICES        "devices"
#define JSON_NAME           "name"
#define JSON_ADDRESS        "address"
#define JSON_DEVICE_TYPE    "type"
#define JSON_SERVICES       "services"

// Status JSON Keys
#define JSON_UPTIME         "uptime"
#define JSON_FREE_HEAP      "free_heap"
#define JSON_BATTERY_LEVEL  "battery_level"
#define JSON_CURRENT_MODE   "mode"
#define JSON_SD_CARD_STATUS "sd_card"
#define JSON_WIFI_STATUS    "wifi_connected"
#define JSON_BLE_STATUS     "ble_connected"

// Packet Monitor JSON Keys
#define JSON_PACKETS_TOTAL  "packets_total"
#define JSON_PACKETS_SEC    "packets_per_sec"
#define JSON_BEACON_COUNT   "beacon_count"
#define JSON_PROBE_COUNT    "probe_count"
#define JSON_DATA_COUNT     "data_count"
#define JSON_MGMT_COUNT     "mgmt_count"

// Security Types
#define SECURITY_OPEN       "OPEN"
#define SECURITY_WEP        "WEP"
#define SECURITY_WPA        "WPA"
#define SECURITY_WPA2       "WPA2"
#define SECURITY_WPA3       "WPA3"
#define SECURITY_WPA_WPA2   "WPA/WPA2"
#define SECURITY_ENTERPRISE "ENTERPRISE"

#endif // MCT2032_PROTOCOL_H