#pragma once

struct IStandardController;

#include <SDL_events.h>

enum class InputResult
{
    Continue,
    SaveState,
    LoadState,
    Quit
};

class SdlInput 
{
public:
    SdlInput(IStandardController* controller0);
    ~SdlInput();

    InputResult CheckInput();

private:
    void HandleKey(SDL_Keycode key, bool pressed);

private:
    IStandardController* _controller0;
};