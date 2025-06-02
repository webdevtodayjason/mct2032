# Recommended ESP32 Boards for MCT2032 with Built-in USB HID

## Current Issue
Your Waveshare ESP32-S3-LCD-1.47 has the hardware capability for USB HID, but PlatformIO's Arduino framework makes it difficult to use both USB HID and other features simultaneously.

## Better Board Options

### 1. **LILYGO T-Dongle-S3** (Recommended)
- **Built-in USB-A connector** - Plugs directly into computer like your current board
- **ESP32-S3 with native USB** - Can do USB HID properly
- **Small OLED display** - For status
- **~$10-15** - Affordable
- **PlatformIO compatible** - Works with SuperWiFiDuck firmware
- **No external components needed**

### 2. **ESP32-S2/S3 DevKit Boards**
- **Cheap (~$5-10)**
- **Native USB support**
- **Proven to work with BadUSB projects**
- Examples:
  - ESP32-S3-DevKitC-1
  - ESP32-S2-Saola-1
  - Wemos S2 Mini

### 3. **M5Stack AtomS3**
- **Tiny form factor** (24x24mm)
- **ESP32-S3 with USB-C**
- **Built-in RGB LED** for status
- **~$15-20**

### 4. **Seeed XIAO ESP32S3**
- **Ultra-small** (21x17.5mm)
- **Native USB support**
- **Can act as USB HID**
- **~$7-10**

## What Makes These Better?

1. **Proven USB HID Support** - Projects like SuperWiFiDuck work on these
2. **Better Documentation** - More examples available
3. **Proper USB Mode Switching** - Can switch between CDC and HID
4. **Active Community** - Others have solved these problems

## Migration Path

If you switch boards, most of your code stays the same:
- ✅ BLE communication code
- ✅ WiFi scanning code
- ✅ Ducky Script parser
- ✅ Command processor
- ❌ Display code (would need adjustment)
- ✅ USB HID (would actually work!)

## My Recommendation

**LILYGO T-Dongle-S3** because:
1. Same "USB stick" form factor as your current device
2. Proven to work with BadUSB/WiFiDuck projects
3. Has a small display for status
4. Affordable and available

Would you like me to help adapt your code for one of these boards?