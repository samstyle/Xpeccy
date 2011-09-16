#include "common.h"
#include "memory.h"
#include "spectrum.h"
#include "settings.h"

//#include <QMessageBox>
#include <string>
#include <vector>
#include <fstream>
#ifdef HAVEZLIB
	#include <zlib.h>
#endif

extern ZXComp* zx;
extern Settings* sets;
std::vector<RZXFrame> rzx;

Memory::Memory(int tp) {
	type = tp;
	pt0 = &rom[0][0];
	pt1 = &ram[5][0];
	pt2 = &ram[2][0];
	pt3 = &ram[0][0];
}

void Memory::setram(uint8_t p) {
	cram = (p & mask);
	pt3 = &ram[cram][0];
}

void Memory::setrom(uint8_t p) {
	crom = p;
	if (p==0xff) {
		pt0 = &ram[0][0];
	} else {
		pt0 = &rom[p][0];
	}
}

uint8_t Memory::rd(uint16_t adr) {
	uint8_t res;
	switch (adr & 0xc000) {
		case 0x0000: res = *(pt0 + (adr & 0x3fff)); break;
		case 0x4000: res = *(pt1 + (adr & 0x3fff)); break;
		case 0x8000: res = *(pt2 + (adr & 0x3fff)); break;
		default: res = *(pt3 + (adr & 0x3fff)); break;
	}
	if (type == MEM_GS) {
		switch (adr & 0xe300) {
			case 0x6000: zx->gs->ch1 = res; break;
			case 0x6100: zx->gs->ch2 = res; break;
			case 0x6200: zx->gs->ch3 = res; break;
			case 0x6300: zx->gs->ch4 = res; break;
		}
	}
	return res;
}

void Memory::wr(uint16_t adr,uint8_t val) {
	switch (adr & 0xc000) {
		case 0x0000: if (crom==0xff) {*(pt0 + (adr&0x3fff)) = val;} break;
		case 0x4000: *(pt1 + (adr&0x3fff)) = val; break;
		case 0x8000: *(pt2 + (adr&0x3fff)) = val; break;
		default: *(pt3 + (adr&0x3fff)) = val; break;
	}
}

void z80uncompress(std::ifstream* file,char* buf,int maxlen) {
	char *ptr = buf;
	uint8_t tmp,tmp2,lst;
	lst = 0xed;
	bool btm = true;
	do {
		tmp = file->get();
		if (tmp == 0xed) {
			tmp = file->get();
			if (tmp == 0xed) {
				tmp2 = file->get();
				if ((lst == 0x00) && (tmp2 == 0x00)) {
					btm = false;	// stop @ 00 ed ed 00
				} else {
					tmp = file->get();
					do {
						*(ptr++) = tmp;
						tmp2--;
					} while (tmp2);
				}
			} else {
				*(ptr++) = 0xed;
				*(ptr++) = tmp;
				lst = tmp;
			}
		} else {
			*(ptr++) = tmp;
			lst = tmp;
		}
	} while (btm && !file->eof() && (ptr - buf < maxlen));
}

uint8_t z80readblock(std::ifstream* file,char* buf) {
	uint8_t tmp,tmp2;
	int adr;
	tmp = file->get(); tmp2 = file->get(); adr = tmp + (tmp2 << 8);	// compr.page size
	tmp = file->get();						// page num
	if (adr == 0xffff) {
		file->read(buf,0x4000);
	} else {
		z80uncompress(file,buf,0x4000);
	}
	return tmp;
}

void Memory::load(std::string sfnam,int typ) {
	std::ifstream file(sfnam.c_str(),std::ios::binary);
	if (!file.good()) {
		shithappens("Can't open file");
		return;
	}
	parse(&file,typ);
}

uint32_t getint(std::ifstream* file) {
	uint32_t wrd = file->get();
	wrd += (file->get() << 8);
	wrd += (file->get() << 16);
	wrd += (file->get() << 24);
	return wrd;
}

#ifdef HAVEZLIB
int zlib_uncompress(char* in, int ilen, char* out, int olen) {
	int ret;
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = inflateInit(&strm);
	if (ret!= Z_OK) {
		printf("Inflate init error\n");
		return 0;
	}
	strm.avail_in = ilen;		// full len
	strm.next_in = (uint8_t*)in;
	strm.avail_out = olen;
	strm.next_out = (uint8_t*)out;
	ret = inflate(&strm,Z_FINISH);
	switch (ret) {
		case Z_NEED_DICT:
//			ret = Z_DATA_ERROR;  
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			inflateEnd(&strm);
			return 0;
	}
	inflateEnd(&strm);
	return (olen - strm.avail_out);
}
#endif

