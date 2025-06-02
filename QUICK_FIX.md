# Quick Fix for USB HID on MCT2032

## Current Situation
- ✅ BLE communication working perfectly
- ✅ Ducky Script parsing works (23 instructions parsed)
- ✅ Chunking works for large scripts
- ❌ USB HID not sending keystrokes (USB mode issue)
- ❌ SD card not detected

## Immediate Solution

### Step 1: Install Arduino IDE
1. Download Arduino IDE 2.x from https://www.arduino.cc/en/software
2. Install ESP32 board support:
   - File → Preferences
   - Add to "Additional Board URLs": 
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Tools → Board → Boards Manager → Search "esp32" → Install

### Step 2: Create Simple USB HID Test
1. Open Arduino IDE
2. Create new sketch with this code:

```cpp
#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard Keyboard;

void setup() {
  Keyboard.begin();
  USB.begin();
  delay(2000);
}

void loop() {
  delay(5000);
  
  // Open Run dialog
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  delay(100);
  Keyboard.releaseAll();
  delay(500);
  
  // Type notepad
  Keyboard.print("notepad");
  delay(100);
  Keyboard.write(KEY_RETURN);
  delay(1000);
  
  // Type message
  Keyboard.print("MCT2032 USB HID Working!");
  
  delay(10000);
}
```

### Step 3: Upload with Correct Settings
1. Tools → Board → "ESP32S3 Dev Module"
2. Tools → USB Mode → "USB-OTG"
3. Tools → USB CDC On Boot → "Disabled"
4. Tools → Upload Mode → "UART0 / Hardware CDC"
5. Tools → Port → Select your device
6. Upload the sketch

### Step 4: Test
1. Disconnect and reconnect the USB cable
2. The device should open Notepad every 15 seconds and type a message
3. Check Windows Device Manager - should show as "USB Input Device"

## Alternative: Dual Firmware Approach

Since PlatformIO doesn't support USB HID mode easily:

1. **Main Firmware** (PlatformIO) - For BLE, WiFi, Display
2. **HID Firmware** (Arduino IDE) - Just for USB keyboard emulation

You can switch between them as needed.

## Future Enhancement

Consider using an additional microcontroller (like ATmega32U4) dedicated to USB HID, connected to the ESP32 via UART. This would allow simultaneous BLE control and USB HID functionality.