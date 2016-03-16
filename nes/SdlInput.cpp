#include "stdafx.h"
#include "sdlInput.h"

SdlInput::SdlInput()
{
    SDL_InitSubSystem(SDL_INIT_EVENTS);
}

SdlInput::~SdlInput()
{
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

InputResult SdlInput::CheckInput(JoypadState& state)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_F1: return InputResult::SaveState;
            case SDLK_F2: return InputResult::LoadState;
            case SDLK_ESCAPE: return InputResult::Quit;
            default:
                HandleKey(event.key.keysym.sym, state, true);
            }
            break;
        case SDL_KEYUP:
            HandleKey(event.key.keysym.sym, state, false);
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                return InputResult::Quit;
            }
        }
    }
    return InputResult::Continue;
}

void SdlInput::HandleKey(SDL_Keycode key, JoypadState& state, bool pressed)
{
    switch (key)
    {
    case SDLK_LALT:
    case SDLK_s:
        state.A = pressed;
        break;
    case SDLK_LCTRL:
    case SDLK_a:
        state.B = pressed;
        break;
    case SDLK_RSHIFT:
    case SDLK_LSHIFT:
    case SDLK_BACKSLASH:
        state.Select = pressed;
        break;
    case SDLK_RETURN:
        state.Start = pressed;
        break;
    case SDLK_UP:
        state.Up = pressed;
        break;
    case SDLK_DOWN:
        state.Down = pressed;
        break;
    case SDLK_LEFT:
        state.Left = pressed;
        break;
    case SDLK_RIGHT:
        state.Right = pressed;
        break;
    }
}