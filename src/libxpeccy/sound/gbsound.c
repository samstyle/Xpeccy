#include "gbsound.h"

#include <stdlib.h>
#include <string.h>

gbSound* gbsCreate() {
	gbSound* gbs = malloc(sizeof(gbSound));
	if (gbs) {
		memset(gbs, 0x00, sizeof(gbSound));
	}
	return gbs;
}

void gbsDestroy(gbSound* gbs) {
	if (!gbs) return;
	free(gbs);
}
