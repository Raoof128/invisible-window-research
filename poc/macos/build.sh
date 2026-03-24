#!/bin/bash
# build.sh — Compile the Invisible Window macOS PoC
# Author: Mohammad Raouf Abedini

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC="$SCRIPT_DIR/invisible_window.swift"
OUT="$SCRIPT_DIR/invisible_window"

echo "╔══════════════════════════════════════════╗"
echo "║  Building Invisible Window — macOS PoC   ║"
echo "╚══════════════════════════════════════════╝"
echo ""

# Check Swift
if ! command -v swiftc &> /dev/null; then
    echo "[ERROR] swiftc not found. Install Xcode or Command Line Tools."
    exit 1
fi

echo "[BUILD] Source: $SRC"
echo "[BUILD] Output: $OUT"
echo "[BUILD] Swift: $(swift --version 2>&1 | head -1)"
echo ""

# Compile
swiftc "$SRC" \
    -o "$OUT" \
    -framework AppKit \
    -framework Foundation \
    -O \
    -warnings-as-errors 2>/dev/null || \
swiftc "$SRC" \
    -o "$OUT" \
    -framework AppKit \
    -framework Foundation \
    -O

echo ""
echo "[BUILD] Success! Binary: $OUT ($(du -h "$OUT" | cut -f1) bytes)"
echo ""
echo "Run with:"
echo "  $OUT"
echo ""
echo "Or interpret directly:"
echo "  swift $SRC"
echo ""
echo "Verify with:"
echo "  python3 $SCRIPT_DIR/capture_verify.py"
