#include "stdafx.h"
#include "..\include\nes_api.h"
#include "nes.h"

std::unique_ptr<Nes> _g_Nes = nullptr;

Nes* Nes_Create(const char* romPath)
{
    // HACK
    // TOOD: fix.
    if (_g_Nes != nullptr)
    {
        return nullptr;
    }
    else
    {
        _g_Nes = Nes::Create(romPath);
        return _g_Nes.get();
    }
}

void Nes_DoFrame(Nes* nes, unsigned char screen[])
{
    if (nes != nullptr)
    {
        nes->DoFrame(screen);
    }
}

IStandardController* Nes_GetStandardController(Nes* nes, unsigned int port)
{
    if (nes != nullptr)
    {
        return nes->GetStandardController(port);
    }
    return nullptr;
}

void Nes_SaveState(Nes* nes)
{
    if (nes != nullptr)
    {
        nes->SaveState();
    }
}

void Nes_LoadState(Nes* nes)
{
    if (nes != nullptr)
    {
        nes->LoadState();
    }
}