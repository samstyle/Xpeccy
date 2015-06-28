#include <stdlib.h>
#include "filetypes.h"

#pragma pack (push, 1)

typedef struct {
	char sign[7];
	char eot;
	char major;
	char minor;
} tzxHead;

#pragma pack(pop)

typedef struct {
	unsigned char id;
	void(*callback)(FILE*, Tape*);
} tzxBCall;

int sigLens[] = {PILOTLEN,SYNC1LEN,SYNC2LEN,SIGN0LEN,SIGN1LEN,SYNC3LEN,-1};

// #10: <pause:2>,<datalen:2>,{data}
void tzxBlock10(FILE* file, Tape* tape) {
	int pause = fgetwLE(file);
	int len = fgetwLE(file);
	char* buf = (char*)malloc(len);
	fread(buf, len, 1, file);
	tape->tmpBlock = tapDataToBlock(buf, len, sigLens);
	blkAddPulse(&tape->tmpBlock, sigLens[5]);
	blkAddPause(&tape->tmpBlock, pause);
	tapAddBlock(tape, tape->tmpBlock);
	blkClear(&tape->tmpBlock);
	free(buf);
}

// #11: <pilot:2>,<sync1:2>,<sync2:2>,<bit0:2>,<bit1:2>,<pilotcnt:2>,<pause:2>,<datalen:3>,{data}
void tzxBlock11(FILE* file, Tape* tape) {
	int altLens[7];
	altLens[0] = fgetwLE(file);	// pilot
	altLens[1] = fgetwLE(file);	// sync1
	altLens[2] = fgetwLE(file);	// sync2
	altLens[3] = fgetwLE(file);	// 0
	altLens[4] = fgetwLE(file);	// 1
	altLens[6] = fgetwLE(file);	// pilot pulses
	fgetc(file);			// TODO: used bits in last byte
	int pause = fgetwLE(file);
	int len = freadLen(file, 3);
	char* buf = (char*)malloc(len);
	fread(buf, len, 1, file);
	tape->tmpBlock = tapDataToBlock(buf, len, altLens);
	blkAddPulse(&tape->tmpBlock, sigLens[5]);
	blkAddPause(&tape->tmpBlock, pause);
	tapAddBlock(tape, tape->tmpBlock);
	blkClear(&tape->tmpBlock);
	free(buf);
}

// #12: <pulselen:2>,<count:2>
void tzxBlock12(FILE* file, Tape* tape) {
	int len = fgetwLE(file);	// Length of one pulse in T-states
	int count = fgetwLE(file);	// Number of pulses
	while (count > 0) {
		blkAddWave(&tape->tmpBlock, len);	// pulse is 1+0 : 2 signals
		count--;
	}
	tapAddBlock(tape, tape->tmpBlock);
	blkClear(&tape->tmpBlock);
	tape->isData = 0;
}

// #13: <count:2>,{len:2}		pulse seq
void tzxBlock13(FILE* file, Tape* tape) {
	int len;
	int count = fgetc(file);
	while (count > 0) {
		len = fgetwLE(file);
		blkAddWave(&tape->tmpBlock, len);
		count--;
	}
	tapAddBlock(tape, tape->tmpBlock);
	blkClear(&tape->tmpBlock);
	tape->isData = 0;
}

// #14: <bit0:2>,<bit1:2>,<usedbits:1>,<pause:2>,<len:3>,{data:len}
void tzxBlock14(FILE* file, Tape* tape) {
	int bit0 = fgetwLE(file);
	int bit1 = fgetwLE(file);
	int data;
	fgetc(file);
	int pause = fgetwLE(file);
	int len = freadLen(file, 3);
	while (len > 0) {
		data = fgetc(file);
		blkAddByte(&tape->tmpBlock, data & 0xff, bit0, bit1);
		len--;
	}
	blkAddPause(&tape->tmpBlock, pause);
	tapAddBlock(tape, tape->tmpBlock);
	blkClear(&tape->tmpBlock);
}

// #15: TODO: direct recording
void tzxBlock15(FILE* file, Tape* tape) {
	fseek(file, 5, SEEK_CUR);
	int len = freadLen(file, 3);
	fseek(file, len, SEEK_CUR);
}

