// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <math.h>
#include <stdio.h>
#include <tchar.h>

#include <atlbase.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <vector>
#include <chrono>

// for Rom and save state paths
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "types.h"
#include "interfaces.h"
#include "util.h"

// Emulator constants
#define CPU_FREQ_NTSC 1789773
#define CPU_FREQ_PAL 1662607

//#define RENDER_GRID
//#define RENDER_NAMETABLE
//#define RENDER_PATTERNTABLE
