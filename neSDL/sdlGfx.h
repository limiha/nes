#pragma once

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

#include <chrono>

//#define RENDER_GRID
//#define RENDER_NAMETABLE
//#define RENDER_PATTERNTABLE

class SdlGfx
{
public:
    SdlGfx(unsigned int scale);
    ~SdlGfx();

    void Blit(unsigned char screen[]);
private:
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    SDL_Texture* _texture;

    std::chrono::time_point<std::chrono::steady_clock> _lastDrawTime;
    std::chrono::time_point<std::chrono::steady_clock> _lastFpsTime;

    unsigned char _frameCounter;

#if defined(RENDER_NAMETABLE)
public:
    void BlitNameTable(u8 screen[], int i);

private:
    SDL_Window* _nt_window[4];
    SDL_Renderer* _nt_renderer[4];
    SDL_Texture* _nt_texture[4];
#endif

#if defined(RENDER_PATTERNTABLE)
public:
    void BlitPatternTable(u8 left[], u8 right[]);

private:
    SDL_Window* _pt_window;
    SDL_Renderer* _pt_renderer;
    SDL_Texture* _pt_texture[2];

    SDL_Rect _left_rect;
    SDL_Rect _right_rect;
#endif
};
