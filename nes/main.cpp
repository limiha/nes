#include "stdafx.h"
#include "nes.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Must provide path to ROM file.");
        return -1;
    }

    std::unique_ptr<Nes> nes = Nes::Create(argv[1]);
    if (nes != nullptr)
        nes->Run();
}