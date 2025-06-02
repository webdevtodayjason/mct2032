# MCT2032 Hardware Documentation
## Waveshare ESP32-S3-LCD-1.47 Complete Technical Reference

### Table of Contents
1. [Hardware Overview](#hardware-overview)
2. [Processor Specifications](#processor-specifications)
3. [Memory Configuration](#memory-configuration)
4. [Display Specifications](#display-specifications)
5. [Pin Assignments](#pin-assignments)
6. [SD Card Interface](#sd-card-interface)
7. [Wireless Capabilities](#wireless-capabilities)
8. [Power Management](#power-management)
9. [Programming and Development](#programming-and-development)
10. [Known Issues and Solutions](#known-issues-and-solutions)

---

## Hardware Overview

The MCT2032 project uses the **Waveshare ESP32-S3-LCD-1.47** development board, a compact yet powerful platform featuring:
- Circular 1.47" color LCD display
- ESP32-S3 dual-core processor
- Built-in WiFi and Bluetooth 5
- SD card slot for data storage
- USB Type-A interface for HID functionality
- Compact form factor ideal for portable cybersecurity tools

## Processor Specifications

- **Model**: ESP32-S3R8
- **Architecture**: Dual-core Xtensa LX7
- **Clock Speed**: Up to 240MHz
- **Features**:
  - Ultra-low power co-processor
  - Hardware acceleration for cryptographic algorithms
  - USB OTG support
  - Temperature sensor
  - Hall sensor

## Memory Configuration

### Internal Memory
- **SRAM**: 512KB
- **ROM**: 384KB
- **RTC SRAM**: 16KB

### External Memory
- **Flash**: 16MB (Quad SPI)
- **PSRAM**: 8MB (Octal SPI)

### Memory Mapping
```
Flash Partition Table (16MB):
- Bootloader: 0x1000
- App Partition: 0x10000 (up to 6.5MB with huge_app.csv)
- SPIFFS/FAT: Remaining space
```

## Display Specifications

### LCD Details
- **Size**: 1.47 inches (circular)
- **Resolution**: 172×320 pixels
- **Color Depth**: 262K colors (18-bit)
- **Controller IC**: ST7789
- **Interface**: SPI
- **Viewing Area**: Circular with masking

### Display Pin Connections
| Function | GPIO Pin | Notes |
|----------|----------|-------|
| MOSI | GPIO45 | SPI Data |
| SCLK | GPIO40 | SPI Clock |
| LCD_CS | GPIO42 | Chip Select |
| LCD_DC | GPIO41 | Data/Command |
| LCD_RST | GPIO39 | Reset |
| LCD_BL | GPIO48 | Backlight PWM |

## Pin Assignments

### SD Card Interface (SPI Mode)
| Function | GPIO Pin | SD Card Pin |
|----------|----------|-------------|
| SD_CMD (MOSI) | GPIO15 | CMD |
| SD_SCK | GPIO14 | CLK |
| SD_D0 (MISO) | GPIO16 | DAT0 |
| SD_D1 | GPIO18 | DAT1 |
| SD_D2 | GPIO17 | DAT2 |
| SD_D3 (CS) | GPIO21 | DAT3/CS |

### Control Pins
| Function | GPIO Pin | Notes |
|----------|----------|-------|
| RGB LED | GPIO38 | WS2812B addressable LED |
| BOOT Button | GPIO0 | Hold during reset for download mode |
| RESET Button | EN | System reset |

### USB Configuration
- **Type**: USB Type-A male connector
- **Mode**: USB OTG (Device/Host switchable)
- **HID Support**: Full HID keyboard/mouse emulation
- **CDC Support**: Serial communication

## SD Card Interface

### Supported Cards
- **Type**: MicroSD/TF cards
- **File System**: FAT32 (recommended)
- **Capacity**: Up to 32GB tested (larger may work)
- **Speed**: Default 10MHz, configurable down to 1MHz for compatibility

### SPI Configuration
```cpp
// Waveshare ESP32-S3-LCD-1.47 SD Card SPI
SPIClass spi(FSPI);  // Use FSPI bus on ESP32-S3
spi.begin(14, 16, 15, 21);  // SCK, MISO, MOSI, CS
```

### Known Working Cards
- SanDisk Ultra 8-32GB
- Cards from Flipper Zero
- Most FAT32 formatted cards

## Wireless Capabilities

### WiFi Specifications
- **Standard**: 802.11 b/g/n
- **Frequency**: 2.4GHz
- **Modes**: Station, AP, Station+AP
- **Features**:
  - WPA/WPA2/WPA3 security
  - Promiscuous mode support
  - Raw packet injection
  - Up to 150Mbps data rate

### Bluetooth Specifications
- **Version**: Bluetooth 5.0
- **Mode**: Classic and BLE
- **Features**:
  - Long range mode
  - 2Mbps PHY
  - Advertising extensions
  - Multiple connections

## Power Management

### Power Supply
- **Input**: 5V via USB
- **Regulator**: ME6217C33M5G LDO
- **Output**: 3.3V @ 800mA max
- **Current Consumption**:
  - Active WiFi: ~150mA
  - BLE only: ~80mA
  - Deep sleep: <10μA

### Power Optimization
- Use `esp_wifi_set_ps(WIFI_PS_MAX_MODEM)` for WiFi power saving
- Implement deep sleep between operations
- Reduce CPU frequency when idle
- Turn off unused peripherals

## Programming and Development

### Development Environment
- **Framework**: Arduino + ESP-IDF
- **Platform**: PlatformIO
- **Required Board**: esp32-s3-devkitc-1
- **Upload Speed**: 921600 baud

### PlatformIO Configuration
```ini
[env:esp32-s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
upload_speed = 921600

; Memory configuration
board_build.partitions = default_16MB.csv
board_build.flash_size = 16MB
board_build.psram_type = opi
board_build.psram_size = 8MB

; Stack size optimization
build_flags = 
    -D CONFIG_BT_NIMBLE_HOST_TASK_STACK_SIZE=8192
    -D CONFIG_ARDUINO_LOOP_STACK_SIZE=16384
```

### Upload Mode
1. Hold BOOT button
2. Press RESET button
3. Release RESET button
4. Release BOOT button
5. Device enters download mode

## Known Issues and Solutions

### 1. SD Card Stack Overflow
**Problem**: BLE stack overflow when accessing SD card
**Solution**: 
- Run SD operations in separate FreeRTOS task
- Increase stack sizes in platformio.ini
- Use static JSON documents

### 2. Display Overlapping Text
**Problem**: LVGL text rendering overlaps on circular display
**Solution**:
- Implement proper display masking
- Use smaller fonts
- Adjust label positioning

### 3. 32GB+ SD Cards
**Problem**: Some large SD cards fail to initialize
**Solution**:
- Use cards ≤32GB
- Format as FAT32 with 32KB cluster size
- Reduce SPI speed to 1MHz if needed

### 4. Flash Size Detection
**Problem**: PlatformIO detects 8MB instead of 16MB
**Solution**:
- Explicitly set board_build.flash_size = 16MB
- Use default_16MB.csv partition table

---

## Development Progress Summary

### Completed Features ✅
1. **BLE Communication**
   - Full bidirectional communication
   - JSON command protocol
   - Stable connection management

2. **SD Card Support**
   - Read/write operations
   - FAT32 file system
   - Stack overflow fix implemented

3. **WiFi Scanning**
   - Network discovery
   - Signal strength reporting
   - Channel information

4. **USB HID**
   - Keyboard emulation
   - Ducky Script framework

5. **Admin Console**
   - Python GUI application
   - Dark theme UI
   - Real-time device communication

### Current Issues 🔧
1. Display text overlap needs fixing
2. Packet monitoring not fully implemented
3. Ducky Script deployment needs chunking for large scripts

### Next Steps 📋
1. Implement packet monitoring
2. Fix display rendering issues
3. Add data export functionality
4. Implement proper Ducky Script chunking
5. Add more payload templates

---

*Last Updated: January 6, 2025*
*Firmware Version: 1.0.0*
*Admin Console Version: 1.0.0*