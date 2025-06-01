#!/usr/bin/env python3
"""
Package the MCT2032 Admin Console for distribution
"""

import os
import sys
import shutil
import zipfile
import platform
from pathlib import Path

def create_package():
    # Get project root
    project_root = Path(__file__).parent.parent
    admin_dir = project_root / "mct2032-admin"
    output_dir = project_root / "releases"
    
    # Create output directory
    output_dir.mkdir(exist_ok=True)
    
    # Get version from CLAUDE.md or default
    version = "v1.0.0"
    
    # Create package name
    system = platform.system().lower()
    package_name = f"mct2032-admin-{version}-{system}.zip"
    package_path = output_dir / package_name
    
    print(f"Creating admin console package: {package_name}")
    
    # Create zip file
    with zipfile.ZipFile(package_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
        # Add all Python files
        for root, dirs, files in os.walk(admin_dir):
            # Skip __pycache__ and other unwanted directories
            dirs[:] = [d for d in dirs if d not in ['__pycache__', '.pytest_cache', 'venv']]
            
            for file in files:
                if file.endswith(('.py', '.txt', '.md', '.json')):
                    file_path = Path(root) / file
                    arcname = file_path.relative_to(admin_dir.parent)
                    zipf.write(file_path, arcname)
        
        # Add assets
        assets_dir = admin_dir / "assets"
        if assets_dir.exists():
            for file in assets_dir.iterdir():
                if file.is_file():
                    arcname = f"mct2032-admin/assets/{file.name}"
                    zipf.write(file, arcname)
        
        # Create run script for the platform
        if system == "windows":
            run_script = """@echo off
echo Starting MCT2032 Admin Console...
cd mct2032-admin
python -m pip install -r requirements.txt --quiet
python main.py
pause
"""
            zipf.writestr("run-admin.bat", run_script)
        else:
            run_script = """#!/bin/bash
echo "Starting MCT2032 Admin Console..."
cd mct2032-admin
python3 -m pip install -r requirements.txt --quiet
python3 main.py
"""
            zipf.writestr("run-admin.sh", run_script)
    
    print(f"Admin console package created: {package_path}")
    return package_path

def create_readme():
    """Create README for the release"""
    readme_content = """# MCT2032 Admin Console

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
"""
    
    project_root = Path(__file__).parent.parent
    readme_path = project_root / "releases" / "README-ADMIN.md"
    readme_path.write_text(readme_content)
    print(f"README created: {readme_path}")

if __name__ == "__main__":
    create_package()
    create_readme()