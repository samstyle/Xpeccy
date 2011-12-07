#include <stdlib.h>
#include "filetypes.h"

void tapAddByte(TapeBlock* block, char val) {
	for (int i = 0; i < 8; i++) {
		if (val & 0x80) {
			block->data.push_back(block->len1);
			block->data.push_back(block->len1);
		} else {
			block->data.push_back(block->len0);
			block->data.push_back(block->len0);
		}
		val = (val << 1);
	}
}

TapeBlock tapDataToBlock(char* data,int len,int* sigLens) {
	TapeBlock block;
	int i;
	char* ptr = data;
	char tmp = *(data);		// block type
	block.plen = sigLens[0];
	block.s1len = sigLens[1];
	block.s2len = sigLens[2];
	block.len0 = sigLens[3];
	block.len1 = sigLens[4];
	block.pdur = (sigLens[6] == -1) ? ((tmp == 0) ? 8063 : 3223) : sigLens[6];
	if (tmp == 0) {
		block.flags |= TBF_HEAD;
	} else {
		block.flags &= ~TBF_HEAD;
	}
	block.data.clear();
	for (i = 0; i < (int)block.pdur; i++)
		block.data.push_back(block.plen);
	if (block.s1len != 0)
		block.data.push_back(block.s1len);
	if (block.s2len != 0)
		block.data.push_back(block.s2len);
	block.dataPos = block.data.size();
	for (i = 0; i < len; i++) {
		tapAddByte(&block,*ptr);
		ptr++;
	}
	block.flags |= TBF_BYTES;
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
			block.data.push_back(sigLens[5]);
			tapAddBlock(tape,block);
		}
	}
	tapSetFlag(tape,TAPE_CANSAVE,true);
	tapSetPath(tape,name);
	if (blockBuf != NULL) free(blockBuf);
	return ERR_OK;
}

int saveTAP(Tape* tape, const char* name) {
	if (!(tapGet(tape,TAPE_FLAGS) & TAPE_CANSAVE)) return ERR_TAP_DATA;
	std::ofstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	
	std::vector<uint8_t> blockData;
	uint32_t len;
	
	for (int i = 0; i < tapGet(tape,TAPE_BLOCKS); i++) {
		blockData = tapGetBlockData(tape,i);
		len = blockData.size();
		file.put(len & 0xff);
		file.put((len & 0xff00) >> 8);
		file.write((char*)&blockData[0],blockData.size());
	}
	tapSetPath(tape,name);

	return ERR_OK;
}