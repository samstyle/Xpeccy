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

int loadTZX(Tape* tape, const char* name) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;

	int sigLens[] = {PILOTLEN,SYNC1LEN,SYNC2LEN,SIGN0LEN,SIGN1LEN,SYNC3LEN,-1};
	int altLens[] = {PILOTLEN,SYNC1LEN,SYNC2LEN,SIGN0LEN,SIGN1LEN,SYNC3LEN,-1};
	//unsigned char buf[256];
	unsigned char tmp;
	unsigned int paulen,len;
	int i;
	unsigned int loopc = 0;
	TapeBlock block,altBlock;
	char* blockBuf = NULL;
	size_t loopos = 0;

	tzxHead hd;
	fread((char*)&hd, sizeof(tzxHead), 1, file);
	if ((strncmp(hd.sign, "ZXTape!",7) != 0) || (hd.eot != 0x1a)) return ERR_TZX_SIGN;

	tapEject(tape);
	tape->path = (char*)realloc(tape->path,sizeof(char) * (strlen(name) + 1));
	strcpy(tape->path,name);
	tape->isData = 1;

	while (!feof(file)) {
		tmp = fgetc(file);		// block type (will be FF @ eof)
		if (feof(file)) break;		// is it the end?
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
/*
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
*/
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
