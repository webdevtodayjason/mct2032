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
        NIMBLE_PROPERTY::WRITE
    );
    cmdCharacteristic->setCallbacks(new CommandCallbacks(this));
    
    // Create Data Characteristic (Read + Notify)
    dataCharacteristic = service->createCharacteristic(
        DATA_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
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
        return false;
    }
    
    try {
        dataCharacteristic->setValue(data.c_str());
        dataCharacteristic->notify();
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
        statusCharacteristic->setValue(status.c_str());
        statusCharacteristic->notify();
        return true;
    } catch(...) {
        Serial.println("BLE: Error sending status");
        return false;
    }
}

bool BLEManager::sendResponse(const String& command, const String& status, JsonDocument& data) {
    if (!deviceConnected) {
        return false;
    }
    
    JsonDocument response;
    response["type"] = "response";
    response["cmd"] = command;
    response["status"] = status;
    response["data"] = data;
    
    String output;
    serializeJson(response, output);
    
    return sendData(output);
}

bool BLEManager::sendError(const String& command, const String& error) {
    if (!deviceConnected) {
        return false;
    }
    
    JsonDocument response;
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