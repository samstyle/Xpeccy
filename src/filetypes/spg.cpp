#include "filetypes.h"

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
	unsigned char buf[0x4000];
	file.read((char*)&hd,sizeof(spgHead));
	file.read((char*)blkInfo,768);
	int blkCount = hd.bcl | (hd.bch << 8);
	int idx = 0;
	int sze = 0;
	unsigned char pg;
	int addr;
	for (int i = 0; i < blkCount; i++) {
		addr = 0xc000 + ((blkInfo[idx] & 0x1f) << 9);	// addr in page (00:C000 .. 1F:FE00)
		sze = ((blkInfo[idx+1] & 0x1f) + 1) << 9;	// block size (00:512 .. 1f:16K
		pg = blkInfo[idx+2];

		file.read((char*)buf,sze);
		switch (blkInfo[idx + 1] & 0xc0) {
			case 0x00:
				printf("(%.2X,%.4X,%.4X) is not compressed\n",pg,addr,sze);
				// memcpy(comp->mem->ramData + (pg << 14) + addr, buf, sze);
				break;
			case 0x40:
				printf("(%.2X,%.4X,%.4X) is MLZ compressed\n",pg,addr,sze);
				break;
			case 0x80:
				printf("(%.2X,%.4X,%.4X) is HRUST compressed\n",pg,addr,sze);
				break;
		}

		if (blkInfo[idx] & 0x80) break;			// last block flag
		idx += 3;
	}


	return ERR_OK;
}
