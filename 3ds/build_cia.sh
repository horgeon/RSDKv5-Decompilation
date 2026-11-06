#!/bin/bash

if [ -z "$1" ]; then
    echo "Missing path to elf file."
    echo "Usage: $0 path/to/file.elf"
    exit 1
fi

APP_NAME="Sonic Mania"
APP_AUTH="SaturnSH2x2, Rubberduckycooly"

mkdir -p out
rm -rf out/*

# make banner
./bannertool makebanner -i ./banner.png -a ./jingle.wav -o out/banner.bnr

# make SMDH
./bannertool makesmdh -s "$APP_NAME" -l "$APP_NAME" -p "$APP_AUTH" -i ./48x48.png -o out/icon.icn

# When ./romfs/Data exists, enable the RSF RomFs section and hand makerom the staging dir: the CIA then embeds the
# game's Data folder as the title RomFS and the game runs standalone (no SD files needed).
RSF_FILE="./SonicMania.rsf"
ROMFS_OPTS=()
if [ -d "./romfs/Data" ]; then
    RSF_FILE="./out/SonicMania_romfs.rsf"
    sed 's/^#RomFs:/RomFs:/; s/^#  RootPath/  RootPath/' ./SonicMania.rsf > "$RSF_FILE"
    ROMFS_OPTS=(-DAPP_ROMFS="$(cd ./romfs && pwd)")
    echo "Embedding RomFS from ./romfs ($(du -sh ./romfs/Data | cut -f1)) ..."
fi

# build CIA
./makerom -f cia -o out/SonicMania.cia -DAPP_ENCRYPTED=false -elf "$1" -rsf "$RSF_FILE" "${ROMFS_OPTS[@]}" -exefslogo -target t -icon out/icon.icn -banner out/banner.bnr

echo "Built .cia"
