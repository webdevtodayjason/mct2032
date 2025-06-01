/**
 * WiFi Scanner for MCT2032
 * Handles WiFi network scanning and analysis
 */

#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include <ArduinoJson.h>

struct NetworkInfo {
    String ssid;
    String bssid;
    int32_t rssi;
    uint8_t channel;
    wifi_auth_mode_t encryptionType;
    bool hidden;
    
    String getSecurityString() const;
};

class WiFiScanner {
private:
    std::vector<NetworkInfo> networks;
    bool scanning;
    unsigned long scanStartTime;
    
public:
    WiFiScanner();
    
    void init();
    bool startScan(uint8_t channel = 0);
    bool isScanning();
    void stopScan();
    void processScanResults();
    
    std::vector<NetworkInfo>& getResults() { return networks; }
    size_t getNetworkCount() const { return networks.size(); }
    
    // JSON serialization
    void toJSON(DynamicJsonDocument& doc);
    String toJSONString();
    
    // Utility methods
    static String encryptionTypeToString(wifi_auth_mode_t encType);
    static int getChannelFromFrequency(int freq);
};

#endif // WIFI_SCANNER_H