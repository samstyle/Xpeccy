#include "hardware.h"

void hw_dum_map(Computer* comp) {

}

unsigned char hw_dum_mrd(Computer* comp, unsigned short adr, int m1) {
	return 0xff;
}

void hw_dum_mwr(Computer* comp, unsigned short adr, unsigned char val) {

}

unsigned char hw_dum_ird(Computer* comp, unsigned short adr) {
	return 0xff;
}

void hw_dum_iwr(Computer* comp, unsigned short adr, unsigned char val) {

}
