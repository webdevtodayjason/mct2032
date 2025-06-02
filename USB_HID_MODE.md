# ESP32-S3 USB HID Mode Configuration

## The Problem
The current implementation logs USB HID actions but doesn't actually send keystrokes to the host computer. This is because:

1. The ESP32-S3 is currently configured as a USB Serial device (CDC)
2. To act as a keyboard, it needs to be configured as a USB HID device
3. PlatformIO doesn't easily support switching USB modes for ESP32-S3

## Solution Options

### Option 1: Arduino IDE (Recommended for Testing)
1. Install Arduino IDE 2.x
2. Add ESP32 board support: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
3. Select board: "ESP32S3 Dev Module"
4. Configure USB settings:
   - USB Mode: **USB-OTG**
   - USB CDC On Boot: **Disabled**
   - USB DFU On Boot: **Disabled**
5. Upload the test sketch

### Option 2: Modify PlatformIO Build (Advanced)
Add these build flags to platformio.ini:
```ini
build_flags = 
    -D ARDUINO_USB_MODE=0
    -D ARDUINO_USB_CDC_ON_BOOT=0
    -D ARDUINO_USB_MSC_ON_BOOT=0
    -D ARDUINO_USB_DFU_ON_BOOT=0
```

### Option 3: Use ESP-IDF Instead
ESP-IDF provides more control over USB configuration but requires rewriting the firmware.

## Testing USB HID

Use the test-usb-hid.ino sketch to verify USB HID is working:
1. Upload via Arduino IDE with proper USB settings
2. Device should type "Hello from ESP32-S3!" every 10 seconds
3. LED should blink when typing

## Important Notes

- When in USB HID mode, you lose serial console access
- The device will appear as a keyboard in Device Manager
- You may need to press the BOOT button when uploading new firmware
- Some antivirus software may flag HID devices as suspicious

## Current Status

The firmware successfully:
- ✅ Receives Ducky Scripts via BLE
- ✅ Parses scripts into instructions (23 commands parsed)
- ✅ Handles large scripts via chunking
- ❌ Doesn't send actual keystrokes (USB mode issue)
- ❌ SD card not detected (separate issue)

Once USB HID mode is properly configured, the Ducky Scripts will execute on the target machine.