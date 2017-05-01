#include "cartridge.h"
#include <stdlib.h>
#include <string.h>

xCartridge* sltCreate() {
	xCartridge* slt = (xCartridge*)malloc(sizeof(xCartridge));
	memset(slt, 0x00, sizeof(xCartridge));
	return slt;
}

void sltDestroy(xCartridge* slt) {
	if (slt == NULL) return;
	sltEject(slt);
	free(slt);
}

void sltEject(xCartridge* slt) {
	if (slt->data)
		free(slt->data);
	slt->data = NULL;
	slt->name[0] = 0x00;
}
