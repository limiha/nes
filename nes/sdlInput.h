#pragma once

#include "IInput.h"
#include <SDL_events.h>

class SdlInput : public IInput
{
public:
    SdlInput();
    ~SdlInput();

    InputResult CheckInput(JoypadState& state);

private:
    void HandleKey(SDL_Keycode key, JoypadState& state, bool pressed);
};