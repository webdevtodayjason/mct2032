#!/usr/bin/env python3
"""
MCT2032 Project Initialization Script
Sets up the complete development environment
"""

import os
import json
import shutil
import subprocess
import sys
from pathlib import Path
from datetime import datetime

def create_directory_structure():
    """Create the project directory structure."""
    directories = [
        "mct2032-firmware/src",
        "mct2032-firmware/include",
        "mct2032-firmware/lib",
        "mct2032-firmware/test",
        "mct2032-admin/mct2032_admin",
        "mct2032-admin/tests",
        "mct2032-admin/assets",
        "docs",
        "logs",
        ".claude",
        "exports",
        "tools"
    ]
    
    for directory in directories:
        Path(directory).mkdir(parents=True, exist_ok=True)
        print(f"✅ Created: {directory}")

def create_claude_files():
    """Create Claude configuration files."""
    claude_files = {
        "CLAUDE.md": """# MCT2032 - Mini Cyber Tool 2032
## Claude Code Project Instructions

[Content from the CLAUDE.md artifact above]
""",
        ".claude/code_style.md": """# MCT2032 Code Style Guide
[Content from code_style.md above]
""",
        ".claude/ui_components.md": """# UI Component Library
[Content from ui_components.md above]
""",
        ".claude/testing_guide.md": """# Testing Guide
[Content from testing_guide.md above]
""",
        ".claude/mcp_workflows.md": """# MCP Tool Workflows
[Content from mcp_workflows.md above]
"""
    }
    
    # Create theme config JSON
    theme_config = {
        "name": "MCT2032 Hacker Theme",
        "colors": {
            "background": {
                "primary": "#0a0e27",
                "secondary": "#050816",
                "tertiary": "#0f172a"
            },
            "foreground": {
                "primary": "#f8fafc",
                "secondary": "#cbd5e1",
                "muted": "#64748b"
            },
            "purple": {
                "50": "#faf5ff",
                "100": "#f3e8ff",
                "200": "#e9d5ff",
                "300": "#d8b4fe",
                "400": "#c084fc",
                "500": "#a855f7",
                "600": "#9333ea",
                "700": "#7c3aed",
                "800": "#6b21a8",
                "900": "#581c87"
            }
        }
    }
    
    # Write Claude files
    for filename, content in claude_files.items():
        with open(filename, 'w') as f:
            f.write(content)
        print(f"✅ Created: {filename}")
    
    # Write theme config
    with open('.claude/theme_config.json', 'w') as f:
        json.dump(theme_config, f, indent=2)
    print("✅ Created: .claude/theme_config.json")

def create_platformio_config():
    """Create PlatformIO configuration for ESP32."""
    platformio_ini = """[env:esp32-s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
upload_speed = 921600

; Dependencies
lib_deps = 
    lvgl/lvgl@^8.3.0
    moononournation/GFX Library for Arduino@^1.3.8
    bblanchon/ArduinoJson@^6.21.3
    h2zero/NimBLE-Arduino@^1.4.1

; Build flags
build_flags = 
    -D LV_CONF_INCLUDE_SIMPLE
    -D LV_TICK_CUSTOM=1
    -D CONFIG_BT_NIMBLE_ENABLED
    -D CORE_DEBUG_LEVEL=3

; Custom board settings for Waveshare
board_build.partitions = huge_app.csv
board_build.flash_mode = qio
"""
    
    with open('mct2032-firmware/platformio.ini', 'w') as f:
        f.write(platformio_ini)
    print("✅ Created: mct2032-firmware/platformio.ini")

def create_python_requirements():
    """Create Python requirements file."""
    requirements = """# MCT2032 Admin Console Requirements
bleak>=0.21.0
tkinter
asyncio
aiofiles>=23.2.1
pandas>=2.0.0
matplotlib>=3.7.0
reportlab>=4.0.0
pytest>=7.4.0
pytest-asyncio>=0.21.0
playwright>=1.40.0
pytest-playwright>=0.4.0
customtkinter>=5.2.0
Pillow>=10.0.0
pyinstaller>=6.0.0
"""
    
    with open('mct2032-admin/requirements.txt', 'w') as f:
        f.write(requirements)
    print("✅ Created: mct2032-admin/requirements.txt")

def create_initial_log():
    """Create initial development log."""
    log_content = f"""# MCT2032 Development Log

## Project Initialized: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

### Initial Setup
- Created project directory structure
- Set up Claude Code configuration
- Initialized PlatformIO project
- Created Python virtual environment

### Next Steps
1. Start with Phase 1.1: ESP32 Project Setup & Display Initialization
2. Update task status in Dart to "Doing"
3. Implement basic LVGL display test
4. Test hardware connections

### Environment
- OS: {sys.platform}
- Python: {sys.version.split()[0]}
- Working Directory: {os.getcwd()}

---

## Development Notes

### Hardware Pins (Waveshare ESP32-S3-LCD-1.47)
- Display DC: GPIO 32
- Display CS: GPIO 14
- Display SCK: GPIO 18
- Display MOSI: GPIO 23
- Display RST: GPIO 33
- IMU I2C SDA: GPIO 21
- IMU I2C SCL: GPIO 22

### BLE UUIDs
- Service: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
- Command: beb5483e-36e1-4688-b7f5-ea07361b26a8
- Data: 1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e
- Status: 8d7e5d2e-bf3d-413a-d8f7-e3f95c9c3319

---
"""
    
    with open('logs/dev_log.md', 'w') as f:
        f.write(log_content)
    print("✅ Created: logs/dev_log.md")

def create_gitignore():
    """Create .gitignore file."""
    gitignore = """# Python
*.pyc
__pycache__/
venv/
*.egg-info/
dist/
build/
.pytest_cache/

# ESP32/PlatformIO
.pio/
.vscode/
*.bin
*.elf

# IDE
.idea/
*.swp
*.swo
.DS_Store

# Project specific
exports/
logs/*.log
*.pcap
node_modules/
.env
secrets.json

# Claude
CLAUDE.local.md
"""
    
    with open('.gitignore', 'w') as f:
        f.write(gitignore)
    print("✅ Created: .gitignore")

def create_readme():
    """Create project README."""
    readme = """# MCT2032 - Mini Cyber Tool 2032

A portable cybersecurity toolkit built on ESP32-S3 with circular display.

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
   source venv/bin/activate  # or venv\\Scripts\\activate on Windows
   pip install -r requirements.txt
   python main.py
   ```

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
├── mct2032-firmware/     # ESP32 firmware
├── mct2032-admin/        # Python admin console  
├── docs/                 # Documentation
├── .claude/              # Claude Code config
└── logs/                 # Development logs
```

## License

MIT License - See LICENSE file for details.
"""
    
    with open('README.md', 'w') as f:
        f.write(readme)
    print("✅ Created: README.md")

def main():
    """Main initialization function."""
    print("🚀 MCT2032 Project Initialization")
    print("=" * 50)
    
    # Create all project files and directories
    create_directory_structure()
    create_claude_files()
    create_platformio_config()
    create_python_requirements()
    create_initial_log()
    create_gitignore()
    create_readme()
    
    print("\n" + "=" * 50)
    print("✅ Project initialization complete!")
    print("\n📋 Next Steps:")
    print("1. Open Dart and update Phase 1.1 task to 'Doing'")
    print("2. Run 'cd mct2032-firmware && pio init' to complete PlatformIO setup")
    print("3. Create Python virtual environment in mct2032-admin")
    print("4. Start implementing Phase 1.1: ESP32 Display Setup")
    print("\n💡 Remember to follow CLAUDE.md for all development!")

if __name__ == "__main__":
    main()
