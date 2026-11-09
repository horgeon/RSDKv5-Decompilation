#!/bin/bash

APP_NAME="Sonic Mania Plus"
APP_AUTH="SaturnSH2x2, Rubberduckycooly"

mkdir -p out

# The game checks for a DLC title sharing the main application's UniqueId, get it from SonicMania.rsf.
UNIQUE_ID=$(sed -n 's/^[[:space:]]*UniqueId[[:space:]]*:[[:space:]]*\(0x[0-9a-fA-F]*\).*/\1/p' ./SonicMania.rsf | head -n 1)
if [ -z "$UNIQUE_ID" ]; then
    echo "Could not read UniqueId from ./SonicMania.rsf."
    exit 1
fi
echo "Using UniqueId $UNIQUE_ID for DLC title."

# The DLC RomFS is a stub: the game only checks that the DLC title is installed, no content needed.
STAGE_DIR="./out/dlc_romfs_stage"
mkdir -p "$STAGE_DIR/00000000"
printf 'Sonic Mania Plus DLC\n' > "$STAGE_DIR/00000000/dlc.info"

# make SMDH
./bannertool makesmdh -s "$APP_NAME" -l "$APP_NAME" -p "$APP_AUTH" -i ./48x48.png -o out/icon.icn

# build DLC CIA
./makerom -f cia -dlc -o out/SonicMania_dlc.cia -rsf ./SonicMania_dlc.rsf -DAPP_UNIQUE_ID="$UNIQUE_ID" -DAPP_ROMFS="$(cd "$STAGE_DIR" && pwd)" -icon out/icon.icn -DAPP_ENCRYPTED=false -target t

echo "Built DLC .cia"
