#include "video.h"

// a0: 0 - rd:status, wr:param fifo
// a0: 1 - rd:fifo rd, wr:command fifo

// status:
// b7: light pen detect
// b6: hblank
// b5: vsync
// b4: dma execute
// b3: drawing in process
// b2: fifo empty
// b1: fifo full
// b0: data ready

// reg[00..0F] = fifo for commands
// reg[10..1F] = output data buffer
#define fifoPos	reg[32]	// +1 each internal rd
#define fifoSiz reg[33]	// +1 each cpu writing, -1 each internal rd. queue size
#define parCnt	reg[34]	// parameters command want to get
#define dOutPos reg[35]	// =0x10 when form output packet, +1 each cpu rd
#define dOutSiz	reg[36]	// output data packet size (0 = no data), -1 each cpu rd

// execute command. reg[0] = com, reg[1..15] = params
void upd7220_exec(Video* vid) {

}

int upd7220_rd(Video* vid, int adr) {
	int res = 0;
	if (adr & 1) {
		// out data rd
		if (vid->dOutSiz > 0) {
			res = vid->reg[vid->dOutPos];
			vid->dOutPos++;
			vid->dOutSiz--;
		} else {
			res = 0xff;
		}
	} else {
		// status rd
		if (vid->hblank) res |= 0x40;
		if (vid->vblank) res |= 0x20;
		if (vid->fifoSiz > 15) {
			res |= 0x02;
		} else if (vid->fifoSiz == 0) {
			res |= 0x04;
		}
		if (vid->dOutSiz > 0) res |= 0x01;
	}
	return res;
}

void upd7220_wr(Video* vid, int adr, int val) {
	if (adr & 1) {
		// fifo wr:command
		vid->reg[0] = val & 0xff;
		vid->fifoPos = 0;
		vid->fifoSiz = 1;
		vid->parCnt = 0;	// TODO:make table com->parcnt
		if (vid->parCnt == 0) {
			upd7220_exec(vid);
		}
	} else {
		// fifo wr:param
		if (vid->parCnt > 0) {
			vid->reg[vid->fifoSiz & 15] = val & 15;
			vid->parCnt--;
			vid->fifoSiz++;
			if (vid->parCnt == 0) {
				upd7220_exec(vid);
			}
		}
	}
}

// drawing

void upd7220_dot(Video* vid) {
	vid_dot_full(vid, 0);
}

void upd7220_line(Video* vid) {

}

void upd7220_frame(Video* vid) {

}