void Memory::parse(std::ifstream* file,int typ) {
	uint8_t tmp,tmp2,lst;
	uint8_t snabank;
	uint32_t flg,len,len2;
	int adr;
	bool btm;
	char* buf = new char[0x1000000];
	char* zbuf;
	std::string onam = sets->opt.workDir + "/lain.tmp";
	std::ofstream ofile;
	std::string tmpStr;
	Z80* cpu = zx->sys->cpu;
	file->seekg(0,std::ios_base::end);
	size_t sz=file->tellg();
	file->seekg(0);
	switch (typ) {
#if HAVEZLIB
		case TYP_RZX:
			file->read(buf,4);
			if (std::string(buf,4) != "RZX!") {
				shithappens("Wrong RZX signature");
				break;
			}
			file->seekg(2,std::ios_base::cur);	// version
			flg = getint(file);			// flags
			if (flg & 1) {
				shithappens("Xpeccy cannot into encrypted RZX :(");
				break;
			}
			btm = true;
			while (btm && (file->tellg() < sz)) {
				tmp = file->get();	// block type
				len = getint(file);	// block len;
				switch (tmp) {
					case 0x30:
						flg = getint(file);
						file->read(buf,4);
						tmpStr = std::string(buf,3);
						tmp = 0xff;
						if ((tmpStr == "z80") || (tmpStr == "Z80")) tmp = TYP_Z80;
						if ((tmpStr == "sna") || (tmpStr == "SNA")) tmp = TYP_SNA;
						len2 = getint(file);	// data length;
						if (flg & 1) {
							printf("External snapshot");
							file->seekg(4,std::ios_base::cur);	// checksum
							file->read(buf,len - 21);		// snapshot file name
							tmpStr = std::string(buf,len - 21);
						} else {
							if (flg & 2) {
								if (buf) free(buf);	// free old buffer
								buf = new char[len2];	// uncompressed data
								zbuf = new char[len];	// compressed data
								file->read(zbuf,len-17);
								len2 = zlib_uncompress(zbuf,len-17,buf,len2);
							} else {
								file->read(buf,len2);
							}
							if (len2 == 0) {
								shithappens("RZX unpack error");
								btm = false;
								break;
							}
							ofile.open(onam.c_str());
							ofile.write(buf,len2);
							ofile.close();
							tmpStr = onam;
						}
						if (tmp != 0xff) load(tmpStr,tmp);		// load snapshot
						btm = false;		// stop after 1st snapshot; TODO: multiple snapshots loading
						break;
					default:
						file->seekg(len-5,std::ios_base::cur);	// skip block
						break;
				}
			}
			break;
#endif
		case TYP_Z80:
			prt0 = 0x10;
			prt1 = 0x00;
			setrom(1);
			setram(0);
			zx->vid->curscr = false;
			cpu->a = file->get(); cpu->f = file->get();
			cpu->c = file->get(); cpu->b = file->get();
			cpu->l = file->get(); cpu->c = file->get();
			cpu->lpc = file->get(); cpu->hpc = file->get();
			cpu->lsp = file->get(); cpu->hsp = file->get();
			cpu->i = file->get(); cpu->r = file->get() & 0x7f;
			tmp = file->get(); if (tmp == 0xff) tmp = 0x01;
			if (tmp & 1) cpu->r |= 0x80;
			zx->vid->brdcol = (tmp >> 1) & 7;
			cpu->e = file->get(); cpu->d = file->get();
			cpu->alt.c = file->get(); cpu->alt.b = file->get();
			cpu->alt.e = file->get(); cpu->alt.d = file->get();
			cpu->alt.l = file->get(); cpu->alt.h = file->get();
			cpu->alt.a = file->get(); cpu->alt.f = file->get();
			cpu->ly = file->get(); cpu->hy = file->get();
			cpu->lx = file->get(); cpu->hx = file->get();
			tmp2 = file->get(); cpu->iff1 = (tmp2 != 0x00);
			tmp2 = file->get(); cpu->iff2 = (tmp2 != 0x00);
			tmp2 = file->get();
			cpu->imode = (tmp2 & 3);
			if (cpu->pc == 0) {
				tmp = file->get(); tmp2 = file->get();
				adr = tmp + (tmp2 << 8);
				cpu->lpc = file->get(); cpu->hpc = file->get();
				lst = file->get();			// 34: HW mode
				tmp = file->get(); zx->sys->out(0x7ffd,tmp); // 35: 7FFD last out
				tmp = file->get();			// 36: skip (IF1)
				tmp = file->get();			// 37: skip (flags) TODO
				tmp = file->get(); zx->sys->out(0xfffd,tmp); // 38: last out to fffd
				for (tmp2=0; tmp2<16; tmp2++) {
					tmp = file->get();
					zx->aym->sc1->reg[tmp2] = tmp;
				}
				if (adr > 23) {
		printf(".z80 version 3\n");
					switch (lst) {
						case 0:
						case 1:
						case 3: lst = 1; break;		// 48K
						case 4:
						case 5:
						case 6:
						case 9: lst = 2; break;		// 128K
						case 10: lst = 3; break;	// 256K
						default: lst = 0; break;	// undef
					}
					file->seekg(adr-23,std::ios_base::cur);	// skip all other bytes
				} else {
		printf(".z80 version 2\n");
//		printf("HW mode = %i\n",lst);
					switch (lst) {
						case 0:
						case 1: lst = 1; break;		// 48K
						case 3:
						case 4:
						case 9: lst = 2; break;		// 128K
						case 10: lst = 3; break;	// 256K
						default: lst = 0; break;	// undef
					}
				}
				switch (lst) {
					case 1:
						btm = true;
						do {
							tmp = z80readblock(file,buf);
//							printf("mem block %i\n",tmp);
							switch (tmp) {
								case 4: memcpy(ram[2],buf,0x4000); break;
								case 5: memcpy(ram[0],buf,0x4000); break;
								case 8: memcpy(ram[5],buf,0x4000); break;
								default: btm = false; break;
							}
						} while (btm && !file->eof());
						break;
					case 2:
						btm = true;
						do {
							tmp = z80readblock(file,buf);
							if ((tmp > 2) && (tmp < 11)) {
								memcpy(ram[tmp-3],buf,0x4000);
							} else {
								btm = false;
							}
						} while (btm && !file->eof());
						break;
					case 3:
						btm = true;
						do {
							tmp = z80readblock(file,buf);
							if ((tmp > 2) && (tmp < 19)) {
								memcpy(ram[tmp-3],buf,0x4000);
							} else {
								btm = false;
							}
						} while (btm && !file->eof());
						break;
					default:
						printf("Hardware mode %i not supported. reset\n",lst);
						cpu->reset();
						break;
				}
			} else {			// version 1
		printf(".z80 version 1\n");
				if (tmp & 0x20) {
					z80uncompress(file,buf,0xc000);
					memcpy((char*)ram[5],buf,0x4000);
					memcpy((char*)ram[2],buf+0x4000,0x4000);
					memcpy((char*)ram[0],buf+0x8000,0x4000);
				} else {
					file->read((char*)ram[5],0x4000);
					file->read((char*)ram[2],0x4000);
					file->read((char*)ram[0],0x4000);
				}
			}
			break;
		case TYP_SNA:
			cpu->i = file->get();
			cpu->alt.l = file->get(); cpu->alt.h = file->get();
			cpu->alt.e = file->get(); cpu->alt.d = file->get();
			cpu->alt.c = file->get(); cpu->alt.b = file->get();
			cpu->alt.f = file->get(); cpu->alt.a = file->get();
			cpu->l = file->get(); cpu->h = file->get();
			cpu->e = file->get(); cpu->d = file->get();
			cpu->c = file->get(); cpu->b = file->get();
			cpu->ly = file->get(); cpu->hy = file->get();
			cpu->lx = file->get(); cpu->hx = file->get();
			tmp = file->get(); cpu->iff1 = cpu->iff2 = (tmp & 4);
			cpu->r = file->get();
			cpu->f = file->get(); cpu->a = file->get();
			cpu->lsp = file->get(); cpu->hsp = file->get();
			cpu->imode = file->get();
			tmp = file->get(); zx->vid->brdcol = (tmp & 7);
			file->read((char*)ram[5],0x4000);
			file->read((char*)ram[2],0x4000);
			file->read(buf,0x4000);
			if (sz < 49180) {
				snabank=0;
				prt0 = 0x10;
				prt1 = 0x00;
				setrom(1);
				setram(0);
				zx->vid->curscr = false;
				cpu->lpc = rd(cpu->sp++);
				cpu->hpc = rd(cpu->sp++);
			} else {
				cpu->lpc = file->get(); cpu->hpc = file->get();
				tmp = file->get(); snabank = (tmp & 7); zx->sys->out(0x7ffd,tmp);
				zx->bdi->active = (file->get() & 1);
				if (snabank!=0) file->read((char*)ram[0],0x4000);
				if (snabank!=1) file->read((char*)ram[1],0x4000);
				if (snabank!=3) file->read((char*)ram[3],0x4000);
				if (snabank!=4) file->read((char*)ram[4],0x4000);
				if (snabank!=6) file->read((char*)ram[6],0x4000);
				if (snabank!=7) file->read((char*)ram[7],0x4000);
			}
			memcpy(ram[snabank],buf,0x4000);
			break;
	}
}

