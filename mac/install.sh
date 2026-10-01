#!/bin/bash
# Installs Warm Drive into your Audio Units folder so GarageBand can find it.
# Run it from Terminal:  type "bash " (with a space), drag this file into the
# Terminal window, then press Return.

set -e

HERE="$(cd "$(dirname "$0")" && pwd)"
NAME="Warm Drive.component"
SRC="$HERE/$NAME"
DEST="$HOME/Library/Audio/Plug-Ins/Components"

if [ ! -d "$SRC" ]; then
    echo "Can't find \"$NAME\" next to this script."
    echo "Keep install.sh and the .component in the same folder and try again."
    exit 1
fi

mkdir -p "$DEST"
rm -rf "$DEST/$NAME"
cp -R "$SRC" "$DEST/"

# Files downloaded from the internet are "quarantined" by macOS, which stops
# GarageBand from loading them. This plugin is your own, so remove that flag.
xattr -dr com.apple.quarantine "$DEST/$NAME" 2>/dev/null || true

# Make macOS re-scan Audio Units.
killall -9 AudioComponentRegistrar 2>/dev/null || true

echo ""
echo "Installed to: $DEST/$NAME"
echo "Now quit GarageBand completely (Cmd+Q) and open it again."
echo "You'll find it under: Plug-ins > Audio Units > Ortiz > Warm Drive"
