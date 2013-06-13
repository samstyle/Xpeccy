#include "filetypes.h"
#include "packers.h"

#include <stdio.h>

struct spgHead {
	char astr[32];
	char sign[12];
	unsigned char fmt;	// 2c : format
	unsigned char day;	// 2d : day
	unsigned char month;	// 2e : month
	unsigned char year;	// 2f : year
	unsigned char pcl,pch;	// 30 : pc
	unsigned char spl,sph;	// 32 : sp
	unsigned char page3;	// 34 : page
	unsigned char flag35;	// 35 : flag35
	unsigned char pal,pah;	// 36 : pager address
	unsigned char ral,rah;	// 38 : resident address
	unsigned char bcl,bch;	// 3A : block count
	unsigned char sec;	// 3C : sec
	unsigned char min;	// 3D : min
	unsigned char hour;	// 3E : hour
	char res3F[17];		// 3F : reserved
	char creator[32];
	char res70[144];	// 70 : reserved
};

int loadSPG(ZXComp* comp, const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	spgHead hd;
	unsigned char blkInfo[768];
	unsigned char inbuf[0x4000];
	unsigned char outbuf[0xffff];
	file.read((char*)&hd,sizeof(spgHead));
	file.read((char*)blkInfo,768);
	int blkCount = hd.bcl | (hd.bch << 8);
	int idx = 0;
	int sze;
	unsigned char pg;
	int addr;
	for (int i = 0; i < blkCount; i++) {
		addr = (blkInfo[idx] & 0x1f) << 9;	// addr in page (00:0000 .. 1F:3E00)
		sze = ((blkInfo[idx+1] & 0x1f) + 1) << 9;	// block size (00:512 .. 1f:16K
		pg = blkInfo[idx+2];

		file.read((char*)inbuf,sze);
		switch (blkInfo[idx + 1] & 0xc0) {
			case 0x00:
				// printf("(%.2X,%.4X,%.4X) is not compressed\n",pg,addr,sze);
				memcpy(comp->mem->ramData + (pg << 14) + addr, inbuf, sze);
				break;
			case 0x40:
				printf("(%.2X,%.4X,%.4X) is MLZ compressed & unsupported\n",pg,addr,sze);
				break;
			case 0x80:
//				printf("(%.2X,%.4X,%.4X) is HRUST compressed ... ",pg,addr,sze);
				sze = dehrust(inbuf,outbuf) - 6; // no 'last 6 bytes'
				memcpy(comp->mem->ramData + (pg << 14) + addr, outbuf, sze);
				break;
		}

		if (blkInfo[idx] & 0x80) break;			// last block flag
		idx += 3;
	}
	SETPC(comp->cpu, (hd.pch << 8) | hd.pcl);
	SETSP(comp->cpu, (hd.sph << 8) | hd.spl);
	switch (hd.flag35 & 0x03) {
		case 0: zxSetFrq(comp,3.5); break;
		case 1: zxSetFrq(comp,7.0); break;
		default: zxSetFrq(comp,14.0); break;
	}
	SETIFF1(comp->cpu,(hd.flag35 & 0x04) ? 1 : 0);
	comp->dosen = 0;
	comp->prt0 = 0x10;
	comp->prt1 = 0x00;
	comp->prt2 = 0x00;
	comp->tsconf.Page3 = hd.page3;
	comp->hw->mapMem(comp);
	memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5);
	memSetBank(comp->mem,MEM_BANK1,MEM_RAM,2);
	memSetBank(comp->mem,MEM_BANK1,MEM_RAM,hd.page3);
	return ERR_OK;
}
