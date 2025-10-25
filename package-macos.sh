#!/bin/bash
set -e

echo "========================================="
echo "  PixlScan macOS Package Builder"
echo "========================================="
echo ""

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Find Qt5 installation
if [ -d "/usr/local/opt/qt@5" ]; then
    QT_DIR="/usr/local/opt/qt@5"
elif [ -d "/opt/homebrew/opt/qt@5" ]; then
    QT_DIR="/opt/homebrew/opt/qt@5"
elif command -v qmake &> /dev/null; then
    QT_DIR=$(qmake -query QT_INSTALL_PREFIX)
else
    echo "❌ Error: Qt5 not found. Please install via: brew install qt@5"
    exit 1
fi

MACDEPLOYQT="${QT_DIR}/bin/macdeployqt"

if [ ! -f "$MACDEPLOYQT" ]; then
    echo "❌ Error: macdeployqt not found at $MACDEPLOYQT"
    exit 1
fi

echo -e "${BLUE}Step 1: Building application...${NC}"
./build.sh

cd build

# Check if pixlscan.app exists
if [ ! -d "pixlscan.app" ]; then
    echo "❌ Error: pixlscan.app bundle not found in build directory"
    echo "Make sure CMake built the app as a bundle (MACOSX_BUNDLE property)"
    exit 1
fi

echo -e "\n${BLUE}Step 2: Deploying Qt frameworks...${NC}"
"$MACDEPLOYQT" pixlscan.app \
    -verbose=1 \
    -always-overwrite

# Copy darkstyle.qss if it exists and isn't already in the bundle
if [ -f "darkstyle.qss" ]; then
    echo -e "\n${BLUE}Step 3: Copying darkstyle.qss...${NC}"
    mkdir -p pixlscan.app/Contents/Resources
    cp darkstyle.qss pixlscan.app/Contents/Resources/
fi

echo -e "\n${BLUE}Step 4: Creating DMG installer...${NC}"
"$MACDEPLOYQT" pixlscan.app \
    -dmg \
    -verbose=1

# Check if DMG was created
if [ -f "pixlscan.dmg" ]; then
    echo ""
    echo -e "${GREEN}=========================================${NC}"
    echo -e "${GREEN}✅ Package created successfully!${NC}"
    echo -e "${GREEN}=========================================${NC}"
    echo ""
    echo "Output: $(pwd)/pixlscan.dmg"
    echo ""
    echo "You can now:"
    echo "  • Test the app: open pixlscan.app"
    echo "  • Test the DMG: open pixlscan.dmg"
    echo "  • Distribute: pixlscan.dmg"
else
    echo "❌ Error: DMG creation failed"
    exit 1
fi
