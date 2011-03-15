#include "tape.h"
//#include "video.h"
//#include "z80.h"
#include "spectrum.h"

#include <QMessageBox>
#include <QString>

extern ZXComp* zx;
//extern Tape* tape;

Tape::Tape() {
	flags = pos = block = lastick = 0;
	siglen = 1;
}

int TapeBlock::gettime(int p=-1) {
	long totsz = 0;
	if (p==-1) p=data.size();
	int i; for(i=0; i<p; i++) totsz += data[i];
	return (totsz / (zx->sys->vid->frmsz * 25));
}

int TapeBlock::getsize() {return (((data.size() - datapos)>>4) - 2);}

std::string TapeBlock::getheader() {
	std::string res;
	int i,pos = datapos+32;
	if (getbyte(datapos)==0x00) {
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
	int dlt = (zx->sys->vid->t - lastick) / 2.0;
	lastick = zx->sys->vid->t;
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
				if (tmpblock.data.back()>4000) storeblock();
			}
		} else {
			siglen -= dlt;
			while (siglen < 1) {
				signal = !signal;
				siglen += data[block].data[pos++];
				if (pos >= data[block].data.size()) {
					siglen += data[block].pause;
					block++;
					pos = -1;
					if (block >= data.size()) {
						flags &= ~TAPE_ON;
//						siglen = 945;
					}
				}
			}
		}
	} else {
		siglen -= dlt;
		while (siglen < 1) {
			signal = !signal;
			siglen += zx->sys->vid->frmsz*25;	// .5 sec
		}
	}
}

void Tape::stop() {
	if (flags & TAPE_ON) {
		flags &= ~TAPE_ON;
		sync();
		if (flags & TAPE_REC) storeblock();
	}
}

void Tape::startplay() {
	if (block < data.size()) {
		flags &= ~TAPE_REC;
		flags |= TAPE_ON;
	}
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
	printf("size: %i\n",siglens.size());
	for (i=0; i<siglens.size();i++) printf("\t%i",siglens[i]);
	printf("\n");
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
		tmpblock.datapos=-1;
		i=1;
		while (tmpblock.data[i] != siglens[2]) i++;
		tmpblock.datapos = i+1;
	} else {
		tmpblock.plen = 2168;
		tmpblock.s1len = 667;
		tmpblock.s2len = 735;
		tmpblock.len0 = 855;
		tmpblock.len1 = 1710;
	}
	data.push_back(tmpblock);
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

