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

class CommandProcessor {
private:
    BLEManager* bleManager;
    WiFiScanner* wifiScanner;
    PacketMonitor* packetMonitor;
    
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
    
public:
    CommandProcessor(BLEManager* ble, WiFiScanner* wifi, PacketMonitor* monitor);
    
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