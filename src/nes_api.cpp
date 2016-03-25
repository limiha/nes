#include "stdafx.h"
#include "..\include\nes_api.h"
#include "nes.h"

bool Nes_Create(const char* romPath, IAudioProvider* audioProvider, INes** ines)
{
    NPtr<Nes> nes;
    if (Nes::Create(romPath, audioProvider, &nes))
    {
        *ines = static_cast<INes*>(nes.Detach());
        return true;
    }
    else
    {
        return false;
    }
}
