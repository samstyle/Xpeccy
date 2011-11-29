#include "tape.h"
#include "spectrum.h"
#include "common.h"

#define	STDFREQ	3584000

extern ZXComp* zx;

Tape::Tape() {
	flags = pos = block = lastick = 0;
	siglen = 1;
	signal = toutold = outsig = false;
}

std::vector<TapeBlockInfo> Tape::getInfo() {
	std::vector<TapeBlockInfo> res;
	TapeBlockInfo inf;
	for (uint i=0; i<data.size(); i++) {
		inf.name = data[i].getheader();
		inf.type = (inf.name == "") ? TAPE_DATA : TAPE_HEAD;
		inf.size = data[i].getsize();
		inf.time = data[i].gettime(-1);
		inf.curtime = (block == i) ? data[i].gettime(pos) : -1;
		inf.flags = data[i].flags;
		res.push_back(inf);
	}
	return res;
}

TapeBlock::TapeBlock() {
	flags = 0;
}

void TapeBlock::normSignals() {
	uint32_t low,hi;
	for (uint i=0; i<data.size(); i++) {
		low = data[i] - 10;
		hi = data[i] + 10;
		if ((plen > low) && (plen < hi)) data[i] = plen;
		if ((s1len > low) && (s1len < hi)) data[i] = s1len;
		if ((s2len > low) && (s2len < hi)) data[i] = s2len;
		if ((len0 > low) && (len0 < hi)) data[i] = len0;
		if ((len1 > low) && (len1 < hi)) data[i] = len1;
	}
}

int TapeBlock::gettime(int p) {
	long totsz = 0;
	if (p==-1) p=data.size();
	int i; for(i=0; i<p; i++) totsz += data[i];
	return (totsz / STDFREQ);
}

int TapeBlock::getsize() {return (((data.size() - datapos)>>4) - 2);}

std::string TapeBlock::getheader() {
	std::string res;
	int i,pos = datapos+32;
	if ((getsize() > 16) && (getbyte(datapos)==0x00)) {
		if (getbyte(datapos+16)==0x00) res = "Prog:"; else res = "Code:";
		for(i=0;i<10;i++) {
			res += getbyte(pos);
			pos+=16;
		}
	}
	return res;
}

uint8_t TapeBlock::getbyte(int pos) {
	uint8_t i,res=0,msk=0x80;
	for (i=0;i<8;i++) {
		if ((data[pos] == len1) && (data[pos+1] == len1)) res |= msk;
		msk >>= 1;
		pos += 2;
	}
	return res;
}

void Tape::eject() {
	flags &= ~(TAPE_ON | TAPE_REC);
	flags |= TAPE_CANSAVE;
	block=pos=0;
	path="";
	data.clear();
}

void Tape::sync() {
	int dlt = (zx->vid->t - lastick) / 2.0;
	lastick = zx->vid->t;
	if (flags & TAPE_ON) {
		if (flags & TAPE_REC) {
			if (flags & TAPE_WAIT) {
				if (toutold != outsig) {
					toutold = outsig;
					flags &= ~TAPE_WAIT;
					tmpblock.data.push_back(0);
				}
			} else {
				tmpblock.data.back() += dlt;
				if (toutold != outsig) {
					toutold = outsig;
					tmpblock.data.push_back(0);
				}
				if (tmpblock.data.back()>3000) storeblock();
			}
		} else {
			siglen -= dlt;
			while (siglen < 1) {
				signal = !signal;
				siglen += data[block].data[pos];
				pos++;
				if (pos >= data[block].data.size()) {
					siglen += data[block].pause * 7168;
					block++;
					pos = 0;
					if (block >= data.size()) {
						flags &= ~TAPE_ON;
					} else {
						if (data[block].flags & TBF_BREAK) flags &= ~TAPE_ON;
					}
				}
			}
		}
	} else {
		siglen -= dlt;
		while (siglen < 1) {
			signal = !signal;
			siglen += STDFREQ;	// .5 sec
		}
	}
}

void Tape::stop(int tk) {
	if (flags & TAPE_ON) {
		flags &= ~TAPE_ON;
		sync();
		if (flags & TAPE_REC) storeblock();
		pos = 0;
	}
}

bool Tape::startplay() {
	if (block < data.size()) {
		flags &= ~TAPE_REC;
		flags |= TAPE_ON;
	}
	return (flags & TAPE_ON);
}

void Tape::startrec() {
	flags |= (TAPE_ON | TAPE_REC | TAPE_WAIT);
	toutold = outsig;
	tmpblock.data.clear();
}

