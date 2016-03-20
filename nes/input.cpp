#pragma once

#include "stdafx.h"
#include "input.h"

Input::Input()
    : Port0(nullptr)
    , Port1(nullptr)
{
}

Input::~Input()
{
}

u8 Input::loadb(u16 addr)
{
    if (addr == 0x4016)
    {
        return (Port0 != nullptr) ? Port0->Read() : false;
    }
    else if (addr == 0x4017)
    {
        return (Port1 != nullptr) ? Port1->Read() : false;
    }
    return 0; // can't happen
}

void Input::storeb(u16 addr, u8 val)
{
    if (addr == 0x4016)
    {
        bool strobeVal = (val & 1) == 1;

        if (Port0 != nullptr)
        {
            Port0->Strobe(strobeVal);
        }

        if (Port1 != nullptr)
        {
            Port1->Strobe(strobeVal);
        }
    }
}

// Standard Controller Implementation

StandardController::StandardController()
    : _A(false)
    , _B(false)
    , _Select(false)
    , _Start(false)
    , _Up(false)
    , _Down(false)
    , _Left(false)
    , _Right(false)
    , _strobe(false)
    , _nextReadIndex(0)
{
}

StandardController::~StandardController()
{
}

void StandardController::Strobe(bool strobe)
{
    if (_strobe == true && strobe == false)
    {
        _nextReadIndex = 0;
    }

    _strobe = strobe;
}

u8 StandardController::Read()
{
    if (_strobe)
    {
        return _A;
    }
    else if (_nextReadIndex < 8)
    {
        u8 currentReadIndex = _nextReadIndex;
        _nextReadIndex++;
        switch (currentReadIndex)
        {
        case 0: return _A;
        case 1: return _B;
        case 2: return _Select;
        case 3: return _Start;
        case 4: return _Up;
        case 5: return _Down;
        case 6: return _Left;
        case 7: return _Right;
        }
        return true;
    }
    else
    {
        return true;
    }
}

void StandardController::A(bool state)
{
    _A = state;
}

void StandardController::B(bool state)
{
    _B = state;
}

void StandardController::Select(bool state)
{
    _Select = state;
}

void StandardController::Start(bool state)
{
    _Start = state;
}

void StandardController::Up(bool state)
{
    _Up = state;
}

void StandardController::Down(bool state)
{
    _Down = state;
}

void StandardController::Left(bool state)
{
    _Left = state;
}

void StandardController::Right(bool state)
{
    _Right = state;
}
