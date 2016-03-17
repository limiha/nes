#pragma once

// Things related to the public interface API

enum class InputResult
{
    Continue,
    SaveState,
    LoadState,
    Quit
};

struct JoypadState
{
    bool A;
    bool B;
    bool Select;
    bool Start;
    bool Up;
    bool Down;
    bool Left;
    bool Right;

    JoypadState()
        : A(false)
        , B(false)
        , Select(false)
        , Start(false)
        , Up(false)
        , Down(false)
        , Left(false)
        , Right(false)
    {
    }

    void Set(const JoypadState& state)
    {
        A = state.A;
        B = state.B;
        Select = state.Select;
        Start = state.Start;
        Up = state.Up;
        Down = state.Down;
        Left = state.Left;
        Right = state.Right;
    }
};

class IInput
{
public:
    virtual InputResult CheckInput(JoypadState& state) = 0;
};