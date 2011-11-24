#include "common.h"
#include "memory.h"
#include "spectrum.h"
#include "emulwin.h"

#include <string>
#include <vector>
#include <fstream>
#ifdef HAVEZLIB
	#include <zlib.h>
#endif

extern ZXComp* zx;

Memory::Memory() {
	pt0 = &rom[0][0];
	pt1 = &ram[5][0];
	pt2 = &ram[2][0];
	pt3 = &ram[0][0];
	prt0 = prt1 = prt2 = 0;
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

uint32_t getword(std::ifstream* file) {
	uint32_t res = file->get();
	res += (file->get() << 8);
	return res;
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

uint8_t Memory::getRZXIn() {
	uint8_t res = 0xff;
	if (rzxFrame < (int)rzx.size()) {
		if (rzxPos < (int)rzx[rzxFrame].in.size()) {
			res = rzx[rzxFrame].in[rzxPos];
			rzxPos++;
		}
	} else {
		zx->rzxPlay = false;
	}
	return res;
}

Z80EX_WORD getLEWord(std::ifstream* file) {
	Z80EX_WORD res = file->get();
	res &= 0x00ff;
	res |= (file->get() << 8);
	return res;
}

Z80EX_WORD getBEWord(std::ifstream* file) {
	Z80EX_WORD res = (file->get() << 8);
	res |= file->get();
	return res;
}

void Memory::parse(std::ifstream* file,int typ) {
	uint8_t tmp,tmp2,lst;
	uint8_t snabank;
	uint32_t flg,len,len2;//,len3;
	int adr;
	bool btm;
	char* buf = new char[0x1000000];
	char* zbuf = NULL;
	std::string onam = "/tmp/lain.tmp";
	std::ofstream ofile;
	std::string tmpStr;
	RZXFrame rzxf;
	Z80EX_CONTEXT* cpu = zx->cpu;
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
			rzx.clear();
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
							if (buf) free(buf);	// free old buffer
							buf = new char[len2];	// uncompressed data
							if (flg & 2) {
								if (zbuf) free(zbuf);
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
					//	btm = false;		// stop after 1st snapshot; TODO: multiple snapshots loading
						break;
/*
					case 0x80:
						len2 = getint(file);	// number of frames
						file->get();		// reserved
						getint(file);		// TStates @ beginning
						flg = getint(file);	// flags
						if (flg & 1) {
							shithappens("Crypted RZX input block");
							btm = false;
							break;
						}
						len3 = len - 18;
						if (flg & 2) {			// get data in buf, uncompress if need
							buf = new char[0x7ffffff];
							zbuf = new char[len3];	// compressed data
							file->read(zbuf,len3);
							len3 = zlib_uncompress(zbuf,len3,buf,0x7ffffff);
						} else {
							buf = new char[len3];
							file->read(buf, len3);
						}
						if (len3 == 0) {
							shithappens("RZX unpack error");
							btm = false;
							break;
						}
						flg = 0;
						while (len2 > 0) {
							rzxf.fetches = (uint8_t)buf[flg] + ((uint8_t)buf[flg+1] << 8); flg += 2;
							len3 = (uint8_t)buf[flg] + ((uint8_t)buf[flg+1] << 8); flg += 2;
							if (len3 != 0xffff) {
								rzxf.in.clear();
								while (len3 > 0) {
									rzxf.in.push_back((uint8_t)buf[flg]);
									flg++;
									len3--;
								}
							}
							rzx.push_back(rzxf);
							len2--;
						}
						rzxFrame = 0;
						rzxPos = 0;
						zx->rzxPlay = true;	// TODO: it will not start until 3000AC :)
						emulUpdateWindow();
						btm = false;
						break;
*/
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
			z80ex_set_reg(cpu,regAF,getBEWord(file));	// cpu->a = file->get(); cpu->f = file->get();
			z80ex_set_reg(cpu,regBC,getLEWord(file));	// cpu->c = file->get(); cpu->b = file->get();
			z80ex_set_reg(cpu,regHL,getLEWord(file));	// cpu->l = file->get(); cpu->c = file->get();
			z80ex_set_reg(cpu,regPC,getLEWord(file));	// cpu->lpc = file->get(); cpu->hpc = file->get();
			z80ex_set_reg(cpu,regSP,getLEWord(file));	// cpu->lsp = file->get(); cpu->hsp = file->get();
			z80ex_set_reg(cpu,regI,file->get());		// cpu->i = file->get();
			tmp2 = file->get() & 0x7f;
			tmp = file->get(); if (tmp == 0xff) tmp = 0x01;
			if (tmp & 1) tmp2 |= 0x80;
			z80ex_set_reg(cpu,regR,tmp);
			z80ex_set_reg(cpu,regR7,tmp);			// cpu->r = file->get() & 0x7f;
			zx->vid->brdcol = (tmp >> 1) & 7;
			z80ex_set_reg(cpu,regDE,getLEWord(file));	// cpu->e = file->get(); cpu->d = file->get();
			z80ex_set_reg(cpu,regBC_,getLEWord(file));	// cpu->alt.c = file->get(); cpu->alt.b = file->get();
			z80ex_set_reg(cpu,regDE_,getLEWord(file));	// cpu->alt.e = file->get(); cpu->alt.d = file->get();
			z80ex_set_reg(cpu,regHL_,getLEWord(file));	// cpu->alt.l = file->get(); cpu->alt.h = file->get();
			z80ex_set_reg(cpu,regAF_,getBEWord(file));	// cpu->alt.a = file->get(); cpu->alt.f = file->get();
			z80ex_set_reg(cpu,regIY,getLEWord(file));	// cpu->ly = file->get(); cpu->hy = file->get();
			z80ex_set_reg(cpu,regIX,getLEWord(file));	// cpu->lx = file->get(); cpu->hx = file->get();
			tmp2 = file->get(); z80ex_set_reg(cpu,regIFF1,tmp2);
			tmp2 = file->get(); z80ex_set_reg(cpu,regIFF2,tmp2);
			tmp2 = file->get();
			z80ex_set_reg(cpu,regIM,tmp2 & 3);	// cpu->imode = (tmp2 & 3);
			if (z80ex_get_reg(cpu,regPC) == 0) {
				tmp = file->get(); tmp2 = file->get();
				adr = tmp + (tmp2 << 8);
				z80ex_set_reg(cpu,regPC,getLEWord(file));	//cpu->lpc = file->get(); cpu->hpc = file->get();
				lst = file->get();			// 34: HW mode
				tmp = file->get(); zx->out(0x7ffd,tmp); // 35: 7FFD last out
				tmp = file->get();			// 36: skip (IF1)
				tmp = file->get();			// 37: skip (flags) TODO
				tmp = file->get(); zx->out(0xfffd,tmp); // 38: last out to fffd
				for (tmp2=0; tmp2<16; tmp2++) {
					tmp = file->get();
					tsSet(zx->ts,CHIP_A_REG,tmp2,tmp);
//					zx->aym->sc1->reg[tmp2] = tmp;
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
						z80ex_reset(cpu);
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
			tmp =file->get(); z80ex_set_reg(cpu,regI,tmp);	//cpu->i = file->get();
			z80ex_set_reg(cpu,regHL_,getLEWord(file));	//cpu->alt.l = file->get(); cpu->alt.h = file->get();
			z80ex_set_reg(cpu,regDE_,getLEWord(file));	// cpu->alt.e = file->get(); cpu->alt.d = file->get();
			z80ex_set_reg(cpu,regBC_,getLEWord(file));	// cpu->alt.c = file->get(); cpu->alt.b = file->get();
			z80ex_set_reg(cpu,regAF_,getLEWord(file));	// cpu->alt.f = file->get(); cpu->alt.a = file->get();
			z80ex_set_reg(cpu,regHL,getLEWord(file));	// cpu->l = file->get(); cpu->h = file->get();
			z80ex_set_reg(cpu,regDE,getLEWord(file));	// cpu->e = file->get(); cpu->d = file->get();
			z80ex_set_reg(cpu,regBC,getLEWord(file));	// cpu->c = file->get(); cpu->b = file->get();
			z80ex_set_reg(cpu,regIY,getLEWord(file));	// cpu->ly = file->get(); cpu->hy = file->get();
			z80ex_set_reg(cpu,regIX,getLEWord(file));	// cpu->lx = file->get(); cpu->hx = file->get();
			tmp = file->get();
			z80ex_set_reg(cpu,regIFF1,tmp & 4);
			z80ex_set_reg(cpu,regIFF2,tmp & 4);	// cpu->iff1 = cpu->iff2 = (tmp & 4);
			tmp = file->get();
			z80ex_set_reg(cpu,regR,tmp);
			z80ex_set_reg(cpu,regR7,tmp);		// cpu->r = file->get();
			z80ex_set_reg(cpu,regAF,getLEWord(file));	// cpu->f = file->get(); cpu->a = file->get();
			z80ex_set_reg(cpu,regSP,getLEWord(file));	// cpu->lsp = file->get(); cpu->hsp = file->get();
			tmp = file->get(); z80ex_set_reg(cpu,regIM,tmp);	// cpu->imode = file->get();
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
				adr = z80ex_get_reg(cpu,regSP);
				tmp = rd(adr++);
				tmp2 = rd(adr++);
				z80ex_set_reg(cpu,regSP,adr);
				z80ex_set_reg(cpu,regPC,tmp | (tmp2 << 8));
			} else {
				z80ex_set_reg(cpu,regPC,getLEWord(file));	// cpu->lpc = file->get(); cpu->hpc = file->get();
				tmp = file->get();
				snabank = (tmp & 7);
				zx->out(0x7ffd,tmp);
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
	if (buf) free(buf);
	if (zbuf) free(zbuf);
}

void putLEWord(std::ofstream* file, Z80EX_WORD wrd) {
	file->put((char)(wrd & 0x00ff));
	file->put((char)((wrd & 0xff00) >> 8));
}

void Memory::save(std::string sfnam,int typ,bool sna48=false) {
	std::ofstream file(sfnam.c_str(),std::ios::binary);
	Z80EX_CONTEXT* cpu = zx->cpu;
	Z80EX_WORD pc,sp;
	switch (typ) {
		case TYP_Z80:
			break;
		case TYP_SNA:
			pc = z80ex_get_reg(cpu,regPC);
			sp = z80ex_get_reg(cpu,regSP);
			if (sna48) {
				wr(--sp,(pc & 0xff00) >> 8);
				wr(--sp,pc & 0x00ff);
				z80ex_set_reg(cpu,regSP,sp);
			}
			file.put((char)z80ex_get_reg(cpu,regI));		// i
			putLEWord(&file,z80ex_get_reg(cpu,regHL_));	// file.put((char)cpu->alt.l).put((char)cpu->alt.h);
			putLEWord(&file,z80ex_get_reg(cpu,regDE_));	// file.put((char)cpu->alt.e).put((char)cpu->alt.d);
			putLEWord(&file,z80ex_get_reg(cpu,regBC_));	// file.put((char)cpu->alt.c).put((char)cpu->alt.b);
			putLEWord(&file,z80ex_get_reg(cpu,regAF_));	// file.put((char)cpu->alt.f).put((char)cpu->alt.a);
			putLEWord(&file,z80ex_get_reg(cpu,regHL));	// file.put((char)cpu->l).put((char)cpu->h);
			putLEWord(&file,z80ex_get_reg(cpu,regDE));	// file.put((char)cpu->e).put((char)cpu->d);
			putLEWord(&file,z80ex_get_reg(cpu,regBC));	// file.put((char)cpu->c).put((char)cpu->b);
			putLEWord(&file,z80ex_get_reg(cpu,regIY));	// file.put((char)cpu->ly).put((char)cpu->hy);	// iy
			putLEWord(&file,z80ex_get_reg(cpu,regIX));	// file.put((char)cpu->lx).put((char)cpu->ly);	// ix
			file.put(z80ex_get_reg(cpu,regIFF2) ? 4 : 0);
			file.put((char)((z80ex_get_reg(cpu,regR) & 0x7f) | (z80ex_get_reg(cpu,regR7) & 0x80)));	//file.put((char)(cpu->iff2?4:0)).put((char)cpu->r);// iff2,r
			putLEWord(&file,z80ex_get_reg(cpu,regAF));	// file.put((char)cpu->f).put((char)cpu->a);		// f,a
			putLEWord(&file,z80ex_get_reg(cpu,regSP));	// file.put((char)cpu->lsp).put((char)cpu->hsp);	// SP
			file.put((char)z80ex_get_reg(cpu,regIM));		// imode
			file.put((char)zx->vid->brdcol);		// border color
			file.write((char*)ram[5],0x4000);		// 0x4000 - 0x7fff
			file.write((char*)ram[2],0x4000);		// 0x8000 - 0xcfff
			if (sna48) {
				file.write((char*)ram[0],0x4000);	// 0xc000 - 0xffff (48K: bank 0)
			} else {
				file.write((char*)ram[cram & 7],0x4000);	// current bank
				putLEWord(&file,z80ex_get_reg(cpu,regPC));	// file.put((char)cpu->lpc).put((char)cpu->hpc);	// pc
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

void Memory::loadromset(std::string romDir) {
	int i,ad;
	std::string fpath;
	std::ifstream file;
	profMask = 0;
	if (romset == NULL) {
		for (i=0; i<16; i++) {for (ad=0; ad<0x4000; ad++) rom[i][ad] = 0xff;}
	} else {
		if (romset->file != "") {
#ifndef WIN32
			fpath = romDir + "/" + romset->file;
#else
			fpath = romDir + "\\" + romset->file;
#endif
			file.open(fpath.c_str());
			if (file.good()) {
				file.seekg(0,std::ios_base::end);
				int prts = file.tellg() / 0x4000;
				profMask = 3;
				if (prts < 9) profMask = 1;
				if (prts < 5) profMask = 0;
				if (prts > 16) prts = 16;
				file.seekg(0,std::ios_base::beg);
				for (i=0; i<prts; i++) {
					file.read((char*)&rom[i][0],0x4000);
				}
				for (i=prts; i<16; i++) {for (ad=0; ad<0x4000; ad++) rom[i][ad] = 0xff;}
			} else {
				printf("Can't open single rom '%s'\n",romset->file.c_str());
				for (i=0; i<16; i++) {for (ad=0; ad<0x4000; ad++) rom[i][ad] = 0xff;}
			}
			file.close();
		} else {
			for (i=0; i<4; i++) {
				if (romset->roms[i].path == "") {
					for (ad=0;ad<0x4000;ad++) rom[i][ad]=0xff;
				} else {
#ifndef WIN32
					fpath = romDir + "/" + romset->roms[i].path;
#else
					fpath = romDir + "\\" + romset->roms[i].path;
#endif
					file.open(fpath.c_str());
					if (file.good()) {
						file.seekg(romset->roms[i].part<<14);
						file.read((char*)&rom[i][0],0x4000);
					} else {
						printf("Can't open rom '%s:%i'\n",romset->roms[i].path.c_str(),romset->roms[i].part);
						for (ad=0;ad<0x4000;ad++) rom[i][ad]=0xff;
					}
					file.close();
				}
			}
		}
	}
	char* buf = (char*)malloc(0x4000 * sizeof(char));
	for (ad = 0; ad < 0x4000; ad++) buf[ad] = 0xff;
	if (zx->opt.GSRom == "") {
		gsSetRom(zx->gs,0,buf);
		gsSetRom(zx->gs,1,buf);
	} else {
#ifndef WIN32
			fpath = romDir + "/" + zx->opt.GSRom;
#else
			fpath = romDir + "\\" + zx->opt.GSRom;
#endif
			file.open(fpath.c_str());
			if (file.good()) {
				file.read(buf,0x4000);
				gsSetRom(zx->gs,0,buf);
				file.read(buf,0x4000);
				gsSetRom(zx->gs,1,buf);
			} else {
				printf("Can't load gs rom '%s'\n",zx->opt.GSRom.c_str());
				gsSetRom(zx->gs,0,buf);
				gsSetRom(zx->gs,1,buf);
			}
			file.close();
	}
}
