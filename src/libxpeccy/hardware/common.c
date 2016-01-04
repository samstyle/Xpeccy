#include "hardware.h"
#include "assert.h"

// debug

unsigned char brkIn(Computer* comp, unsigned short port) {
	printf("IN %.4X\n",port);
	assert(0);
	comp->brk = 1;
	return 0xff;
}

void brkOut(Computer* comp, unsigned short port, unsigned char val) {
	printf("OUT %.4X,%.2X\n",port,val);
	assert(0);
	comp->brk = 1;
}

unsigned char dummyIn(Computer* comp, unsigned short port) {
	return 0xff;
}

void dummyOut(Computer* comp, unsigned short port, unsigned char val) {

}

// in

unsigned char xIn1F(Computer* comp, unsigned short port) {
	return joyInput(comp->joy);
}

unsigned char xInFE(Computer* comp, unsigned short port) {
	unsigned char res = keyInput(comp->keyb, (port & 0xff00) >> 8, 0);
	res |= (comp->tape->levPlay ? 0x40 : 0x00);
	return res;
}

unsigned char xInFFFD(Computer* comp, unsigned short port) {
	return tsIn(comp->ts, 0xfffd);
}

unsigned char xInFADF(Computer* comp, unsigned short port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->buttons : 0xff;
}

unsigned char xInFBDF(Computer* comp, unsigned short port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->xpos : 0xff;
}

unsigned char xInFFDF(Computer* comp, unsigned short port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->ypos : 0xff;
}

// out

void xOutFE(Computer* comp, unsigned short port, unsigned char val) {
	comp->vid->nextbrd = val & 0x07;
	if (!comp->vid->border4t) comp->vid->brdcol = val & 0x07;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}

void xOutBFFD(Computer* comp, unsigned short port, unsigned char val) {
	tsOut(comp->ts, 0xbffd, val);
}

void xOutFFFD(Computer* comp, unsigned short port, unsigned char val) {
	tsOut(comp->ts, 0xfffd, val);
}
