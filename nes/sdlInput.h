#pragma once

#include "IInput.h"
#include <SDL_events.h>

class SdlInput : public IHostInput
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