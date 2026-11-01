EE_BIN = FREEDIVER.ELF
EE_OBJS = src/main.o src/player.o src/swimming.o src/physics.o src/camera.o src/levels.o src/menu.o src/audio.o
EE_LIBS = -lgsKit -ldmaKit -lpatches

all: $(EE_BIN)

clean:
	 rm -f $(EE_OBJS) $(EE_BIN)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
