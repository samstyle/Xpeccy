#ifndef _ULAPLUS_H
#define _ULAPLUS_H

typedef struct {
	unsigned enabled:1;	// enabled by settings
	unsigned active:1;	// colorspace is on. else use std zx palette
	unsigned palchan:1;	// palette changed

	unsigned char reg;	// bf3b .w register
	unsigned char data;	// ff3b rw data
	unsigned char pal[64];
} ulaPlus;

ulaPlus* ulaCreate();
void ulaDestroy(ulaPlus*);

int ulaOut(ulaPlus*, int, int);
int ulaIn(ulaPlus*, int, int*);

#endif
