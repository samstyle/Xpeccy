#pragma once

enum {
	CONT_NONE = 0,
	CONT_PATA,
	CONT_PATB
};

typedef struct {
	unsigned enabled:1;	// enabled by settings
	unsigned active:1;	// colorspace is on. else use std zx palette
	unsigned palchan:1;	// palette changed
	unsigned early:1;	// early contention timing (2 dots earlier than 'late')

	unsigned char reg;	// bf3b .w register
	unsigned char data;	// ff3b rw data
	int conttype;
	unsigned char pal[64];
} ulaPlus;

ulaPlus* ula_create();
void ula_destroy(ulaPlus*);

int ula_wr(ulaPlus*, int, int);
int ula_rd(ulaPlus*, int, int*);
