# Freediver's Dream 3D - PS2 homebrew (PS2SDK + gsKit)
# Build inside the official ps2dev Docker image (see build.sh) or with a local ps2dev install.

GSKIT ?= $(PS2DEV)/gsKit

EE_BIN  = FREEDIVE.ELF
EE_OBJS = main.o
EE_INCS += -I$(GSKIT)/include
EE_LDFLAGS += -L$(GSKIT)/lib
EE_LIBS = -lgskit -ldmakit -lpad -lm
EE_CFLAGS += -O2 -Wall -std=gnu99

all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)
	rm -rf iso_root FreediversDream3D.iso

iso: $(EE_BIN)
	sh make_iso.sh $(EE_BIN) FreediversDream3D.iso

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
