#!/bin/bash

# GitHub Release Creation Helper

echo "📦 MCT2032 GitHub Release Helper"
echo "================================"
echo ""
echo "This script will help you create a GitHub release."
echo "Make sure you have the GitHub CLI (gh) installed."
echo ""

# Check if gh is installed
if ! command -v gh &> /dev/null; then
    echo "❌ GitHub CLI (gh) is not installed."
    echo "Install it with: brew install gh"
    echo "Then authenticate with: gh auth login"
    exit 1
fi

# Get the directory of this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

cd "$PROJECT_ROOT"

# First, let's commit and push the release files
echo "📝 Committing release files..."
git add releases/
git add web-installer/manifest-prod.json
git add scripts/
git commit -m "Add v1.0.0 release files and scripts"

echo ""
echo "🚀 Pushing to GitHub..."
git push origin main

echo ""
echo "🏷️  Creating GitHub release..."
gh release create v1.0.0 \
    --title "MCT2032 v1.0.0 - Initial Release" \
    --notes-file releases/RELEASE-NOTES.md \
    releases/mct2032-firmware-v1.0.0.bin \
    releases/mct2032-admin-v1.0.0-darwin.zip \
    releases/README-ADMIN.md

echo ""
echo "✅ Release created!"
echo ""
echo "🌐 Next steps:"
echo "1. Visit https://github.com/webdevtodayjason/MCT2032/releases"
echo "2. Verify the release looks correct"
echo "3. Update the web installer index.html to use manifest-prod.json for production"
echo "4. Test the web installer at https://webdevtodayjason.github.io/MCT2032/"