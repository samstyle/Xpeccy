#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tape.h"

#define	FRAMEDOTS	71680
#define SECDOTS		(FRAMEDOTS * 50)
#define	MSDOTS		(FRAMEDOTS / 20)

// NEW STUFF

struct Tape {
	int flags;
	int block;
	int pos;
	bool signal;
	bool toutold;
	bool outsig;
	int sigLen;
	double sigCount;
	std::string path;
	TapeBlock tmpBlock;
	std::vector<TapeBlock> data;
};

Tape* tapCreate() {
	Tape* tap = new Tape;
	tap->flags = 0;
	tap->block = 0;
	tap->pos = 0;
	tap->sigLen = 0;
	tap->signal = false;
	tap->sigLen = 0;
	tap->sigCount = 0;
	tap->toutold = false;
	tap->outsig = false;
	tap->path = "";
	return tap;
}

void tapDestroy(Tape* tap) {
	delete(tap);
}

// blocks

int tapGetBlockTime(TapeBlock* block, int pos) {
	long totsz = 0;
	if (pos == -1) pos = block->data.size();
	for (int i = 0; i < pos; i++) totsz += block->data[i];
	return (totsz / SECDOTS);
}

int tapGetBlockSize(TapeBlock* block) {
	return (((block->data.size() - block->dataPos) >> 4) - 2);
}

uint8_t tapGetBlockByte(TapeBlock* block, int bytePos) {
	uint8_t res = 0x00;
	int sigPos = block->dataPos + (bytePos << 4);
	for (int i = 0; i < 8; i++) {
		res = res << 1;
		if (sigPos < (int)(block->data.size() - 1)) {
			if ((block->data[sigPos] == block->len1) && (block->data[sigPos + 1] == block->len1)) {
				res |= 1;
			}
			sigPos += 2;
		}
	}
	return res;
}

std::vector<uint8_t> tapGetBlockData(Tape* tape, int blockNum) {
	std::vector<uint8_t> res;
	TapeBlock* block = &tape->data[blockNum];
	uint32_t pos = block->dataPos;
	int bytePos = 0;
	do {
		res.push_back(tapGetBlockByte(block,bytePos));
		bytePos++;
		pos += 16;
	} while (pos < (block->data.size() - 1));
	return res;
}

std::string tapGetBlockHeader(Tape* tap, int blk) {
	std::string res;
	TapeBlock* block = &tap->data[blk];
	int i;
	if (block->flags & TBF_HEAD) {
		if (tapGetBlockByte(block, 1)==0x00) res = "Prog:"; else res = "Code:";
		for(i = 2; i < 12; i++) {
			res += tapGetBlockByte(block,i);
		}
	}
	return res;
}

TapeBlockInfo tapGetBlockInfo(Tape* tap, int blk) {
	TapeBlock* block = &tap->data[blk];
	TapeBlockInfo inf;
	inf.name = tapGetBlockHeader(tap,blk);
	inf.type = (inf.name == "") ? TAPE_DATA : TAPE_HEAD;
	inf.size = tapGetBlockSize(block);
	inf.time = tapGetBlockTime(block,-1);
	inf.curtime = (tap->block == blk) ? tapGetBlockTime(block,tap->pos) : -1;
	inf.flags = block->flags;
	return inf;
}

std::vector<TapeBlockInfo> tapGetBlocksInfo(Tape* tap) {
	std::vector<TapeBlockInfo> res;
	TapeBlockInfo inf;
	for (int i=0; i < (int)tap->data.size(); i++) {
		inf = tapGetBlockInfo(tap,i);
		res.push_back(inf);
	}
	return res;
}

void tapNormSignals(TapeBlock* block) {
	int low,hi;
	for (int i=0; i < (int)block->data.size(); i++) {
		low = block->data[i] - 10;
		hi = block->data[i] + 10;
		if ((block->plen > low) && (block->plen < hi)) block->data[i] = block->plen;
		if ((block->s1len > low) && (block->s1len < hi)) block->data[i] = block->s1len;
		if ((block->s2len > low) && (block->s2len < hi)) block->data[i] = block->s2len;
		if ((block->len0 > low) && (block->len0 < hi)) block->data[i] = block->len0;
		if ((block->len1 > low) && (block->len1 < hi)) block->data[i] = block->len1;
	}
}

void tapSwapBlocks(Tape* tap, int b1, int b2) {
	if ((b1 < (int)tap->data.size()) && (b2 < (int)tap->data.size())) {
		TapeBlock tmp = tap->data[b1];
		tap->data[b1] = tap->data[b2];
		tap->data[b2] = tmp;
	}
}

void tapDelBlock(Tape* tap, int blk) {
	if (blk < (int)tap->data.size()) {
		tap->data.erase(tap->data.begin() + blk);
	}
}

// tape

