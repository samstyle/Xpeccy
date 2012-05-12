#include <stdlib.h>
#include "filetypes.h"

TapeBlock tapDataToBlock(char* data,int len,int* sigLens) {
	TapeBlock block;
	int i;
	char* ptr = data;
	char tmp = data[0];		// block type
	block.plen = sigLens[0];
	block.s1len = sigLens[1];
	block.s2len = sigLens[2];
	block.len0 = sigLens[3];
	block.len1 = sigLens[4];
	block.pdur = (sigLens[6] == -1) ? ((tmp == 0) ? 8063 : 3223) : sigLens[6];
	block.flag = TBF_BYTES;
	if (tmp == 0) block.flag |= TBF_HEAD;
	block.sigCount = 0;
	block.sigData = NULL;
	//block.data.clear();
	for (i = 0; i < (int)block.pdur; i++)
		blkAddSignal(&block,block.plen);
	if (block.s1len != 0)
		blkAddSignal(&block,block.s1len);
	if (block.s2len != 0)
		blkAddSignal(&block,block.s2len);
	block.dataPos = block.sigCount;
	for (i = 0; i < len; i++) {
		addBlockByte(&block,*ptr);
		ptr++;
	}
	return block;
}

int loadTAP(Tape* tape, const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	uint16_t len;
	char* blockBuf = NULL;
	TapeBlock block;
	int sigLens[] = {PILOTLEN,SYNC1LEN,SYNC2LEN,SIGN0LEN,SIGN1LEN,SYNC3LEN,-1};
	tapEject(tape);
	while (!file.eof()) {
		len = getLEWord(&file);
		if (!file.eof()) {
			blockBuf = (char*)realloc(blockBuf,len * sizeof(char));
			file.read(blockBuf,len);
			block = tapDataToBlock(blockBuf,len,sigLens);
			block.pause = (block.pdur == 8063) ? 500 : 1000;
			blkAddSignal(&block,sigLens[5]);
			tapAddBlock(tape,block);
		}
	}
	tape->flag |= TAPE_CANSAVE;
	tape->path = (char*)realloc(tape->path,sizeof(char) * (strlen(name) + 1));
	strcpy(tape->path,name);
	if (blockBuf != NULL) free(blockBuf);
	return ERR_OK;
}

int saveTAP(Tape* tape, const char* name) {
	if (!(tape->flag & TAPE_CANSAVE)) return ERR_TAP_DATA;
	std::ofstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;

	TapeBlockInfo inf;
	uint8_t* blockData = NULL;

	for (int i = 0; i < tape->blkCount; i++) {
		inf = tapGetBlockInfo(tape, i);
		inf.size += 2;	// +flag +crc
		blockData = (uint8_t*)realloc(blockData,inf.size);
		tapGetBlockData(tape,i,blockData);
		file.put(inf.size & 0xff);
		file.put((inf.size & 0xff00) >> 8);
		file.write((char*)blockData,inf.size);
	}
	tape->path = (char*)realloc(tape->path,sizeof(char) * (strlen(name) + 1));
	strcpy(tape->path,name);
	if (blockData) free(blockData);

	return ERR_OK;
}
