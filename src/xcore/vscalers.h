#ifndef X_VSCALERS_H
#define X_VSCALERS_H

#include "../libxpeccy/video/vidcommon.h"

/*
void scrX1(unsigned char*, int, int, unsigned char*);
void scrX2(unsigned char*, int, int, unsigned char*);
void scrX3(unsigned char*, int, int, unsigned char*);
void scrX4(unsigned char*, int, int, unsigned char*);
void scrFS(unsigned char*, int, int, unsigned char*, int, int);
*/

void vid_set_zoom(int);
void vid_set_fullscreen(int);
void vid_set_ratio(int);

#endif
