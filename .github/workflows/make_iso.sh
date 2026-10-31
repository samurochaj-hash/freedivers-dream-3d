#!/bin/sh
# Packs FREEDIVE.ELF + SYSTEM.CNF into an ISO9660 image that PCSX2 / OPL can boot.
# Usage: sh make_iso.sh [elf] [output.iso]
set -e
ELF="${1:-FREEDIVE.ELF}"
OUT="${2:-FreediversDream3D.iso}"

[ -f "$ELF" ] || { echo "ELF not found: $ELF (run make first)"; exit 1; }

rm -rf iso_root
mkdir iso_root
cp "$ELF" iso_root/FREEDIVE.ELF
printf 'BOOT2 = cdrom0:\\FREEDIVE.ELF;1\r\nVER = 1.00\r\nVMODE = NTSC\r\n' > iso_root/SYSTEM.CNF

if command -v xorriso >/dev/null 2>&1; then
    xorriso -as mkisofs -iso-level 1 -V FREEDIVERS_DREAM -sysid PLAYSTATION -o "$OUT" iso_root
elif command -v genisoimage >/dev/null 2>&1; then
    genisoimage -iso-level 1 -V FREEDIVERS_DREAM -sysid PLAYSTATION -o "$OUT" iso_root
elif command -v mkisofs >/dev/null 2>&1; then
    mkisofs -iso-level 1 -V FREEDIVERS_DREAM -sysid PLAYSTATION -o "$OUT" iso_root
else
    echo "Need xorriso, genisoimage or mkisofs to build the ISO."; exit 1
fi
echo "Created $OUT"