void tapStoreBlock(Tape* tap) {
	if (tap->tmpBlock.data.size() > 0) {
		tap->tmpBlock.pause = 1000;
		tap->tmpBlock.data.pop_back();
	}
	if (tap->tmpBlock.data.size() == 0) return;
	uint32_t i,j;
	bool same;
	std::vector<uint32_t> siglens;
	for (i=0; i < tap->tmpBlock.data.size(); i++) {
		same = false;
		for (j = 0; j < siglens.size(); j++) {
			if ((unsigned)(tap->tmpBlock.data[i] - siglens[j] + 10) < 20) {
				siglens[j] = tap->tmpBlock.data[i];
				same = true;
			}
		}
		if (!same) siglens.push_back(tap->tmpBlock.data[i]);
	}
	
//	printf("size: %i\n",siglens.size());
//	for (i=0; i<siglens.size();i++) printf("\t%i",siglens[i]);
//	printf("\n");
	
	if (siglens.size() == 5) {
		if (siglens[3] > 1000) siglens.insert(siglens.begin() + 3, SIGN0LEN); else siglens.insert(siglens.begin() + 3, SIGN1LEN);
	}
	if (siglens.size() == 6) {
		tap->tmpBlock.plen = siglens[0];
		tap->tmpBlock.s1len = siglens[1];
		tap->tmpBlock.s2len = siglens[2];
		if (siglens[4] > siglens[3]) {
			tap->tmpBlock.len0 = siglens[3];
			tap->tmpBlock.len1 = siglens[4];
		} else {
			tap->tmpBlock.len0 = siglens[4];
			tap->tmpBlock.len1 = siglens[3];
		}
		tap->tmpBlock.flags |= TBF_BYTES;
	} else {
		tap->tmpBlock.plen = PILOTLEN;
		tap->tmpBlock.s1len = SYNC1LEN;
		tap->tmpBlock.s2len = SYNC2LEN;
		tap->tmpBlock.len0 = SIGN0LEN;
		tap->tmpBlock.len1 = SIGN1LEN;
	}
	tap->tmpBlock.dataPos = -1;
	i=1;
	while ((i < tap->tmpBlock.data.size()) && (tap->tmpBlock.data[i] != tap->tmpBlock.s2len)) i++;
	if (i < tap->tmpBlock.data.size()) tap->tmpBlock.dataPos = i + 1;
	tapNormSignals(&tap->tmpBlock);
	if (tap->tmpBlock.dataPos != -1) {
		if (tapGetBlockByte(&tap->tmpBlock,0) == 0) {
			tap->tmpBlock.flags |= TBF_HEAD;
		} else {
			tap->tmpBlock.flags &= ~TBF_HEAD;
		}
	}
	tap->data.push_back(tap->tmpBlock);
		
	tap->tmpBlock.data.clear();
	tap->flags |= TAPE_WAIT;
}

void tapEject(Tape* tap) {
	tap->flags &= ~(TAPE_ON | TAPE_REC);
	tap->block = 0;
	tap->pos = 0;
	tap->path = "";
	tap->data.clear();
}

void tapStop(Tape* tap) {
	if (tap->flags & TAPE_ON) {
		tap->flags &= ~TAPE_ON;
		if (tap->flags & TAPE_REC) tapStoreBlock(tap);
		tap->pos = 0;
	}
}

bool tapPlay(Tape* tap) {
	if (tap->block < (int)tap->data.size()) {
		tap->flags &= ~TAPE_REC;
		tap->flags |= TAPE_ON;
	}
	return (tap->flags & TAPE_ON);
}

void tapRec(Tape* tap) {
	tap->flags |= (TAPE_ON | TAPE_REC | TAPE_WAIT);
	tap->toutold = tap->outsig;
	tap->tmpBlock.data.clear();
}

void tapRewind(Tape* tap, int blk) {
	if (blk < (int)tap->data.size()) {
		tap->block = blk;
		tap->pos = 0;
	} else {
		tap->flags &= ~TAPE_ON;
	}
}

void tapSync(Tape* tap,int tks) {
	tap->sigCount += tks / 2.0;
	int tk = (int)tap->sigCount;
	tap->sigCount -= tk;
	if (tap->flags & TAPE_ON) {
		if (tap->flags & TAPE_REC) {
			if (tap->flags & TAPE_WAIT) {
				if (tap->toutold != tap->outsig) {
					tap->toutold = tap->outsig;
					tap->flags &= ~TAPE_WAIT;
					tap->tmpBlock.data.push_back(0);
				}
			} else {
				tap->tmpBlock.data.back() += tk;
				if (tap->toutold != tap->outsig) {
					tap->toutold = tap->outsig;
					tap->tmpBlock.data.push_back(0);
				}
				if (tap->tmpBlock.data.back() > FRAMEDOTS) tapStoreBlock(tap);
			}
		} else {
			tap->sigLen -= tk;
			while (tap->sigLen < 1) {
				tap->signal = !tap->signal;
				tap->sigLen += tap->data[tap->block].data[tap->pos];
				tap->pos++;
				if (tap->pos >= (int)tap->data[tap->block].data.size()) {
					tap->sigLen += tap->data[tap->block].pause * MSDOTS;
					tap->block++;
					tap->pos = 0;
					if (tap->block >= (int)tap->data.size()) {
						tap->flags &= ~TAPE_ON;
					} else {
						if (tap->data[tap->block].flags & TBF_BREAK) tap->flags &= ~TAPE_ON;
					}
				}
			}
		}
	} else {
		tap->sigLen -= tk;
		while (tap->sigLen < 1) {
			tap->signal = !tap->signal;
			tap->sigLen += FRAMEDOTS * 25;	// .5 sec
		}
	}
}

