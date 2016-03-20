#pragma once

// Things related to the public interface API

enum class InputResult
{
    Continue,
    SaveState,
    LoadState,
    Quit
};

class IHostInput
{
public:
    virtual InputResult CheckInput() = 0;
};

// Controller Port Device Interface
class IControllerPortDevice
{
public:
    virtual void Strobe(bool strobe) = 0;
    virtual u8 Read() = 0;
};

// Standard Controller Interface
class IStandardController
{
public:
    virtual void A(bool state) = 0;
    virtual void B(bool state) = 0;
    virtual void Select(bool state) = 0;
    virtual void Start(bool state) = 0;
    virtual void Up(bool state) = 0;
    virtual void Down(bool state) = 0;
    virtual void Left(bool state) = 0;
    virtual void Right(bool state) = 0;
};
