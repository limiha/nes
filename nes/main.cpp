#include "stdafx.h"
#include "nes.h"
#include "IInput.h"
#include "sdlGfx.h"
#include "sdlInput.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Must provide path to ROM file.");
        return -1;
    }

    std::unique_ptr<Nes> nes = Nes::Create(argv[1]);
    IStandardController* controller0 = nes->GetStandardController(0);

    SdlGfx gfx(3);
    SdlInput input(controller0);

    if (nes != nullptr)
        nes->Run(&gfx, &input);
}