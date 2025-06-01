/**
 * WiFi Scanner implementation
 */

#include "WiFiScanner.h"
#include "protocol.h"

String NetworkInfo::getSecurityString() const {
    return WiFiScanner::encryptionTypeToString(encryptionType);
}

WiFiScanner::WiFiScanner() : scanning(false), scanStartTime(0) {
}

void WiFiScanner::init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    Serial.println("WiFi Scanner initialized");
}

bool WiFiScanner::startScan(uint8_t channel) {
    if (scanning) {
        return false;
    }
    
    networks.clear();
    scanning = true;
    scanStartTime = millis();
    
    // Start async scan
    if (channel == 0) {
        WiFi.scanNetworks(true, true); // async, show hidden
    } else {
        // Single channel scan not directly supported, scan all then filter
        WiFi.scanNetworks(true, true);
    }
    
    Serial.println("WiFi scan started");
    return true;
}

bool WiFiScanner::isScanning() {
    int16_t result = WiFi.scanComplete();
    
    if (result == WIFI_SCAN_RUNNING) {
        return true;
    } else if (result >= 0) {
        // Scan complete
        processScanResults();
        scanning = false;
        return false;
    } else {
        // Scan failed
        scanning = false;
        return false;
    }
}

void WiFiScanner::processScanResults() {
    int16_t count = WiFi.scanComplete();
    
    if (count <= 0) {
        Serial.println("No networks found");
        return;
    }
    
    Serial.printf("Found %d networks\n", count);
    
    for (int i = 0; i < count; i++) {
        NetworkInfo info;
        info.ssid = WiFi.SSID(i);
        info.bssid = WiFi.BSSIDstr(i);
        info.rssi = WiFi.RSSI(i);
        info.channel = WiFi.channel(i);
        info.encryptionType = WiFi.encryptionType(i);
        info.hidden = (info.ssid.length() == 0);
        
        networks.push_back(info);
    }
    
    // Clean up
    WiFi.scanDelete();
}

void WiFiScanner::toJSON(JsonDocument& doc) {
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