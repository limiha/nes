#include "stdafx.h"
#include "gfx.h"

#include <SDL.h>

#if !defined(RENDER_GRID)
const u32 SCREEN_WIDTH = 256;
const u32 SCREEN_HEIGHT = 240;
#else
const u32 SCREEN_WIDTH = 256 +32;
const u32 SCREEN_HEIGHT = 240 +30;

u8 grid_screen[SCREEN_HEIGHT * SCREEN_WIDTH * 3];

u8 grid_color = 0xff;
#endif


Gfx::Gfx(u32 scale)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER);

    _window = SDL_CreateWindow(
        "NES",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * scale ,
        SCREEN_HEIGHT * scale,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
        );

    _renderer = SDL_CreateRenderer(
        _window,
        -1,
        SDL_RENDERER_ACCELERATED// | SDL_RENDERER_PRESENTVSYNC
        );

    _texture = SDL_CreateTexture(
        _renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
        );

	_lastTime = std::chrono::high_resolution_clock::now();

#if defined(RENDER_NAMETABLE)
    for (int i = 0; i < 4; i++)
    {
        char name[sizeof("ntxx") + 1];
        sprintf(name, "nt%02d", i);

        _nt_window[i] = SDL_CreateWindow(
            name,
            256 * 2 * i,
            240,
            256 * 2,
            240 * 2,
            SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
            );

        _nt_renderer[i] = SDL_CreateRenderer(
            _nt_window[i],
            -1,
            SDL_RENDERER_ACCELERATED
            );

        _nt_texture[i] = SDL_CreateTexture(
            _nt_renderer[i],
            SDL_PIXELFORMAT_RGB24,
            SDL_TEXTUREACCESS_STREAMING,
            256,
            240
            );
    }
#endif
}

Gfx::~Gfx()
{
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);

    SDL_Quit();
}

#if defined(RENDER_GRID)
void render_grid(u8 screen[])
{
    ZeroMemory(grid_screen, SCREEN_HEIGHT * SCREEN_WIDTH * 3);

    int screen_index = 0;

    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 3;)
    {
        if (i % 7776 == 0)
        {
            for (int j = 0; j < 864; j++)
            {
                grid_screen[i++] = grid_color;
            }
        }
        else 
            if (i % (9 * 3) == 0)
        {
            grid_screen[i++] = grid_color;
            grid_screen[i++] = grid_color;
            grid_screen[i++] = grid_color;
        }
        else
        {
            grid_screen[i++] = screen[screen_index++];
            grid_screen[i++] = screen[screen_index++];
            grid_screen[i++] = screen[screen_index++];
        }
    }
}
#endif

void Gfx::Blit(u8 screen[])
{
    void* screen_to_render = (void*)screen;
#if defined(RENDER_GRID)
    render_grid(screen);
    screen_to_render = (void*)grid_screen;
#endif

    std::chrono::time_point<std::chrono::steady_clock> now;
    long long duration;
    do
    {
        now = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - _lastTime).count();
    } while (duration < 16666667); // 60 fps
    _lastTime = now;

    SDL_UpdateTexture(_texture, NULL, (void*)screen_to_render, SCREEN_WIDTH * 3);
    SDL_RenderClear(_renderer);
    SDL_RenderCopy(_renderer, _texture, NULL, NULL);
    SDL_RenderPresent(_renderer);
}

#if defined(RENDER_NAMETABLE)
void Gfx::BlitNameTable(u8 screen[], int i)
{
    SDL_UpdateTexture(_nt_texture[i], NULL, (void*)screen, SCREEN_WIDTH * 3);
    SDL_RenderClear(_nt_renderer[i]);
    SDL_RenderCopy(_nt_renderer[i], _nt_texture[i], NULL, NULL);
    SDL_RenderPresent(_nt_renderer[i]);
}
#endif
