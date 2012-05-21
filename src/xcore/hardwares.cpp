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
	addHardware("ZX48K",HW_ZX48,0x00,0);
	addHardware("Pentagon",HW_PENT,0x05,0);
	addHardware("Pentagon1024SL",HW_P1024,0x08,0);
	addHardware("Scorpion",HW_SCORP,0x0a,IO_WAIT);
}
