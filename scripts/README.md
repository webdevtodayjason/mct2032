# MCT2032 Release Scripts

This directory contains automation scripts for building and releasing MCT2032.

## Scripts

### build-release.sh
Builds the firmware and admin console for release.

**Usage:**
```bash
./build-release.sh [version]
# Example: ./build-release.sh v1.0.1
# Default: v1.0.0
```

**What it does:**
- Cleans previous builds
- Builds ESP32 firmware with PlatformIO
- Creates combined binary with bootloader
- Builds admin console executable for current platform
- Generates release notes
- Creates manifest.json for web installer
- Packages everything into a release archive

**Output:**
- `release-build/` directory with all artifacts

### upload-release.sh
Creates a GitHub release and uploads all artifacts.

**Usage:**
```bash
./upload-release.sh <version>
# Example: ./upload-release.sh v1.0.0
```

**Requirements:**
- GitHub CLI (`gh`) must be installed
- Must be authenticated with `gh auth login`
- Should be run from main branch with all changes committed

**What it does:**
- Creates git tag if needed
- Creates/updates GitHub release
- Uploads firmware and admin console binaries
- Updates gh-pages branch with new manifest
- Publishes the release

### update-manifest-local.sh
Updates the manifest for local testing without creating a release.

**Usage:**
```bash
./update-manifest-local.sh
```

**What it does:**
- Builds firmware if needed
- Copies firmware to web-installer directory
- Updates manifest-local.json
- Allows testing web installer locally

**Testing locally:**
```bash
cd web-installer
python3 -m http.server 8000
# Open http://localhost:8000/index-local.html
```

## Release Process

1. Ensure all changes are committed and pushed
2. Run `./build-release.sh v1.0.0` (replace with your version)
3. Review the generated files in `release-build/`
4. Run `./upload-release.sh v1.0.0`
5. The release will be created and web installer updated automatically

## Notes

- These scripts are in .gitignore and not tracked in the repository
- Always test firmware thoroughly before creating a release
- Use semantic versioning (v1.0.0, v1.0.1, v1.1.0, etc.)
- The web installer will automatically use the latest release