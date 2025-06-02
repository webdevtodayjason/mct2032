# CH9329 USB HID Solution for MCT2032

## Why This Solves Our Problem

The CH9329 is a UART-to-USB HID chip that:
- Receives serial commands from ESP32
- Appears as a real USB keyboard/mouse to the computer
- Requires NO driver installation
- Works with our existing architecture

## How Bruce Does It

Bruce firmware uses CH9329 for BadUSB on devices like M5StickC:
- ESP32 → (UART) → CH9329 → (USB) → Target Computer
- Simple serial commands control keyboard/mouse
- Supports full Ducky Script implementation

## Hardware Requirements

1. **CH9329 Module** (~$5-10)
   - Available on Amazon, AliExpress, Adafruit
   - Size: About 20x15mm
   - Connections: VCC, GND, TX, RX

2. **Wiring** (4 wires):
   ```
   ESP32-S3 → CH9329
   3.3V/5V  → VCC
   GND      → GND
   TX       → RX
   RX       → TX
   ```

## Software Implementation

The CH9329 accepts simple serial commands:

```cpp
// Send keystroke
Serial.write(0x57); // Header
Serial.write(0xAB); // Header
Serial.write(0x00); // Address
Serial.write(0x02); // Command (keyboard)
Serial.write(0x08); // Length
Serial.write(0x00); // Modifier (none)
Serial.write(0x00); // Reserved
Serial.write(0x04); // Key code (A)
Serial.write(0x00); // Remaining keys...
Serial.write(0x00);
Serial.write(0x00);
Serial.write(0x00);
Serial.write(0x00);
Serial.write(checksum); // Sum of all bytes

// Release all keys
Serial.write(0x57);
Serial.write(0xAB);
Serial.write(0x00);
Serial.write(0x02);
Serial.write(0x08);
Serial.write(0x00); // All zeros = release
// ... rest zeros
```

## Integration Options

### Option 1: External Module
- Wire CH9329 module to ESP32 UART pins
- Minimal code changes
- Can test immediately

### Option 2: Integrated Design
- Add CH9329 chip to PCB design
- More compact final product
- Same software interface

## Implementation Steps

1. **Order CH9329 module** (Adafruit has good one: ID 5973)
2. **Wire to ESP32**:
   - Use UART2 (pins 16,17) to keep UART0 for debugging
3. **Update USBKeyboard.cpp**:
   - Send CH9329 protocol instead of debug logs
4. **Test with existing Ducky Scripts**

## Code Example

```cpp
class CH9329Keyboard {
  HardwareSerial ch9329Serial;
  
  void begin() {
    ch9329Serial.begin(9600, SERIAL_8N1, 16, 17); // RX, TX
  }
  
  void sendKey(uint8_t key, uint8_t modifier = 0) {
    uint8_t packet[14] = {
      0x57, 0xAB, 0x00, 0x02, 0x08,
      modifier, 0x00, key, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    uint8_t sum = 0;
    for(int i = 0; i < 13; i++) sum += packet[i];
    packet[13] = sum;
    
    ch9329Serial.write(packet, 14);
  }
  
  void releaseAll() {
    uint8_t packet[14] = {
      0x57, 0xAB, 0x00, 0x02, 0x08,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    uint8_t sum = 0;
    for(int i = 0; i < 13; i++) sum += packet[i];
    packet[13] = sum;
    
    ch9329Serial.write(packet, 14);
  }
};
```

## Advantages

1. **No firmware changes needed** - Just update USBKeyboard class
2. **Works with PlatformIO** - No USB mode issues
3. **Proven solution** - Bruce uses it successfully
4. **Cheap and available** - ~$5-10 component
5. **Maintains all features** - BLE, WiFi, display all work

## Next Steps

1. Order CH9329 module
2. Wire to ESP32 (can test with breadboard)
3. Update USBKeyboard.cpp to send CH9329 commands
4. Test with your existing Ducky Scripts

This is the same approach Bruce uses and it works reliably!