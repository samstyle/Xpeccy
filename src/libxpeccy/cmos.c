#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "cmos.h"

/*
00	secunds
01	alarm sec
02	minutes
03	alarm min
04	hours
05	alarm hour
06	weekday (1-7, 1=sunday)
07	day of month (1-31)
08	month (1-12)
09	year (0-99)
0A	status reg A
	bit 0-3: timer rate. frq = 0x8000>>(rate-1) (2-15), in Hz (0110)
	base:30517ns 2:61035ns, 15:457763ns = base*rate
	bit 4-6: 010
	bit 7: update in progress
0B	status reg B
	bit 0: summer time on
	bit 1: 24h format (0:12h, hour.bit7 = pm)
	bit 2: binary mode (0:bcd)
	bit 3: square wave on
	bit 4: enable 1sec interrupt
	bit 5: enable timer interrupt
	bit 6: enable period interrupt
	bit 7: update in progress
0C	occured interrupts
0D	bit 7 = 1 (power on)
0E..7F	memory
*/

int toBCD(int val) {
	int rrt = val % 10;
	rrt |= ((val/10) << 4);
	return rrt;
}

// static int last_time = 0;
// static int cur_time = 0;

int rtc_read(CMOS* cms) {
	int res = -1;
	time_t rtime;
	struct tm* ctime;
	time(&rtime);
	ctime = localtime(&rtime);
//	int cur_time = (ctime->tm_hour << 16) + (ctime->tm_min << 8) + ctime->tm_sec;
	switch (cms->adr) {
		// TODO: non-bcd mode
		case 0x00: res = toBCD(ctime->tm_sec); break;
		case 0x02: res = toBCD(ctime->tm_min); break;
		case 0x04: res = toBCD(ctime->tm_hour); break;
		case 0x06: res = toBCD(ctime->tm_wday); break;
		case 0x07: res = toBCD(ctime->tm_mday); break;
		case 0x08: res = toBCD(ctime->tm_mon + 1); break;	// tm_mon = 0..11, cmos = 1..12
		case 0x09: res = toBCD(ctime->tm_year % 100); break;
		case 0x0a: res = cms->data[0x0a] & 0x7f; break; // if (cur_time == last_time) {res = 0x26;} else {res=0xa6; last_time = cur_time;} break;
		case 0x0b: res = cms->data[0x0b]; break;	// TODO: bin/12h bits
		case 0x0c: res = cms->data[0x0c]; break;
		case 0x0d: res = 0x80; break;
	}
	return res;
}

unsigned char cmos_rd(CMOS* cms, int port) {
	int res = -1;
	if (port == CMOS_DATA) {
		if (cms->adr < 0x0e) {
			res = rtc_read(cms);
		} else {
			res = cms->data[cms->adr];
		}
	}
	return res & 0xff;
}

void cmos_wr(CMOS* cms, int port, int val) {
	switch(port) {
		case CMOS_ADR:
			cms->adr = val & 0x7f;
			if (val & 0x80) {
				cms->inten &= ~CMOS_NMI;
			} else {
				cms->inten |= CMOS_NMI;
			}
			break;
		case CMOS_DATA:
			cms->data[cms->adr] = val & 0xff;
			break;
	}
}
