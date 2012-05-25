#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tape.h"

#define	FRAMEDOTS	71680
#define SECDOTS		(FRAMEDOTS * 50)
#define	MSDOTS		(FRAMEDOTS / 20)

// NEW STUFF

Tape* tapCreate() {
	Tape* tap = (Tape*)malloc(sizeof(Tape));
	tap->flag = TAPE_CANSAVE;
	tap->block = 0;
	tap->pos = 0;
	tap->sigLen = 0;
	tap->signal = 0;
	tap->sigLen = 0;
	tap->sigCount = 0;
	tap->toutold = 0;
	tap->outsig = 0;
	tap->path = NULL;

	tap->blkCount = 0;
	tap->blkData = NULL;

	tap->tmpBlock.sigCount = 0;
	tap->tmpBlock.sigData = NULL;

	return tap;
}

void tapDestroy(Tape* tap) {
	free(tap);
}

// blocks

void blkClear(TapeBlock *blk) {
	blk->sigCount = 0;
	if(blk->sigData) free(blk->sigData);
	blk->sigData = NULL;
}

void blkAddSignal(TapeBlock* blk, int sig) {
	if ((blk->sigCount & 0xffff) == 0) {
		blk->sigData = (int*)realloc(blk->sigData,(blk->sigCount + 0x10000) * sizeof(int));	// allocate mem for next 0x10000 signals
	}
	blk->sigData[blk->sigCount] = sig;
	blk->sigCount++;
}

int tapGetBlockTime(Tape* tape, int blk, int pos) {
	long totsz = 0;
	if (pos < 0) pos = tape->blkData[blk].sigCount;
	for (int i = 0; i < pos; i++) totsz += tape->blkData[blk].sigData[i];
	return (totsz / SECDOTS);
}

inline int tapGetBlockSize(TapeBlock* block) {
	return (((block->sigCount - block->dataPos) >> 4) - 2);
}

uint8_t tapGetBlockByte(TapeBlock* block, int bytePos) {
	uint8_t res = 0x00;
	int sigPos = block->dataPos + (bytePos << 4);
	for (int i = 0; i < 8; i++) {
		res = res << 1;
		if (sigPos < (int)(block->sigCount - 1)) {
			if ((block->sigData[sigPos] == block->len1) && (block->sigData[sigPos + 1] == block->len1)) {
				res |= 1;
			}
			sigPos += 2;
		}
	}
	return res;
}

int tapGetBlockData(Tape* tape, int blockNum, unsigned char* dst) {
	TapeBlock* block = &tape->blkData[blockNum];
	uint32_t pos = block->dataPos;
	int bytePos = 0;
	do {
		dst[bytePos] = tapGetBlockByte(block,bytePos);
		bytePos++;
		pos += 16;
	} while (pos < (block->sigCount - 1));
	return bytePos;
}

char* tapGetBlockHeader(Tape* tap, int blk) {
	char* res = (char*)malloc(16 * sizeof(char));
	TapeBlock* block = &tap->blkData[blk];
	int i;
	if (block->flag & TBF_HEAD) {
		if (tapGetBlockByte(block, 1)==0x00) {
			strcpy(res,"Prog:");
		} else {
			strcpy(res,"Code:");
		}
		for(i = 2; i < 12; i++) {
			res[i + 3] = tapGetBlockByte(block,i);
		}
		res[15] = 0x00;
	} else {
		res[0] = 0x00;
	}
	return res;
}

TapeBlockInfo tapGetBlockInfo(Tape* tap, int blk) {
	TapeBlock* block = &tap->blkData[blk];
	TapeBlockInfo inf;
	inf.name = tapGetBlockHeader(tap,blk);
	inf.type = (strcmp(inf.name,"") == 0) ? TAPE_DATA : TAPE_HEAD;
	inf.size = tapGetBlockSize(block);
	inf.time = tapGetBlockTime(tap,blk,-1);
	inf.curtime = (tap->block == blk) ? tapGetBlockTime(tap,blk,tap->pos) : -1;
	inf.flag = block->flag;
	return inf;
}

