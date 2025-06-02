# SD Card Fix Instructions

## Problem
The SD card is not being recognized by the ESP32. Error: "There is no valid FAT volume"

## Solution Steps

### 1. Format SD Card Properly
The ESP32 requires the SD card to be formatted as FAT32 with specific parameters:

#### On macOS:
```bash
# 1. Insert SD card and find its disk identifier
diskutil list

# 2. Unmount the SD card (replace diskX with your disk)
diskutil unmountDisk /dev/diskX

# 3. Format as FAT32 with 32KB cluster size
sudo newfs_msdos -F 32 -c 64 -n MCT2032 /dev/diskX

# 4. Eject the card
diskutil eject /dev/diskX
```

#### Using SD Card Formatter (Recommended):
1. Download SD Card Formatter from: https://www.sdcard.org/downloads/formatter/
2. Select your SD card
3. Format Type: "Full (Overwrite)"
4. Volume Label: "MCT2032"
5. Click Format

### 2. Test with Different SD Cards
Try different cards if available:
- Preferred: SanDisk or Samsung brand
- Size: 4GB to 32GB (larger cards may have issues)
- Class: Class 10 or UHS-I

### 3. Hardware Check
Ensure the SD card is:
- Fully inserted into the slot
- Not damaged or worn
- Clean contacts (use isopropyl alcohol if needed)

### 4. Alternative Pin Configuration
If formatting doesn't work, we might need to try different CS pins:

```cpp
// Current configuration
#define SD_CS_PIN   21  // Try changing to 10, 11, or 13

// Also verify these match your hardware:
#define SD_SCK_PIN  14  
#define SD_MISO_PIN 16  
#define SD_MOSI_PIN 15  
```

### 5. Debug Steps
1. Try initializing SD before display in setup()
2. Use even slower SPI speed (400kHz)
3. Add pull-up resistor on CS pin
4. Check if display and SD card share SPI conflict

## Expected Result
After proper formatting, the SD test should show:
- Initialized: true
- Total space: [size] MB
- Write test: true
- Read test: true