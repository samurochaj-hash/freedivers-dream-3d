# Freediver's Dream 3D — PS2 remake scaffold

This is an initial source-project scaffold, not a finished game or ISO.

## Requirements
- PS2SDK installed and configured
- gsKit and dmaKit available in the selected PS2SDK build
- MIPS/EE toolchain provided by PS2SDK

## Build
From this directory, run `make` in a PS2SDK-configured environment.
The Makefile uses the standard PS2SDK sample makefile includes. Depending on
your PS2SDK installation, adjust `PS2SDK` and library flags as needed.

## Current status
- Project folders and module interfaces: created
- Minimal gsKit screen-clear entry point: provided as an integration starting point
- Player, physics, camera, levels, menu and audio: placeholders/interfaces only
- Model skeleton and animation-state scaffold: added; no mesh/keyframe playback yet
- Original-game assets and verified mechanics: not supplied
- ELF compilation, emulator tests and ISO: not performed

## Important
This project does not contain proprietary source code or assets from the original
Freediver's Dream 3D. Add only files you own or are authorized to use.