// #20,<len:2> : pause or stop tape
void tzxBlock20(FILE* file, Tape* tape) {
	int len = fgetwLE(file);
	if (len) {
		blkAddPause(&tape->tmpBlock, len);	// TODO: if 0, next block must be breakpoint
	} else {
		tape->tmpBlock.breakPoint = 1;
	}
}

// #21,<len:1>,{text:len}		group start
void tzxBlock21(FILE* file, Tape* tape) {
	int len = fgetc(file);
	fseek(file, len, SEEK_CUR);
}

// #22					group end
void tzxBlock22(FILE* file, Tape* tape) {
}

// #23,<offset:2>			TODO: jump to block
void tzxBlock23(FILE* file, Tape* tape) {
	fseek(file, 2, SEEK_CUR);
}

unsigned short loopCount = 0;
size_t loopPos = 0;

// #24,<count>				loop start
void tzxBlock24(FILE* file, Tape* tape) {
	loopCount = fgetwLE(file);
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
	int len = fgetwLE(file);
	fseek(file, len * 2, SEEK_CUR);
}

// #27					return
void tzxBlock27(FILE* file, Tape* tape) {
}

// #28,<len:2>,<count:2>,{selectblock}	select block
void tzxBlock28(FILE* file, Tape* tape) {
	int len = fgetwLE(file);
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

void tzxBlock30(FILE* file, Tape* tape) {
	int len = fgetc(file);
	fseek(file, len, SEEK_CUR);
}

void tzxBlock31(FILE* file, Tape* tape) {
	fgetc(file);
	int len = fgetc(file);
	fseek(file, len, SEEK_CUR);
}

void tzxBlock32(FILE* file, Tape* tape) {
	int len = fgetwLE(file);
	fseek(file, len, SEEK_CUR);
}

void tzxBlock33(FILE* file, Tape* tape) {
	int len = fgetc(file);
	fseek(file, len * 3, SEEK_CUR);
}

void tzxBlock35(FILE* file, Tape* tape) {
	fseek(file, 10, SEEK_CUR);
	int len = freadLen(file, 4);
	fseek(file, len, SEEK_CUR);
}

void tzxBlock5A(FILE* file, Tape* tape) {
	fseek(file, 9, SEEK_CUR);
}

// unknown block: id,<len>,{data...}
void tzxBlockXX(FILE* file, Tape* tape) {
	int len = freadLen(file,4);
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


int loadTZX(Tape* tape, const char* name) {
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
			if (feof(file)) break;		// is it the end?
			i = 0;
			while(i < 256) {
				if ((tzxBlockTab[i].id == 0xff) || (tzxBlockTab[i].id == type)) {
					tzxBlockTab[i].callback(file, tape);
					i = 256;
				}
				i++;
			}
		}
		tape->path = (char*)realloc(tape->path,sizeof(char) * (strlen(name) + 1));
		strcpy(tape->path,name);
	}
	fclose(file);
	return err;
}

