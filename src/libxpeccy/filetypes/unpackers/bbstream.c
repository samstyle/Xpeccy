#include "unpackers.h"

const WORD mask[]  = { 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100, 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1 };

void bsInit(bbStream* bs, BYTE* from, int type) {
	bs->base = bs->p = from;

	bs->pType = type;
	bs->len = 0xffff;
	bs->eof = 0;

	if (bs->pType == PAK_HRS) {
		bs->idx = 0;
		bs->bits  = bsGetByte(bs);
		bs->bits += (bsGetByte(bs) << 8);
	} else if (bs->pType == PAK_MLZ) {
		bs->idx = 16;
	}
}

BYTE bsGetByte(bbStream* bs) {
	if(bs->p - bs->base == bs->len ) {
		bs->eof = 1;
		return 0;
	}
	return *bs->p++;
}

BYTE bsGetBit(bbStream* bs) {
	if ((bs->idx > 15) && (bs->pType == PAK_MLZ)) {
		bs->bits = bsGetByte(bs);
		bs->idx = 8;
	}
	BYTE bit = (bs->bits & mask[bs->idx]) ? 1 : 0;
	if ((bs->idx == 15) && (bs->pType == PAK_HRS)) {
		bs->bits = bsGetByte(bs);
		bs->bits += (bsGetByte(bs) << 8);
		bs->idx = -1;
	}
	bs->idx++;
	return bit;
}

BYTE bsGetBits(bbStream* bs, int n) {
	BYTE r = 0;
	do {
		r = (r << 1) | bsGetBit(bs);
	} while(--n);
	return r;
}
