#pragma once

#include "nes_interfaces.h"
#include "object.h"
#include "nptr.h"

#if defined(_WIN32)
  #if defined(NESDLL_EXPORTS)
    #define NES_API __declspec(dllexport)
  #else
    #define NES_API __declspec(dllimport)
  #endif
#else
  #define NES_API
#endif

extern "C"
{
    NES_API bool Nes_Create(const char* romPath, IAudioProvider* audioProvider, INes** ines);
}