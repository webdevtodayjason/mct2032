# MCT2032 - Mega Cyber Tool 2032

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
   source venv/bin/activate  # or venv\Scripts\activate on Windows
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