int tapGetBlocksInfo(Tape* tap, TapeBlockInfo* dst) {
	int cnt = 0;
	for (int i=0; i < (int)tap->blkCount; i++) {
		dst[cnt] = tapGetBlockInfo(tap,i);
		cnt++;
	}
	return cnt;
}

void tapNormSignals(TapeBlock* block) {
	int low,hi;
	for (int i=0; i < (int)block->sigCount; i++) {
		low = block->sigData[i] - 10;
		hi = block->sigData[i] + 10;
		if ((block->plen > low) && (block->plen < hi)) block->sigData[i] = block->plen;
		if ((block->s1len > low) && (block->s1len < hi)) block->sigData[i] = block->s1len;
		if ((block->s2len > low) && (block->s2len < hi)) block->sigData[i] = block->s2len;
		if ((block->len0 > low) && (block->len0 < hi)) block->sigData[i] = block->len0;
		if ((block->len1 > low) && (block->len1 < hi)) block->sigData[i] = block->len1;
	}
}

void tapSwapBlocks(Tape* tap, int b1, int b2) {
	if ((b1 < tap->blkCount) && (b2 < tap->blkCount)) {
		TapeBlock tmp = tap->blkData[b1];
		tap->blkData[b1] = tap->blkData[b2];
		tap->blkData[b2] = tmp;
	}
}

void tapDelBlock(Tape* tap, int blk) {
	if (blk < tap->blkCount) {
		int idx = blk;
		free(tap->blkData[idx].sigData);
		while (idx < tap->blkCount - 1) {
			tap->blkData[idx] = tap->blkData[idx+1];
			idx++;
		}
		tap->blkCount--;
//		tap->data.erase(tap->data.begin() + blk);		// TODO: do
	}
}

// tape

void tapStoreBlock(Tape* tap) {
	if (tap->tmpBlock.sigCount > 0) {
		tap->tmpBlock.pause = 1000;
		tap->tmpBlock.sigCount--;
	}
	if (tap->tmpBlock.sigCount == 0) return;
	uint32_t i,j;
	int same;
	int diff;
	int siglens[10];
	int cnt = 0;
	for (i=0; i < tap->tmpBlock.sigCount; i++) {
		same = 0;
		for (j = 0; j < cnt; j++) {
			diff = tap->tmpBlock.sigData[i] - siglens[j];
			if ((diff > -11) && (diff < 11)) {
				same = 1;
			}
		}
		if ((same == 0) && (cnt < 10)) {
			siglens[cnt] = tap->tmpBlock.sigData[i];
			cnt++;
		}
	}

//	printf("size: %i\n",siglens.size());
//	for (i=0; i<siglens.size();i++) printf("\t%i",siglens[i]);
//	printf("\n");

	if (cnt == 5) {
		siglens[5] = siglens[4];
		siglens[4] = siglens[3];
		siglens[3] = (siglens[3] > 1000) ? SIGN0LEN : SIGN1LEN;
	}
	tap->tmpBlock.flag = 0;
	if (cnt == 6) {
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
		tap->tmpBlock.flag |= TBF_BYTES;
	} else {
		tap->tmpBlock.plen = PILOTLEN;
		tap->tmpBlock.s1len = SYNC1LEN;
		tap->tmpBlock.s2len = SYNC2LEN;
		tap->tmpBlock.len0 = SIGN0LEN;
		tap->tmpBlock.len1 = SIGN1LEN;
	}
	tap->tmpBlock.dataPos = -1;
	i = 1;
	while ((i < tap->tmpBlock.sigCount) && (tap->tmpBlock.sigData[i] != tap->tmpBlock.s2len)) i++;
	if (i < tap->tmpBlock.sigCount) tap->tmpBlock.dataPos = i + 1;
	tapNormSignals(&tap->tmpBlock);
	if (tap->tmpBlock.dataPos != -1) {
		if (tapGetBlockByte(&tap->tmpBlock,0) == 0) {
			tap->tmpBlock.flag |= TBF_HEAD;
		}
	}
	tapAddBlock(tap,tap->tmpBlock);

	blkClear(&tap->tmpBlock);
	tap->flag |= TAPE_WAIT;
}

