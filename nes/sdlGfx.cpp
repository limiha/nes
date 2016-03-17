#include "stdafx.h"
#include "sdlGfx.h"

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


SdlGfx::SdlGfx(u32 scale)
    : _frameCounter(0)
{
    SDL_InitSubSystem(SDL_INIT_VIDEO);

#if defined(RENDER_NAMETABLE)
    for (int i = 0; i < 4; i++)
    {
        char name[sizeof("ntxx") + 1];
        sprintf(name, "nt%02d", i);

        _nt_window[i] = SDL_CreateWindow(
            name,
            256 * (2 * (i % 2 )),
            240 * (2 * (i / 2)),
            256 * 2,
            240 * 2,
            SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS
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

#if defined(RENDER_PATTERNTABLE)
    _pt_window = SDL_CreateWindow(
        "pt",
        40,
        40,
        16 * 8 * 2,
        32 * 8 * 2,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
        );

    _pt_renderer = SDL_CreateRenderer(
        _pt_window,
        -1,
        SDL_RENDERER_ACCELERATED
        );

    for (int i = 0; i < 2; i++)
    {
        _pt_texture[i] = SDL_CreateTexture(
            _pt_renderer,
            SDL_PIXELFORMAT_RGB24,
            SDL_TEXTUREACCESS_STREAMING,
            8 * 8,
            32 * 8
            );
    }

    _left_rect.x = 0;
    _left_rect.y = 0;
    _left_rect.w = 8 * 8 * 2;
    _left_rect.h = 32 * 8 * 2;

    _right_rect.x = 8 * 8 * 2;
    _right_rect.y = 0;
    _right_rect.w = 8 * 8 * 2;
    _right_rect.h = 32 * 8 * 2;
#endif

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

    _lastDrawTime = std::chrono::high_resolution_clock::now();
    _lastFpsTime = std::chrono::high_resolution_clock::now();
}

SdlGfx::~SdlGfx()
{
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
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

void SdlGfx::Blit(u8 screen[])
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
        duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - _lastDrawTime).count();
    } while (duration < 16666667); // 60 fps
    _lastDrawTime = now;

    SDL_UpdateTexture(_texture, NULL, (void*)screen_to_render, SCREEN_WIDTH * 3);
    SDL_RenderClear(_renderer);
    SDL_RenderCopy(_renderer, _texture, NULL, NULL);
    SDL_RenderPresent(_renderer);

    _frameCounter++;
    now = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - _lastFpsTime).count() >= 1)
    {
        printf("fps: %d\n", _frameCounter);
        _frameCounter = 0;
        _lastFpsTime = now;
    }
}

#if defined(RENDER_NAMETABLE)
void SdlGfx::BlitNameTable(u8 screen[], int i)
{
    SDL_UpdateTexture(_nt_texture[i], NULL, (void*)screen, SCREEN_WIDTH * 3);
    SDL_RenderClear(_nt_renderer[i]);
    SDL_RenderCopy(_nt_renderer[i], _nt_texture[i], NULL, NULL);
    SDL_RenderPresent(_nt_renderer[i]);
}
#endif

#if defined(RENDER_PATTERNTABLE)
void SdlGfx::BlitPatternTable(u8 left[], u8 right[])
{
    SDL_UpdateTexture(_pt_texture[0], NULL, (void*)left, 8 * 8 * 3);
    SDL_UpdateTexture(_pt_texture[1], NULL, (void*)right, 8 * 8 * 3);
    SDL_RenderClear(_pt_renderer);
    SDL_RenderCopy(_pt_renderer, _pt_texture[0], NULL, &_left_rect);
    SDL_RenderCopy(_pt_renderer, _pt_texture[1], NULL, &_right_rect);
    SDL_RenderPresent(_pt_renderer);
}
#endif