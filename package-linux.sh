#!/bin/bash
set -e

echo "========================================="
echo "  PixlScan Linux AppImage Builder"
echo "========================================="
echo ""

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if we're on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo -e "${YELLOW}⚠️  Warning: This script is designed for Linux${NC}"
    echo "Current OS: $OSTYPE"
    echo ""
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Tools directory
TOOLS_DIR="$(pwd)/packaging-tools"
mkdir -p "$TOOLS_DIR"

# Download linuxdeploy if not present
LINUXDEPLOY="$TOOLS_DIR/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT="$TOOLS_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"

if [ ! -f "$LINUXDEPLOY" ]; then
    echo -e "${BLUE}Downloading linuxdeploy...${NC}"
    wget -O "$LINUXDEPLOY" \
        "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    chmod +x "$LINUXDEPLOY"
fi

if [ ! -f "$LINUXDEPLOY_QT" ]; then
    echo -e "${BLUE}Downloading linuxdeploy-plugin-qt...${NC}"
    wget -O "$LINUXDEPLOY_QT" \
        "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
    chmod +x "$LINUXDEPLOY_QT"
fi

echo -e "\n${BLUE}Step 1: Building application...${NC}"
./build.sh

cd build

# Check if executable exists
if [ ! -f "pixlscan" ]; then
    echo "❌ Error: pixlscan executable not found in build directory"
    exit 1
fi

# Clean previous AppDir if exists
rm -rf AppDir

echo -e "\n${BLUE}Step 2: Creating AppDir structure...${NC}"

# Set up environment for linuxdeploy
export OUTPUT="pixlscan-x86_64.AppImage"
export QMAKE=$(which qmake || which qmake-qt5)

# Use absolute paths for plugins
export PATH="$TOOLS_DIR:$PATH"

# Create icon if it doesn't exist (create a simple placeholder)
ICON_FILE="../pixlscan.png"
if [ ! -f "$ICON_FILE" ]; then
    echo -e "${YELLOW}⚠️  Warning: pixlscan.png not found, AppImage will have no icon${NC}"
    echo "   Consider creating an icon at: $ICON_FILE"
fi

echo -e "\n${BLUE}Step 3: Bundling dependencies with linuxdeploy...${NC}"

# Run linuxdeploy with Qt plugin
LINUXDEPLOY_ARGS=(
    --appdir AppDir
    --executable pixlscan
    --plugin qt
)

# Add desktop file if it exists
if [ -f "../pixlscan.desktop" ]; then
    LINUXDEPLOY_ARGS+=(--desktop-file ../pixlscan.desktop)
fi

# Add icon if it exists
if [ -f "$ICON_FILE" ]; then
    LINUXDEPLOY_ARGS+=(--icon-file "$ICON_FILE")
fi

# Add output AppImage
LINUXDEPLOY_ARGS+=(--output appimage)

"$LINUXDEPLOY" "${LINUXDEPLOY_ARGS[@]}"

# Copy darkstyle.qss into AppDir
if [ -f "darkstyle.qss" ]; then
    echo -e "\n${BLUE}Step 4: Copying darkstyle.qss...${NC}"
    mkdir -p AppDir/usr/share/pixlscan
    cp darkstyle.qss AppDir/usr/share/pixlscan/
fi

# Check if AppImage was created
if [ -f "$OUTPUT" ]; then
    echo ""
    echo -e "${GREEN}=========================================${NC}"
    echo -e "${GREEN}✅ Package created successfully!${NC}"
    echo -e "${GREEN}=========================================${NC}"
    echo ""
    echo "Output: $(pwd)/$OUTPUT"
    echo ""
    echo "You can now:"
    echo "  • Test the AppImage: ./$OUTPUT"
    echo "  • Make it executable: chmod +x $OUTPUT"
    echo "  • Distribute: $OUTPUT (single portable file)"
    echo ""

    # Make it executable
    chmod +x "$OUTPUT"

    echo "File size: $(du -h "$OUTPUT" | cut -f1)"
else
    echo "❌ Error: AppImage creation failed"
    exit 1
fi
