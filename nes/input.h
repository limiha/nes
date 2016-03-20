#pragma once

#include "mem.h"
#include "IInput.h"

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

public:
    // This class is not an responsible for deleting whatever these pointers point to.
    IControllerPortDevice* Port0;
    IControllerPortDevice* Port1;
};

class StandardController : public IControllerPortDevice, public IStandardController
{
public:
    StandardController();
    ~StandardController();

    // IControllerPortDevice
public:
    void Strobe(bool strobe);
    u8 Read();

    // IStandardController
public:
    void A(bool state);
    void B(bool state);
    void Select(bool state);
    void Start(bool state);
    void Up(bool state);
    void Down(bool state);
    void Left(bool state);
    void Right(bool state);

private:
    std::atomic<bool> _A;
    std::atomic<bool> _B;
    std::atomic<bool> _Select;
    std::atomic<bool> _Start;
    std::atomic<bool> _Up;
    std::atomic<bool> _Down;
    std::atomic<bool> _Left;
    std::atomic<bool> _Right;

    std::atomic<bool> _strobe;
    u8 _nextReadIndex;
};
