/**
 * Command Processor for MCT2032
 * Handles incoming commands and routes to appropriate handlers
 */

#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include <map>
#include "protocol.h"
#include "BLEManager.h"
#include "WiFiScanner.h"
#include "PacketMonitor.h"
#include "USBKeyboard.h"
#include "SDCardManager.h"
#include "DuckyScript.h"

// Device Modes
enum DeviceMode {
    MODE_IDLE = 0,
    MODE_SCANNING = 1,
    MODE_MONITORING = 2,
    MODE_ATTACKING = 3,
    MODE_BEACON_SPAM = 4,
    MODE_EVIL_PORTAL = 5,
    MODE_PCAP_CAPTURE = 6,
    MODE_USB_HID = 7,
    MODE_HOST_RECON = 8,
    MODE_EXFILTRATING = 9
};

class CommandProcessor {
private:
    BLEManager* bleManager;
    WiFiScanner* wifiScanner;
    PacketMonitor* packetMonitor;
    USBKeyboard* usbKeyboard;
    SDCardManager* sdCard;
    DuckyScript* duckyScript;
    
    // Device state
    uint8_t currentMode;
    uint32_t startTime;
    
    // Command handlers
    std::map<String, std::function<void(JsonVariant)>> commandHandlers;
    
    // Handler methods
    void handleScanWiFi(JsonVariant params);
    void handleScanBLE(JsonVariant params);
    void handleGetStatus(JsonVariant params);
    void handleSetChannel(JsonVariant params);
    void handleMonitorStart(JsonVariant params);
    void handleMonitorStop(JsonVariant params);
    void handleExportData(JsonVariant params);
    void handleClearData(JsonVariant params);
    
    // Advanced handlers
    void handleDeauthAttack(JsonVariant params);
    void handleBeaconSpam(JsonVariant params);
    void handleRickroll(JsonVariant params);
    void handlePCAPStart(JsonVariant params);
    void handlePCAPStop(JsonVariant params);
    
    // USB HID handlers
    void handleUSBSetMode(JsonVariant params);
    void handleUSBHIDExecute(JsonVariant params);
    void handleUSBTypeString(JsonVariant params);
    void handleHostNetworkScan(JsonVariant params);
    void handleHostSystemInfo(JsonVariant params);
    
    // SD Card handlers
    void handleSDListDir(JsonVariant params);
    void handleSDListPayloads(JsonVariant params);
    void handleSDLoadPayload(JsonVariant params);
    void handleSDSavePayload(JsonVariant params);
    void handleSDDeleteFile(JsonVariant params);
    void handleSDGetInfo(JsonVariant params);
    void handleSDTest(JsonVariant params);
    
    // Ducky Script handlers
    void handleDuckyScript(JsonVariant params);
    void handleDuckyStop(JsonVariant params);
    void handleDuckyChunk(JsonVariant params);
    
    // Chunking support for large Ducky Scripts
    String pendingDuckyScript;
    int expectedChunks;
    int receivedChunks;
    unsigned long lastChunkTime;
    
public:
    CommandProcessor(BLEManager* ble, WiFiScanner* wifi, PacketMonitor* monitor,
                    USBKeyboard* usb = nullptr, SDCardManager* sd = nullptr, DuckyScript* ducky = nullptr);
    
    void init();
    void processCommand(const String& commandStr);
    
    // Status helpers
    uint32_t getUptime() const;
    uint32_t getFreeHeap() const;
    uint8_t getBatteryLevel() const;
    bool isSDCardPresent() const;
    
    // Mode management
    uint8_t getCurrentMode() const { return currentMode; }
    void setMode(uint8_t mode) { currentMode = mode; }
};

#endif // COMMAND_PROCESSOR_H