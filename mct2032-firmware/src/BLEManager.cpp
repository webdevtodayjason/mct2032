/**
 * BLE Manager implementation
 */

#include "BLEManager.h"

BLEManager::BLEManager() : 
    server(nullptr), 
    service(nullptr), 
    cmdCharacteristic(nullptr),
    dataCharacteristic(nullptr),
    statusCharacteristic(nullptr),
    deviceConnected(false),
    oldDeviceConnected(false) {
}

void BLEManager::init() {
    Serial.println("BLE: Initializing...");
    
    // Create BLE Device
    NimBLEDevice::init("CyberTool");
    
    // Create BLE Server
    server = NimBLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks(this));
    
    // Create BLE Service
    service = server->createService(SERVICE_UUID);
    
    // Create Command Characteristic (Write)
    cmdCharacteristic = service->createCharacteristic(
        CMD_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE,
        512  // Max size for command
    );
    cmdCharacteristic->setCallbacks(new CommandCallbacks(this));
    
    // Create Data Characteristic (Read + Notify)
    dataCharacteristic = service->createCharacteristic(
        DATA_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY,
        512  // Max size per BLE spec
    );
    dataCharacteristic->setValue("Ready");
    
    // Create Status Characteristic (Notify)
    statusCharacteristic = service->createCharacteristic(
        STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::NOTIFY
    );
    
    // Start the service
    service->start();
    
    // Start advertising
    startAdvertising();
    
    Serial.println("BLE: Initialized successfully");
    Serial.println("BLE: Waiting for client connection...");
}

void BLEManager::startAdvertising() {
    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    advertising->setMaxPreferred(0x12);
    advertising->start();
}

void BLEManager::setCommandCallback(std::function<void(String)> callback) {
    commandCallback = callback;
}

bool BLEManager::sendData(const String& data) {
    if (!deviceConnected) {
        Serial.println("BLE: Not connected, cannot send data");
        return false;
    }
    
    try {
        Serial.printf("BLE: Sending data, length: %d\n", data.length());
        Serial.printf("BLE: Data content: %s\n", data.c_str());
        
        // Use uint8_t array to ensure proper data transmission
        dataCharacteristic->setValue((uint8_t*)data.c_str(), data.length());
        dataCharacteristic->notify();
        
        Serial.println("BLE: Data sent successfully");
        return true;
    } catch(...) {
        Serial.println("BLE: Error sending data");
        return false;
    }
}

bool BLEManager::sendStatus(const String& status) {
    if (!deviceConnected) {
        return false;
    }
    
    try {
        // Use uint8_t array to ensure proper data transmission
        statusCharacteristic->setValue((uint8_t*)status.c_str(), status.length());
        statusCharacteristic->notify();
        return true;
    } catch(...) {
        Serial.println("BLE: Error sending status");
        return false;
    }
}

bool BLEManager::sendResponse(const String& command, const String& status, const DynamicJsonDocument& data) {
    if (!deviceConnected) {
        Serial.println("BLE: Not connected for sendResponse");
        return false;
    }
    
    Serial.printf("BLE: Preparing response for command: %s, status: %s\n", command.c_str(), status.c_str());
    
    DynamicJsonDocument response(1024);
    response["type"] = "response";
    response["cmd"] = command;
    response["status"] = status;
    response["data"] = data;
    
    String output;
    serializeJson(response, output);
    
    Serial.printf("BLE: Response JSON: %s\n", output.c_str());
    
    return sendData(output);
}

bool BLEManager::sendError(const String& command, const String& error) {
    if (!deviceConnected) {
        return false;
    }
    
    DynamicJsonDocument response(512);
    response["type"] = "response";
    response["cmd"] = command;
    response["status"] = STATUS_ERROR;
    response["error"] = error;
    
    String output;
    serializeJson(response, output);
    
    return sendData(output);
}

void BLEManager::notifyData(const String& data) {
    if (deviceConnected) {
        dataCharacteristic->setValue(data.c_str());
        dataCharacteristic->notify();
    }
}

void BLEManager::notifyStatus(const String& status) {
    if (deviceConnected) {
        statusCharacteristic->setValue(status.c_str());
        statusCharacteristic->notify();
    }
}

bool BLEManager::sendChunkedResponse(const String& command, const String& status, DynamicJsonDocument& data) {
    if (!deviceConnected) {
        Serial.println("BLE: Not connected for sendChunkedResponse");
        return false;
    }
    
    Serial.printf("BLE: Preparing chunked response for command: %s\n", command.c_str());
    
    // Convert data to JSON array of networks
    String jsonData;
    serializeJson(data, jsonData);
    
    // For WiFi scan results, we'll send each network individually
    if (data.containsKey("networks")) {
        JsonArray networks = data["networks"];
        int totalNetworks = networks.size();
        
        Serial.printf("BLE: Sending %d networks in chunks\n", totalNetworks);
        
        // Send initial response with total count
        DynamicJsonDocument startResponse(256);
        startResponse["type"] = "response";
        startResponse["cmd"] = command;
        startResponse["status"] = "chunked";
        startResponse["totalChunks"] = totalNetworks;
        
        String startOutput;
        serializeJson(startResponse, startOutput);
        
        if (!sendData(startOutput)) {
            Serial.println("BLE: Failed to send chunk start");
            return false;
        }
        
        delay(50); // Small delay between chunks
        
        // Send each network as a separate chunk
        int chunkIndex = 0;
        for (JsonVariant network : networks) {
            DynamicJsonDocument chunkDoc(512);
            chunkDoc["type"] = "chunk";
            chunkDoc["cmd"] = command;
            chunkDoc["chunkIndex"] = chunkIndex;
            chunkDoc["totalChunks"] = totalNetworks;
            chunkDoc["network"] = network;
            
            String chunkOutput;
            serializeJson(chunkDoc, chunkOutput);
            
            Serial.printf("BLE: Sending chunk %d/%d, size: %d\n", 
                          chunkIndex + 1, totalNetworks, chunkOutput.length());
            
            if (!sendData(chunkOutput)) {
                Serial.printf("BLE: Failed to send chunk %d\n", chunkIndex);
                return false;
            }
            
            chunkIndex++;
            delay(20); // Small delay between chunks to avoid overwhelming
        }
        
        // Send completion message
        DynamicJsonDocument endResponse(256);
        endResponse["type"] = "response";
        endResponse["cmd"] = command;
        endResponse["status"] = STATUS_SUCCESS;
        endResponse["message"] = "All chunks sent";
        
        String endOutput;
        serializeJson(endResponse, endOutput);
        
        return sendData(endOutput);
    }
    
    // For non-network data, fall back to regular send
    return sendResponse(command, status, data);
}

void BLEManager::checkConnection() {
    // Disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // Give BLE stack time to get ready
        server->startAdvertising();
        Serial.println("BLE: Started advertising again");
        oldDeviceConnected = deviceConnected;
    }
    
    // Connecting
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
}