void Memory::save(std::string sfnam,int typ,bool sna48=false) {
	std::ofstream file(sfnam.c_str(),std::ios::binary);
	Z80* cpu = zx->sys->cpu;
	switch (typ) {
		case TYP_Z80:
			break;
		case TYP_SNA:
			if (sna48) {wr(--cpu->sp,cpu->hpc);wr(--cpu->sp,cpu->lpc);}
			file.put((char)cpu->i);		// i
			file.put((char)cpu->alt.l).put((char)cpu->alt.h);
			file.put((char)cpu->alt.e).put((char)cpu->alt.d);
			file.put((char)cpu->alt.c).put((char)cpu->alt.b);
			file.put((char)cpu->alt.f).put((char)cpu->alt.a);
			file.put((char)cpu->l).put((char)cpu->h);
			file.put((char)cpu->e).put((char)cpu->d);
			file.put((char)cpu->c).put((char)cpu->b);
			file.put((char)cpu->ly).put((char)cpu->hy);	// iy
			file.put((char)cpu->lx).put((char)cpu->ly);	// ix
			file.put((char)(cpu->iff2?4:0)).put((char)cpu->r);// iff2,r
			file.put((char)cpu->f).put((char)cpu->a);		// f,a
			file.put((char)cpu->lsp).put((char)cpu->hsp);	// SP
			file.put((char)cpu->imode).put((char)zx->vid->brdcol);		// imode, border color
			file.write((char*)ram[5],0x4000);		// 0x4000 - 0x7fff
			file.write((char*)ram[2],0x4000);		// 0x8000 - 0xcfff
			if (sna48) {
				file.write((char*)ram[0],0x4000);	// 0xc000 - 0xffff (48K: bank 0)
			} else {
				file.write((char*)ram[cram & 7],0x4000);	// current bank
				file.put((char)cpu->lpc).put((char)cpu->hpc);	// pc
				file.put((char)prt0);			// 7ffd
				file.put((char)(zx->bdi->active?0xff:0x00));
				uint8_t bnk = cram & 7;
				if (bnk!=0) file.write((char*)ram[0],0x4000);
				if (bnk!=1) file.write((char*)ram[1],0x4000);
				if (bnk!=3) file.write((char*)ram[3],0x4000);
				if (bnk!=4) file.write((char*)ram[4],0x4000);
				if (bnk!=6) file.write((char*)ram[6],0x4000);
				if (bnk!=7) file.write((char*)ram[7],0x4000);
			}
			break;
	}
}

