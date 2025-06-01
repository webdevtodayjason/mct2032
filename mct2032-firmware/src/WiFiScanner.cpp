/**
 * WiFi Scanner implementation
 */

#include "WiFiScanner.h"
#include "protocol.h"
#include <esp_wifi.h>

String NetworkInfo::getSecurityString() const {
    return WiFiScanner::encryptionTypeToString(encryptionType);
}

WiFiScanner::WiFiScanner() : scanning(false), scanStartTime(0) {
}

void WiFiScanner::init() {
    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    // Configure for better scanning (similar to Marauder)
    WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
    WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
    
    Serial.println("WiFi Scanner initialized");
}

bool WiFiScanner::startScan(uint8_t channel) {
    if (scanning) {
        Serial.println("Scan already in progress");
        return false;
    }
    
    // Clear previous results
    networks.clear();
    scanning = true;
    scanStartTime = millis();
    
    // Make sure we're not in promiscuous mode
    esp_wifi_set_promiscuous(false);
    delay(50);
    
    // Ensure WiFi is properly initialized and in station mode
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    delay(100);
    
    // Start async scan with parameters similar to Marauder
    // true = async, true = show hidden, false = passive scan, 300ms = dwell time per channel
    int result = WiFi.scanNetworks(true, true, false, 300, channel);
    
    if (result == WIFI_SCAN_RUNNING) {
        Serial.printf("WiFi scan started successfully (channel: %d)\n", channel);
        return true;
    } else {
        Serial.printf("WiFi scan failed to start (error: %d)\n", result);
        scanning = false;
        return false;
    }
}

bool WiFiScanner::isScanning() {
    // Add timeout check (10 seconds)
    if (scanning && (millis() - scanStartTime > 10000)) {
        Serial.println("WiFi scan timeout - forcing completion");
        WiFi.scanDelete();
        scanning = false;
        return false;
    }
    
    int16_t result = WiFi.scanComplete();
    
    if (result == WIFI_SCAN_RUNNING) {
        return true;
    } else if (result >= 0) {
        // Scan complete
        Serial.printf("WiFi scan complete with result: %d\n", result);
        processScanResults();
        scanning = false;
        return false;
    } else {
        // Scan failed
        Serial.printf("WiFi scan failed with error: %d\n", result);
        scanning = false;
        return false;
    }
}

void WiFiScanner::stopScan() {
    if (scanning) {
        Serial.println("Stopping WiFi scan");
        WiFi.scanDelete();
        scanning = false;
    }
}

void WiFiScanner::processScanResults() {
    int16_t count = WiFi.scanComplete();
    
    if (count == WIFI_SCAN_FAILED) {
        Serial.println("WiFi scan failed!");
        return;
    }
    
    if (count == 0) {
        Serial.println("No networks found");
        return;
    }
    
    Serial.printf("Processing %d networks...\n", count);
    
    // Reserve space for efficiency
    networks.reserve(count);
    
    for (int i = 0; i < count; i++) {
        NetworkInfo info;
        info.ssid = WiFi.SSID(i);
        info.bssid = WiFi.BSSIDstr(i);
        info.rssi = WiFi.RSSI(i);
        info.channel = WiFi.channel(i);
        info.encryptionType = WiFi.encryptionType(i);
        info.hidden = (info.ssid.length() == 0);
        
        // Debug output for each network
        Serial.printf("  [%d] SSID: %s, RSSI: %d, CH: %d\n", 
                      i, info.ssid.c_str(), info.rssi, info.channel);
        
        networks.push_back(info);
    }
    
    Serial.printf("Scan processing complete. Total networks: %d\n", networks.size());
    
    // Clean up scan results
    WiFi.scanDelete();
}

void WiFiScanner::toJSON(DynamicJsonDocument& doc) {
    JsonArray networkArray = doc.createNestedArray(JSON_NETWORKS);
    
    for (const auto& net : networks) {
        JsonObject netObj = networkArray.createNestedObject();
        netObj[JSON_SSID] = net.ssid;
        netObj[JSON_BSSID] = net.bssid;
        netObj[JSON_RSSI] = net.rssi;
        netObj[JSON_CHANNEL] = net.channel;
        netObj[JSON_SECURITY] = net.getSecurityString();
        netObj[JSON_HIDDEN] = net.hidden;
    }
}

String WiFiScanner::toJSONString() {
    DynamicJsonDocument doc(4096);
    toJSON(doc);
    
    String output;
    serializeJson(doc, output);
    return output;
}

String WiFiScanner::encryptionTypeToString(wifi_auth_mode_t encType) {
    switch (encType) {
        case WIFI_AUTH_OPEN:
            return SECURITY_OPEN;
        case WIFI_AUTH_WEP:
            return SECURITY_WEP;
        case WIFI_AUTH_WPA_PSK:
            return SECURITY_WPA;
        case WIFI_AUTH_WPA2_PSK:
            return SECURITY_WPA2;
        case WIFI_AUTH_WPA_WPA2_PSK:
            return SECURITY_WPA_WPA2;
        case WIFI_AUTH_WPA2_ENTERPRISE:
            return SECURITY_ENTERPRISE;
        case WIFI_AUTH_WPA3_PSK:
            return SECURITY_WPA3;
        default:
            return "UNKNOWN";
    }
}

int WiFiScanner::getChannelFromFrequency(int freq) {
    if (freq >= 2412 && freq <= 2484) {
        return (freq - 2412) / 5 + 1;
    } else if (freq == 2484) {
        return 14;
    }
    return -1;
}