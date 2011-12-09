#include "filetypes.h"

int loadFDI(Floppy* flp,const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
	
	uint8_t* buf = new uint8_t[14];
	Sector sct;
	std::vector<Sector> trkimg;
	int i,j,scnt;
	uint8_t fcnt,tmp;
	uint16_t tmpa,tmpb,slen;
	size_t tmpd,tmph,tmpt,tmps,cpos;
	
	file.read((char*)buf,14);
	if (std::string((char*)buf,3) != "FDI") return ERR_FDI_SIGN;
	bool err = (buf[3] != 0);
	tmpa = buf[4] + (buf[5] << 8);		// cylinders
	tmpb = buf[6] + (buf[7] << 8);		// heads
	if ((tmpb != 1) && (tmpb != 2)) return ERR_FDI_HEAD;
	tmpd = buf[10] + (buf[11] << 8);	// sectors data pos
	tmph = buf[12] + (buf[13] << 8) + 14;	// track headers data pos
	file.seekg(tmph);				// read tracks data
	for (i = 0; i < tmpa; i++) {
		for (j = 0; j < tmpb; j++) {
			trkimg.clear();
			tmpt = tmpd + getlen(&file,4);
			file.seekg(2,std::ios_base::cur);		// skip 2 bytes
			tmp = file.get();			// sectors in disk;
			for (scnt=0; scnt < tmp; scnt++) {
				sct.cyl = file.get();
				sct.side = file.get();
				sct.sec = file.get();
				sct.len = file.get();
				fcnt = file.get();				// flag
				sct.type = (fcnt & 0x80) ? 0xf8 : 0xfb;
				tmps = tmpt + getlen(&file,2);
				cpos = file.tellg();			// remember current pos
				file.seekg(tmps);
				slen = (128 << sct.len);		// sector len
				sct.data = new uint8_t[slen];
				file.read((char*)sct.data,slen);	// read sector data
				file.seekg(cpos);
				trkimg.push_back(sct);
			}
			flpFormTrack(flp, (i << 1) + j, trkimg);
		}
	}
	flpSetFlag(flp,FLP_PROTECT,err);
	flpSetFlag(flp,FLP_INSERT,true);
	flpSetFlag(flp,FLP_CHANGED,false);
	flp->path = std::string(name);
	
	return ERR_OK;
}
