# MCT2032 Flashing Guide

## ESP32-S3 Flash Methods

### 🔴 Important: Waveshare ESP32-S3-LCD-1.47 Buttons

Your board has:
- **BOOT button**: Forces download mode
- **RESET button**: Resets the device
- **USB-C port**: For power and programming

### Method 1: Web Installer Flash

1. Open Chrome/Edge browser
2. Navigate to the web installer
3. Connect ESP32-S3 via USB-C
4. **HOLD the BOOT button**
5. Click "Install Firmware" 
6. Keep holding BOOT until "Connected" appears
7. Release BOOT button
8. Wait for installation to complete

### Method 2: Manual Download Mode

1. Connect USB-C cable
2. Hold BOOT button
3. Press and release RESET button
4. Release BOOT button
5. The device is now in download mode
6. Click "Install Firmware"

### Method 3: PlatformIO Upload

If web installer keeps failing:

```bash
cd mct2032-firmware

# Put device in download mode (BOOT + RESET method)
# Then run:
pio run -t upload

# Or try with specific port:
pio run -t upload --upload-port /dev/cu.usbmodem11201
```

### Troubleshooting

#### "Failed to initialize"
- Not in download mode - use BOOT button
- Wrong USB cable - use data cable, not charge-only
- Driver issues - install CH340/CP2102 drivers

#### "Serial port not ready"  
- Close browser tabs using the port
- Unplug and replug device
- Kill processes: `lsof | grep usbmodem`

#### "No serial port found"
- Check USB cable is data-capable
- Try different USB port
- Install drivers: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

#### Device Not Recognized
1. Check device manager / system info
2. Try USB 2.0 port instead of USB 3.0
3. Use shorter USB cable
4. Disable USB hubs

### After Successful Flash

1. The device will reset automatically
2. You should see "MCT2032" on the circular display
3. The device will broadcast BLE as "CyberTool"
4. Use the admin console to connect

### LED Indicators

- **During flash**: LED may blink rapidly
- **After flash**: Normal operation mode
- **If no display**: Check power, try reset button

## Direct Serial Flash

If nothing else works:

```bash
# Find your port
ls /dev/cu.*

# Use esptool directly
esptool.py --chip esp32s3 --port /dev/cu.usbmodem11201 --baud 460800 write_flash -z 0x0 releases/mct2032-firmware-v1.0.0.bin
```

## Still Having Issues?

1. Check the USB-C cable orientation (try flipping it)
2. Some USB-C cables are charge-only - use a data cable
3. Try a different computer/port
4. Post issue at: https://github.com/webdevtodayjason/MCT2032/issues