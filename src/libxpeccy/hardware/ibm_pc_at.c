#include "hardware.h"

void ibm_init(Computer* comp) {}

void ibm_mem_map(Computer* comp) {}	// no hw memory mapping, it's work of cpu

void ibm_reset(Computer* comp) {}

// ibm pc/at
// 00000..9FFFF : ram
// A0000..BFFFF : video
// C0000..CFFFF : adapter roms
// D0000..DFFFF : ram pages
// E0000..EFFFF :
// F0000..FFFFF : bios
// 100000+	: ram

int ibm_mrd(Computer* comp, int adr, int m1) {
	int res = -1;
	if (adr < 0xa0000) {
		res = comp->mem->ramData[adr & comp->mem->ramMask];		// ram up to 640K
	} else if (adr < 0xc0000) {
		printf("video mem rd %.6X\n",adr);
		// videomem
	} else if (adr < 0xd0000) {
		// ext.bios
	} else if (adr < 0xe0000) {
		res = comp->mem->ramData[adr & comp->mem->ramMask];		// ram pages ?
	} else if (adr < 0x100000) {
		res = comp->mem->romData[adr & comp->mem->romMask];
	} else {
		res = comp->mem->ramData[adr & comp->mem->ramMask];		// ram 640K+
	}
	return res;
}

void ibm_mwr(Computer* comp, int adr, int val) {
	if (adr < 0xa0000) {
		comp->mem->ramData[adr & comp->mem->ramMask] = val & 0xff;
	} else if (adr < 0xc0000) {
		printf("video mem wr %.6X,%.2X\n",adr,val);
		// video mem
	} else if (adr < 0xd0000) {
		// ext.bios
	} else if (adr < 0xe0000) {
		comp->mem->ramData[adr & comp->mem->ramMask] = val & 0xff;
	} else if (adr < 0x100000) {
		// bios
	} else {
		comp->mem->ramData[adr & comp->mem->ramMask] = val & 0xff;
	}
}

// in/out

/*
	040-05F  8253 or 8254 Programmable Interval Timer (PIT, see ~8253~)
	040 8253 channel 0, counter divisor
	041 8253 channel 1, RAM refresh counter
	042 8253 channel 2, Cassette and speaker functions
	043 8253 mode control  (see 8253)
	044 8254 PS/2 extended timer
	047 8254 Channel 3 control byte

	060-067  8255 Programmable Peripheral Interface  (PC,XT, PCjr)
	060 8255 Port A keyboard input/output buffer (output PCjr)
	061 8255 Port B output
	062 8255 Port C input
	063 8255 Command/Mode control register
OR
	060-06F  8042 Keyboard Controller  (AT,PS/2)
	060 8042 Keyboard input/output buffer register
	061 8042 system control port (for compatability with 8255)
	064 8042 Keyboard command/status register
*/

int ibm_in64(Computer* comp, int adr) {
	return 4;
}

// cmos

/*
0070	w	CMOS RAM index register port (ISA, EISA)
		 bit 7	 = 1  NMI disabled
			 = 0  NMI enabled
		 bit 6-0      CMOS RAM index (64 bytes, sometimes 128 bytes)

		any write to 0070 should be followed by an action to 0071
		or the RTC wil be left in an unknown state.

0071	r/w	CMOS RAM data port (ISA, EISA)
*/
void ibm_out70(Computer* comp, int adr, int val) {
	comp->cmos.adr = val & 0x7f;
}

void ibm_out71(Computer* comp, int adr, int val) {
	cmsWr(comp, val);
}

int ibm_in71(Computer* comp, int adr) {
	return cmsRd(comp);
}

static xPort ibmPortMap[] = {
	{0xffff,0x0064,2,2,2,ibm_in64,	NULL},
	{0xffff,0x0070,2,2,2,NULL,	ibm_out70},
	{0xffff,0x0071,2,2,2,ibm_in71,	ibm_out71},
	{0x0000,0x0000,2,2,2,NULL,	NULL}
};

int ibm_iord(Computer* comp, int adr, int nonsence) {
//	printf("in %.4X\n",adr);
	return hwIn(ibmPortMap, comp, adr, 0);
}

void ibm_iowr(Computer* comp, int adr, int val, int nonsense) {
//	printf("out %.4X\n",adr);
}

void ibm_sync(Computer* comp, int ns) {

}

void ibm_keyp(Computer* comp, keyEntry kent) {

}

void ibm_keyr(Computer* comp, keyEntry kent) {

}

sndPair ibm_vol(Computer* comp, sndVolume* vol) {
	sndPair res;
	res.left = 0;
	res.right = 0;
	return res;
}
