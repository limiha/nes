//#include "nes.h"
//#include "IInput.h"
//#include "sdlGfx.h"
//#include "sdlInput.h"

#include <stdio.h>
#include <memory>

#include <nes_api.h>

#include "sdlGfx.h"
#include "sdlInput.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Must provide path to ROM file.\n");
        return -1;
    }

    INes* nes = Nes_Create(argv[1]);
    if (nes == nullptr)
    {
        return 1;
    }

    IStandardController* controller0 = nes->GetStandardController(0);
    SdlInput input(controller0);

    SdlGfx gfx(3);

    unsigned char screen[256 * 240 * 3];
    for (;;)
    {
        memset(screen, 0, sizeof(screen));

        // TODO: get joypadState
        InputResult result = input.CheckInput();

        if (result == InputResult::SaveState)
        {
            nes->SaveState();
        }
        else if (result == InputResult::LoadState)
        {
            nes->LoadState();
        }
        else if (result == InputResult::Quit)
        {
            break;
        }

        nes->DoFrame(screen);
        gfx.Blit(screen);
    }
}