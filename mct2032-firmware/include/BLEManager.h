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
            Serial.printf("BLE: onWrite called, data length: %d\n", value.length());
            
            if (value.length() > 0) {
                Serial.print("BLE: Raw bytes: ");
                for (size_t i = 0; i < value.length(); i++) {
                    Serial.printf("%02X ", (uint8_t)value[i]);
                }
                Serial.println();
                Serial.printf("BLE: Command string: %s\n", value.c_str());
                
                if (parent->commandCallback) {
                    Serial.println("BLE: Calling command callback");
                    parent->commandCallback(String(value.c_str()));
                } else {
                    Serial.println("BLE: ERROR - No command callback registered!");
                }
            } else {
                Serial.println("BLE: ERROR - Empty command received");
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
    bool sendResponse(const String& command, const String& status, const DynamicJsonDocument& data);
    bool sendError(const String& command, const String& error);
    
    // Chunked data sending for large responses
    bool sendChunkedResponse(const String& command, const String& status, DynamicJsonDocument& data);
    
    // Notification helpers
    void notifyData(const String& data);
    void notifyStatus(const String& status);
    
    // Connection management
    void checkConnection();
    void startAdvertising();
};

#endif // BLE_MANAGER_H