#!/bin/bash

# MCT2032 Local Testing Script

echo "🧪 MCT2032 Local Testing Guide"
echo "=============================="
echo ""

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

echo "📁 Project root: $PROJECT_ROOT"
echo ""

echo "1️⃣  To test the web installer locally:"
echo "   cd $PROJECT_ROOT/web-installer"
echo "   python3 -m http.server 8000"
echo "   Then open: http://localhost:8000"
echo ""

echo "2️⃣  To test the admin console:"
echo "   cd $PROJECT_ROOT/mct2032-admin"
echo "   pip install -r requirements.txt"
echo "   python main.py"
echo ""

echo "3️⃣  To flash firmware directly via USB:"
echo "   cd $PROJECT_ROOT/mct2032-firmware"
echo "   pio run -t upload"
echo ""

echo "4️⃣  To monitor serial output:"
echo "   cd $PROJECT_ROOT/mct2032-firmware"
echo "   pio device monitor"
echo ""

echo "📦 Release files are in: $PROJECT_ROOT/releases/"
echo ""
echo "🔧 Firmware binary: $PROJECT_ROOT/releases/mct2032-firmware-v1.0.0.bin"
echo "💻 Admin package: $PROJECT_ROOT/releases/mct2032-admin-v1.0.0-darwin.zip"
echo ""

echo "🚀 To create GitHub release:"
echo "   $PROJECT_ROOT/scripts/create-github-release.sh"