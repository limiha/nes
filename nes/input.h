#pragma once

#include "mem.h"

#include <SDL_events.h>

enum class JoypadButton : u8
{
    A = 1 << 0,
    B = 1 << 1,
    Select = 1 << 2,
    Start = 1 << 3,
    Up = 1 << 4,
    Down = 1 << 5,
    Left = 1 << 6,
    Right = 1 << 7
};

enum class InputResult
{
    Continue,
    SaveState,
    LoadState,
    Quit
};

class Joypad
{
public:
    Joypad();
    ~Joypad();

    bool load();
    void store(u8 val);

    void HandleKeyPress(JoypadButton button, bool isDown);
private:
    // Stores the button state
    // From LSB to MSB: A, B, Select, Start, Up, Down, Left, Right
    u8 _state;

    // holds a mask to & with _state to read the controller status
    // shifted left one on each read
    u8 _nextRead;
};

class Input : public IMem
{
public:
    Input();
    ~Input();

    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    // ISaveState
    // don't bother saving the state of input as it will be read again after 
    // the save state is loaded and before the next cpu instruction
    void Save() {}
    void Load() {}

    InputResult CheckInput();

private:
    void HandleKeyPress(SDL_Keycode code, bool isDown);

private:
    Joypad _joypad;
};
