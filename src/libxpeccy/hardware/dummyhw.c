#include "hardware.h"

void hw_dum_map(Computer* comp) {

}

int hw_dum_mrd(Computer* comp, int adr, int m1) {
	return 0xff;
}

void hw_dum_mwr(Computer* comp, int adr, int val) {

}

int hw_dum_ird(Computer* comp, int adr) {
	return 0xff;
}

void hw_dum_iwr(Computer* comp, int adr, int val) {

}

sndPair hw_dum_vol(Computer* comp, sndVolume* xv) {
	sndPair vol;
	vol.left = 0;
	vol.right = 0;
	return vol;
}

HardWare dum_hw_core = {HW_DUMMY,HWG_NULL,"Dummy","Dummy",16,MEM_256,1.0,NULL,16,NULL,
			NULL, hw_dum_map, hw_dum_iwr, hw_dum_ird, hw_dum_mrd, hw_dum_mwr, NULL, NULL, NULL, NULL, NULL, NULL, hw_dum_vol};
