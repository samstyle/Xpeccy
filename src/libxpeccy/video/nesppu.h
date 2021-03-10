#pragma once

#include "video.h"

void ppuReset(Video*);

void ppuDraw(Video*);
void ppuHBL(Video*);
void ppuLine(Video*);
void ppuFram(Video*);

// rd/wr ppu registers
void ppuWrite(Video*, int, int);
int ppuRead(Video*, int);

void ppuRenderBGLine(Video*, unsigned char*, unsigned short, int, unsigned short);
int ppuRenderSpriteLine(Video*, int, unsigned char*, unsigned char*, unsigned short, int);
unsigned short ppuYinc(unsigned short);