void tapEject(Tape* tap) {
	tap->flag = TAPE_CANSAVE;
	tap->block = 0;
	tap->pos = 0;
	tap->path = NULL;
	tap->blkCount = 0;
	if (tap->blkData) free(tap->blkData);
	tap->blkData = NULL;
}

void tapStop(Tape* tap) {
	if (tap->flag & TAPE_ON) {
		tap->flag &= ~TAPE_ON;
		if (tap->flag & TAPE_REC) tapStoreBlock(tap);
		tap->pos = 0;
	}
}

int tapPlay(Tape* tap) {
	if (tap->block < tap->blkCount) {
		tap->flag &= ~TAPE_REC;
		tap->flag |= TAPE_ON;
	}
	return ((tap->flag & TAPE_ON) ? 1 : 0);
}

void tapRec(Tape* tap) {
	tap->flag |= (TAPE_ON | TAPE_REC | TAPE_WAIT);
	tap->toutold = tap->outsig;
	tap->tmpBlock.sigCount = 0;
	if (tap->tmpBlock.sigData) free(tap->tmpBlock.sigData);
}

void tapRewind(Tape* tap, int blk) {
	if (blk < tap->blkCount) {
		tap->block = blk;
		tap->pos = 0;
	} else {
		tap->flag &= ~TAPE_ON;
	}
}

