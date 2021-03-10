#pragma once

#include "video.h"

#define VDP_IE1	0x10		// R0.4 = hint enable
#define VDP_IE0 0x20		// R1.5 = vint enable

// typedef struct Video Video;

void vdpReset(Video*);
void vdpWrite(Video*, int, unsigned char);
unsigned char vdpRead(struct Video*, int);

// drawers
void vdpText1(Video*);
void vdpMultcol(Video*);
void vdpGra1(Video*);
void vdpGra2(Video*);
void vdpGra3(Video*);
void vdpGra4(Video*);
void vdpGra5(Video*);
void vdpGra6(Video*);
void vdpGra7(Video*);
void vdpDummy(Video*);
// blank
void vdp_line(Video*);
void vdp_linex(Video*);
void vdpHBlk(Video*);
void vdpVBlk(Video*);
