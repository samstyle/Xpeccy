#include "i8237_dma.h"

#include <stdlib.h>
#include <string.h>

i8237DMA* dma_create(void* p) {
	i8237DMA* dma = (i8237DMA*)malloc(sizeof(i8237DMA));
	if (dma) {
		memset(dma, 0, sizeof(i8237DMA));
		dma->ptr = p;
	}
	return dma;
}

void dma_destroy(i8237DMA* dma) {
	if (dma) free(dma);
}
