#!/usr/bin/env bash

# Simple script to run the pixlscan executable.
# It will build the project if the binary does not exist.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
EXECUTABLE="$BUILD_DIR/pixlscan"

# Log directory
LOG_DIR="$SCRIPT_DIR/logs"
mkdir -p "$LOG_DIR"

# Ensure we have a fresh executable
[ -f "$EXECUTABLE" ] && rm "$EXECUTABLE"
# Build the project (log output)
"$SCRIPT_DIR/build.sh" 2>&1 | tee "$LOG_DIR/run.log"

# Run the executable (log output)

[ -f "$EXECUTABLE" ] || exit 1
echo "Running pixlscan..." | tee -a "$LOG_DIR/run.log"
"$EXECUTABLE" 2>&1 | tee -a "$LOG_DIR/run.log"
