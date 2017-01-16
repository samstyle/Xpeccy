#include "gbsound.h"

#include <stdlib.h>
#include <string.h>

gbSound* gbsCreate() {
	gbSound* gbs = malloc(sizeof(gbSound));
	if (!gbs) return NULL;
	memset(gbs, 0x00, sizeof(gbSound));
	gbs->ch1 = bcCreate();
	gbs->ch2 = bcCreate();
	gbs->ch3 = bcCreate();
	gbs->ch4 = bcCreate();
	return gbs;
}

void gbsDestroy(gbSound* gbs) {
	if (!gbs) return;
	bcDestroy(gbs->ch1);
	bcDestroy(gbs->ch2);
	bcDestroy(gbs->ch3);
	bcDestroy(gbs->ch4);
	free(gbs);
}

void gbsSync(gbSound* gbs, int ns) {
	bcSync(gbs->ch1, ns);
	bcSync(gbs->ch2, ns);
	bcSync(gbs->ch3, ns);
	bcSync(gbs->ch4, ns);
}

int getLeft(bitChan* ch) {
	if (!ch->on) return 0;
	if (!ch->left) return 0;
	return ch->val;
}

int getRight(bitChan* ch) {
	if (!ch->on) return 0;
	if (!ch->right) return 0;
	return ch->val;
}

sndPair gbsVolume(gbSound* gbs) {
	int left = 0;
	int right = 0;
	if (gbs->enable) {
		left = getLeft(gbs->ch1) + getLeft(gbs->ch2) + getLeft(gbs->ch3) + getLeft(gbs->ch4);
		right = getRight(gbs->ch1) + getRight(gbs->ch2) + getRight(gbs->ch3) + getRight(gbs->ch4);
		left >>= 6;
		right >>= 6;
	}
	sndPair res;
	res.left = (left > 0xff) ? 0xff : left;
	res.right = (right > 0xff) ? 0xff : right;
	return res;
}
