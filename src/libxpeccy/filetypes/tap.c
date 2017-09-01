#include <stdlib.h>
#include "filetypes.h"

void blkFromData(TapeBlock* blk, char* data, int len, int* sigLens) {
	int i;
	char* ptr = data;
	char tmp = data[0];		// block type (00:head, xx:data)
	blkClear(blk);
	blk->plen = sigLens[0];
	blk->s1len = sigLens[1];
	blk->s2len = sigLens[2];
	blk->len0 = sigLens[3];
	blk->len1 = sigLens[4];
	blk->pdur = (sigLens[6] == -1) ? ((tmp == 0) ? 8063 : 3223) : sigLens[6];
	blk->breakPoint = 0;
	blk->hasBytes = 1;
	blk->isHeader = (tmp == 0) ? 1 : 0;
	for (i = 0; i < (int)blk->pdur; i++)
		blkAddPulse(blk, blk->plen);
	if (blk->s1len)
		blkAddPulse(blk, blk->s1len);
	if (blk->s2len)
		blkAddPulse(blk, blk->s2len);
	blk->dataPos = blk->sigCount;
	for (i = 0; i < len; i++) {
		blkAddByte(blk,*ptr,0,0);
		ptr++;
	}
}

TapeBlock tapDataToBlock(char* data,int len,int* sigLens) {
	TapeBlock block;
	block.sigCount = 0;
	block.sigData = NULL;
	blkFromData(&block, data, len, sigLens);
//	printf("tapDataToBlock: %i bytes -> %i signals\t datapos = %i\n",len,block.sigCount,block.dataPos);
	return block;
}

int loadTAP(Tape* tape, const char* name) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;

	unsigned short len;
	char blockBuf[0x10000];
	TapeBlock block;
	int sigLens[] = {PILOTLEN,SYNC1LEN,SYNC2LEN,SIGN0LEN,SIGN1LEN,SYNC3LEN,-1};
	tapEject(tape);

	while (!feof(file)) {
		len = fgetw(file);
		if (!feof(file)) {
			fread(blockBuf, len, 1, file);
			block = tapDataToBlock(blockBuf, len, sigLens);
			// blkAddPulse(&block, sigLens[5]);
			blkAddPause(&block, (block.pdur == 8063) ? 500 : 1000);		// pause
			tapAddBlock(tape, block);
			blkClear(&block);
		}
	}
	fclose(file);

	tape->isData = 1;
	tape->path = (char*)realloc(tape->path,sizeof(char) * (strlen(name) + 1));
	strcpy(tape->path,name);

	return ERR_OK;
}

int saveTAP(Tape* tape, const char* name) {
	if (!tape->isData) return ERR_TAP_DATA;
	FILE* file = fopen(name, "wb");
	if (!file) return ERR_CANT_OPEN;

	TapeBlockInfo inf;
	unsigned char blockData[0x100000];

	for (int i = 0; i < tape->blkCount; i++) {
		inf = tapGetBlockInfo(tape, i);
		inf.size += 2;	// +flag +crc
		tapGetBlockData(tape, i, blockData,inf.size);
		fputwLE(file, inf.size);
		fwrite((char*)blockData, inf.size, 1, file);
	}
	fclose(file);

	tape->path = (char*)realloc(tape->path,sizeof(char) * (strlen(name) + 1));
	strcpy(tape->path,name);

	return ERR_OK;
}
