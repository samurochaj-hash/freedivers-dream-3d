#!/bin/sh
# One-command build using the official ps2dev toolchain image (needs Docker).
# Output: FREEDIVE.ELF and FreediversDream3D.iso in this folder.
set -e
docker run --rm -v "$PWD":/src -w /src ps2dev/ps2dev:latest sh -c \
  "apk add --no-cache xorriso make >/dev/null && make clean && make && sh make_iso.sh FREEDIVE.ELF FreediversDream3D.iso"
ls -l FREEDIVE.ELF FreediversDream3D.iso
