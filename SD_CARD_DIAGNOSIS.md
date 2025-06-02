# SD Card Diagnosis for MCT2032

## Current Symptoms
1. **CRC Errors**: `[W][sd_diskio.cpp:180] sdCommand(): crc error`
2. **No Token Received**: `[W][sd_diskio.cpp:174] sdCommand(): no token received`
3. **FAT Volume Error**: `f_mount failed: (13) There is no valid FAT volume`
4. **Watchdog Timeout**: ESP32 crashes during SD initialization

## Root Cause Analysis

The SD card hardware is detected but communication is failing. This indicates:
- ✅ SD card is inserted (otherwise different error)
- ✅ SD card has FAT32 format (your Mac shows this)
- ❌ SPI communication is failing
- ❌ Possible pin conflict or wiring issue

## Most Likely Issues

### 1. **SPI Bus Conflict with Display**
The display uses SPI pins that might conflict with SD card:
- Display: SCK=40, MOSI=45, CS=42
- SD Card: SCK=14, MOSI=15, CS=21

### 2. **Power Issue**
SD cards can draw significant current during initialization. The 3.3V rail might be insufficient.

### 3. **Card Compatibility**
Some SD cards don't work well with ESP32. Your 32GB card might be the issue.

## Solutions to Try

### Solution 1: Try a Different SD Card
Best options:
- **8GB or 16GB card** (not 32GB)
- **Class 10 speed**
- **SanDisk or Samsung brand**
- Format with **SD Card Formatter tool** (not Mac Disk Utility)

### Solution 2: Modify Hardware Initialization Order
```cpp
// In main.cpp setup(), initialize SD before display:
void setup() {
    Serial.begin(115200);
    
    // Initialize SD Card FIRST (before display)
    sdCard = new SDCardManager();
    if (sdCard->begin(21)) {
        Serial.println("SD Card initialized");
    }
    
    // THEN initialize display
    // ... rest of setup
}
```

### Solution 3: Use Different Pins
The Waveshare board might have different SD wiring than documented. Try:
- CS Pin: 10, 11, or 13 instead of 21
- Or check the board schematic

### Solution 4: Add Capacitor
Add a 100µF capacitor between 3.3V and GND near the SD card slot to stabilize power.

## Immediate Workaround

For now, disable SD card initialization to prevent crashes:
```cpp
// In main.cpp setup()
// Comment out SD initialization
// sdCard = new SDCardManager();
// if (sdCard->begin(21)) { ... }
```

## What's Working
- WiFi Scanning ✅
- BLE Communication ✅
- Display ✅
- RGB LED ✅

Focus on these features while we solve the SD card issue.