#include "hardware.h"

// debug


unsigned char brkIn(Computer* comp, unsigned short port) {
	printf("IN %.4X (dos:rom:cpm = %i:%i:%i)\n",port,comp->dos,comp->rom,comp->cpm);
	assert(0);
	comp->brk = 1;
	return 0xff;
}

void brkOut(Computer* comp, unsigned short port, unsigned char val) {
	printf("OUT %.4X,%.2X (dos:rom:cpm = %i:%i:%i)\n",port,val,comp->dos,comp->rom,comp->cpm);
	assert(0);
	comp->brk = 1;
}

unsigned char dummyIn(Computer* comp, unsigned short port) {
	return 0xff;
}

void dummyOut(Computer* comp, unsigned short port, unsigned char val) {

}

// INT handle/check

extern int res1,res2,res4;
int zxINT(Computer* comp, unsigned char vect) {
	res4 = 0;
	comp->intVector = vect;
	res2 = comp->cpu->intr(comp->cpu);
	res1 += res2;
	vidSync(comp->vid,(res2 - res4) * comp->nsPerTick);
	return res2;
}

int stdINT(Computer* comp) {
	int res = 1;
	if (comp->vid->intFRAME) {
		zxINT(comp, 0xff);
	} else if (comp->vid->intLINE) {
		if (zxINT(comp,0xfd))
			comp->vid->intLINE = 0;
	} else if (comp->vid->intDMA) {
		if (zxINT(comp,0xfb))
			comp->vid->intDMA = 0;
	} else {
		res = 0;
	}
	return res;
}

// beeper transient response emulation

#define OVERSHOOT 22500			// ns to overshoot process

void beepSync(Computer* comp) {
	int amp = comp->beepAmp;
	if (comp->beeplev) {		// going up
		amp += 256 * comp->beepNs / OVERSHOOT;
		if (amp > 255) amp = 255;
	} else {			// going down
		amp -= 256 * comp->beepNs / OVERSHOOT;
		if (amp < 0) amp = 0;
	}
	comp->beepAmp = amp & 0xff;
	comp->beepNs = 0;
}

// in

unsigned char xIn1F(Computer* comp, unsigned short port) {
	return joyInput(comp->joy);
}

unsigned char xInFE(Computer* comp, unsigned short port) {
	unsigned char res = keyInput(comp->keyb, (port & 0xff00) | 0xfe);
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
	beepSync(comp);
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}

void xOutBFFD(Computer* comp, unsigned short port, unsigned char val) {
	tsOut(comp->ts, 0xbffd, val);
}

void xOutFFFD(Computer* comp, unsigned short port, unsigned char val) {
	tsOut(comp->ts, 0xfffd, val);
}
