/**
 * SD Card Manager for MCT2032
 * 
 * Handles all SD card operations including:
 * - Payload storage and retrieval
 * - Data logging (PCAP, credentials, network maps)
 * - Plugin system for custom attacks
 * - Encrypted storage for sensitive data
 */

#ifndef SDCARD_MANAGER_H
#define SDCARD_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <vector>

// SD Card pins for Waveshare ESP32-S3-LCD-1.47
// Note: These are defined in main.cpp with correct Waveshare values

// Directory structure
#define DIR_ROOT        "/"
#define DIR_PAYLOADS    "/payloads"
#define DIR_LOGS        "/logs"
#define DIR_CAPTURES    "/captures"
#define DIR_WORDLISTS   "/wordlists"
#define DIR_PLUGINS     "/plugins"
#define DIR_CONFIG      "/config"
#define DIR_TEMP        "/temp"

// File size limits
#define MAX_PCAP_SIZE   (100 * 1024 * 1024)  // 100MB per PCAP file
#define MAX_LOG_SIZE    (10 * 1024 * 1024)   // 10MB per log file

class SDCardManager {
private:
    bool initialized;
    bool mounted;
    uint64_t totalSpace;
    uint64_t usedSpace;
    SPIClass* spi;
    
    // Encryption key for sensitive data
    uint8_t encryptionKey[32];
    
    // Helper functions
    String getTimestamp();
    String sanitizeFilename(String filename);
    bool ensureDirectory(const char* path);
    
public:
    SDCardManager();
    bool begin(int csPin = -1);  // Add simple begin method
    ~SDCardManager();
    
    // Initialization
    bool init();
    bool mount();
    void unmount();
    bool isInitialized() const { return initialized; }
    bool isMounted() const { return mounted; }
    
    // Space management
    uint64_t getTotalSpace();
    uint64_t getUsedSpace();
    uint64_t getFreeSpace();
    bool hasSpace(size_t bytes);
    
    // Directory operations
    bool createDirectory(const char* path);
    bool removeDirectory(const char* path);
    std::vector<String> listDirectory(const char* path);
    
    // File operations
    bool writeFile(const char* path, const uint8_t* data, size_t len);
    bool appendFile(const char* path, const uint8_t* data, size_t len);
    bool readFile(const char* path, uint8_t* buffer, size_t maxLen);
    bool deleteFile(const char* path);
    bool fileExists(const char* path);
    size_t getFileSize(const char* path);
    
    // Payload management
    String loadPayload(const char* os, const char* payloadName);
    bool savePayload(const char* os, const char* payloadName, const char* content);
    std::vector<String> listPayloads(const char* os = nullptr);
    
    // Data logging
    bool logWiFiScan(const JsonDocument& scanData);
    bool logNetworkScan(const JsonDocument& networkData);
    bool logCredentials(const String& credentials, bool encrypt = true);
    bool logPacketCapture(const uint8_t* packet, size_t len);
    
    // PCAP operations
    File startPCAPCapture(const char* filename);
    bool writePCAPPacket(File& pcapFile, const uint8_t* packet, size_t len, uint32_t timestamp);
    void closePCAPCapture(File& pcapFile);
    
    // Plugin system
    std::vector<String> listPlugins();
    String loadPlugin(const char* pluginName);
    bool executePlugin(const char* pluginName);
    
    // Configuration
    bool saveConfig(const char* configName, const JsonDocument& config);
    bool loadConfig(const char* configName, JsonDocument& config);
    
    // Security functions
    bool encryptData(const uint8_t* data, size_t len, uint8_t* output);
    bool decryptData(const uint8_t* data, size_t len, uint8_t* output);
    bool secureDelete(const char* path);  // Overwrite before delete
    
    // Wordlist operations
    bool loadWordlist(const char* name, std::vector<String>& words);
    bool saveWordlist(const char* name, const std::vector<String>& words);
    
    // Stealth operations
    bool hideFile(const char* path);
    bool unhideFile(const char* path);
    bool createDecoyStructure();  // Create fake music/photo folders
    
    // Maintenance
    bool checkAndRepair();
    bool defragment();
    void cleanTempFiles();
    
    // Self-destruct
    bool enableSelfDestruct(uint32_t triggerPin);
    bool wipeAllData();  // Emergency wipe
};

// Global instance - remove this since we're declaring as pointer in main.cpp
// extern SDCardManager sdCard;

// Helper class for automatic file closing
class AutoFile {
    File file;
public:
    AutoFile(const char* path, const char* mode) {
        file = SD.open(path, mode);
    }
    ~AutoFile() {
        if (file) file.close();
    }
    operator File&() { return file; }
    File* operator->() { return &file; }
};

#endif // SDCARD_MANAGER_H