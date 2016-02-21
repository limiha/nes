#pragma once

#include "mem.h"

class Ppu : public IMem
{
public:
	Ppu();
	~Ppu();

	// IMem
	u8 loadb(u16 addr);
	void storeb(u16 addr, u8 val);
};