void Memory::setromptr(std::string nam) {
	uint32_t i;
	romset = NULL;
	for (i=0;i<rsetlist.size();i++) {
		if (rsetlist[i].name == nam) {romset = &rsetlist[i]; break;}
	}
}

void Memory::loadromset(std::string romDir) {
	int i,ad;
	std::string fpath;
	for (i=0; i<8; i++) {
		if (romset->roms[i].path == "") {
			for (ad=0;ad<0x4000;ad++) rom[i][ad]=0xff;
		} else {
#ifndef WIN32
			fpath = romDir + "/" + romset->roms[i].path;
#else
			fpath = romDir + "\\" + romset->roms[i].path;
#endif
			std::ifstream file(fpath.c_str());
			if (file.good()) {
				file.seekg(romset->roms[i].part<<14);
				file.read((char*)&rom[i][0],0x4000);
			} else {
				printf("Can't load rom '%s:%i'\n",romset->roms[i].path.c_str(),romset->roms[i].part);
				for (ad=0;ad<0x4000;ad++) rom[i][ad]=0xff;
			}
			file.close();
		}
	}
	if (zx->opt.GSRom == "") {
		for (ad=0;ad<0x4000;ad++) {
			zx->gs->sys->mem->rom[0][ad]=0xff;
			zx->gs->sys->mem->rom[1][ad]=0xff;
		}
	} else {
#ifndef WIN32
			fpath = romDir + "/" + zx->opt.GSRom;
#else
			fpath = romDir + "\\" + zx->opt.GSRom;
#endif
			std::ifstream file(fpath.c_str());
			if (file.good()) {
				file.read((char*)&zx->gs->sys->mem->rom[0][0],0x4000);
				file.read((char*)&zx->gs->sys->mem->rom[1][0],0x4000);
			} else {
				printf("Can't load gs rom '%s'\n",zx->opt.GSRom.c_str());
				for (ad=0;ad<0x4000;ad++) {
					zx->gs->sys->mem->rom[0][ad]=0xff;
					zx->gs->sys->mem->rom[1][ad]=0xff;
				}
			}
			file.close();
	}
}
