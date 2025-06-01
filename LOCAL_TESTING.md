# Local Testing Guide for MCT2032 Web Installer

## Quick Start

### 1. Start the Local Web Server

```bash
cd /Users/jasonbrashear/code/MCT2032
python3 local-server.py
```

### 2. Open in Browser

Visit: http://localhost:8000/

- For testing with placeholder: http://localhost:8000/index-local.html
- For production version: http://localhost:8000/index.html

### 3. Test the Installation Process

1. Click "Test Install" button
2. Browser will prompt for serial port access
3. Select your ESP32 device (if connected)
4. Watch the installation process

## Building Real Firmware

### Option 1: Wait for PlatformIO

The firmware is being compiled in the background. Once complete:

```bash
cd mct2032-firmware
platformio run

# Find firmware at:
# .pio/build/esp32-s3/firmware.bin
```

### Option 2: Quick Arduino IDE Build

If you have Arduino IDE installed:

1. Open `mct2032-firmware/src/main.cpp`
2. Install required libraries:
   - LVGL
   - Arduino_GFX_Library
   - ArduinoJson
   - NimBLE-Arduino
3. Select board: ESP32-S3 Dev Module
4. Click Compile

## Testing Without Hardware

You can test the web installer interface even without an ESP32:

1. Open http://localhost:8000/index-local.html
2. Click "Test Install"
3. The browser will show "No compatible devices found"
4. This confirms the Web Serial API is working

## Features to Test

- ✅ Matrix rain animation
- ✅ Hover effects on buttons
- ✅ Responsive design (resize browser)
- ✅ Smooth scrolling
- ✅ Feature cards animations
- ✅ Device preview floating animation

## Troubleshooting

### "No compatible devices found"
- Normal if no ESP32 is connected
- Connect ESP32 via USB to test actual flashing

### Web Serial not supported
- Must use Chrome or Edge browser
- Safari and Firefox don't support Web Serial API

### CORS errors
- Make sure you're using the local-server.py script
- Don't open HTML files directly in browser

### PlatformIO taking too long
- First run downloads toolchains (can take 10-20 minutes)
- Subsequent builds will be much faster

## Next Steps

Once firmware is compiled:

1. Copy firmware to web installer:
```bash
cp mct2032-firmware/.pio/build/esp32-s3/firmware.bin web-installer/mct2032_firmware_v1.0.0.bin
```

2. Update manifest-local.json to point to real firmware:
```json
"path": "./mct2032_firmware_v1.0.0.bin"
```

3. Test actual firmware installation!

## Server Commands

- Start server: `python3 local-server.py`
- Stop server: Press `Ctrl+C`
- Default port: 8000
- Change port: Edit PORT variable in local-server.py