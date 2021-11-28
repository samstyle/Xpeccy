#pragma once

#include "video.h"

enum {
	VGA_REGNUM = 0,
	VGA_REGVAL,
	VGA_MODE,
	VGA_STAT1
};

int vga_rd(Video*, int);
void vga_wr(Video*, int, int);