// переконвертить len байт из *file в сигналы мофона. длины сигналов в slens. на выходе - блок
TapeBlock Tape::parse(std::ifstream *file, uint32_t len, std::vector<int> slens) {
	TapeBlock newb;
	uint32_t i;
	uint32_t j;
	uint8_t tmp = file->peek();
	newb.plen=slens[0];
	newb.s1len=slens[1];
	newb.s2len=slens[2];
	newb.len0=slens[3];
	newb.len1=slens[4];
	newb.pdur = (slens[6]==-1)?((tmp&0x80)?3223:8063):slens[6];
	newb.data.clear();
	if (newb.pdur!=0) {
		for (i=0;i<newb.pdur;i++) newb.data.push_back(newb.plen);
	}
	if (newb.s1len!=0) newb.data.push_back(newb.s1len);
	if (newb.s2len!=0) newb.data.push_back(newb.s2len);
	newb.datapos = newb.data.size();
	for (i=0;i<len;i++) {
		file->read((char*)&tmp,1);
		for (j=0;j<8;j++) {
			if (tmp & 0x80) {newb.data.push_back(newb.len1);newb.data.push_back(newb.len1);}
				else {newb.data.push_back(newb.len0);newb.data.push_back(newb.len0);}
			tmp = (tmp<<1);
		}
	}
	if (slens[5]!=0) newb.data.push_back(slens[5]);
	return newb;
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
	QMessageBox mbx; mbx.setIcon(QMessageBox::Critical); mbx.setWindowTitle("Error");
	std::ifstream file(sfnam.c_str(),std::ios::binary);
	if (!file.good()) {
		mbx.setText("<b>Can't open file</b>");
		mbx.setInformativeText(sfnam.c_str());
		mbx.exec();
		return;
	}
	uint32_t len,paulen,loopc=0;
	std::streampos loopos = 0;
	int sd[] = {2168,667,735,855,1710,954,-1};	// длины сигналов (pilot,s1,s2,0,1,s3,pilotsize (-1==auto))
	std::vector<int> slens = std::vector<int>(sd,sd + sizeof(sd) / sizeof(int));
	std::vector<int> alens = slens;
	uint8_t *buf = new uint8_t[256];
	uint8_t tmp;
	bool err = true;
	TapeBlock newb;
	eject();
	switch (type) {
		case TYPE_TAP:
			while (!file.eof()) {
				len = getlen(&file,2);
				if (!file.eof()) {
					newb = parse(&file,len,slens);
					newb.pause = zx->sys->vid->frmsz * ((newb.pdur==8063)?50:25);
					data.push_back(newb);
				}
			}
			path=sfnam;
			break;
		case TYPE_TZX:
//			printf("Loading TZX in under construction\n"); break;
			file.read((char*)buf,10);
			if ((std::string((char*)buf,7) != "ZXTape!") || (buf[7]!=0x1a)) {
				mbx.setText("<b>Wrong TZX file</b>");
				mbx.setInformativeText("Incorrect signature");
				mbx.exec();
				break;
			}
			file.read((char*)&tmp,1);
			do {
				switch (tmp) {
					case 0x10:
						paulen = getlen(&file,2);
						len = getlen(&file,2);
						newb = parse(&file,len,slens);
						newb.data.push_back((zx->sys->vid->frmsz/20)*paulen);
						data.push_back(newb);
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
						newb = parse(&file,len,alens);
						newb.data.push_back((zx->sys->vid->frmsz/20)*paulen);
						data.push_back(newb);
						flags &= ~TAPE_CANSAVE;
						break;
					case 0x12:
						if (data.size()==0) {newb.data.clear(); data.push_back(newb);}
						paulen = getlen(&file,2);
						len = getlen(&file,2);
						while (len>0) {data.back().data.push_back(paulen); len--;}
						flags &= ~TAPE_CANSAVE;
						break;
					case 0x13:
						if (data.size()==0) {newb.data.clear(); data.push_back(newb);}
						len = file.get();
						while (len>0) {data.back().data.push_back(file.get()); len--;}
						flags &= ~TAPE_CANSAVE;
						break;
					case 0x14:
						alens[1]=alens[2]=alens[6]=0;	// no pilot, sync1, sync2
						alens[3]=getlen(&file,2);
						alens[4]=getlen(&file,2);
						file.get();
						paulen = getlen(&file,2);
						len = getlen(&file,3);
						newb = parse(&file,len,alens);
						newb.data.push_back((zx->sys->vid->frmsz/20)*paulen);
						data.push_back(newb);
						flags &= ~TAPE_CANSAVE;
						break;
					case 0x15:
						printf("Block 0x15 skipped\n");
						file.seekg(5,std::ios::cur);
						len = getlen(&file,3);
						file.seekg(len,std::ios::cur);
						break;
					case 0x18:
						printf("Block 0x18 skipped\n");
						len = getlen(&file,4);
						file.seekg(len,std::ios::cur);
						break;
					case 0x19:
						printf("Block 0x19 skipped\n");
						len = getlen(&file,4);
						file.seekg(len,std::ios::cur);
						break;
					case 0x20:
						if (data.size()==0) {newb.data.clear(); data.push_back(newb);}
						len = getlen(&file,2);
						data.back().data.push_back(len);
						break;
					case 0x21:
						printf("Block 0x21 skipped\n");
						len = file.get();
						file.seekg(len,std::ios::cur);
						break;
					case 0x22:
						break;
					case 0x23:
						printf("Block 0x23 skipped\n");
						file.seekg(2,std::ios::cur);
						break;
					case 0x24:
						loopc = getlen(&file,2);
						loopos = file.tellg();
						break;
					case 0x25:
						if (loopc>0) {loopc--; if (loopc!=0) file.seekg(loopos,std::ios::beg);}
						break;
					case 0x26:
						printf("Block 0x26 skipped\n");
						len = getlen(&file,2);
						file.seekg(len<<1,std::ios::cur);
						break;
					case 0x27:
						break;
					case 0x28:
						printf("Block 0x28 skipped\n");
						len = getlen(&file,2);
						file.seekg(len<<1,std::ios::cur);
						break;
					case 0x2a:
						printf("Block 0x2a skipped\n");
						file.seekg(4,std::ios::cur);
						break;
					case 0x2b:
						printf("Block 0x2b skipped\n");
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
						mbx.setText("Unknown TZX block");
						mbx.setDetailedText(QString("block type\t0x").append(QString::number(tmp,16)));
						mbx.exec();
						err = false;
						break;
				}
				file.read((char*)&tmp,1);
			} while (!file.eof() && err);
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
			break;
	}
	file.close();
}