#pragma once

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class Gfx
{
public:
    Gfx();
    ~Gfx();

    void Blit(u8 screen[]);
private:
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    SDL_Texture* _texture;
};
