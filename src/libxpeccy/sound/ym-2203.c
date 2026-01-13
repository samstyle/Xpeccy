#include "ayym.h"

void ym2203_reset(aymChip* ym) {

}

void ym2203_sync(aymChip* ym, int ns) {

}

sndPair ym2203_vol(aymChip* ym) {
	sndPair v;
	v.left = 0;
	v.right = 0;
	return v;
}

int ym2203_rd(aymChip* ym, int adr) {
	return -1;
}

void ym2203_wr(aymChip* ym, int adr, int val) {

}
