#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tape.h"

// NEW STUFF

Tape* tapCreate() {
	Tape* tap = (Tape*)malloc(sizeof(Tape));
	memset(tap,0x00,sizeof(Tape));
	tap->isData = 1;
	tap->path = NULL;
	tap->blkData = NULL;
	tap->tmpBlock.data = NULL;
	return tap;
}

void tapDestroy(Tape* tap) {
	if (tap->path) free(tap->path);
	tapEject(tap);
	if (tap->tmpBlock.data)
		free(tap->tmpBlock.data);
	free(tap);
}

void tape_set_path(Tape* tap, const char* path) {
	if (path != NULL) {
		tap->path = realloc(tap->path, strlen(path) + 1);
		strcpy(tap->path, path);
	} else {
		free(tap->path);
		tap->path = NULL;
	}
}

// blocks

void blkClear(TapeBlock *blk) {
	if(blk->data) {
		free(blk->data);
		blk->data = NULL;
	}
	blk->breakPoint = 0;
	blk->isHeader = 0;
	blk->hasBytes = 0;
	blk->sigCount = 0;
	blk->dataPos = -1;
}

// add signal (1 level change)
void blkAddPulse(TapeBlock* blk, int len, int vol) {
	if (len < 1) return;
	if ((blk->sigCount & 0xffff) == 0) {
		blk->data = realloc(blk->data,(blk->sigCount + 0x10000) * sizeof(TapeSignal));	// allocate mem for next 0x10000 signals
	}
	if (vol < 0)
		vol = blk->vol ? 0xb0 : 0x50;
	blk->data[blk->sigCount].size = len;
	blk->data[blk->sigCount].vol = vol & 0xff;
	blk->vol = (vol & 0x80) ? 0 : 1;
	blk->sigCount++;
}

// add pause. duration in mks
void blkAddPause(TapeBlock* blk, int len) {
	if (len < 1) return;
	blkAddPulse(blk, len, -1);
}

// add wave (2 pulses)
void blkAddWave(TapeBlock* blk, int len) {
	if (len < 1) return;
	blkAddPulse(blk,len, -1);
	blkAddPulse(blk,len, -1);
}

// add byte. b0len/b1len = duration of 0/1 bits. When 0, it takes from block signals data
void blkAddByte(TapeBlock* blk, unsigned char data, int b0len, int b1len) {
	if (b0len == 0) b0len = blk->len0;
	if (b1len == 0) b1len = blk->len1;
	for (int i = 0; i < 8; i++) {
		blkAddWave(blk, (data & 0x80) ? b1len : b0len);
		data <<= 1;
	}
}

int tapGetBlockTime(Tape* tape, int blk, int pos) {
	long totsz = 0;
	int i;
	if (pos > tape->blkData[blk].sigCount)
		pos = tape->blkData[blk].sigCount;
	else if (pos < 0)
		pos = tape->blkData[blk].sigCount;
	for (i = 0; i < tape->blkData[blk].sigCount; i++)
		totsz += tape->blkData[blk].data[i].size;
	return (totsz / 1e6);			// mks -> sec
}

int tapGetBlockSize(TapeBlock* block) {
	return (((block->sigCount - block->dataPos) >> 4) - 2);
}

unsigned char tapGetBlockByte(TapeBlock* block, int bytePos) {
	unsigned char res = 0x00;
	int i;
	int sigPos = block->dataPos + (bytePos << 4);
	for (i = 0; i < 8; i++) {
		res <<= 1;
		if (sigPos < (int)(block->sigCount - 1)) {
			if ((block->data[sigPos].size == block->len1) && (block->data[sigPos + 1].size == block->len1)) {
				res |= 1;
			}
			sigPos += 2;
		}
	}
	return res;
}

