#pragma once

#include "video.h"

void gbcvReset(Video*);
void gbcvDraw(Video*);
void gbcvLine(Video*);
void gbcvFram(Video*);
void gbcvVBL(Video*);

void gbcv_wr(Video*, int, int);
int gbcv_rd(Video*, int);
