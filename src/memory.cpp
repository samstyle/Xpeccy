//#include "common.h"
#include "memory.h"
#include "spectrum.h"

extern ZXComp* zx;

#include <string>
#include <vector>
#include <fstream>

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

// NEW THINGZ

void memSetPage(Memory* mem, int type, int page, char* src) {
	switch(type) {
		case MEM_ROM:
			memcpy(mem->rom[page],src,0x4000);
			break;
		case MEM_RAM:
			memcpy(mem->ram[page],src,0x4000);
			break;
	}
}

void memGetPage(Memory* mem, int type, int page, char* dst) {
	switch(type) {
		case MEM_ROM:
			memcpy(dst,mem->rom[page],0x4000);
			break;
		case MEM_RAM:
			memcpy(dst,mem->ram[page],0x4000);
			break;
	}
}