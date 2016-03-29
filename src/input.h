#pragma once

#include "..\include\nes_api.h"
#include "mem.h"

// Controller Port Device Interface
class IControllerPortDevice
{
public:
    virtual ~IControllerPortDevice() { }
public:
    virtual void Strobe(bool strobe) = 0;
    virtual u8 Read() = 0;
};

class EmptyPort : public IControllerPortDevice, public NesObject
{
public:
    EmptyPort() { }

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    // IControllerPortDevice
public:
    void Strobe(bool strobe) { }
    u8 Read() { return 0; }
};

class Input : public IMem, public NesObject
{
public:
    Input();
    virtual ~Input();

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    // ISaveState
    // don't bother saving the state of input as it will be read again after 
    // the save state is loaded and before the next cpu instruction
    void Save() {}
    void Load() {}

public:
    IStandardController* GetStandardController(unsigned int port);

private:
    std::unique_ptr<IControllerPortDevice> _port0;
    std::unique_ptr<IControllerPortDevice> _port1;
};

#ifndef DAC_BUILD

class StandardController : public IControllerPortDevice, public IStandardController, public NesObject
{
public:
    StandardController();
    virtual ~StandardController();

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

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
    std::mutex _mutex;

    std::atomic<bool> _A;
    std::atomic<bool> _B;
    std::atomic<bool> _Select;
    std::atomic<bool> _Start;

    // for up/down/left/right, we don't allow left/right or up/down to be pressed at the same time
    // if both up/down or left/right are pressed, we treat it like neither are
    // to accomplish this we have to track two things: the current state the buttons are actually in
    // and the state that we will send to the nes
    // This is important to make the following scenario work:
    // User is holding left: left is the input
    // User holds down right in addition to holding down left: input is neither right nor left is pressed
    // User releases right, still holding down left: left is the input

    // NES visible
    std::atomic<bool> _Up;
    std::atomic<bool> _Down;
    std::atomic<bool> _Left;
    std::atomic<bool> _Right;

    // Actual states
    std::atomic<bool> _Up_actual;
    std::atomic<bool> _Down_actual;
    std::atomic<bool> _Left_actual;
    std::atomic<bool> _Right_actual;

    std::atomic<bool> _strobe;
    u8 _nextReadIndex;
};

#endif