void tapSync(Tape* tap,int tks) {
	tap->sigCount += tks / 2.0;
	int tk = tap->sigCount;
	tap->sigCount -= tk;
	if (tap->flag & TAPE_ON) {
		if (tap->flag & TAPE_REC) {
			if (tap->flag & TAPE_WAIT) {
				if (tap->toutold != tap->outsig) {
					tap->toutold = tap->outsig;
					tap->flag &= ~TAPE_WAIT;
					blkAddSignal(&tap->tmpBlock,0);
				}
			} else {
				tap->tmpBlock.sigData[tap->tmpBlock.sigCount - 1] += tk;
				if (tap->toutold != tap->outsig) {
					tap->toutold = tap->outsig;
					blkAddSignal(&tap->tmpBlock,0);
				}
				if (tap->tmpBlock.sigData[tap->tmpBlock.sigCount - 1] > FRAMEDOTS) {
					tapStoreBlock(tap);
				}
			}
		} else {
			tap->sigLen -= tk;
			while (tap->sigLen < 1) {
				tap->signal = !tap->signal;
				tap->sigLen += tap->blkData[tap->block].sigData[tap->pos];
				tap->pos++;
				if (tap->pos >= (int)tap->blkData[tap->block].sigCount) {
					tap->sigLen += tap->blkData[tap->block].pause * MSDOTS;
					tap->flag |= TAPE_BLOCK_CHANGED;
					tap->block++;
					tap->pos = 0;
					if (tap->block >= (int)tap->blkCount) {
						tap->flag &= ~TAPE_ON;
					} else {
						if (tap->blkData[tap->block].flag & TBF_BREAK) tap->flag &= ~TAPE_ON;
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

void tapNextBlock(Tape* tap) {
	tap->block++;
	tap->flag |= TAPE_BLOCK_CHANGED;
	if (tap->block < tap->blkCount) return;
	tap->block = 0;
	tapStop(tap);

}

// add file to tape

void addBlockByte(TapeBlock* blk, uint8_t bt) {
	for (int i=0; i < 8; i++) {
		if (bt & 0x80) {
			blkAddSignal(blk,blk->len1);
			blkAddSignal(blk,blk->len1);
		} else {
			blkAddSignal(blk,blk->len0);
			blkAddSignal(blk,blk->len0);
		}
		bt = (bt << 1);
	}
}

TapeBlock makeTapeBlock(uint8_t* ptr, int ln, int hd) {
	TapeBlock nblk;
	int i;
	uint8_t tmp;
	uint8_t crc;
	nblk.plen = PILOTLEN;
	nblk.s1len = SYNC1LEN;
	nblk.s2len = SYNC2LEN;
	nblk.len0 = SIGN0LEN;
	nblk.len1 = SIGN1LEN;
	nblk.flag = TBF_BYTES;
	nblk.sigCount = 0;
	nblk.sigData = NULL;
	if (hd) {
		nblk.pdur = 8063;
		nblk.pause = 500;
		nblk.flag |= TBF_HEAD;
		crc = 0x00;
	} else {
		nblk.pdur = 3223;
		nblk.pause = 1000;
		crc = 0xff;
	}
	for (i=0; i < nblk.pdur; i++)
		blkAddSignal(&nblk,nblk.plen);
	if (nblk.s1len != 0)
		blkAddSignal(&nblk,nblk.s1len);
	if (nblk.s2len != 0)
		blkAddSignal(&nblk,nblk.s2len);
	nblk.dataPos = nblk.sigCount;
	addBlockByte(&nblk,crc);
	for (i=0; i < ln; i++) {
		tmp = *ptr;
		crc ^= tmp;
		addBlockByte(&nblk,tmp);
		ptr++;
	}
	addBlockByte(&nblk,crc);
	blkAddSignal(&nblk,SYNC3LEN);
	return nblk;
}

// tapeAddFile(tape, filename, type(0,3 = basic,code), start, lenght, autostart, pointer to data, is header)
void tapAddFile(Tape* tap, const char* nm, int tp, uint16_t st, uint16_t ln, uint16_t as, uint8_t* ptr, int hdr) {
	TapeBlock block;
	if (hdr) {
		uint8_t* hdbuf = (uint8_t*)malloc(19 * sizeof(uint8_t));
		hdbuf[0] = tp & 0xff;						// type (0:basic, 3:code)
		memset(&hdbuf[1],10,' ');
		memcpy(&hdbuf[1],nm,(strlen(nm) < 10) ? strlen(nm) : 10);
		if (tp == 0) {
			hdbuf[11] = st & 0xff; hdbuf[12] = ((st & 0xff00) >> 8);
			hdbuf[13] = as & 0xff; hdbuf[14] = ((as & 0xff00) >> 8);
			hdbuf[15] = ln & 0xff; hdbuf[16] = ((ln & 0xff00) >> 8);
		} else {
			hdbuf[11] = ln & 0xff; hdbuf[12] = ((ln & 0xff00) >> 8);
			hdbuf[13] = st & 0xff; hdbuf[14] = ((st & 0xff00) >> 8);
			hdbuf[15] = as & 0xff; hdbuf[16] = ((as & 0xff00) >> 8);
		}
		block = makeTapeBlock(hdbuf,17,1);
		tapAddBlock(tap,block);
		free(hdbuf);
	}
	block = makeTapeBlock(ptr,ln,0);
	tapAddBlock(tap,block);
}

void tapAddBlock(Tape* tap, TapeBlock block) {
	TapeBlock blk = block;
	blk.sigData = (int*)malloc(blk.sigCount * sizeof(int));
	memcpy(blk.sigData,block.sigData,blk.sigCount * sizeof(int));

	tap->blkCount++;
	tap->blkData = (TapeBlock*)realloc(tap->blkData,tap->blkCount * sizeof(TapeBlock));
	tap->blkData[tap->blkCount - 1] = blk;

	tap->flag |= TAPE_NEW_BLOCK;
}
