// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// C includes
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>

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
#ifndef DAC_BUILD
#include <mutex>
#endif

// for Rom and save state paths
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "../include/baseinterface.h"
#include "../include/nptr.h"
#include "../include/object.h"
#include "dac.h"
#include "types.h"
#include "util.h"

// Emulator constants
#define CPU_FREQ_NTSC 1789773
#define CPU_FREQ_PAL 1662607