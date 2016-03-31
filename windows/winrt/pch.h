#pragma once

#include <atlbase.h>
#include <collection.h>
#include <ppltasks.h>
#include <xaudio2.h>
using namespace concurrency;


// Nes dependencies
#include "..\..\include\nes_interfaces.h"
#include "..\..\include\object.h"
#include "..\..\include\nptr.h"
#include "..\..\src\types.h"
#include "..\..\src\nes.h"

#define IfFailRet(HR) do { hr = (HR); if (FAILED(hr)) return hr; } while (false)
#define IfNullRet(P) do { if (!(P)) return E_FAIL; } while (false)