void Tape::storeblock() {
	if (tmpblock.data.size()>0) tmpblock.data.pop_back();
	if (tmpblock.data.size()==0) return;
	uint32_t i,j;
	bool same;
	std::vector<uint32_t> siglens;
	for (i=0; i < tmpblock.data.size(); i++) {
		same = false;
		for (j=0; j<siglens.size(); j++) {
			if ((unsigned)(tmpblock.data[i] - siglens[j] + 10) < 20) {
				siglens[j] = tmpblock.data[i];
				same = true;
			}
		}
		if (!same) siglens.push_back(tmpblock.data[i]);
	}
//	printf("size: %i\n",siglens.size());
//	for (i=0; i<siglens.size();i++) printf("\t%i",siglens[i]);
//	printf("\n");
	if (siglens.size() == 5) {
		if (siglens[3]>1000) siglens.insert(siglens.begin()+3,855); else siglens.insert(siglens.begin()+3,1710);
	}
	if (siglens.size() == 6) {
		tmpblock.plen = siglens[0];
		tmpblock.s1len = siglens[1];
		tmpblock.s2len = siglens[2];
		if (siglens[4] > siglens[3]) {
			tmpblock.len0 = siglens[3];
			tmpblock.len1 = siglens[4];
		} else {
			tmpblock.len0 = siglens[4];
			tmpblock.len1 = siglens[3];
		}
		tmpblock.flags |= TBF_BYTES;
	} else {
		tmpblock.plen = 2168;
		tmpblock.s1len = 667;
		tmpblock.s2len = 735;
		tmpblock.len0 = 855;
		tmpblock.len1 = 1710;
	}
	tmpblock.datapos=-1;
	i=1;
	while ((i < tmpblock.data.size()) && (tmpblock.data[i] != tmpblock.s2len)) i++;
	if (i < tmpblock.data.size()) tmpblock.datapos = i+1;
	tmpblock.normSignals();
	data.push_back(tmpblock);
	TapeBlock* blk = &data[data.size() - 1];
	if (blk->datapos != -1) {
		setFlagBit(getdata(data.size() - 1, -1, 1)[0] == 0, &blk->flags,TBF_HEAD);
	}
	tmpblock.data.clear();
	flags |= TAPE_WAIT;
}


void Tape::swapblocks(int r1,int r2) {
	TapeBlock tb1 = data[r1];
	data[r1] = data[r2];
	data[r2] = tb1;
}

uint8_t Tape::getbyte(int blk, int pos) {return data[blk].getbyte(pos);}

std::vector<uint8_t> Tape::getdata(int blk, int pos, int len) {
	std::vector<uint8_t> res;
	if (pos==-1) pos=data[blk].datapos;		// с начала
	if (len==-1) len=int((data[blk].data.size()-pos)>>4);	// до конца (в байтах)
	while (len!=0) {
		res.push_back(getbyte(blk,pos));
		pos += 16;
		len--;
	}
	return res;
}

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

void Tape::addBlock(uint8_t* ptr, int ln, bool hd) {
	TapeBlock nblk;
	uint i;
	uint8_t tmp;
	nblk.plen = 2168;
	nblk.s1len = 667;
	nblk.s2len = 735;
	nblk.len0 = 855;
	nblk.len1 = 1710;
	nblk.pdur = hd ? 8063 : 3223;
	nblk.pause = hd ? 1000 : 300;
	setFlagBit(hd, &nblk.flags, TBF_HEAD);
	nblk.data.clear();
	for (i=0; i<nblk.pdur; i++) nblk.data.push_back(nblk.plen);
	if (nblk.s1len != 0) nblk.data.push_back(nblk.s1len);
	if (nblk.s2len != 0) nblk.data.push_back(nblk.s2len);
	nblk.datapos = nblk.data.size();
	uint8_t crc = hd ? 0x00 : 0xff;
	addBlockByte(&nblk,crc);
	for (i=0; i < (uint)ln; i++) {
		tmp = *ptr;
		crc ^= tmp;
		addBlockByte(&nblk,tmp);
		ptr++;
	}
	addBlockByte(&nblk,crc);
	nblk.data.push_back(954);
	nblk.flags |= TBF_BYTES;
	data.push_back(nblk);
}

void Tape::addFile(std::string nm,int tp,uint16_t st,uint16_t ln,uint16_t as,uint8_t* ptr,bool hdr) {
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
		addBlock(hdbuf,17,true);
	}
	addBlock(ptr,ln,false);
}


// NEW THINGZ

uint8_t tapGetBlockByte(TapeBlock* block, uint32_t sigPos) {
	uint8_t res = 0x00;
	for (int i = 0; i < 8; i++) {
		res = res << 1;
		if (sigPos < (block->data.size() - 1)) {
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
	uint32_t pos = block->datapos;
	do {
		res.push_back(tapGetBlockByte(block,pos));
		pos += 16;
	} while (pos < (block->data.size() - 1));
	return res;
}
