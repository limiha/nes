#pragma once

#include "stdafx.h"
#include "input.h"

Joypad::Joypad()
    : _state(0x00)
    , _nextRead(1)
{
}

Joypad::~Joypad()
{
}

bool Joypad::load()
{
    bool state = (_state & _nextRead) != 0;
    _nextRead = (_nextRead << 1);
    if (_nextRead == 0) _nextRead = 1;
    return state;
}

void Joypad::store(u8 val)
{
    _nextRead = 1;
}

void Joypad::HandleKeyPress(JoypadButton button, bool isDown)
{
    if (isDown)
    {
        _state |= (u8)button;
    }
    else
    {
        _state &= ~((u8)button);
    }
}

Input::Input()
{
}

Input::~Input()
{
}

u8 Input::loadb(u16 addr)
{
    if (addr == 0x4016)
    {
        return (u8)_joypad.load();
    }
    return 0;
}

void Input::storeb(u16 addr, u8 val)
{
    if (addr == 0x4016)
    {
        _joypad.store(val);
    }
}

void Input::HandleKeyPress(SDL_Keycode code, bool isDown)
{
    switch (code)
    {
    case SDLK_LALT:
        _joypad.HandleKeyPress(JoypadButton::A, isDown);
        break;
    case SDLK_LCTRL:
        _joypad.HandleKeyPress(JoypadButton::B, isDown);
        break;
    case SDLK_RSHIFT:
    case SDLK_LSHIFT:
        _joypad.HandleKeyPress(JoypadButton::Select, isDown);
        break;
    case SDLK_RETURN:
        _joypad.HandleKeyPress(JoypadButton::Start, isDown);
        break;
    case SDLK_UP:
        _joypad.HandleKeyPress(JoypadButton::Up, isDown);
        break;
    case SDLK_DOWN:
        _joypad.HandleKeyPress(JoypadButton::Down, isDown);
        break;
    case SDLK_LEFT:
        _joypad.HandleKeyPress(JoypadButton::Left, isDown);
        break;
    case SDLK_RIGHT:
        _joypad.HandleKeyPress(JoypadButton::Right, isDown);
        break;
    }
}

void Input::CheckInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN)
        {
            HandleKeyPress(event.key.keysym.sym, true);
        }
        else if (event.type == SDL_KEYUP)
        {
            HandleKeyPress(event.key.keysym.sym, false);
        }
        else if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                // FIXME: This should be cleaner
                exit(0);
            }
        }
    }
}
