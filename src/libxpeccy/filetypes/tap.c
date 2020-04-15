#include <stdlib.h>
#include "filetypes.h"

static int zx_std_siglens[] = {PILOTLEN,SYNC1LEN,SYNC2LEN,SIGN0LEN,SIGN1LEN,0,-1};		// 0->SYNC3LEN

void blkFromData(TapeBlock* blk, char* data, int len, int* sigLens) {
}

TapeBlock tapDataToBlock(char* data,int len,int* sigLens) {
	TapeBlock blk;
	blk.data = NULL;
	blkClear(&blk);
	int i;
	char* ptr = data;
	char tmp = data[0] & 0x80;	// block type (00:head, 80:data)
	if (sigLens == NULL)
		sigLens = zx_std_siglens;
	blk.plen = sigLens[0];
	blk.s1len = sigLens[1];
	blk.s2len = sigLens[2];
	blk.len0 = sigLens[3];
	blk.len1 = sigLens[4];
	blk.pdur = (sigLens[6] == -1) ? ((tmp == 0) ? 8063 : 3223) : sigLens[6];
	blk.breakPoint = 0;
	blk.hasBytes = 1;
	blk.isHeader = (tmp == 0) ? 1 : 0;
	for (i = 0; i < (int)blk.pdur; i++)
		blkAddPulse(&blk, blk.plen,-1);
	if (blk.s1len)
		blkAddPulse(&blk, blk.s1len,-1);
	if (blk.s2len)
		blkAddPulse(&blk, blk.s2len,-1);
	blk.dataPos = blk.sigCount;
	for (i = 0; i < len; i++) {
		blkAddByte(&blk,*ptr,0,0);
		ptr++;
	}
	return blk;
}

int loadTAP(Computer* comp, const char* name, int drv) {
	Tape* tape = comp->tape;
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;

	unsigned short len;
	char blockBuf[0x10000];
	TapeBlock block;
	tapEject(tape);

	while (!feof(file)) {
		len = fgetw(file);
		if (!feof(file)) {
			fread(blockBuf, len, 1, file);
			block = tapDataToBlock(blockBuf, len, NULL);
			blkAddPause(&block, 1e6);
			tapAddBlock(tape, block);
			blkClear(&block);
		}
	}
	fclose(file);

	tape->isData = 1;
	tape_set_path(tape, name);
	strcpy(tape->path,name);

	return ERR_OK;
}

int saveTAP(Computer* comp, const char* name, int drv) {
	Tape* tape = comp->tape;
	if (!tape->isData) return ERR_TAP_DATA;
	FILE* file = fopen(name, "wb");
	if (!file) return ERR_CANT_OPEN;

	TapeBlockInfo inf;
	unsigned char blockData[0x100000];

	for (int i = 0; i < tape->blkCount; i++) {
		inf = tapGetBlockInfo(tape, i, TFRM_ZX);
		inf.size += 2;	// +flag +crc
		tapGetBlockData(tape, i, blockData,inf.size);
		fputwLE(file, inf.size);
		fwrite((char*)blockData, inf.size, 1, file);
	}
	fclose(file);

	tape_set_path(tape, name);

	return ERR_OK;
}
