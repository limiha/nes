#include "stdafx.h"
#include "input.h"

Input::Input()
    : _port0(std::make_unique<EmptyPort>())
    , _port1(std::make_unique<EmptyPort>())
{
}

Input::~Input()
{
}

u8 Input::loadb(u16 addr)
{
    if (addr == 0x4016)
    {
        return (_port0 != nullptr) ? _port0->Read() : false;
    }
    else if (addr == 0x4017)
    {
        return (_port1 != nullptr) ? _port1->Read() : false;
    }
    return 0;
}

void Input::storeb(u16 addr, u8 val)
{
    if (addr == 0x4016)
    {
        bool strobeVal = (val & 1) == 1;

        if (_port0 != nullptr)
        {
            _port0->Strobe(strobeVal);
        }

        if (_port1 != nullptr)
        {
            _port1->Strobe(strobeVal);
        }
    }
}

IStandardController* Input::GetStandardController(unsigned int port)
{
#ifdef DAC_BUILD
	return nullptr;
#else
    if (port >= 2)
    {
        return nullptr;
    }

    std::unique_ptr<StandardController> controller = std::make_unique<StandardController>();
    IStandardController* ptr = static_cast<IStandardController*>(controller.get());
    if (port == 0)
    {
        _port0 = std::move(controller);
    }
    else if (port == 1)
    {
        _port1 = std::move(controller);
    }

    return ptr;
#endif
}

#ifndef DAC_BUILD

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
    , _Up_actual(false)
    , _Down_actual(false)
    , _Left_actual(false)
    , _Right_actual(false)
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
    std::lock_guard<std::mutex> lock(_mutex);
    if (_Down_actual)
    {
        _Up = false;
        _Down = !state;
    }
    else
    {
        _Up = state;
    }

    _Up_actual = state;
}

void StandardController::Down(bool state)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_Up_actual)
    {
        _Down = false;
        _Up = !state;
    }
    else
    {
        _Down = state;
    }
    _Down_actual = state;
}

void StandardController::Left(bool state)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_Right_actual)
    {
        _Left = false;
        _Right = !state;
    }
    else
    {
        _Left = state;
    }
    _Left_actual = state;
}

void StandardController::Right(bool state)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_Left_actual)
    {
        _Right = false;
        _Left = !state;
    }
    else
    {
        _Right = state;
    }
    _Right_actual = state;
}

#endif