int tapGetBlockData(Tape* tape, int blockNum, unsigned char* dst,int maxsize) {
	TapeBlock* block = &tape->blkData[blockNum];
	int pos = block->dataPos;
	int bytePos = 0;
	do {
		dst[bytePos] = tapGetBlockByte(block,bytePos);
		bytePos++;
		pos += 16;
	} while ((pos < (block->sigCount - 1)) && (bytePos < maxsize));
	return bytePos;
}

// TODO: leak ?
char* tapGetBlockHeader(Tape* tap, int blk) {
	char* res = (char*)malloc(16 * sizeof(char));
	TapeBlock* block = &tap->blkData[blk];
	int i;
	if (block->isHeader) {
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
	inf.type = (strlen(inf.name) == 0) ? TAPE_DATA : TAPE_HEAD;
	inf.size = tapGetBlockSize(block);
	inf.time = tapGetBlockTime(tap,blk,-1);
	inf.curtime = (tap->block == blk) ? tapGetBlockTime(tap,blk,tap->pos) : -1;
	inf.breakPoint = block->breakPoint;
	return inf;
}

int tapGetBlocksInfo(Tape* tap, TapeBlockInfo* dst) {
	int cnt = 0;
	int i;
	for (i=0; i < (int)tap->blkCount; i++) {
		dst[cnt] = tapGetBlockInfo(tap,i);
		cnt++;
	}
	return cnt;
}

void tapNormSignals(TapeBlock* block) {
	int low,hi;
	int i;
	for (i = 0; i < (int)block->sigCount; i++) {
		low = block->data[i].size - 3;
		hi = block->data[i].size + 3;
		if ((block->plen > low) && (block->plen < hi)) block->data[i].size = block->plen;
		if ((block->s1len > low) && (block->s1len < hi)) block->data[i].size = block->s1len;
		if ((block->s2len > low) && (block->s2len < hi)) block->data[i].size = block->s2len;
		if ((block->len0 > low) && (block->len0 < hi)) block->data[i].size = block->len0;
		if ((block->len1 > low) && (block->len1 < hi)) block->data[i].size = block->len1;
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
		if (tap->blkData[idx].data) {
			free(tap->blkData[idx].data);
			tap->blkData[idx].data = NULL;
		}
		while (idx < tap->blkCount - 1) {
			tap->blkData[idx] = tap->blkData[idx+1];
			idx++;
		}
		tap->blkCount--;
	}
}

// tape

void tapStoreBlock(Tape* tap) {
	unsigned int i,j;
	int same;
	int diff;
	int siglens[11];
	int cnt = 0;
	TapeBlock* tblk = &tap->tmpBlock;
	if (tblk->sigCount < 1) return;
	if (!tblk->data) return;
	for (i = 0; i < tblk->sigCount; i++) {
		same = 0;
		for (j = 0; j < cnt; j++) {
			// printf("%i %i %p %i\n",i,j,tblk->data,tblk->data->size);
			diff = (tblk->data[i].size - siglens[j]) * 100 / siglens[j];
			if ((diff > -5) && (diff < 5)) {
				same = 1;
			}
		}
		if ((same == 0) && (cnt < 10)) {
			siglens[cnt] = tblk->data[i].size;
			cnt++;
		}
	}
//	tblk->data[tblk->sigCount-1].size = 1e6;		// last signal is 1 sec (pause)
//	tblk->data[tblk->sigCount-1].vol = tblk->vol ? 0x60 : 0xa0;

	// if there's only 00 or FF (1 bit not presented)
	if (cnt == 5) {
		siglens[5] = siglens[4];
		siglens[4] = siglens[3];
		siglens[3] = (siglens[3] > 350) ? SIGN0LEN : SIGN1LEN;
		cnt++;
	}
	printf("size: %i\t",cnt);
	for (i=0; i<cnt;i++) printf("\t%i",siglens[i]);
	printf("\n");

	tblk->breakPoint = 0;
	tblk->hasBytes = 0;
	tblk->isHeader = 0;
	if (cnt == 6) {
		tblk->plen = siglens[0];
		tblk->s1len = siglens[1];
		tblk->s2len = siglens[2];
		if (siglens[4] > siglens[3]) {
			tblk->len0 = siglens[3];
			tblk->len1 = siglens[4];
		} else {
			tblk->len0 = siglens[4];
			tblk->len1 = siglens[3];
		}
		tblk->hasBytes = 1;
	} else {
		tblk->plen = PILOTLEN;
		tblk->s1len = SYNC1LEN;
		tblk->s2len = SYNC2LEN;
		tblk->len0 = SIGN0LEN;
		tblk->len1 = SIGN1LEN;
	}
	tblk->dataPos = -1;
	tapNormSignals(tblk);
	i = 1;
	while ((i < tblk->sigCount) && (tblk->data[i].size != tblk->s2len))
		i++;
	if (i < tblk->sigCount)
		tblk->dataPos = i + 1;
	if (tblk->dataPos != -1) {
		if (tapGetBlockByte(tblk,0) == 0) {
			tblk->isHeader = 1;
		}
	}
	tapAddBlock(tap,tap->tmpBlock);

	blkClear(tblk);
	tap->wait = 1;
}

void tapEject(Tape* tap) {
	int i;
	tap->isData = 1;
	tap->block = 0;
	tap->pos = 0;
	tape_set_path(tap, NULL);
	if (tap->blkData) {
		for (i = 0; i < tap->blkCount; i++) {
			if (tap->blkData[i].data) {
				free(tap->blkData[i].data);
				tap->blkData[i].data = NULL;
			}
		}
		free(tap->blkData);
	}
	tap->blkCount = 0;
	tap->blkData = NULL;
}

void tapStop(Tape* tap) {
	if (tap->on) {
		tap->on = 0;
		if (tap->rec)
			tapStoreBlock(tap);
		tap->volPlay = (tap->volPlay & 0x80) ? 0x7f : 0x81;
		// tap->pos = 0;
	}
}

int tapPlay(Tape* tap) {
	if ((tap->block < tap->blkCount) && !tap->on) {
		tap->rec = 0;
		tap->on = 1;
		tap->volPlay = (tap->volPlay & 0x80) ? 0x7f : 0x81;
	}
	return tap->on;
}

void tapRec(Tape* tap) {
	tap->on = 1;
	tap->rec = 1;
	tap->wait = 1;
	tap->levRec = 0;
	tap->oldRec = tap->levRec;
	blkClear(&tap->tmpBlock);
}

void tapRewind(Tape* tap, int blk) {
	if (blk < tap->blkCount) {
		tap->block = blk;
		tap->pos = 0;
	} else {
		tap->on = 0;
	}
}

// input : tks is time (ns) to sync
void tapSync(Tape* tap, int ns) {
	tap->time += ns;
	int mks = tap->time / 1000;
	tap->time %= 1000;
	if (tap->on) {
		if (tap->rec) {
			if (tap->wait) {
				if (tap->oldRec != tap->levRec) {
					tap->oldRec = tap->levRec;
					tap->wait = 0;
					blkAddPulse(&tap->tmpBlock,0,-1);
				}
			} else {
				if (tap->oldRec != tap->levRec) {
					tap->oldRec = tap->levRec;
					blkAddPulse(&tap->tmpBlock,mks,-1);
				} else {
					tap->tmpBlock.data[tap->tmpBlock.sigCount - 1].size += mks;
				}
				if (tap->tmpBlock.data[tap->tmpBlock.sigCount - 1].size > 2e5) {
					tap->tmpBlock.sigCount--;
					tapStoreBlock(tap);
				}
			}
		} else if (tap->blkCount > 0) {
			tap->sigLen -= mks;
			while ((tap->sigLen < 1) && tap->on) {
				if (tap->pos >= (int)tap->blkData[tap->block].sigCount) {
					tap->blkChange = 1;
					tap->block++;
					tap->pos = 0;
					if (tap->block >= (int)tap->blkCount) {
						tap->on = 0;
					} else if (tap->blkData[tap->block].breakPoint) {
						tap->on = 0;
					}
				} else {
					tap->sigLen += tap->blkData[tap->block].data[tap->pos].size;
					tap->volPlay = tap->blkData[tap->block].data[tap->pos].vol;
					tap->pos++;
				}
			}
		} else {
			tap->sigLen -= mks;
			while (tap->sigLen < 1) {
				tap->volPlay = (tap->volPlay & 0x80) ? 0x7f : 0x81;
				tap->sigLen += 5e5;	// .5 sec
			}
		}
	} else {
		// tape stoped
		tap->sigLen -= mks;
		while (tap->sigLen < 1) {
			tap->volPlay = (tap->volPlay & 0x80) ? 0x7f : 0x81;
			tap->sigLen += 5e5;	// .5 sec
		}
	}
}

void tapNextBlock(Tape* tap) {
	tap->block++;
	tap->blkChange = 1;
	if (tap->block < tap->blkCount) {
		tap->blkData[tap->block].vol = 0;
		tap->volPlay = 0x7f;
	} else {
		tap->block = 0;
		tapStop(tap);
	}
}

// add file to tape

TapeBlock makeTapeBlock(unsigned char* ptr, int ln, int hd) {
	TapeBlock nblk;
	int i;
	unsigned char tmp;
	unsigned char crc;
	nblk.plen = PILOTLEN;
	nblk.s1len = SYNC1LEN;
	nblk.s2len = SYNC2LEN;
	nblk.len0 = SIGN0LEN;
	nblk.len1 = SIGN1LEN;
	nblk.breakPoint = 0;
	nblk.hasBytes = 1;
	nblk.isHeader = 0;
	nblk.sigCount = 0;
	nblk.data = NULL;
	nblk.vol = 0;
	if (hd) {
		nblk.pdur = 8063;
		nblk.isHeader = 1;
		crc = 0x00;
	} else {
		nblk.pdur = 3223;
		crc = 0xff;
	}
	for (i = 0; i < nblk.pdur; i++)
		blkAddPulse(&nblk,nblk.plen,-1);
	if (nblk.s1len != 0)
		blkAddPulse(&nblk,nblk.s1len,-1);
	if (nblk.s2len != 0)
		blkAddPulse(&nblk,nblk.s2len,-1);
	nblk.dataPos = nblk.sigCount;
	blkAddByte(&nblk,crc,0,0);
	for (i = 0; i < ln; i++) {
		tmp = *ptr;
		crc ^= tmp;
		blkAddByte(&nblk,tmp,0,0);
		ptr++;
	}
	blkAddByte(&nblk,crc,0,0);
	return nblk;
}

// tapeAddFile(tape, filename, type(0,3 = basic,code), start, lenght, autostart, pointer to data, is header)
void tapAddFile(Tape* tap, const char* nm, int tp, unsigned short st, unsigned short ln, unsigned short as, unsigned char* ptr, int hdr) {
	TapeBlock block;
	if (hdr) {
		unsigned char hdbuf[19];
		hdbuf[0] = tp & 0xff;						// type (0:basic, 3:code)
		memset(hdbuf + 1,' ',10);
		memcpy(hdbuf + 1, nm, (strlen(nm) < 10) ? strlen(nm) : 10);
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
		blkClear(&block);
	}
	block = makeTapeBlock(ptr,ln,0);
	tapAddBlock(tap,block);
	blkClear(&block);
}

void tapAddBlock(Tape* tap, TapeBlock block) {
	if (block.sigCount == 0) return;
	TapeBlock blk = block;
	blk.data = malloc(blk.sigCount * sizeof(TapeSignal));
	memcpy(blk.data, block.data, blk.sigCount * sizeof(TapeSignal));

	tap->blkCount++;
	tap->blkData = (TapeBlock*)realloc(tap->blkData,tap->blkCount * sizeof(TapeBlock));
	tap->blkData[tap->blkCount - 1] = blk;

	tap->newBlock = 1;
}
