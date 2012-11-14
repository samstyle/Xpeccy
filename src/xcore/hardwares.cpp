#include "xcore.h"

std::vector<HardWare> hwList;

void addHardware(const char* nam, int typ, int msk,
		 void(*fmap)(ZXComp*),
		 void(*fout)(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int),
		 Z80EX_BYTE(*fin)(ZXComp*,Z80EX_WORD,int)) {
	HardWare nhw;
	nhw.name = nam;
	nhw.type = typ;
	nhw.mask = msk;
	nhw.mapMem = fmap;
	nhw.out = fout;
	nhw.in = fin;
	hwList.push_back(nhw);
}

void setHardware(ZXComp* comp, std::string nam) {
	for (unsigned int i = 0; i < hwList.size(); i++) {
		if (hwList[i].name == nam) {
			comp->hw = &hwList[i];
			break;
		}
	}
}

std::vector<std::string> getHardwareNames() {
	std::vector<std::string> res;
	for (unsigned int i=0; i<hwList.size(); i++) {
		res.push_back(hwList[i].name);
	}
	return res;
}

std::vector<HardWare> getHardwareList() {
	return hwList;
}

void initHardware() {
	addHardware("ZX48K", HW_ZX48, MEM_48, &speMapMem, &speOut, &speIn);
	addHardware("Pentagon", HW_PENT, MEM_128 | MEM_512, &penMapMem, &penOut, &penIn);
	addHardware("Pentagon1024SL", HW_P1024, MEM_1M, &p1mMapMem, &p1mOut, &p1mIn);
	addHardware("PentEvo",HW_PENTEVO,MEM_4M, &evoMapMem, &evoOut, &evoIn);
	addHardware("Scorpion", HW_SCORP, MEM_256 | MEM_1M, &scoMapMem, &scoOut, &scoIn);
	addHardware("ATM 2",HW_ATM2,MEM_128 | MEM_256 | MEM_512 | MEM_1M, &atm2MapMem, &atm2Out, &atm2In);
	addHardware("Spectrum +2", HW_PLUS2, MEM_128, &pl2MapMem, &pl2Out, &pl2In);
	addHardware("Spectrum +3",HW_PLUS3,MEM_128, &pl2MapMem, &pl3Out, &pl3In);		// mem map - same as in +2
}
