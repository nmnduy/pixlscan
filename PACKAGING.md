# PixlScan Packaging Guide

This document describes how to package PixlScan for distribution on Linux and macOS.

## Prerequisites

### macOS
- Qt5 installed via Homebrew: `brew install qt@5`
- Xcode Command Line Tools
- OpenCV

### Linux
- Qt5 development packages: `sudo apt install qt5-default qtbase5-dev`
- OpenCV development libraries
- Internet connection (for downloading linuxdeploy tools)

## Quick Start

### Package for macOS

```bash
./package-macos.sh
```

This script will:
1. Build the application as a macOS .app bundle
2. Deploy Qt frameworks into the bundle using `macdeployqt`
3. Copy the darkstyle.qss stylesheet
4. Create a distributable DMG installer

**Output**: `build/pixlscan.dmg`

### Package for Linux (AppImage)

```bash
./package-linux.sh
```

This script will:
1. Download linuxdeploy tools (first run only)
2. Build the application
3. Create an AppDir with all dependencies
4. Bundle Qt plugins and libraries
5. Generate a portable AppImage

**Output**: `build/pixlscan-x86_64.AppImage`

## What Gets Packaged

### macOS (.app bundle)
- Main executable: `pixlscan.app/Contents/MacOS/pixlscan`
- Qt frameworks: automatically bundled in `Contents/Frameworks/`
- Resources: `darkstyle.qss` in `Contents/Resources/`
- All dependencies are self-contained in the .app bundle

### Linux (AppImage)
- Main executable and all shared libraries
- Qt plugins (platforms, styles, etc.)
- OpenCV libraries
- Desktop integration file
- Application icon (if provided)
- Single portable file that runs on most Linux distributions

## Distribution

### macOS
Distribute the **DMG file** (`pixlscan.dmg`):
- Users can drag the app to their Applications folder
- The DMG provides a professional installation experience
- Code signing recommended for distribution (see below)

### Linux
Distribute the **AppImage file** (`pixlscan-x86_64.AppImage`):
- Single executable file - no installation needed
- Works on most Linux distributions (Ubuntu, Fedora, Arch, etc.)
- Users just need to: `chmod +x pixlscan-x86_64.AppImage && ./pixlscan-x86_64.AppImage`

## Advanced Configuration

### Adding an Application Icon

For better desktop integration, create a 256x256 PNG icon:

```bash
# Place icon in project root
cp /path/to/icon.png pixlscan.png
```

The packaging scripts will automatically include it if present.

### Code Signing (macOS)

To distribute outside the Mac App Store, sign your app:

```bash
# Sign the .app bundle
codesign --deep --force --verify --verbose \
  --sign "Developer ID Application: Your Name" \
  build/pixlscan.app

# Notarize for macOS Gatekeeper
xcrun notarytool submit build/pixlscan.dmg \
  --apple-id "your@email.com" \
  --team-id "YOUR_TEAM_ID" \
  --password "app-specific-password"
```

### Custom Build Options

Both packaging scripts use `./build.sh` internally. To customize the build:

1. Edit `build.sh` for compile flags
2. Modify `CMakeLists.txt` for dependencies
3. Run the packaging script

## Troubleshooting

### macOS: "macdeployqt not found"

Install Qt5 via Homebrew:
```bash
brew install qt@5
```

### Linux: "AppImage creation failed"

Ensure you have required libraries:
```bash
sudo apt install libfuse2 file
```

### Missing darkstyle.qss

The stylesheet should be automatically copied. If missing:
```bash
# Rebuild to regenerate it
rm -rf build && ./build.sh
```

### OpenCV libraries not found

Ensure OpenCV is properly installed and CMake can find it:
```bash
# macOS
brew install opencv

# Linux
sudo apt install libopencv-dev
```

## File Structure

```
pixlscan/
├── CMakeLists.txt              # Updated with packaging configuration
├── package-macos.sh            # macOS packaging script
├── package-linux.sh            # Linux packaging script
├── pixlscan.desktop            # Linux desktop entry file
├── pixlscan.png                # Application icon (optional)
└── build/
    ├── pixlscan.app            # macOS bundle
    ├── pixlscan.dmg            # macOS installer
    └── pixlscan-x86_64.AppImage # Linux portable binary
```

## Notes

- The first packaging run downloads tools (~10MB) and may take longer
- DMG creation can take 1-2 minutes due to compression
- AppImage includes all dependencies, resulting in larger file sizes (50-100MB typical)
- Both packaging methods create fully portable, self-contained distributions
