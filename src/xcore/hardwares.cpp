#include "xcore.h"

std::vector<HardWare> hwList;

void addHardware(const char* nam, int typ, int msk, int flg) {
	HardWare nhw;
	nhw.name = nam;
	nhw.type = typ;
	nhw.mask = msk;
	nhw.flag = flg;
	hwList.push_back(nhw);
}

void setHardware(ZXComp* comp, std::string nam) {
	for (unsigned int i = 0; i < hwList.size(); i++) {
		if (hwList[i].name == nam) {
			comp->hw = &hwList[i];
			comp->hwFlags = comp->hw->flag;
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
	addHardware("ZX48K", HW_ZX48, MEM_48, 0);
	addHardware("Pentagon", HW_PENT, MEM_128 | MEM_512, 0);
	addHardware("Pentagon1024SL", HW_P1024, MEM_1M, 0);
	addHardware("Scorpion", HW_SCORP, MEM_256 | MEM_1M, IO_WAIT);
//	addHardware("ATM 1",HW_ATM1,MEM_512, 0);
//	addHardware("ATM 2",HW_ATM2,MEM_1M,0);
	addHardware("Spectrum +2", HW_PLUS2, MEM_128, 0);
	addHardware("Spectrum +3",HW_PLUS3,MEM_128,0);
}
