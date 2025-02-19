#include <stdlib.h>
#include "filetypes.h"

#pragma pack (push, 1)

typedef struct {
	char sign[7];
	char eot;
	char major;
	char minor;
} tzxHead;

#pragma pack (pop)

typedef struct {
	unsigned char id;
	void(*callback)(FILE*, Tape*);
} tzxBCall;

//#define NS_TICK TAPCPUNS
//#define NS_TICK 284
// #define NS_TICK tape->t_ns

static int sigLens[] = {PILOTLEN,SYNC1LEN,SYNC2LEN,SIGN0LEN,SIGN1LEN,0,-1};	// 0->SYNC3LEN

// #10: <pause:2>,<datalen:2>,{data}
void tzxBlock10(FILE* file, Tape* tape) {
	int pause = fgetw(file) * 1e6 / TAPTICKNS;		// ms -> ticks
	int len = fgetw(file);
	char buf[0x10000];
	fread(buf, len, 1, file);
	tape->tmpBlock = tapDataToBlock(buf, len, sigLens);
	blkAddPause(&tape->tmpBlock, pause);
	tap_add_block(tape, tape->tmpBlock);
	blkClear(&tape->tmpBlock);
}

// #11: <pilot:2>,<sync1:2>,<sync2:2>,<bit0:2>,<bit1:2>,<pilotcnt:2>,<usedbits:1>,<pause:2>,<datalen:3>,{data}
void tzxBlock11(FILE* file, Tape* tape) {
	int altLens[7];
	altLens[0] = fgetw(file) * TAPCPUNS / TAPTICKNS;	// pilot
	altLens[1] = fgetw(file) * TAPCPUNS / TAPTICKNS;	// sync1
	altLens[2] = fgetw(file) * TAPCPUNS / TAPTICKNS;	// sync2
	altLens[3] = fgetw(file) * TAPCPUNS / TAPTICKNS;	// 0
	altLens[4] = fgetw(file) * TAPCPUNS / TAPTICKNS;	// 1
	altLens[6] = fgetw(file);			// pilot pulses
	int bits = fgetc(file);				// TODO: used bits in last byte
	// printf("bits:%i\n",bits);
	int pause = fgetw(file) * 1e6 / TAPTPS;
	int len = fgett(file);		// freadLen(file, 3);
	char* buf = (char*)malloc(len);
	fread(buf, len-1, 1, file);
	tape->tmpBlock = tapDataToBlock(buf, len-1, altLens);
	int data = fgetc(file);		// last byte
	if (bits > 8) bits = 8;
	if (bits != 8) tape->isData = 0;
	while (bits > 0) {
		blkAddWave(&tape->tmpBlock, (data & 0x80) ? altLens[4] : altLens[3]);
		bits--;
		data <<= 1;
	}
	blkAddPause(&tape->tmpBlock, pause);
	tap_add_block(tape, tape->tmpBlock);
	blkClear(&tape->tmpBlock);
	free(buf);
}

// #12: <pulselen:2>,<count:2>
void tzxBlock12(FILE* file, Tape* tape) {
	int len = fgetw(file) * TAPCPUNS / TAPTICKNS;	// Length of one pulse in T-states -> mks
	int count = fgetw(file);		// Number of pulses
	while (count > 0) {
		blkAddPulse(&tape->tmpBlock, len, -1);
		count--;
	}
	tape->isData = 0;
}

// #13: <count:1>,{len:2*count}		pulse seq
void tzxBlock13(FILE* file, Tape* tape) {
	int len;
	int count = fgetc(file);
	while (count > 0) {
		len = fgetw(file) * TAPCPUNS / TAPTICKNS;
		blkAddPulse(&tape->tmpBlock, len, -1);
		count--;
	}
	tape->isData = 0;
}

