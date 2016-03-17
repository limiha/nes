#pragma once

#include "stdafx.h"
#include "input.h"

Input::Input()
    : _nextRead(0)
{
}

Input::~Input()
{
}

u8 Input::loadb(u16 addr)
{
    if (addr == 0x4016)
    {
        int currentRead = _nextRead;
        _nextRead++;
        if (_nextRead == 8)
        {
            _nextRead = 0;
        }
        switch (currentRead)
        {
        case 0: return State.A;
        case 1: return State.B;
        case 2: return State.Select;
        case 3: return State.Start;
        case 4: return State.Up;
        case 5: return State.Down;
        case 6: return State.Left;
        case 7: return State.Right;
        }
    }
    return 0;
}

void Input::storeb(u16 addr, u8 val)
{
    _nextRead = 0;
}