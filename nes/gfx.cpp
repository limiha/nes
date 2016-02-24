#include "stdafx.h"
#include "gfx.h"
#include "ppu.h" // for screen size

#include <sdl.h>

Gfx::Gfx()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER);

    _window = SDL_CreateWindow(
        "NES",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
        );

    _renderer = SDL_CreateRenderer(
        _window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );

    _texture = SDL_CreateTexture(
        _renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
        );
}

Gfx::~Gfx()
{
    if (_texture)
    {
        SDL_DestroyTexture(_texture);
    }
    if (_renderer)
    {
        SDL_DestroyRenderer(_renderer);
    }
    if (_window)
    {
        SDL_DestroyWindow(_window);
    }

    SDL_Quit();
}

void Gfx::Blit(u8* screen)
{
    SDL_UpdateTexture(_texture, NULL, (void*)screen, SCREEN_WIDTH * 3);
    SDL_RenderClear(_renderer);
    SDL_RenderCopy(_renderer, _texture, NULL, NULL);
    SDL_RenderPresent(_renderer);
}
