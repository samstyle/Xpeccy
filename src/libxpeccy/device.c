#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "device.h"

// id, create, destroy, reset, rd, wr, sync, {NULL}
xDevice devTab[] = {
	{DEV_NONE, NULL, NULL, NULL, NULL, NULL, NULL, {NULL}}
};

xDevice* getDevDsc(int type) {
	int idx = 0;
	while((devTab[idx].type != DEV_NONE) && (devTab[idx].type != type)) {
		idx++;
	}
	return &devTab[idx];
}

xDevice* devCreate(int type) {
	xDevice* dsc = getDevDsc(type);
	if (dsc->type == DEV_NONE)
		return NULL;
	xDevice* dev = (xDevice*)malloc(sizeof(xDevice));
	*dev = *dsc;
	if (dev->create)
		dev->ptr = dsc->create();
	return dev;
}

void devDestroy(xDevice* dev) {
	if (!dev) return;
	if (dev->destroy)
		dev->destroy(dev->ptr);
	free(dev);
}

void devRequest(xDevice* dev, xDevBus* bus) {
	if (bus->rd) {
		if (dev->rd)
			bus->iorge = dev->rd(dev->ptr, bus);
	} else if (bus->wr) {
		if (dev->wr)
			bus->iorge = dev->wr(dev->ptr, bus);
	}
}

/*
void devRead(xDevice* dev, xDevBus* bus) {
	if (bus->iorge) return;
	if (!dev) return;
	if (!dev->rd) return;
	dev->rd(dev->ptr, bus);
}

void devWrite(xDevice* dev, xDevBus* bus) {
	if (bus->iorge) return;
	if (!dev) return;
	if (!dev->wr) return;
	dev->wr(dev->ptr, bus);
}
*/
