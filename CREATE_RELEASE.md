# Quick Release Guide

## Step 1: Create Initial Release on GitHub

1. Go to: https://github.com/webdevtodayjason/mct2032/releases/new

2. Fill in:
   - **Tag**: `v1.0.0`
   - **Title**: `MCT2032 v1.0.0 - Initial Release`
   - **Description**:
     ```
     ## MCT2032 - Mini Cyber Tool 2032
     
     Initial release of the portable cybersecurity toolkit.
     
     ### Features
     - WiFi network scanning
     - BLE device detection  
     - Real-time packet monitoring
     - Dark-themed admin console
     - Web-based installer
     
     ### Installation
     1. Flash firmware via https://webdevtodayjason.github.io/mct2032
     2. Download admin console for your OS below
     3. Connect via Bluetooth
     
     ### Known Issues
     - Initial firmware compilation in progress
     - Admin console binaries coming soon
     
     **Note**: This is a pre-release while we set up automated builds.
     ```

3. Check "This is a pre-release"

4. Click "Publish release"

## Step 2: Create Simple Test Firmware

While PlatformIO downloads, let's create a minimal working firmware: