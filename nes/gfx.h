#pragma once

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class Gfx
{
public:
    Gfx(u32 scale);
    ~Gfx();

    void Blit(u8 screen[]);
private:
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    SDL_Texture* _texture;

#if defined(RENDER_NAMETABLE)
    SDL_Window* _nt_window[4];
    SDL_Renderer* _nt_renderer[4];
    SDL_Texture* _nt_texture[4];

public:
    void BlitNameTable(u8 screens[], int i);
#endif
};