// #14: <bit0:2>,<bit1:2>,<usedbits:1>,<pause:2>,<len:3>,{data:len}
void tzxBlock14(FILE* file, Tape* tape) {
	int bit0 = fgetw(file) * TAPCPUNS / TAPTICKNS;
	int bit1 = fgetw(file) * TAPCPUNS / TAPTICKNS;
	int data;
	int bits = fgetc(file);			// used bits in last byte
	// printf("%i : %i : last bits %i\n",bit0,bit1,bits);
	int pausems = fgetw(file);
	int pause = pausems * 1e6 / TAPTPS;	// ms -> mks
	int len = fgett(file);
	while (len > 1) {
		data = fgetc(file);
		blkAddByte(&tape->tmpBlock, data & 0xff, bit0, bit1);
		len--;
	}
	// last byte
	data = fgetc(file);
	if (bits > 8) bits = 8;
	while (bits > 0) {
		blkAddWave(&tape->tmpBlock, (data & 0x80) ? bit1 : bit0);
		bits--;
		data <<= 1;
	}
	if (pausems > 100) {		// .1 sec will be block separator
		blkAddPause(&tape->tmpBlock, pause);
		tap_add_block(tape, tape->tmpBlock);
		blkClear(&tape->tmpBlock);
	}
	tape->isData = 0;
}

// #15,<step:2>,<pause:2>,<last:1>,<len:3>,{data:len}
// direct recording
void tzxBlock15(FILE* file, Tape* tape) {
	int size = fgetw(file) * TAPCPUNS / TAPTICKNS;
	int pausems = fgetw(file);
	int pause = pausems * 1e6 / TAPTICKNS;
	int last = fgetc(file) & 7;
	int len = (fgett(file) - 1) * 8 + last;		// bits
	int data = 0;
	int cnt = 0;
	int first = 1;
	int tmp = 0;
	int memt = 0;
	for (cnt = 0; cnt < len; cnt++) {
		if ((cnt & 7) == 0)
			data = fgetc(file) & 0xff;
		if ((tmp ^ data) & 0x80) {	// bit changed
			if (first) {
				memt += size;
				first = 0;
			} else {
				blkAddPulse(&tape->tmpBlock, memt, -1);
				memt = 0;
			}
		} else {
			memt += size;
		}
		tmp = data;
		data <<= 1;
	}
	if (memt > 0)
		blkAddPulse(&tape->tmpBlock, memt, -1);
	blkAddPause(&tape->tmpBlock, pause);
	tape->isData = 0;
}

// #16: c64 block data	TODO
// #17: c64 turbo block data	TODO

// #20,<len:2> : pause or stop tape
void tzxBlock20(FILE* file, Tape* tape) {
	int len = fgetw(file) * 1000 / TAPTICKNS;
	if (len) {
		blkAddPause(&tape->tmpBlock, len);
	} else {
		tape->tmpBlock.breakPoint = 1;
	}
}

// #21,<len:1>,{text:len}		group start
void tzxBlock21(FILE* file, Tape* tape) {
	int len = fgetc(file);
	char* buf = (char*)malloc(len + 1);
	fread(buf, len, 1, file);
	buf[len] = 0;
	// printf("%s\n", buf);
	free(buf);
}

// #22					group end
void tzxBlock22(FILE* file, Tape* tape) {
}

// #23,<offset:2>			TODO: jump to block
void tzxBlock23(FILE* file, Tape* tape) {
	fseek(file, 2, SEEK_CUR);
}

static unsigned short loopCount = 0;
static long loopPos = 0;

// #24,<count>				loop start
void tzxBlock24(FILE* file, Tape* tape) {
	loopCount = fgetw(file);
	loopPos = ftell(file);
}

// #25					loop end
void tzxBlock25(FILE* file, Tape* tape) {
	if (loopCount > 1) {
		loopCount--;
		fseek(file, loopPos, SEEK_SET);
	}
}

// #26,<len:2>,{offsets:2}		TODO: call sequence
void tzxBlock26(FILE* file, Tape* tape) {
	int len = fgetw(file);
	fseek(file, len * 2, SEEK_CUR);
}

// #27					return
void tzxBlock27(FILE* file, Tape* tape) {
}

// #28,<len:2>,<count:2>,{selectblock}	select block
void tzxBlock28(FILE* file, Tape* tape) {
	int len = fgetw(file);
	fseek(file, len, SEEK_CUR);
}

