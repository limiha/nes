#pragma once

// Graphics interface
class IGfx
{
public:
    virtual void Blit(unsigned char screen[]) = 0;

    //Optional
    virtual void BlitNameTable(unsigned char screen[], int i) { }
    virtual void BlitPatternTable(unsigned char left[], unsigned char right[]) { }
};