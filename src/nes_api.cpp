#include "stdafx.h"
#include "..\include\nes_api.h"
#include "nes.h"

std::unique_ptr<Nes> _g_Nes = nullptr;

INes* Nes_Create(const char* romPath)
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
