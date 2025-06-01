/**
 * Command Processor implementation
 */

#include "CommandProcessor.h"
#include <SD.h>

CommandProcessor::CommandProcessor(BLEManager* ble, WiFiScanner* wifi, PacketMonitor* monitor) :
    bleManager(ble),
    wifiScanner(wifi),
    packetMonitor(monitor),
    currentMode(MODE_IDLE),
    startTime(millis()) {
}

void CommandProcessor::init() {
    // Register command handlers
    commandHandlers[CMD_SCAN_WIFI] = [this](JsonVariant params) { handleScanWiFi(params); };
    commandHandlers[CMD_SCAN_BLE] = [this](JsonVariant params) { handleScanBLE(params); };
    commandHandlers[CMD_GET_STATUS] = [this](JsonVariant params) { handleGetStatus(params); };
    commandHandlers[CMD_SET_CHANNEL] = [this](JsonVariant params) { handleSetChannel(params); };
    commandHandlers[CMD_MONITOR_START] = [this](JsonVariant params) { handleMonitorStart(params); };
    commandHandlers[CMD_MONITOR_STOP] = [this](JsonVariant params) { handleMonitorStop(params); };
    commandHandlers[CMD_EXPORT_DATA] = [this](JsonVariant params) { handleExportData(params); };
    commandHandlers[CMD_CLEAR_DATA] = [this](JsonVariant params) { handleClearData(params); };
    
    // Advanced command handlers
    commandHandlers[CMD_DEAUTH_ATTACK] = [this](JsonVariant params) { handleDeauthAttack(params); };
    commandHandlers[CMD_BEACON_SPAM] = [this](JsonVariant params) { handleBeaconSpam(params); };
    commandHandlers[CMD_RICKROLL] = [this](JsonVariant params) { handleRickroll(params); };
    commandHandlers[CMD_PCAP_START] = [this](JsonVariant params) { handlePCAPStart(params); };
    commandHandlers[CMD_PCAP_STOP] = [this](JsonVariant params) { handlePCAPStop(params); };
    
    Serial.println("Command processor initialized with advanced features");
}

void CommandProcessor::processCommand(const String& commandStr) {
    Serial.println("=== COMMAND PROCESSOR ===");
    Serial.printf("Received command string: '%s'\n", commandStr.c_str());
    Serial.printf("Command length: %d\n", commandStr.length());
    
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, commandStr);
    
    if (error) {
        Serial.printf("Failed to parse command: %s\n", error.c_str());
        Serial.printf("JSON Error code: %d\n", error.code());
        bleManager->sendError("", "Invalid JSON");
        return;
    }
    
    Serial.println("JSON parsed successfully");
    
    String cmd = doc[JSON_CMD] | "";
    if (cmd.isEmpty()) {
        bleManager->sendError("", "Missing command");
        return;
    }
    
    Serial.printf("Processing command: %s\n", cmd.c_str());
    
    // Find and execute handler
    auto it = commandHandlers.find(cmd);
    if (it != commandHandlers.end()) {
        JsonVariant params = doc[JSON_PARAMS];
        it->second(params);
    } else {
        bleManager->sendError(cmd, "Unknown command");
    }
}

void CommandProcessor::handleScanWiFi(JsonVariant params) {
    Serial.println("=== handleScanWiFi called ===");
    
    if (currentMode != MODE_IDLE) {
        Serial.printf("ERROR: Device busy, current mode: %d\n", currentMode);
        bleManager->sendError(CMD_SCAN_WIFI, "Device busy");
        return;
    }
    
    int duration = params["duration"] | 3000;
    int channel = params["channel"] | 0;
    
    Serial.printf("Scan params - duration: %d, channel: %d\n", duration, channel);
    
    currentMode = MODE_SCANNING;
    Serial.printf("Mode changed to: %d\n", currentMode);
    
    // Start scan
    if (wifiScanner->startScan(channel)) {
        // Don't send immediate response - wait for scan results
        // The main loop will send the results when scan completes
        Serial.println("WiFi scan started successfully");
    } else {
        currentMode = MODE_IDLE;
        Serial.println("ERROR: Failed to start WiFi scan!");
        bleManager->sendError(CMD_SCAN_WIFI, "Failed to start scan");
    }
}

void CommandProcessor::handleScanBLE(JsonVariant params) {
    // TODO: Implement BLE scanning
    bleManager->sendError(CMD_SCAN_BLE, "Not implemented yet");
}

