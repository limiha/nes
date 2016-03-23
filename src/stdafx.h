// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// std C++ includes
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>

// for Rom and save state paths
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "types.h"
#include "interfaces.h"
#include "util.h"

// Emulator constants
#define CPU_FREQ_NTSC 1789773
#define CPU_FREQ_PAL 1662607

// Still required for APU
#define SDL_MAIN_HANDLED
#include <SDL.h>