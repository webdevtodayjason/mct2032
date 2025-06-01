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

class CommandProcessor {
private:
    BLEManager* bleManager;
    WiFiScanner* wifiScanner;
    
    // Device state
    uint8_t currentMode;
    uint32_t startTime;
    
    // Command handlers
    std::map<String, std::function<void(JsonDocument&)>> commandHandlers;
    
    // Handler methods
    void handleScanWiFi(JsonDocument& params);
    void handleScanBLE(JsonDocument& params);
    void handleGetStatus(JsonDocument& params);
    void handleSetChannel(JsonDocument& params);
    void handleMonitorStart(JsonDocument& params);
    void handleMonitorStop(JsonDocument& params);
    void handleExportData(JsonDocument& params);
    void handleClearData(JsonDocument& params);
    
public:
    CommandProcessor(BLEManager* ble, WiFiScanner* wifi);
    
    void init();
    void processCommand(const String& commandStr);
    
    // Status helpers
    uint32_t getUptime() const;
    uint32_t getFreeHeap() const;
    uint8_t getBatteryLevel() const;
    bool isSDCardPresent() const;
};

#endif // COMMAND_PROCESSOR_H