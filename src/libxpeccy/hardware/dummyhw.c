#include "hardware.h"

void hw_dum_map(Computer* comp) {

}

int hw_dum_mrd(Computer* comp, int adr, int m1) {
	return 0xff;
}

void hw_dum_mwr(Computer* comp, int adr, int val) {

}

int hw_dum_ird(Computer* comp, int adr, int dos) {
	return 0xff;
}

void hw_dum_iwr(Computer* comp, int adr, int val, int dos) {

}

sndPair hw_dum_vol(Computer* comp, sndVolume* xv) {
	sndPair vol;
	vol.left = 0;
	vol.right = 0;
	return vol;
}
