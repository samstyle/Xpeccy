#pragma once

#include "video.h"

enum {
	VGA_CRTRN = 0,		// crt registers
	VGA_CRTRD,
	VGA_SEQRN,		// sync registers
	VGA_SEQRD,
	VGA_GRFRN,		// graphic mode
	VGA_GRFRD,
	VGA_MODE,
	VGA_STAT1
};

// vid->reg (registers)
#define	VGA_CRN	0x7f	// crt register num
#define VGA_CRC	0x18	// crt registers count-1
#define VGA_SRN 0x9f	// syncronisator registers
#define VGA_SRC 0x05
#define VGA_GRN 0xaf	// graphic registers (3ce/3cf)
#define VGA_GRC 0x08

int vga_rd(Video*, int);
void vga_wr(Video*, int, int);

int vga_mrd(Video*, int);
void vga_mwr(Video*, int, int);
