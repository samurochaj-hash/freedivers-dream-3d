#include <tamtypes.h>
#include <kernel.h>
#include <gsKit.h>
#include <dmaKit.h>
#include "game.h"

static GSGLOBAL *gsGlobal;

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_UNSPEC, D_CTRL_RCYC_8, 1);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    gsGlobal = gsKit_init_global();
    if (!gsGlobal) {
        return 1;
    }

    gsGlobal->Mode = GS_MODE_NTSC;
    gsGlobal->Width = 640;
    gsGlobal->Height = 448;
    gsKit_init_screen(gsGlobal);

    Diver player;
    Player_Init(&player);
    Camera_Init();
    Levels_Init();
    Menu_Init();
    Audio_Init();

    while (1) {
        // Placeholder update loop: controls and real rendering are not integrated yet.
        Player_Update(&player, 0.0f, 0.0f, 1.0f / 30.0f);
        Physics_Update(&player, 1.0f / 30.0f);
        Camera_Update(&player);
        Levels_Update();
        Menu_Update();
        Audio_Update();

        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0, 40, 90, 0x80, 0));
        gsKit_sync_flip(gsGlobal);
    }

    return 0;
}
