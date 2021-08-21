#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cmos.h"

int toBCD(int val) {
	int rrt = val % 10;
	rrt |= ((val/10) << 4);
	return rrt;
}

unsigned char cmos_rd(CMOS* cms, int port) {
	int res = 0xff;
	time_t rtime;
	struct tm* ctime;
	if (port == CMOS_DATA) {
		time(&rtime);
		ctime = localtime(&rtime);
		switch (cms->adr) {
			case 0x00: res = toBCD(ctime->tm_sec); break;
			case 0x02: res = toBCD(ctime->tm_min); break;
			case 0x04: res = toBCD(ctime->tm_hour); break;
			case 0x06: res = toBCD(ctime->tm_wday); break;
			case 0x07: res = toBCD(ctime->tm_mday); break;
			case 0x08: res = toBCD(ctime->tm_mon); break;
			case 0x09: res = toBCD(ctime->tm_year % 100); break;
			case 0x0a: res = 0x00; break;
			case 0x0b: res = 0x02; break;
			case 0x0c: res = 0x00; break;
			case 0x0d: res = 0x80; break;
			default: res = cms->data[cms->adr];
				break;
		}
	}
	return res & 0xff;
}

void cmos_wr(CMOS* cms, int port, int val) {
	if (port == CMOS_ADR) {
		cms->adr = val & 0xff;
	} else if (port == CMOS_DATA) {
		cms->data[cms->adr] = val & 0xff;
	}
}