/*

		switch (tmp) {
			case 0x10:
				paulen = fgetwLE(file);
				len = fgetwLE(file);
				blockBuf = (char*)realloc(blockBuf, len);
				fread(blockBuf, len, 1, file);
				block = tapDataToBlock(blockBuf, len, sigLens);
				blkAddPulse(&block, sigLens[5]);
				blkAddPause(&block, paulen);
				tapAddBlock(tape, block);
				blkClear(&block);
				break;
			case 0x11:
				altLens[0] = fgetwLE(file);	// pilot
				altLens[1] = fgetwLE(file);	// sync1
				altLens[2] = fgetwLE(file);	// sync2
				altLens[3] = fgetwLE(file);	// 0
				altLens[4] = fgetwLE(file);	// 1
				altLens[6] = fgetwLE(file);	// pilot pulses
				fgetc(file);				// TODO: used bits in last byte
				paulen = fgetwLE(file);
				len = freadLen(file, 3);
				blockBuf = (char*)realloc(blockBuf, len);
				fread(blockBuf, len, 1, file);
				block = tapDataToBlock(blockBuf, len, altLens);
				blkAddPulse(&block, sigLens[5]);
				blkAddPause(&block, paulen);
				tapAddBlock(tape, block);
				blkClear(&block);
				break;
			case 0x12:				// pure tone
				paulen = fgetwLE(file);		// Length of one pulse in T-states
				len = fgetwLE(file);		// Number of pulses
				while (len > 0) {
					blkAddWave(&block, paulen);	// pulse is 1+0 : 2 signals
					len--;
				}
				tape->isData = 0;
				break;
			case 0x13:			// pulses sequence
				len = fgetc(file);
				while (len > 0) {
					paulen = fgetwLE(file);
					blkAddWave(&block, paulen);
					len--;
				}
				tape->isData = 0;
				break;
			case 0x14:
				altLens[0] = 0;
				altLens[1] = 0;
				altLens[2] = 0;
				altLens[5] = 0;
				altLens[6] = 0;			// no pilot, syncs
				altLens[3] = fgetwLE(file);	// bit 0
				altLens[4] = fgetwLE(file);	// bit 1
				fgetc(file);			// TODO : used bits in last byte
				paulen = fgetwLE(file);
				len = freadLen(file, 3);
				blockBuf = (char*)realloc(blockBuf, len);
				fread(blockBuf, len, 1, file);
				altBlock = tapDataToBlock(blockBuf, len, altLens);
				block.len0 = altBlock.len0;
				block.len1 = altBlock.len1;
				block.dataPos = -1;
				for (i = altBlock.dataPos; i < altBlock.sigCount; i++) {
					blkAddPulse(&block,altBlock.sigData[i]);
				}
				blkAddPulse(&block, SYNC3LEN);
				blkAddPause(&block, paulen);
				tape->isData = 0;
				break;
//==
			case 0x15:
				file.seekg(5,std::ios::cur);
				len = getlen(&file,3);
				file.seekg(len,std::ios::cur);
				break;
			case 0x18:
				len = getlen(&file,4);
				file.seekg(len,std::ios::cur);
				break;
			case 0x19:
				len = getlen(&file,4);
				file.seekg(len,std::ios::cur);
				break;
//==
			case 0x20:
				len = fgetwLE(file);
				blkAddPulse(&block, len);		// TODO: if 0, next block must be breakpoint
				break;
			case 0x21:
				len = fgetc(file);
				fseek(file, len, SEEK_CUR);
				break;
			case 0x22:
				if (block.sigCount != 0) {
					blkAddPulse(&block,sigLens[5]);
					block.hasBytes = 0;
					tapAddBlock(tape,block);
					blkClear(&block);
				}
				break;
			case 0x23:
				fseek(file, 2, SEEK_CUR);
				break;
			case 0x24:
				loopc = fgetwLE(file);
				loopos = ftell(file);
				break;
			case 0x25:
				if (loopc > 0) {
					loopc--;
					if (loopc != 0) fseek(file, loopos, SEEK_SET);
				}
				break;
//			case 0x26:			// TODO: block calls sequence
//				len = getlen(&file,2);
//				file.seekg(len << 1,std::ios::cur);
//				break;
			case 0x27:
				break;
//			case 0x28:			// TODO: select blocks
//				len = getlen(&file,2);
//				file.seekg(len<<1,std::ios::cur);
//				break;
			case 0x2a:
				fseek(file, 4, SEEK_CUR);
				break;
			case 0x2b:
				fseek(file, 5, SEEK_CUR);
				break;
			case 0x30:
				len = fgetc(file);
				fseek(file, len, SEEK_CUR);
				break;
			case 0x31:
				fgetc(file);
				len = fgetc(file);
				fseek(file, len, SEEK_CUR);
				break;
			case 0x32:
				len = fgetwLE(file);
				fseek(file, len, SEEK_CUR);
				break;
			case 0x33:
				len = fgetc(file);
				fseek(file, len * 3, SEEK_CUR);
				break;
			case 0x34:					// ???
				fseek(file, 4, SEEK_CUR);
				len = freadLen(file, 4);
				fseek(file, len, SEEK_CUR);
				break;
			case 0x5a:
				fseek(file, 9, SEEK_CUR);
				break;
			default:
				printf("TZX unsupported block: %.2X\n",tmp);
				return ERR_TZX_UNKNOWN;
		}
	}
	if (block.sigCount != 0) {
//		blkAddSignal(&block,sigLens[5]);		// FIXME: not really
		block.hasBytes = 0;
		tapAddBlock(tape,block);
		blkClear(&block);
	}
	if (blockBuf) free(blockBuf);
	return ERR_OK;
}
*/
