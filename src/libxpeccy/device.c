#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "device.h"

// id, create, destroy, reset, request, sync, vol, {NULL}
xDevice devTab[] = {
	{DEV_GSOUND, gsCreate, gsDestroy, gsReset, gsRequest, gsSync, gsFlush, gsVolume, {NULL}},
	{DEV_SDRIVE, sdCreate, sdDestroy, NULL, sdRequest, NULL, NULL, sdVolume, {NULL}},
	{DEV_NONE, NULL, NULL, NULL, NULL, NULL, NULL, NULL, {NULL}}
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
	if (!dsc) return NULL;
	if (dsc->type == DEV_NONE) return NULL;
	xDevice* dev = (xDevice*)malloc(sizeof(xDevice));
	*dev = *dsc;
	if (dev->create) {
		dev->ptr.ptr = dsc->create();
	} else {
		dev->ptr.ptr = NULL;
	}
	return dev;
}

void devDestroy(xDevice* dev) {
	if (!dev) return;
	if (dev->destroy)
		dev->destroy(dev->ptr.ptr);
	free(dev);
}

/*
void devRequest(xDevice* dev, xDevBus* bus) {
	if (bus->busy) return;					// avoid bus conflict
	bus->busy = dev->req(dev->ptr.ptr, bus) ? 1 : 0;	// 0 if device reject request
}
*/
