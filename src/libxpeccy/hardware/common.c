#include "hardware.h"
#include "assert.h"

// debug

Z80EX_BYTE brkIn(ZXComp* comp, Z80EX_WORD port) {
	printf("IN %.4X\n",port);
	assert(0);
	comp->brk = 1;
	return 0xff;
}

void brkOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	printf("OUT %.4X,%.2X\n",port,val);
	assert(0);
	comp->brk = 1;
}

Z80EX_BYTE dummyIn(ZXComp* comp, Z80EX_WORD port) {
	return 0xff;
}

void dummyOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {

}

// in

Z80EX_BYTE xIn1F(ZXComp* comp, Z80EX_WORD port) {
	return joyInput(comp->joy);
}

Z80EX_BYTE xInFE(ZXComp* comp, Z80EX_WORD port) {
	Z80EX_BYTE res = keyInput(comp->keyb, (port & 0xff00) >> 8);
	res |= (comp->tape->levPlay ? 0x40 : 0x00);
	return res;
}

Z80EX_BYTE xInFFFD(ZXComp* comp, Z80EX_WORD port) {
	return tsIn(comp->ts, 0xfffd);
}

Z80EX_BYTE xInFADF(ZXComp* comp, Z80EX_WORD port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->buttons : 0xff;
}

Z80EX_BYTE xInFBDF(ZXComp* comp, Z80EX_WORD port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->xpos : 0xff;
}

Z80EX_BYTE xInFFDF(ZXComp* comp, Z80EX_WORD port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->ypos : 0xff;
}

// out

void xOutFE(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->vid->nextbrd = val & 0x07;
	if (!comp->vid->border4t) comp->vid->brdcol = val & 0x07;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}

void xOutBFFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	tsOut(comp->ts, 0xbffd, val);
}

void xOutFFFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	tsOut(comp->ts, 0xfffd, val);
}
