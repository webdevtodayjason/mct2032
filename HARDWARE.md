# MCT2032 Hardware Documentation
## Waveshare ESP32-S3-LCD-1.47 Development Board

### Board Overview
- **Model**: ESP32-S3-LCD-1.47
- **Manufacturer**: Waveshare
- **MCU**: ESP32-S3R8 (Dual-core Xtensa® 32-bit LX7 @ 240MHz)
- **Flash**: 16MB
- **PSRAM**: 8MB  
- **Display**: 1.47" Round LCD (172×320 pixels, ST7789 driver)
- **Form Factor**: Compact circular design
- **Purchase Link**: [Waveshare Official](https://www.waveshare.com/esp32-s3-lcd-1.47.htm)

### Pin Definitions

#### Display Pins (ST7789)
```c
#define TFT_DC      41   // Data/Command
#define TFT_CS      42   // Chip Select
#define TFT_RST     39   // Reset
#define TFT_SCK     40   // SPI Clock
#define TFT_MOSI    45   // SPI MOSI
#define TFT_MISO    -1   // Not used
#define TFT_BL      48   // Backlight control
```

#### SD Card Pins (TF Card Slot)
```c
#define SD_CMD      15   // GPIO15 (MOSI)
#define SD_SCK      14   // GPIO14 (Clock)
#define SD_D0       16   // GPIO16 (MISO)
#define SD_D1       18   // GPIO18
#define SD_D2       17   // GPIO17
#define SD_D3       21   // GPIO21 (CS/Chip Select)
```

#### Other Peripherals
```c
#define RGB_LED_PIN 38   // Onboard RGB LED (WS2812)
#define BOOT_BTN    0    // Boot button
```

### Display Specifications
- **Type**: IPS LCD with ST7789 driver
- **Resolution**: 172×320 pixels (circular display)
- **Color Depth**: 262K colors
- **Interface**: SPI
- **Backlight**: Adjustable via GPIO48
- **Physical Shape**: Round with 86px radius
- **Orientation**: Portrait mode (172 wide × 320 tall)

### SD Card Support
- **Interface**: 4-bit SDIO/SPI
- **Supported Cards**: microSD/TF cards up to 32GB
- **File Systems**: FAT32, exFAT
- **Speed**: Up to 40MHz SPI clock
- **Usage**: Store images, logs, payloads, configuration files

### Wireless Capabilities
- **WiFi**: 802.11 b/g/n (2.4GHz)
- **Bluetooth**: BLE 5.0 + Bluetooth Mesh
- **Antenna**: Onboard PCB antenna
- **RF Performance**: Up to +20dBm transmit power

### Power Specifications
- **Input Voltage**: 5V via USB-C
- **Operating Voltage**: 3.3V (internal)
- **Power Consumption**: ~120mA typical (WiFi active)
- **Deep Sleep Current**: <10μA

### Memory Layout
- **Flash**: 16MB QSPI
  - Application: Up to 8MB
  - SPIFFS/LittleFS: 2MB
  - OTA: 2MB
  - NVS: 16KB
- **PSRAM**: 8MB (Octal SPI)
- **Internal SRAM**: 512KB

### Development Support
- **Frameworks**: Arduino IDE, ESP-IDF, MicroPython
- **USB**: USB-C with CP2102 USB-to-UART bridge
- **Programming**: Auto-download circuit (no button press needed)
- **Debug**: UART output via USB

### Special Features
- **RGB LED**: Addressable WS2812 for status indication
- **Boot Button**: Can be used as user input
- **Circular Display Mask**: Hardware supports circular masking in LVGL
- **Low Power**: Multiple sleep modes supported
- **Security**: Secure Boot, Flash Encryption capable

### Known Issues & Workarounds

#### 1. SD Card Initialization
- Must use proper SPI configuration with correct pins
- Some cards require slower initial clock speed (10MHz)
- Format card as FAT32 for best compatibility

#### 2. Display Artifacts
- ST7789 requires proper initialization sequence
- Column/row offsets: col_offset1=34, row_offset1=0
- Anti-aliasing disabled for better clarity

#### 3. WiFi Promiscuous Mode
- Conflicts with normal WiFi scanning
- Must disable promiscuous mode before WiFi.scanNetworks()
- Add delays between mode switches

### Useful Resources
- **Official Wiki**: https://www.waveshare.com/wiki/ESP32-S3-LCD-1.47
- **Demo Code**: https://files.waveshare.com/wiki/ESP32-S3-LCD-1.47/ESP32-S3-LCD-1.47-Demo.zip
- **Schematic**: Available on Waveshare Wiki
- **ESP32-S3 Datasheet**: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf

### PlatformIO Configuration
```ini
[env:esp32-s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
upload_speed = 921600
board_build.partitions = huge_app.csv
board_build.flash_mode = qio
build_flags = 
    -D ARDUINO_USB_MODE=1
    -D ARDUINO_USB_CDC_ON_BOOT=1
```

### LVGL Display Configuration
```c
// Display buffer size for smooth rendering
static lv_color_t buf[SCREEN_WIDTH * 20];

// Display driver settings
disp_drv.hor_res = 172;
disp_drv.ver_res = 320;
disp_drv.antialiasing = 0;  // Better for small displays
disp_drv.dpi = 130;

// Circular mask for round display
lv_obj_set_style_radius(mask_circle, 86, 0);
lv_obj_set_style_clip_corner(mask_circle, true, 0);
```

### USB HID Capabilities
- **USB OTG**: ESP32-S3 supports USB OTG
- **HID Modes**: Keyboard, Mouse, Composite devices
- **CDC Serial**: Available for debug output
- **Requires**: TinyUSB library or ESP-IDF USB stack

### Tested Configurations
1. **Arduino IDE**: Version 2.x with ESP32 board package 2.0.14+
2. **PlatformIO**: Version 6.x with espressif32 platform 6.x
3. **SD Cards**: SanDisk 16GB/32GB Class 10
4. **Power Supply**: 5V 1A minimum via USB-C

### Hardware Modifications
- **External Antenna**: U.FL connector can be added for better range
- **Battery Power**: 3.7V LiPo can be connected with proper power management
- **GPIO Expansion**: Several GPIOs available on test points

### Comparison with Similar Boards
| Feature | ESP32-S3-LCD-1.47 | ESP32-S3-GEEK | T-Display-S3 |
|---------|-------------------|----------------|---------------|
| Display | 1.47" Round | 1.14" Rectangle | 1.9" Rectangle |
| Resolution | 172×320 | 240×135 | 170×320 |
| SD Card | Yes | Yes | No |
| RGB LED | Yes | Yes | No |
| Price | ~$15 | ~$20 | ~$18 |

### Tips for Development
1. Always initialize SD card before WiFi to avoid SPI conflicts
2. Use hardware SPI (HSPI) for best performance
3. Implement watchdog timer for stability
4. Use PSRAM for large buffers (WiFi scan results, etc.)
5. Enable USB CDC on boot for easier debugging

### Future Hardware Considerations
- Consider ESP32-S3-WROOM-2 module for more GPIOs
- External high-gain antenna for better packet capture
- Add accelerometer for motion detection
- Include buzzer for audio feedback
- Battery management IC for portable operation

---
*Last Updated: January 2025*
*Document Version: 1.0*