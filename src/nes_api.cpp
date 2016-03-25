#include "stdafx.h"
#include "..\include\nes_api.h"
#include "nes.h"

static Nes* _g_Nes = nullptr;

INes* Nes_Create(const char* romPath, IAudioProvider* audioProvider)
{
    // HACK
    // TOOD: fix.
    if (_g_Nes != nullptr)
    {
        return nullptr;
    }
    else
    {
        Nes::Create(romPath, audioProvider, &_g_Nes);
        return _g_Nes;
    }
}

void Nes_Destroy(INes* nes)
{
    _g_Nes->Release();
}
