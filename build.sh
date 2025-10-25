#!/usr/bin/env bash

# Exit on any error
set -e

# Directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Build directory
BUILD_DIR="$SCRIPT_DIR/build"

# Log directory
LOG_DIR="$SCRIPT_DIR/logs"
mkdir -p "$LOG_DIR"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Optionally source Qt from an explicit prefix (set PIXLSCAN_QT_PREFIX before running)
QT_PREFIX="${PIXLSCAN_QT_PREFIX:-}"
if [[ -n "$QT_PREFIX" ]]; then
  export PATH="$QT_PREFIX/bin:$PATH"
  export CMAKE_PREFIX_PATH="$QT_PREFIX${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
  export Qt5_DIR="$QT_PREFIX/lib/cmake/Qt5"
  echo "Configured Qt prefix: $QT_PREFIX" | tee -a "$LOG_DIR/build.log"
fi

# Run CMake configuration (use C++20 and generate compile_commands.json)
# Log both stdout and stderr to logs/build.log
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release 2>&1 | tee "$LOG_DIR/build.log"

# Build the project (append to the same log)
cmake --build "$BUILD_DIR" -- -j$(nproc) 2>&1 | tee -a "$LOG_DIR/build.log"

# Optionally run the executable (uncomment the line below)
# "$BUILD_DIR/pixlscan"

echo "Build completed successfully. Executable is at $BUILD_DIR/pixlscan" | tee -a "$LOG_DIR/build.log"
