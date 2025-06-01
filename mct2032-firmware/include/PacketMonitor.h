/**
 * Packet Monitor for MCT2032
 * Handles promiscuous mode packet capture and analysis
 */

#ifndef PACKET_MONITOR_H
#define PACKET_MONITOR_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <vector>
#include <functional>
#include <SD.h>
#include <FS.h>

// Frame types
#define FRAME_TYPE_MGMT     0x00
#define FRAME_TYPE_CTRL     0x01
#define FRAME_TYPE_DATA     0x02

// Management frame subtypes
#define FRAME_SUBTYPE_BEACON        0x08
#define FRAME_SUBTYPE_PROBE_REQ     0x04
#define FRAME_SUBTYPE_PROBE_RESP    0x05
#define FRAME_SUBTYPE_DEAUTH        0x0C
#define FRAME_SUBTYPE_AUTH          0x0B
#define FRAME_SUBTYPE_ASSOC_REQ     0x00
#define FRAME_SUBTYPE_ASSOC_RESP    0x01

struct PacketInfo {
    uint8_t type;
    uint8_t subtype;
    uint8_t channel;
    int8_t rssi;
    uint8_t srcMAC[6];
    uint8_t dstMAC[6];
    uint32_t timestamp;
    uint16_t length;
};

class PacketMonitor {
private:
    bool monitoring;
    uint8_t currentChannel;
    
    // Packet statistics
    uint32_t packetsTotal;
    uint32_t beaconCount;
    uint32_t probeCount;
    uint32_t deauthCount;
    uint32_t dataCount;
    uint32_t mgmtCount;
    uint32_t ctrlCount;
    
    // Timing
    uint32_t startTime;
    uint32_t lastPacketTime;
    
    // Callbacks
    std::function<void(const PacketInfo&)> packetCallback;
    
    // PCAP capture
    bool pcapActive;
    File pcapFile;
    
    // Static callback for promiscuous mode
    static void promiscuousCallback(void* buf, wifi_promiscuous_pkt_type_t type);
    static PacketMonitor* instance;
    
    // Helper methods
    void processMgmtFrame(const uint8_t* payload, uint16_t len, int8_t rssi);
    void processDataFrame(const uint8_t* payload, uint16_t len, int8_t rssi);
    void processCtrlFrame(const uint8_t* payload, uint16_t len, int8_t rssi);
    
    void writePCAP_GlobalHeader();
    void writePCAP_PacketHeader(uint32_t ts_sec, uint32_t ts_usec, uint32_t len);
    void writePCAP_PacketData(const uint8_t* data, uint32_t len);
    
public:
    PacketMonitor();
    
    void init();
    bool startMonitor(uint8_t channel = 0);
    void stopMonitor();
    bool isMonitoring() const { return monitoring; }
    
    // Channel hopping
    void setChannel(uint8_t channel);
    void hopChannel();
    
    // Packet injection
    bool injectPacket(const uint8_t* data, uint16_t len);
    bool sendDeauth(const uint8_t* apMAC, const uint8_t* stationMAC, uint8_t reason);
    bool sendBeacon(const char* ssid, uint8_t channel);
    
    // PCAP capture
    bool startPCAP(const char* filename);
    void stopPCAP();
    bool isPCAPActive() const { return pcapActive; }
    
    // Statistics
    uint32_t getPacketsTotal() const { return packetsTotal; }
    uint32_t getPacketsPerSec() const;
    uint32_t getBeaconCount() const { return beaconCount; }
    uint32_t getProbeCount() const { return probeCount; }
    uint32_t getDeauthCount() const { return deauthCount; }
    uint32_t getDataCount() const { return dataCount; }
    uint32_t getMgmtCount() const { return mgmtCount; }
    
    void resetStats();
    
    // Callback
    void setPacketCallback(std::function<void(const PacketInfo&)> callback) {
        packetCallback = callback;
    }
};

#endif // PACKET_MONITOR_H