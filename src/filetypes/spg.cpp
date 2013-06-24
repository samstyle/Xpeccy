#include "filetypes.h"
#include "unpackers/unpackers.h"

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

	printf("spg ver %i.%i\n",(hd.fmt & 0xf0) >> 4, hd.fmt & 0x0f);

	int blkCount = hd.bcl | (hd.bch << 8);
	int idx = 0;
	int sze;
	unsigned char pg;
	int addr;
	for (int i = 0; i < blkCount; i++) {
		addr = (blkInfo[idx] & 0x1f) << 9;		// addr in page (00:0000 .. 1F:3E00)
		sze = ((blkInfo[idx+1] & 0x1f) + 1) << 9;	// block size (00:512 .. 1f:16K
		pg = blkInfo[idx+2];

		file.read((char*)inbuf,sze);
		switch (blkInfo[idx + 1] & 0xc0) {
			case 0x00:
				memcpy(comp->mem->ramData + (pg << 14) + addr, inbuf, sze);
				break;
			case 0x40:
				sze = demegalz(inbuf,outbuf);			// it works?
				memcpy(comp->mem->ramData + (pg << 14) + addr, outbuf, sze);
				break;
			case 0x80:
				sze = dehrust(inbuf,outbuf);	// no 'last 6 bytes'
				memcpy(comp->mem->ramData + (pg << 14) + addr, outbuf, sze);
				break;
			default:
				printf("(%.2X,%.4X,%.4X) unknown compression\n",pg,addr,sze);
				break;
		}

		if (blkInfo[idx] & 0x80) break;			// last block flag
		idx += 3;
	}
#ifdef ISDEBUG
	printf("pc = %.4X\n",(hd.pch << 8) | hd.pcl);
	printf("sp = %.4X\n",(hd.sph << 8) | hd.spl);
	printf("flag35 = %.2X\n",hd.flag35);
	printf("page3 = %.2X\n",hd.page3);
#endif
	SETPC(comp->cpu, (hd.pch << 8) | hd.pcl);
	SETSP(comp->cpu, (hd.sph << 8) | hd.spl);
	switch (hd.flag35 & 0x03) {
		case 0: zxSetFrq(comp,3.5); break;
		case 1: zxSetFrq(comp,7.0); break;
		default: zxSetFrq(comp,14.0); break;
	}
	SETIFF1(comp->cpu,(hd.flag35 & 0x04) ? 1 : 0);	// int enabled/disabled
	SETIM(comp->cpu,1);				// im 1
	SETI(comp->cpu,0x3f);				// i = 3F
	comp->dosen = 0;				// basic 48 in bank0
	comp->prt0 = 0x10;
	comp->prt1 = 0x00;
	comp->prt2 = 0x00;
	comp->tsconf.Page3 = hd.page3;
	comp->hw->mapMem(comp);
	memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5);
	memSetBank(comp->mem,MEM_BANK2,MEM_RAM,2);
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,hd.page3);
	comp->vid->tsconf.tconfig = 0;
	comp->vid->flags &= ~VID_NOGFX;
	return ERR_OK;
}
