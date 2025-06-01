# MCT2032 Admin Console

## Installation

1. Extract this ZIP file to your desired location
2. Ensure Python 3.8+ is installed
3. Run the appropriate script:
   - Windows: Double-click `run-admin.bat`
   - macOS/Linux: Run `./run-admin.sh` (may need to `chmod +x run-admin.sh` first)

## Manual Installation

If the run script doesn't work:

1. Open a terminal/command prompt
2. Navigate to the extracted folder
3. Run:
   ```
   cd mct2032-admin
   pip install -r requirements.txt
   python main.py
   ```

## Requirements

- Python 3.8 or higher
- Bluetooth LE support on your system
- For Linux: May need to run with sudo for BLE access

## Troubleshooting

- **Bluetooth not found**: Ensure Bluetooth is enabled on your system
- **Permission denied (Linux)**: Try running with sudo
- **Module not found**: Install requirements manually with pip

## Usage

1. Connect your MCT2032 device via USB to flash firmware
2. Once flashed, the device will appear via Bluetooth
3. Use the admin console to control and monitor the device

For more information, visit: https://github.com/webdevtodayjason/MCT2032
