# GitHub Deployment Guide for MCT2032

This guide will walk you through deploying MCT2032 to GitHub with automated builds and web installer.

## 📋 Prerequisites

- GitHub account
- Git installed on your computer
- PlatformIO Core CLI (for local testing)

## 🚀 Step-by-Step Deployment

### Step 1: Create GitHub Repository

1. Go to https://github.com/new
2. Repository name: `mct2032`
3. Description: "Portable cybersecurity toolkit for ESP32-S3"
4. Set as **Public** (required for GitHub Pages)
5. **DON'T** initialize with README (we already have one)
6. Click "Create repository"

### Step 2: Update Files with Your GitHub Username

Replace `yourusername` with your actual GitHub username in these files:

```bash
# In terminal, run these commands:
cd /Users/jasonbrashear/code/MCT2032

# Replace in all files (on macOS)
find . -type f -name "*.html" -o -name "*.json" -o -name "*.yml" -o -name "*.md" | xargs sed -i '' 's/yourusername/YOUR_ACTUAL_USERNAME/g'

# Or manually edit these files:
# - web-installer/index.html (multiple places)
# - web-installer/manifest.json
# - .github/workflows/build-and-release.yml
```

### Step 3: Initialize Git and Push Code

```bash
# Initialize git repository
git init

# Add all files
git add .

# Create initial commit
git commit -m "Initial commit: MCT2032 cybersecurity toolkit"

# Add GitHub remote (replace YOUR_USERNAME)
git remote add origin https://github.com/YOUR_USERNAME/mct2032.git

# Push to main branch
git branch -M main
git push -u origin main
```

### Step 4: Configure GitHub Repository Settings

1. Go to your repository: `https://github.com/YOUR_USERNAME/mct2032`
2. Click "Settings" tab
3. Scroll down to "Pages" section
4. Under "Source", select "Deploy from a branch"
5. Select "main" branch and "/ (root)" folder
6. Click "Save"

### Step 5: Enable GitHub Actions

1. Go to "Actions" tab in your repository
2. You should see "Build and Release MCT2032" workflow
3. Click "Enable workflows" if prompted

### Step 6: Create Your First Release (Manual Method)

Since we need a compiled firmware file first:

#### Option A: Build Locally and Upload

```bash
# Build firmware locally
cd mct2032-firmware
pio run

# The firmware will be at:
# .pio/build/esp32-s3/firmware.bin
```

1. Go to `https://github.com/YOUR_USERNAME/mct2032/releases`
2. Click "Create a new release"
3. Click "Choose a tag" and type `v1.0.0`
4. Release title: "MCT2032 v1.0.0 - Initial Release"
5. Upload the `firmware.bin` file
6. Rename it to `mct2032_firmware_v1.0.0.bin` before uploading
7. Click "Publish release"

#### Option B: Trigger Automatic Build

```bash
# Create and push a tag to trigger the workflow
git tag v1.0.0
git push origin v1.0.0
```

This will trigger the GitHub Actions workflow to:
- Build the firmware
- Create executables for admin console
- Create a release automatically

### Step 7: Update Web Installer Manifest

After creating the release:

1. Go to your release page
2. Right-click on `mct2032_firmware_v1.0.0.bin` and copy the URL
3. Edit `web-installer/manifest.json`:

```json
{
  "name": "MCT2032 - Mini Cyber Tool 2032",
  "version": "1.0.0",
  "home_assistant_domain": "mct2032",
  "funding_url": "https://github.com/YOUR_USERNAME/mct2032",
  "new_install_prompt_erase": false,
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "parts": [
        {
          "path": "PASTE_THE_FIRMWARE_URL_HERE",
          "offset": 0
        }
      ]
    }
  ]
}
```

4. Commit and push the change:

```bash
git add web-installer/manifest.json
git commit -m "Update manifest with firmware URL"
git push
```

### Step 8: Deploy Web Installer to GitHub Pages

The workflow should automatically deploy, but if not:

1. Go to Actions tab
2. Find "Deploy to GitHub Pages" workflow
3. Click "Run workflow" if available

Your web installer will be available at:
- `https://YOUR_USERNAME.github.io/mct2032`

### Step 9: Test the Installation

1. Connect your ESP32-S3 device via USB
2. Open Chrome or Edge browser
3. Visit `https://YOUR_USERNAME.github.io/mct2032`
4. Click "Install Firmware"
5. Select the serial port when prompted
6. Wait for installation to complete

## 📁 Repository Structure Explanation

```
mct2032/
├── .github/
│   └── workflows/
│       └── build-and-release.yml    # Automated builds
├── mct2032-firmware/                 # ESP32 source code
├── mct2032-admin/                    # Python admin console
├── web-installer/                    # GitHub Pages site
│   ├── index.html                    # Landing page
│   ├── manifest.json                 # Firmware metadata
│   └── css/js/assets/                # Web resources
└── README.md                         # Project documentation
```

## 🔄 Workflow Explanation

The GitHub Actions workflow (`build-and-release.yml`) does:

1. **On every push to main:**
   - Builds firmware
   - Runs tests
   - Updates web installer

2. **On creating a tag (v*):**
   - Builds firmware
   - Creates admin console executables
   - Creates GitHub release
   - Uploads all artifacts

3. **Automatic deployment:**
   - Deploys web installer to GitHub Pages
   - Updates manifest version

## 🐛 Troubleshooting

### PlatformIO not found in Actions
- The workflow installs it automatically
- Check the Actions log for errors

### GitHub Pages not working
1. Check Settings > Pages
2. Ensure source is set correctly
3. Wait 5-10 minutes for deployment
4. Check `https://YOUR_USERNAME.github.io/mct2032`

### Web Serial not working
- Must use HTTPS (GitHub Pages provides this)
- Only works in Chrome/Edge
- User must approve serial port access

### Firmware won't flash
1. Check USB connection
2. Try different USB cable
3. Hold BOOT button while connecting
4. Check serial port permissions

## 🎯 Next Steps

1. **Custom Domain (Optional)**
   - Add CNAME file to web-installer/
   - Configure DNS settings

2. **Automated Testing**
   - Add unit tests to firmware
   - Add Python tests for admin console

3. **Documentation**
   - Create Wiki pages
   - Add code documentation
   - Create video tutorials

## 📝 Updating the Project

To release a new version:

```bash
# Make your changes
git add .
git commit -m "Description of changes"
git push

# Create new release
git tag v1.1.0
git push origin v1.1.0
```

The workflow will automatically:
- Build new firmware
- Create release
- Update web installer

## 🔗 Important URLs

After deployment, you'll have:
- Repository: `https://github.com/YOUR_USERNAME/mct2032`
- Web Installer: `https://YOUR_USERNAME.github.io/mct2032`
- Releases: `https://github.com/YOUR_USERNAME/mct2032/releases`
- Actions: `https://github.com/YOUR_USERNAME/mct2032/actions`

## ✅ Deployment Checklist

- [ ] Created GitHub repository
- [ ] Updated username in all files
- [ ] Pushed code to GitHub
- [ ] Enabled GitHub Pages
- [ ] Created first release
- [ ] Updated manifest.json with firmware URL
- [ ] Tested web installer
- [ ] Verified firmware flashing works

Congratulations! Your MCT2032 project is now live on GitHub with automated builds and web-based installation!