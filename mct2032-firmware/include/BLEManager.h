/**
 * BLE Manager for MCT2032
 * Handles BLE GATT server and communication
 */

#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <functional>
#include "protocol.h"

class BLEManager {
private:
    NimBLEServer* server;
    NimBLEService* service;
    NimBLECharacteristic* cmdCharacteristic;
    NimBLECharacteristic* dataCharacteristic;
    NimBLECharacteristic* statusCharacteristic;
    
    bool deviceConnected;
    bool oldDeviceConnected;
    
    // Command callback
    std::function<void(String)> commandCallback;
    
    // Server callbacks
    class ServerCallbacks : public NimBLEServerCallbacks {
        BLEManager* parent;
    public:
        ServerCallbacks(BLEManager* p) : parent(p) {}
        
        void onConnect(NimBLEServer* pServer) {
            parent->deviceConnected = true;
            Serial.println("BLE: Client connected");
        }
        
        void onDisconnect(NimBLEServer* pServer) {
            parent->deviceConnected = false;
            Serial.println("BLE: Client disconnected");
        }
    };
    
    // Characteristic callbacks
    class CommandCallbacks : public NimBLECharacteristicCallbacks {
        BLEManager* parent;
    public:
        CommandCallbacks(BLEManager* p) : parent(p) {}
        
        void onWrite(NimBLECharacteristic* pCharacteristic) {
            std::string value = pCharacteristic->getValue();
            if (value.length() > 0) {
                Serial.printf("BLE: Received command: %s\n", value.c_str());
                if (parent->commandCallback) {
                    parent->commandCallback(String(value.c_str()));
                }
            }
        }
    };

public:
    BLEManager();
    
    void init();
    bool isConnected() { return deviceConnected; }
    void setCommandCallback(std::function<void(String)> callback);
    
    // Send data methods
    bool sendData(const String& data);
    bool sendStatus(const String& status);
    bool sendResponse(const String& command, const String& status, JsonDocument& data);
    bool sendError(const String& command, const String& error);
    
    // Notification helpers
    void notifyData(const String& data);
    void notifyStatus(const String& status);
    
    // Connection management
    void checkConnection();
    void startAdvertising();
};