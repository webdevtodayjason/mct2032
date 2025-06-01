#!/bin/bash

# Update GitHub username in all project files

if [ -z "$1" ]; then
    echo "Usage: ./update-username.sh YOUR_GITHUB_USERNAME"
    echo "Example: ./update-username.sh jasonbrashear"
    exit 1
fi

NEW_USERNAME=$1
OLD_USERNAME="yourusername"

echo "Updating GitHub username from '$OLD_USERNAME' to '$NEW_USERNAME'..."

# Files to update
FILES=(
    "web-installer/index.html"
    "web-installer/manifest.json"
    ".github/workflows/build-and-release.yml"
    "README.md"
)

# Update each file
for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        # Create backup
        cp "$file" "$file.bak"
        
        # Replace username
        if [[ "$OSTYPE" == "darwin"* ]]; then
            # macOS
            sed -i '' "s/$OLD_USERNAME/$NEW_USERNAME/g" "$file"
        else
            # Linux
            sed -i "s/$OLD_USERNAME/$NEW_USERNAME/g" "$file"
        fi
        
        echo "✅ Updated: $file"
    else
        echo "⚠️  File not found: $file"
    fi
done

echo ""
echo "Username update complete!"
echo ""
echo "Next steps:"
echo "1. Review the changes: git diff"
echo "2. If everything looks good, remove backup files: rm **/*.bak"
echo "3. Commit changes: git add . && git commit -m 'Update GitHub username'"
echo ""
echo "Optional: Update custom domain in .github/workflows/build-and-release.yml"
echo "  Change: cname: mct2032.yourdomain.com"
echo "  To: cname: mct2032.$NEW_USERNAME.io (or remove the line)"