#!/bin/bash

# MCT2032 Release Preparation Script

echo "🚀 Preparing MCT2032 Release..."

# Get the directory of this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

# Create releases directory
mkdir -p "$PROJECT_ROOT/releases"

# Check if firmware binary exists
FIRMWARE_BIN="$PROJECT_ROOT/mct2032-firmware/.pio/build/esp32-s3/firmware.bin"
if [ ! -f "$FIRMWARE_BIN" ]; then
    echo "❌ Firmware binary not found at: $FIRMWARE_BIN"
    echo "Please build the firmware first with: cd mct2032-firmware && pio run"
    exit 1
fi

# Copy firmware to releases
cp "$FIRMWARE_BIN" "$PROJECT_ROOT/releases/mct2032-firmware-v1.0.0.bin"
echo "✅ Firmware copied to releases/"

# Package admin console
echo "📦 Packaging admin console..."
cd "$PROJECT_ROOT"
python3 scripts/package-admin.py

# Create release notes
cat > "$PROJECT_ROOT/releases/RELEASE-NOTES.md" << EOF
# MCT2032 Release v1.0.0

## 🎉 Initial Release

This is the first release of the MCT2032 Mega Cyber Tool 2032!

### Features

- **ESP32-S3 Firmware**
  - BLE connectivity for admin console
  - WiFi network scanning
  - Circular OLED display with dark theme
  - Command processing system
  
- **Admin Console**
  - Dark hacker-themed GUI
  - Real-time device monitoring
  - WiFi scan results display
  - Device status monitoring

### Installation

1. **Flash Firmware**: Use the web installer at https://webdevtodayjason.github.io/MCT2032/
2. **Run Admin Console**: Download and extract the admin package for your OS

### Hardware

- Waveshare ESP32-S3-LCD-1.47 development board
- 1.47" circular LCD display (172x320)
- USB-C connectivity

### Known Issues

- Custom fonts are disabled (using default LVGL fonts)
- Packet monitoring not yet implemented
- SD card support pending

### Next Release

- Phase 2: Packet capture and analysis
- Phase 3: Deauth detection
- Phase 4: Advanced features

EOF

echo "✅ Release notes created"

# Create a release manifest
cat > "$PROJECT_ROOT/releases/manifest.json" << EOF
{
  "version": "1.0.0",
  "date": "$(date -u +"%Y-%m-%d")",
  "firmware": {
    "file": "mct2032-firmware-v1.0.0.bin",
    "sha256": "$(shasum -a 256 "$PROJECT_ROOT/releases/mct2032-firmware-v1.0.0.bin" | cut -d' ' -f1)",
    "size": $(stat -f%z "$PROJECT_ROOT/releases/mct2032-firmware-v1.0.0.bin" 2>/dev/null || stat -c%s "$PROJECT_ROOT/releases/mct2032-firmware-v1.0.0.bin" 2>/dev/null || echo "0")
  },
  "admin": {
    "windows": "mct2032-admin-v1.0.0-windows.zip",
    "darwin": "mct2032-admin-v1.0.0-darwin.zip",
    "linux": "mct2032-admin-v1.0.0-linux.zip"
  },
  "changelog": "Initial release with basic WiFi scanning and BLE connectivity"
}
EOF

echo "✅ Release manifest created"

echo ""
echo "📋 Release contents:"
ls -la "$PROJECT_ROOT/releases/"

echo ""
echo "🎯 Next steps:"
echo "1. Commit these files to git"
echo "2. Push to GitHub"
echo "3. Create a GitHub release with these files"
echo "4. Update web installer to use the release URL"