#include "sdlInput.h"
#include <SDL.h>
#include <nes_api.h>

SdlInput::SdlInput(IStandardController* controller0)
{
    _controller0 = controller0;
    SDL_InitSubSystem(SDL_INIT_EVENTS);
}

SdlInput::~SdlInput()
{
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

InputResult SdlInput::CheckInput()
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
                HandleKey(event.key.keysym.sym, true);
            }
            break;
        case SDL_KEYUP:
            HandleKey(event.key.keysym.sym, false);
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

void SdlInput::HandleKey(SDL_Keycode key, bool pressed)
{
    if (_controller0 != nullptr)
    {
        switch (key)
        {
        case SDLK_LALT:
        case SDLK_s:
            _controller0->A(pressed);
            break;
        case SDLK_LCTRL:
        case SDLK_a:
            _controller0->B(pressed);
            break;
        case SDLK_RSHIFT:
        case SDLK_LSHIFT:
        case SDLK_BACKSLASH:
            _controller0->Select(pressed);
            break;
        case SDLK_RETURN:
            _controller0->Start(pressed);
            break;
        case SDLK_UP:
            _controller0->Up(pressed);
            break;
        case SDLK_DOWN:
            _controller0->Down(pressed);
            break;
        case SDLK_LEFT:
            _controller0->Left(pressed);
            break;
        case SDLK_RIGHT:
            _controller0->Right(pressed);
            break;
        }
    }
}