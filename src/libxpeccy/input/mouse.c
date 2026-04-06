#include <stdlib.h>
#include <string.h>

#include "input.h"

// mouse

Mouse* mouseCreate(cbirq cb, void* p) {
	Mouse* mou = (Mouse*)malloc(sizeof(Mouse));
	memset(mou,0x00,sizeof(Mouse));
	mou->sensitivity = 1.0f;
	mou->pcmode = MOUSE_SERIAL;
	mou->xirq = cb;
	mou->xptr = p;
	return mou;
}

void mouseDestroy(Mouse* mou) {
	free(mou);
}

void mouseReleaseAll(Mouse* mou) {
	mou->lmb = 0;
	mou->rmb = 0;
	mou->mmb = 0;
	mou->autox = 0;
	mou->autoy = 0;
}

void mousePress(Mouse* mou, int wut, int val) {
	switch(wut) {
		case XM_LMB: mou->lmb = 1; break;
		case XM_RMB: mou->rmb = 1; break;
		case XM_MMB: mou->mmb = 1; break;
		case XM_WHEELDN: mou->wheel++; break;
		case XM_WHEELUP: mou->wheel--; break;
		case XM_UP: mou->autoy = val; break;
		case XM_DOWN: mou->autoy = -val; break;
		case XM_LEFT: mou->autox = -val; break;
		case XM_RIGHT: mou->autox = val; break;
	}
}

void mouseRelease(Mouse* mou, int wut) {
	switch(wut) {
		case XM_LMB: mou->lmb = 0; break;
		case XM_RMB: mou->rmb = 0; break;
		case XM_MMB: mou->mmb = 0; break;
		case XM_UP:
		case XM_DOWN: mou->autoy = 0; break;
		case XM_LEFT:
		case XM_RIGHT: mou->autox = 0; break;
	}
}

int mouseGetX(Mouse* mou) {return mou->xpos * mou->sensitivity;}
int mouseGetY(Mouse* mou) {return mou->ypos * mou->sensitivity;}

// interrupt packet (ps/2 mouse):
// byte1:	b7:Y overflow
//		b6:X overflow
//		b5:Y delta sign
//		b4:X delta sign
//		b3: 1
//		b2: mmb
//		b1: rmb
//		b0: lmb
// byte2	abs X delta
// byte3	abs Y delta

void mouse_interrupt(Mouse* mouse) {
	if (mouse->queueSize > 0) return;
	if (mouse->lock) return;
	switch (mouse->pcmode) {
		case MOUSE_SERIAL:			// microsoft serial mouse
			mouse->outbuf = 0x40;
			if (mouse->lmb) mouse->outbuf |= 0x20;
			if (mouse->rmb) mouse->outbuf |= 0x10;
			mouse->outbuf |= ((mouse->ydelta & 0xc0) >> 4);
			mouse->outbuf |= ((mouse->xdelta & 0xc0) >> 6);
			mouse->outbuf |= ((mouse->xdelta & 0x3f) << 8);
			mouse->outbuf |= ((mouse->ydelta & 0x3f) << 16);
			mouse->queueSize = 3;
			mouse->xirq(IRQ_MOUSE_DATA, mouse->xptr);
			break;
		case MOUSE_PS2:
			// ps/2 mouse
			mouse->outbuf = (abs(mouse->ydelta) & 0xff) << 8;
			mouse->outbuf |= ((abs(mouse->xdelta) & 0xff) << 16);
			if (mouse->lmb) mouse->outbuf |= (1 << 0);
			if (mouse->rmb) mouse->outbuf |= (1 << 1);
			// b2: mmb
			mouse->outbuf |= (1 << 3);
			if (mouse->xdelta < 0) mouse->outbuf |= (1 << 4);
			if (mouse->ydelta < 0) mouse->outbuf |= (1 << 5);
			// b6,7: x,y overflow
			mouse->queueSize = 3;
			mouse->xirq(IRQ_MOUSE_DATA, mouse->xptr);
			break;
		default:
			break;
	}
	mouse->xdelta = 0;
	mouse->ydelta = 0;
}

int mouse_rd(Mouse* mouse) {
	int res = -1;
	if (mouse->queueSize > 0) {
		res = mouse->outbuf & 0xff;
		mouse->data = res;
		mouse->outbuf >>= 8;
		mouse->queueSize--;
	}
	return res;
}

void mouse_ack(Mouse* mou, int d) {
	if (mou->lock) return;
	mou->outbuf = d;
	mou->queueSize = 1;
	mou->xirq(IRQ_MOUSE_ACK, mou->xptr);
}

void mouse_wr(Mouse* mou, int d) {
	if (mou->com < 0) {
		switch (d) {
			case 0xe6: mouse_ack(mou, 0xfa); break;	// set scale 1:1
			case 0xe7: mouse_ack(mou, 0xfa); break;	// set scale 2:1
			case 0xe8: mouse_ack(mou, 0xfa);
				mou->com = d;
				break;		// +data: set resolution
			case 0xe9: break;				// status request
			case 0xea: mouse_ack(mou, 0xfa); break;	// set stream mode
			case 0xeb: break;				// read data
			case 0xec: mouse_ack(mou, 0xfa); break;	// reset wrap mode
			case 0xee: mouse_ack(mou, 0xfa); break;	// set wrap mode
			case 0xf0: mouse_ack(mou, 0xfa); break;	// set remote mode
			case 0xf2: mouse_ack(mou, 0x00); break;	// get device id (00 - standard ps/2 mouse)
			case 0xf3: mouse_ack(mou, 0xfa);
				mou->com = d;
				break;		// set sample rate
			case 0xf4: mou->lock = 0;
				mouse_ack(mou, 0xfa);
				break;		// enable data reporting
			case 0xf5: mou->lock = 1;
				mouse_ack(mou, 0xfa);
				break;		// disable data reporting
			case 0xf6: mouse_ack(mou, 0xfa); break;	// set defaults
			case 0xfe: mouse_ack(mou, mou->data); break;	// resend
			case 0xff: mouse_ack(mou, 0x00); break;	// reset & send id
		}
	} else {
		switch (mou->com) {
			case 0xe8:
				mouse_ack(mou, 0xfa);
				break;				// d - resolution
			case 0xf3:
				mouse_ack(mou, 0xfa);
				break;				// d - sample rate
		}
		mou->com = -1;
	}
//	printf("%s : %X\n",__FUNCTION__,d);
}
