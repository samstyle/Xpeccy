#include "tape.h"
#include "spectrum.h"
#include "common.h"

#include <QString>

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
				siglen += data[block].data[pos++];
				if (pos >= data[block].data.size()) {
					siglen += data[block].pause * 7168;
					block++;
					pos = -1;
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

// переконвертить len байт из *file в сигналы мофона. длины сигналов в slens. на выходе - блок
TapeBlock Tape::parse(std::ifstream *file, uint32_t len, std::vector<int> slens,uint8_t bits) {
	TapeBlock newb;
	uint32_t i;
	if (bits > 8) bits = 8;
	uint8_t tmp = file->peek();
	newb.plen=slens[0];
	newb.s1len=slens[1];
	newb.s2len=slens[2];
	newb.len0=slens[3];
	newb.len1=slens[4];
	newb.pdur = (slens[6] == -1) ? ((tmp == 0) ? 8063 : 3223) : slens[6];
	setFlagBit(tmp==0, &newb.flags, TBF_HEAD);
	newb.data.clear();
	for (i=0; i<newb.pdur; i++) newb.data.push_back(newb.plen);
	if (newb.s1len!=0) newb.data.push_back(newb.s1len);
	if (newb.s2len!=0) newb.data.push_back(newb.s2len);
	newb.datapos = newb.data.size();
	for (i=0;i<len;i++) {
		tmp = file->get();
		addBlockByte(&newb,tmp);
	}
	newb.flags |= TBF_BYTES;
//	if (slens[5]!=0) newb.data.push_back(slens[5]);
	return newb;
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

uint32_t getlen(std::ifstream *file,uint8_t n) {
	uint32_t len;
	uint8_t tm;
	file->read((char*)&tm,1); len = tm;
	if (n > 1) {file->read((char*)&tm,1); len += (tm<<8);}
	if (n > 2) {file->read((char*)&tm,1); len += (tm<<16);}
	if (n > 3) {file->read((char*)&tm,1); len += (tm<<24);}
	return len;
}

void Tape::load(std::string sfnam,uint8_t type) {
	std::ifstream file(sfnam.c_str(),std::ios::binary);
	if (!file.good()) {
		shithappens(std::string("Can't open file '" + sfnam + "'"));
		return;
	}
	uint32_t i,len,paulen,loopc=0;
	std::streampos loopos = 0;
	int sd[] = {2168,667,735,855,1710,954,-1};	// длины сигналов (pilot,s1,s2,0,1,s3,pilotsize (-1==auto))
	std::vector<int> slens = std::vector<int>(sd,sd + sizeof(sd) / sizeof(int));
	std::vector<int> alens = slens;
	uint8_t *buf = new uint8_t[256];
	uint8_t tmp,bits;
	bool err = true;
	TapeBlock newb,altb;
	eject();
	switch (type) {
		case TYPE_TAP:
			while (!file.eof()) {
				len = getlen(&file,2);
				if (!file.eof()) {
					newb = parse(&file,len,slens,8);
					newb.pause = (newb.pdur==8063) ? 500 : 1000;
					newb.data.push_back(slens[5]);
					data.push_back(newb);
				}
			}
			path=sfnam;
			break;
		case TYPE_TZX:
			file.read((char*)buf,10);
			if ((std::string((char*)buf,7) != "ZXTape!") || (buf[7]!=0x1a)) {
				shithappens("Wrong TZX signature");
				break;
			}
			file.read((char*)&tmp,1);
			do {
printf("TZX block %.2X @ %.8X\n",tmp,(int)file.tellg()-1);
				switch (tmp) {
					case 0x10:
						paulen = getlen(&file,2);
						len = getlen(&file,2);
						newb = parse(&file,len,slens,8);
						newb.pause = paulen;
						newb.data.push_back(slens[5]);
						data.push_back(newb);
						newb.data.clear();
printf("add block\n");
						break;
					case 0x11:
						alens[0] = getlen(&file,2);	// pilot
						alens[1] = getlen(&file,2);	// sync1
						alens[2] = getlen(&file,2);	// sync2
						alens[3] = getlen(&file,2);	// 0
						alens[4] = getlen(&file,2);	// 1
						alens[6] = getlen(&file,2);	// pilot pulses
						file.get();
						paulen = getlen(&file,2);
						len = getlen(&file,3);
						newb = parse(&file,len,alens,8);
						newb.pause = paulen;
						newb.data.push_back(slens[5]);
						data.push_back(newb);
						newb.data.clear();
printf("add block\n");
						flags &= ~TAPE_CANSAVE;
						break;
					case 0x12:
						paulen = getlen(&file,2);
						len = getlen(&file,2);
						while (len>0) {
							newb.data.push_back(paulen);
							len--;
						}
						flags &= ~TAPE_CANSAVE;
						break;
					case 0x13:
						len = file.get();
						while (len>0) {
							paulen = getlen(&file,2);
							newb.data.push_back(paulen);
							len--;
						}
						flags &= ~TAPE_CANSAVE;
						break;
					case 0x14:
						alens[0] = alens[1] = alens[2] = alens[5] = alens[6] = 0;	// no pilot, sync1, sync2
						alens[3]=getlen(&file,2);
						alens[4]=getlen(&file,2);
						bits = file.get();
						paulen = getlen(&file,2);
						len = getlen(&file,3);
						altb = parse(&file,len,alens,bits);
						newb.len0 = altb.len0;
						newb.len1 = altb.len1;
						newb.datapos = -1;
						for (i=0; i<altb.data.size(); i++) newb.data.push_back(altb.data[i]);
						newb.pause = paulen;
						flags &= ~TAPE_CANSAVE;
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
						len = getlen(&file,2);
						newb.data.push_back(len);
						break;
					case 0x21:
						len = file.get();
						file.seekg(len,std::ios::cur);
						break;
					case 0x22:
						if (newb.data.size() != 0) {
							newb.data.push_back(slens[5]);
							newb.flags &= ~TBF_BYTES;
							data.push_back(newb);
							newb.data.clear();
printf("add block\n");
						}
						break;
					case 0x23:
						file.seekg(2,std::ios::cur);
						break;
					case 0x24:
						loopc = getlen(&file,2);
						loopos = file.tellg();
						break;
					case 0x25:
						if (loopc>0) {
							loopc--;
							if (loopc!=0) file.seekg(loopos,std::ios::beg);
						}
						break;
					case 0x26:
						len = getlen(&file,2);
						file.seekg(len<<1,std::ios::cur);
						break;
					case 0x27:
						break;
					case 0x28:
						len = getlen(&file,2);
						file.seekg(len<<1,std::ios::cur);
						break;
					case 0x2a:
						file.seekg(4,std::ios::cur);
						break;
					case 0x2b:
						file.seekg(5,std::ios::cur);
						break;
					case 0x30:
						len = file.get();
						file.seekg(len,std::ios::cur);
						break;
					case 0x31:
						file.get();
						len = file.get();
						file.seekg(len,std::ios::cur);
						break;
					case 0x32:
						len = getlen(&file,2);
						file.seekg(len,std::ios::cur);
						break;
					case 0x33:
						len = getlen(&file,1);
						file.seekg(len*3,std::ios::cur);
						break;
					case 0x34:
						file.seekg(4,std::ios::cur);
						len = getlen(&file,4);
						file.seekg(len,std::ios::cur);
						break;
					case 0x5a:
						file.seekg(9,std::ios::cur);
						break;
					default:
						shithappens(std::string("Unknown TZX block") + int2str(tmp));
						err = false;
						break;
				}
				file.read((char*)&tmp,1);
			} while (!file.eof() && err);
			if (err && (newb.data.size() != 0)) {
				newb.data.push_back(slens[5]);
				newb.flags &= ~TBF_BYTES;
				data.push_back(newb);
			}
	}
}

void Tape::save(std::string fname, uint8_t type) {
	std::ofstream file(fname.c_str(),std::ios::binary);
	std::vector<uint8_t> buf;
	uint8_t lenh,lenl;
	uint32_t i;
	switch (type) {
		case TYPE_TAP:
			for (i=0;i<data.size();i++) {
				buf = getdata(i,-1,-1);
				lenl = buf.size()&0xff;
				lenh = ((buf.size()&0xff00)>>8);
				file.write((char*)&lenl,1);
				file.write((char*)&lenh,1);
				file.write((char*)&buf[0],buf.size());
			}
			path = fname;
			break;
	}
	file.close();
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
