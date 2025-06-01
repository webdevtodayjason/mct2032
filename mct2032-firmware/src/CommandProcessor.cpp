/**
 * Command Processor implementation
 */

#include "CommandProcessor.h"
#include <SD.h>

CommandProcessor::CommandProcessor(BLEManager* ble, WiFiScanner* wifi) :
    bleManager(ble),
    wifiScanner(wifi),
    currentMode(MODE_IDLE),
    startTime(millis()) {
}

void CommandProcessor::init() {
    // Register command handlers
    commandHandlers[CMD_SCAN_WIFI] = [this](JsonDocument& params) { handleScanWiFi(params); };
    commandHandlers[CMD_SCAN_BLE] = [this](JsonDocument& params) { handleScanBLE(params); };
    commandHandlers[CMD_GET_STATUS] = [this](JsonDocument& params) { handleGetStatus(params); };
    commandHandlers[CMD_SET_CHANNEL] = [this](JsonDocument& params) { handleSetChannel(params); };
    commandHandlers[CMD_MONITOR_START] = [this](JsonDocument& params) { handleMonitorStart(params); };
    commandHandlers[CMD_MONITOR_STOP] = [this](JsonDocument& params) { handleMonitorStop(params); };
    commandHandlers[CMD_EXPORT_DATA] = [this](JsonDocument& params) { handleExportData(params); };
    commandHandlers[CMD_CLEAR_DATA] = [this](JsonDocument& params) { handleClearData(params); };
    
    Serial.println("Command processor initialized");
}

void CommandProcessor::processCommand(const String& commandStr) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, commandStr);
    
    if (error) {
        Serial.printf("Failed to parse command: %s\n", error.c_str());
        bleManager->sendError("", "Invalid JSON");
        return;
    }
    
    String cmd = doc[JSON_CMD] | "";
    if (cmd.isEmpty()) {
        bleManager->sendError("", "Missing command");
        return;
    }
    
    Serial.printf("Processing command: %s\n", cmd.c_str());
    
    // Find and execute handler
    auto it = commandHandlers.find(cmd);
    if (it != commandHandlers.end()) {
        JsonDocument params = doc[JSON_PARAMS];
        it->second(params);
    } else {
        bleManager->sendError(cmd, "Unknown command");
    }
}

void CommandProcessor::handleScanWiFi(JsonDocument& params) {
    if (currentMode != MODE_IDLE) {
        bleManager->sendError(CMD_SCAN_WIFI, "Device busy");
        return;
    }
    
    int duration = params["duration"] | 3000;
    int channel = params["channel"] | 0;
    
    currentMode = MODE_SCANNING;
    
    // Start scan
    if (wifiScanner->startScan(channel)) {
        // Send immediate acknowledgment
        DynamicJsonDocument response(256);
        response["message"] = "WiFi scan started";
        response["duration"] = duration;
        bleManager->sendResponse(CMD_SCAN_WIFI, STATUS_SUCCESS, response);
        
        // Schedule result sending
        // In real implementation, this would be handled by a task or timer
    } else {
        currentMode = MODE_IDLE;
        bleManager->sendError(CMD_SCAN_WIFI, "Failed to start scan");
    }
}

void CommandProcessor::handleScanBLE(JsonDocument& params) {
    // TODO: Implement BLE scanning
    bleManager->sendError(CMD_SCAN_BLE, "Not implemented yet");
}

void CommandProcessor::handleGetStatus(JsonDocument& params) {
    DynamicJsonDocument status(512);
    
    status[JSON_UPTIME] = getUptime();
    status[JSON_FREE_HEAP] = getFreeHeap();
    status[JSON_BATTERY_LEVEL] = getBatteryLevel();
    status[JSON_CURRENT_MODE] = currentMode;
    status[JSON_SD_CARD_STATUS] = isSDCardPresent();
    status[JSON_WIFI_STATUS] = WiFi.isConnected();
    status[JSON_BLE_STATUS] = bleManager->isConnected();
    
    bleManager->sendResponse(CMD_GET_STATUS, STATUS_SUCCESS, status);
}

void CommandProcessor::handleSetChannel(JsonDocument& params) {
    int channel = params["channel"] | 0;
    
    if (channel < 0 || channel > 14) {
        bleManager->sendError(CMD_SET_CHANNEL, "Invalid channel");
        return;
    }
    
    // TODO: Implement channel setting for monitor mode
    DynamicJsonDocument response(256);
    response["channel"] = channel;
    bleManager->sendResponse(CMD_SET_CHANNEL, STATUS_SUCCESS, response);
}

void CommandProcessor::handleMonitorStart(JsonDocument& params) {
    if (currentMode != MODE_IDLE) {
        bleManager->sendError(CMD_MONITOR_START, "Device busy");
        return;
    }
    
    int channel = params["channel"] | 0;
    
    // TODO: Implement packet monitoring
    currentMode = MODE_MONITORING;
    
    DynamicJsonDocument response(256);
    response["message"] = "Monitor mode started";
    response["channel"] = channel;
    bleManager->sendResponse(CMD_MONITOR_START, STATUS_SUCCESS, response);
}

void CommandProcessor::handleMonitorStop(JsonDocument& params) {
    if (currentMode != MODE_MONITORING) {
        bleManager->sendError(CMD_MONITOR_STOP, "Not in monitor mode");
        return;
    }
    
    // TODO: Stop packet monitoring
    currentMode = MODE_IDLE;
    
    DynamicJsonDocument response(256);
    response["message"] = "Monitor mode stopped";
    bleManager->sendResponse(CMD_MONITOR_STOP, STATUS_SUCCESS, response);
}

void CommandProcessor::handleExportData(JsonDocument& params) {
    if (!isSDCardPresent()) {
        bleManager->sendError(CMD_EXPORT_DATA, "SD card not present");
        return;
    }
    
    // TODO: Implement data export
    bleManager->sendError(CMD_EXPORT_DATA, "Not implemented yet");
}

void CommandProcessor::handleClearData(JsonDocument& params) {
    // Clear any stored data
    wifiScanner->getResults().clear();
    
    DynamicJsonDocument response(256);
    response["message"] = "Data cleared";
    bleManager->sendResponse(CMD_CLEAR_DATA, STATUS_SUCCESS, response);
}

uint32_t CommandProcessor::getUptime() const {
    return millis() - startTime;
}

uint32_t CommandProcessor::getFreeHeap() const {
    return esp_get_free_heap_size();
}

uint8_t CommandProcessor::getBatteryLevel() const {
    // TODO: Implement battery monitoring
    return 100; // Dummy value
}

bool CommandProcessor::isSDCardPresent() const {
    // TODO: Implement SD card detection
    return false;
}