// #2A,<len:2>				stop if 48K
void tzxBlock2A(FILE* file, Tape* tape) {
	fseek(file, 2, SEEK_CUR);
}

// #2B,<len:2>,<lev:1>			set signal level
void tzxBlock2B(FILE* file, Tape* tape) {
	fseek(file, 2, SEEK_CUR);
	int len = fgetc(file);
	tape->levRec = len ? 1 : 0;
	tape->oldRec = tape->levRec;
}

// #30,<len:1>,<text:len>		description
void tzxBlock30(FILE* file, Tape* tape) {
	int len = fgetc(file);
	fseek(file, len, SEEK_CUR);
}

// #31,<time:1>,<len:2>,<text:len>	show message for <time> sec
void tzxBlock31(FILE* file, Tape* tape) {
	fgetc(file);
	int len = fgetc(file);
	fseek(file, len, SEEK_CUR);
}

// #32 : archive info
void tzxBlock32(FILE* file, Tape* tape) {
	int len = fgetw(file);
	fseek(file, len, SEEK_CUR);
}

// #33 : hardware type
void tzxBlock33(FILE* file, Tape* tape) {
	int len = fgetc(file);
	fseek(file, len * 3, SEEK_CUR);
}

// #34 : emulation info	TODO

// #35 : custom info block
void tzxBlock35(FILE* file, Tape* tape) {
	fseek(file, 10, SEEK_CUR);
	int len = fgeti(file);
	fseek(file, len, SEEK_CUR);
}

// #40,<type:1>,<len:3>,<data:len>	snapshot	TODO
// type: 00:z80, 01:sna

void tzxBlock5A(FILE* file, Tape* tape) {
	fseek(file, 9, SEEK_CUR);
}

// unknown block: id,<len>,{data...}
void tzxBlockXX(FILE* file, Tape* tape) {
	int len = fgeti(file);
	fseek(file, len, SEEK_CUR);
}

tzxBCall tzxBlockTab[] = {
	{0x10, tzxBlock10},
	{0x11, tzxBlock11},
	{0x12, tzxBlock12},
	{0x13, tzxBlock13},
	{0x14, tzxBlock14},
	{0x15, tzxBlockXX},
	{0x18, tzxBlockXX},
	{0x19, tzxBlockXX},
	{0x20, tzxBlock20},
	{0x21, tzxBlock21},
	{0x22, tzxBlock22},
	{0x23, tzxBlock23},
	{0x24, tzxBlock24},
	{0x25, tzxBlock25},
	{0x26, tzxBlock26},
	{0x27, tzxBlock27},
	{0x28, tzxBlock28},
	{0x2a, tzxBlock2A},
	{0x2b, tzxBlock2B},
	{0x30, tzxBlock30},
	{0x31, tzxBlock31},
	{0x32, tzxBlock32},
	{0x33, tzxBlock33},
	{0x35, tzxBlock35},
	{0x5a, tzxBlock5A},

	{0xff, tzxBlockXX}
};


int loadTZX(Computer* comp, const char* name, int drv) {
	Tape* tape = comp->tape;
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;

	unsigned char type;
	int i;
	int err = ERR_OK;

	tzxHead hd;
	fread((char*)&hd, sizeof(tzxHead), 1, file);
	if ((strncmp(hd.sign, "ZXTape!",7) != 0) || (hd.eot != 0x1a)) {
		err = ERR_TZX_SIGN;
	} else {

		tapEject(tape);
		tape->isData = 1;

		while (!feof(file)) {
			type = fgetc(file);		// block type (will be FF @ eof)
//			printf("block %.2X @ %.6X\n",type, ftell(file) - 1);
			if (feof(file)) break;		// is it the end?
			i = 0;
			while((tzxBlockTab[i].id != 0xff) && (tzxBlockTab[i].id != type))
				i++;
			tzxBlockTab[i].callback(file, tape);
		}
		if (comp->tape->tmpBlock.sigCount > 0)
			tap_add_block(comp->tape, comp->tape->tmpBlock);
		tape_set_path(tape, name);
	}
	fclose(file);
	return err;
}