void CommandProcessor::handleGetStatus(JsonVariant params) {
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

void CommandProcessor::handleSetChannel(JsonVariant params) {
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

void CommandProcessor::handleMonitorStart(JsonVariant params) {
    if (currentMode != MODE_IDLE) {
        bleManager->sendError(CMD_MONITOR_START, "Device busy");
        return;
    }
    
    int channel = params["channel"] | 0;
    
    // Start packet monitoring
    if (packetMonitor->startMonitor(channel)) {
        currentMode = MODE_MONITORING;
        
        DynamicJsonDocument response(256);
        response["message"] = "Monitor mode started";
        response["channel"] = channel;
        bleManager->sendResponse(CMD_MONITOR_START, STATUS_SUCCESS, response);
        
        // Set up periodic stats updates
        // This will be handled in main loop
    } else {
        bleManager->sendError(CMD_MONITOR_START, "Failed to start monitor mode");
    }
}

void CommandProcessor::handleMonitorStop(JsonVariant params) {
    if (currentMode != MODE_MONITORING) {
        bleManager->sendError(CMD_MONITOR_STOP, "Not in monitor mode");
        return;
    }
    
    // Stop packet monitoring
    packetMonitor->stopMonitor();
    currentMode = MODE_IDLE;
    
    // Send final stats
    DynamicJsonDocument response(512);
    response["message"] = "Monitor mode stopped";
    response["stats"]["packets_total"] = packetMonitor->getPacketsTotal();
    response["stats"]["beacons"] = packetMonitor->getBeaconCount();
    response["stats"]["probes"] = packetMonitor->getProbeCount();
    response["stats"]["deauths"] = packetMonitor->getDeauthCount();
    response["stats"]["data"] = packetMonitor->getDataCount();
    
    bleManager->sendResponse(CMD_MONITOR_STOP, STATUS_SUCCESS, response);
}

void CommandProcessor::handleExportData(JsonVariant params) {
    if (!isSDCardPresent()) {
        bleManager->sendError(CMD_EXPORT_DATA, "SD card not present");
        return;
    }
    
    // TODO: Implement data export
    bleManager->sendError(CMD_EXPORT_DATA, "Not implemented yet");
}

void CommandProcessor::handleClearData(JsonVariant params) {
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

// Advanced command handlers
void CommandProcessor::handleDeauthAttack(JsonVariant params) {
    if (currentMode != MODE_IDLE) {
        bleManager->sendError(CMD_DEAUTH_ATTACK, "Device busy");
        return;
    }
    
    String targetMAC = params["target"] | "";
    String apMAC = params["ap"] | "";
    int duration = params["duration"] | 10; // seconds
    
    if (targetMAC.isEmpty() || apMAC.isEmpty()) {
        bleManager->sendError(CMD_DEAUTH_ATTACK, "Missing target or AP MAC address");
        return;
    }
    
    currentMode = MODE_ATTACKING;
    
    DynamicJsonDocument response(256);
    response["message"] = "Deauth attack started";
    response["target"] = targetMAC;
    response["ap"] = apMAC;
    response["duration"] = duration;
    bleManager->sendResponse(CMD_DEAUTH_ATTACK, STATUS_SUCCESS, response);
    
    // TODO: Implement actual deauth attack using PacketMonitor
    
    currentMode = MODE_IDLE;
}

void CommandProcessor::handleBeaconSpam(JsonVariant params) {
    if (currentMode != MODE_IDLE) {
        bleManager->sendError(CMD_BEACON_SPAM, "Device busy");
        return;
    }
    
    JsonArray ssids = params["ssids"];
    int interval = params["interval"] | 100; // milliseconds
    
    if (!ssids || ssids.size() == 0) {
        bleManager->sendError(CMD_BEACON_SPAM, "No SSIDs provided");
        return;
    }
    
    currentMode = MODE_BEACON_SPAM;
    
    DynamicJsonDocument response(256);
    response["message"] = "Beacon spam started";
    response["count"] = ssids.size();
    response["interval"] = interval;
    bleManager->sendResponse(CMD_BEACON_SPAM, STATUS_SUCCESS, response);
    
    // TODO: Implement beacon spam using PacketMonitor
    
    currentMode = MODE_IDLE;
}

void CommandProcessor::handleRickroll(JsonVariant params) {
    if (currentMode != MODE_IDLE) {
        bleManager->sendError(CMD_RICKROLL, "Device busy");
        return;
    }
    
    // Rickroll SSIDs
    const char* rickrollSSIDs[] = {
        "Never Gonna Give You Up",
        "Never Gonna Let You Down",
        "Never Gonna Run Around",
        "Never Gonna Desert You",
        "Never Gonna Make You Cry",
        "Never Gonna Say Goodbye",
        "Never Gonna Tell a Lie",
        "Never Gonna Hurt You",
        "Rick Astley Free WiFi",
        "You've Been Rickrolled"
    };
    
    currentMode = MODE_BEACON_SPAM;
    
    DynamicJsonDocument response(256);
    response["message"] = "Rickroll beacon spam started";
    response["song"] = "Never Gonna Give You Up";
    bleManager->sendResponse(CMD_RICKROLL, STATUS_SUCCESS, response);
    
    // TODO: Implement rickroll beacon spam
    
    currentMode = MODE_IDLE;
}

void CommandProcessor::handlePCAPStart(JsonVariant params) {
    if (!isSDCardPresent()) {
        bleManager->sendError(CMD_PCAP_START, "SD card not present");
        return;
    }
    
    if (packetMonitor->isPCAPActive()) {
        bleManager->sendError(CMD_PCAP_START, "PCAP already active");
        return;
    }
    
    String filename = params["filename"] | "capture.pcap";
    
    if (packetMonitor->startPCAP(filename.c_str())) {
        DynamicJsonDocument response(256);
        response["message"] = "PCAP capture started";
        response["filename"] = filename;
        bleManager->sendResponse(CMD_PCAP_START, STATUS_SUCCESS, response);
    } else {
        bleManager->sendError(CMD_PCAP_START, "Failed to start PCAP capture");
    }
}

void CommandProcessor::handlePCAPStop(JsonVariant params) {
    if (!packetMonitor->isPCAPActive()) {
        bleManager->sendError(CMD_PCAP_STOP, "PCAP not active");
        return;
    }
    
    packetMonitor->stopPCAP();
    
    DynamicJsonDocument response(256);
    response["message"] = "PCAP capture stopped";
    bleManager->sendResponse(CMD_PCAP_STOP, STATUS_SUCCESS, response);
}