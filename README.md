# MCT2032 - Mega Cyber Tool 2032

A portable cybersecurity toolkit built on the Waveshare ESP32-S3-LCD-1.47 development board. Features a circular 1.47" display (172x320), Bluetooth LE connectivity, and advanced WiFi security research capabilities.

## Features

### Current Capabilities
- **WiFi Scanner**: Scan and analyze WiFi networks with detailed information (SSID, BSSID, RSSI, Channel, Security)
- **BLE Communication**: Admin console connects via Bluetooth LE with chunked data transfer for large payloads
- **Circular Display UI**: Custom LVGL-based interface with cyberpunk aesthetic
- **Real-time Status**: Live connection status, mode indicators, and scan progress
- **Multi-channel Support**: Scan specific channels or all channels

### Advanced Features (In Development)
- **Packet Monitor**: Promiscuous mode packet capture and analysis
- **Deauth Attacks**: 802.11 deauthentication frame injection
- **Beacon Spam**: Custom beacon frame generation
- **Evil Portal**: Captive portal deployment
- **PCAP Capture**: Save packet captures to SD card
- **Probe Flood**: Probe request flooding
- **Rickroll**: Fun WiFi pranks

## Hardware

- **Board**: Waveshare ESP32-S3-LCD-1.47
- **Display**: 1.47" Round LCD (172x320) with ST7789 driver
- **MCU**: ESP32-S3 with WiFi and Bluetooth
- **Features**: RGB LED, SD card slot (optional)

## Quick Start

1. **Initialize Project**
   ```bash
   python init_project.py
   ```

2. **Set Up ESP32 Firmware**
   ```bash
   cd mct2032-firmware
   pio run -t upload
   ```

3. **Run Admin Console**
   ```bash
   cd mct2032-admin
   python -m venv venv
   source venv/bin/activate  # or venv\Scripts\activate on Windows
   pip install -r requirements.txt
   python main.py
   ```

## UI Design

### ESP32 Display
- **Background**: Deep dark blue (#0a0e27, #0f0f23)
- **Border**: Purple gradient with glow effect (#8b5cf6)
- **Status Text**: Terminal green (#00ff41)
- **Mode Indicators**: Color-coded (Idle=Green, Scanning=Cyan, Attack=Red, etc.)
- **Connection Status**: Visual indicator (Red=Disconnected, Green=Connected)

### Admin Console
- **Framework**: Python Tkinter with CustomTkinter
- **Theme**: Dark hacker aesthetic matching the device
- **Features**: 
  - Real-time WiFi network display with signal strength indicators
  - Console log with timestamp
  - Tabbed interface for different functions
  - Stop scan functionality

## Technical Details

### Communication Protocol
- **Transport**: Bluetooth LE with custom GATT service
- **Chunking**: Automatic chunking for responses > 512 bytes
- **Format**: JSON-based command/response protocol
- **Commands**: SCAN_WIFI, SCAN_BLE, GET_STATUS, and more

### Security Features
- **Passive Scanning**: Non-intrusive network discovery
- **Channel Hopping**: Monitor multiple channels
- **Packet Analysis**: Detailed frame type statistics
- **Research Mode**: Advanced features for security testing

## Development with Claude Code

This project is configured for Claude Code. See `CLAUDE.md` for detailed instructions.

### Key Points:
- Always update tasks in Dart before starting work
- Use Context7 for latest documentation
- Follow the dark hacker theme with purple gradients
- Test with Playwright
- Log all development activities

## Project Structure
```
mct2032/
├── mct2032-firmware/     # ESP32 firmware (PlatformIO)
│   ├── include/          # Header files
│   ├── src/              # Source files
│   └── platformio.ini    # Build configuration
├── mct2032-admin/        # Python admin console
│   ├── mct2032_admin/    # Package source
│   └── requirements.txt  # Python dependencies
├── web-installer/        # Web-based firmware installer
├── docs/                 # Additional documentation
├── scripts/              # Build and release scripts
├── .claude/              # Claude Code configuration
└── logs/                 # Development logs
```

## Building from Source

### Firmware
```bash
cd mct2032-firmware
pio run                    # Build
pio run -t upload          # Upload to device
pio device monitor         # Serial monitor
```

### Admin Console
```bash
cd mct2032-admin
pip install -r requirements.txt
python main.py
```

## Contributing

1. Check Dart for open tasks
2. Follow the established code style
3. Test all changes thoroughly
4. Update documentation as needed

## Safety and Legal Notice

This tool is designed for legitimate security research and testing on networks you own or have explicit permission to test. Always comply with local laws and regulations.

## License

MIT License - See LICENSE file for details.
