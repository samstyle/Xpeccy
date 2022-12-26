#pragma once

#include "video.h"

#define	VGA_CRTRN	0x3d4		// crt registers
#define	VGA_CRTRD	0x3d5
#define	VGA_SEQRN	0x3c4		// sync registers
#define	VGA_SEQRD	0x3c5
#define	VGA_GRFRN	0x3ce		// graphic mode
#define	VGA_GRFRD	0x3cf
#define	VGA_ATREG	0x3c0		// atribute idx/data
#define	VGA_MODE	0x3d8
#define VGA_STAT1	0x3da

// vid->reg (registers)

#define EGA_3C2	0x42
#define CGA_3D8	0x58
#define CGA_3D9 0x59
#define CGA_3DA 0x5a

#define	VGA_CRB	0x80	// crt registers
#define VGA_CRC	0x18	// count
#define VGA_SRB 0xA0	// syncronisator registers
#define VGA_SRC 0x05
#define VGA_GRB 0xb0	// graphic registers (3ce/3cf)
#define VGA_GRC 0x08
#define VGA_ATB 0xc0	// atribute registers
#define VGA_ATC 0x14

#define CRT_IDX		vid->vga.crt_idx
#define CRT_REG(_n)	vid->reg[VGA_CRB + (_n)]
#define CRT_CUR_REG	CRT_REG(CRT_IDX)

#define SEQ_IDX		vid->vga.seq_idx
#define SEQ_REG(_n)	vid->reg[VGA_SRB + (_n)]
#define SEQ_CUR_REG	SEQ_REG(SEQ_IDX)

#define GRF_IDX		vid->vga.grf_idx
#define GRF_REG(_n)	vid->reg[VGA_GRB + (_n)]
#define GRF_CUR_REG	GRF_REG(GRF_IDX)

#define ATR_IDX		vid->vga.atr_idx
#define ATR_REG(_n)	vid->reg[VGA_ATB + (_n)]
#define ATR_CUR_REG	ATR_REG(ATR_IDX)

void vga_reset(Video*);
int vga_rd(Video*, int);
void vga_wr(Video*, int, int);

int vga_mrd(Video*, int);
void vga_mwr(Video*, int, int);
