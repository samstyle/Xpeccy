#include "upd4990.h"

#include <stdlib.h>
#include <string.h>

#define RTC_PERIOD 244140

upd4990* upd4990_create(cbirq f, void* p) {
	upd4990* rtc = (upd4990*)malloc(sizeof(upd4990));
	if (rtc) {
		memset(rtc, 0x00, sizeof(upd4990));
		upd4990_reset(rtc);
	}
	return rtc;
}

void upd4990_destroy(upd4990* rtc) {
	free(rtc);
}

void upd4990_reset(upd4990* rtc) {
	rtc->test = 0;
	rtc->rh = 1;
	rtc->stp = 1;
	rtc->time = RTC_PERIOD;
}

void upd4990_sync(upd4990* rtc, int ns) {
	rtc->time -= ns;
	while (rtc->time <= 0) {		// tick time
		rtc->time += RTC_PERIOD;
		if (!rtc->stp) {		// if not stopped
			rtc->tp_cnt--;
			if (rtc->tp_cnt < 1) {	// if counter reach 0
				rtc->tp_cnt = rtc->tp_per;		// set new period
				rtc->xirq(IRQ_RTC_TP, rtc->xptr);	// send signal
			}
		}
	}
}

// b0,1,2: c0,1,2
//	000	register hold		data out=1Hz
//	001	register shift		data out=lsb (0,1)
//	010	time set & counter hold	data out=lsb
//	011	time read		data out=1Hz
//	100	tp=64Hz
//	101	tp=256Hz
//	110	tp=2048Hz
//	111	serial command transfer mode (see below, shift register is 52bits)
//	0111	tp=4096Hz
//	1000	tp=1sec (reset counter & start)
//	1001	tp=10sec (-"-)
//	1010	tp=30sec (-"-)
//	1011	tp=60sec (-"-)
//	1100	interval output flag reset
//	1101	interval timer clock run
//	1110	interval timer clock stop
//	1111	test mode
// b3: stb	1->0 execute command
// b4: clk	1->0 write DI to C3'..C0' command register (shift right + write C3'); if not held, send LSB of output register to DO and shift it
// b5: di	data in
// data shift register: 1:second(bcd) 2:minute(bcd) 3:hour(bcd) 4:day(bcd) 5:month(hex),4bits 6:year(bcd,00-99), 7:coma(4bits)

void upd4990_set_period(upd4990* rtc, int per) {
	rtc->tp_per = per;
	rtc->tp_cnt = per;
	rtc->time = RTC_PERIOD;
}

void upd4990_wr(upd4990* rtc, int val) {
	rtc->com = val & 7;
	if (rtc->clk && !(val & 16)) {	// clk 1->0
		rtc->coma >>= 1;
		if (val & 0x20) {
			rtc->coma |= 16;
		}
		if (!rtc->rh) {
			rtc->data = rtc->reg & 1;
			rtc->reg >>= 1;
		}
	}
	rtc->clk = !!(val & 16);
	if (rtc->stb && !(val & 8)) {	// stb 1->0
		unsigned char fcom = rtc->com;
		if (fcom == 7) {
			fcom = rtc->coma;
		} else {
			rtc->test = 0;
		}
		switch (fcom) {
			case 0: rtc->rh = 1; break;
			case 1: rtc->rh = 0; break;
			case 2: break;			// TODO
			case 3:				// time read: to form reg
				break;
			case 4:	upd4990_set_period(rtc, 64); break;		// 64 Hz (4096/64 = 64)
			case 5:	upd4990_set_period(rtc, 16); break;		// 256 Hz (4096/256 = 16)
			case 6:	upd4990_set_period(rtc, 2); break;		// 2048 Hz
			case 7: upd4990_set_period(rtc, 1); break;		// 4096 Hz
			case 8: upd4990_set_period(rtc, 4096); break;		// 1 sec
			case 9: upd4990_set_period(rtc, 4096 * 10); break;	// 10 sec
			case 10: upd4990_set_period(rtc, 4096 * 30); break;	// 30 sec
			case 11: upd4990_set_period(rtc, 4096 * 60); break;	// 60 sec
			case 12: break;
			case 13: rtc->stp = 0; rtc->tp_cnt = rtc->tp_per; rtc->time = RTC_PERIOD; break;
			case 14: rtc->stp = 1; break;
			case 15: rtc->test = 1; break;				// test mode
		}
	}
	rtc->stb = !!(val & 8);
}