bool tapGetSignal(Tape* tap) {
	return tap->signal;
}

bool tapGetOutsig(Tape* tap) {
	return tap->outsig;
}

std::string tapGetPath(Tape* tap) {
	return tap->path;
}

void tapSetPath(Tape* tap,const char* nm) {
	tap->path = std::string(nm);
}

void tapSetSignal(Tape* tap, bool sig) {
	tap->outsig = sig;
}

int tapGet(Tape* tap, int wut) {
	int res = 0;
	switch (wut) {
		case TAPE_FLAGS: res = tap->flags; break;
		case TAPE_BLOCKS: res = tap->data.size(); break;
		case TAPE_BLOCK: res = tap->block; break;
	}
	return res;
}

int tapGet(Tape* tap, int blk, int wut) {
	int res = 0;
	switch (wut) {
		case TAPE_BFLAG: res = tap->data[blk].flags; break;
		case TAPE_BFTIME: res = tapGetBlockTime(&tap->data[blk],-1); break;
		case TAPE_BCTIME: res = tapGetBlockTime(&tap->data[blk],tap->pos); break;
	}
	return res;
}

void tapSet(Tape* tap, int blk, int wut, int val) {
	switch (wut) {
		case TAPE_BFLAG: tap->data[blk].flags = val; break;
	}
}

void tapSetFlag(Tape* tap, int mask, bool set) {
	if (set) {
		tap->flags |= mask;
	} else {
		tap->flags &= ~mask;
	}
}

// add file to tape

void addBlockByte(TapeBlock* blk, uint8_t bt) {
	for (int i=0; i < 8; i++) {
		if (bt & 0x80) {
			blk->data.push_back(blk->len1);
			blk->data.push_back(blk->len1);
		} else {
			blk->data.push_back(blk->len0);
			blk->data.push_back(blk->len0);
		}
		bt = (bt << 1);
	}
}

TapeBlock makeTapeBlock(uint8_t* ptr, int ln, bool hd) {
	TapeBlock nblk;
	int i;
	uint8_t tmp;
	uint8_t crc;
	nblk.plen = PILOTLEN;
	nblk.s1len = SYNC1LEN;
	nblk.s2len = SYNC2LEN;
	nblk.len0 = SIGN0LEN;
	nblk.len1 = SIGN1LEN;
	nblk.flags = TBF_BYTES;
	if (hd) {
		nblk.pdur = 8063;
		nblk.pause = 500;
		nblk.flags |= TBF_HEAD;
		crc = 0x00;
	} else {
		nblk.pdur = 3223;
		nblk.pause = 1000;
		crc = 0xff;
	}	
	nblk.data.clear();
	for (i=0; i < nblk.pdur; i++) nblk.data.push_back(nblk.plen);
	if (nblk.s1len != 0) nblk.data.push_back(nblk.s1len);
	if (nblk.s2len != 0) nblk.data.push_back(nblk.s2len);
	nblk.dataPos = nblk.data.size();
	addBlockByte(&nblk,crc);
	for (i=0; i < ln; i++) {
		tmp = *ptr;
		crc ^= tmp;
		addBlockByte(&nblk,tmp);
		ptr++;
	}
	addBlockByte(&nblk,crc);
	nblk.data.push_back(SYNC3LEN);
	return nblk;
}

// tapeAddFile(tape, filename, type(0,3 = basic,code), start, lenght, autostart, pointer to data, is header)
void tapAddFile(Tape* tap,std::string nm,int tp,uint16_t st,uint16_t ln,uint16_t as,uint8_t* ptr,bool hdr) {
	TapeBlock block;
	if (hdr) {
		uint8_t* hdbuf = new uint8_t[19];
		hdbuf[0] = tp & 0xff;						// type (0:basic, 3:code)
		memcpy(&hdbuf[1],nm.c_str(),10);				// name (10)
		if (tp == 0) {
			hdbuf[11] = st & 0xff; hdbuf[12] = ((st & 0xff00) >> 8);
			hdbuf[13] = as & 0xff; hdbuf[14] = ((as & 0xff00) >> 8);
			hdbuf[15] = ln & 0xff; hdbuf[16] = ((ln & 0xff00) >> 8);
		} else {
			hdbuf[11] = ln & 0xff; hdbuf[12] = ((ln & 0xff00) >> 8);
			hdbuf[13] = st & 0xff; hdbuf[14] = ((st & 0xff00) >> 8);
			hdbuf[15] = as & 0xff; hdbuf[16] = ((as & 0xff00) >> 8);
		}
		block = makeTapeBlock(hdbuf,17,true);
		tap->data.push_back(block);
		free(hdbuf);
	}
	block = makeTapeBlock(ptr,ln,false);
	tap->data.push_back(block);
}

void tapAddBlock(Tape* tap, TapeBlock block) {
	tap->data.push_back(block);
}
