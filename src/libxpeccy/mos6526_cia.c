#include "mos6526_cia.h"

#include <stdlib.h>
#include <string.h>

CIA* cia_create(int in, cbirq cb, void* p) {
	CIA* cia = (CIA*)malloc(sizeof(CIA));
	if (cia) {
		memset(cia, 0, sizeof(CIA));
		cia->xirqn = in;
		cia->xirq = cb;
		cia->xptr = p;
		cia->pard = NULL;
		cia->pawr = NULL;
		cia->pbrd = NULL;
		cia->pbwr = NULL;
	}
	return cia;
}

void cia_destroy(CIA* cia) {
	free(cia);
}

void cia_set_port(CIA* cia, int p, cbxrd cr, cbxwr cw) {
	if (p) {
		cia->pbrd = cr;
		cia->pbwr = cw;
	} else {
		cia->pard = cr;
		cia->pawr = cw;
	}
}

void cia_irq(CIA* cia, int msk) {
	cia->intrq |= msk;	// set irq
	cia->intrq &= 0x1f;
	if (cia->intrq & cia->inten) {
		cia->intrq |= 0x80;
		cia->xirq(cia->xirqn, cia->xptr);
	}
}

extern int toBCD(int);

int c64_cia_rd(CIA* cia, int adr) {
	unsigned char res = 0xff;
	switch (adr & 0x0f) {
		case 0x00: res = cia->pard ? cia->pard(adr, cia->xptr) : 0xff; break;
		case 0x01: res = cia->pbrd ? cia->pbrd(adr, cia->xptr) : 0xff;
			if (cia->timerA.flags & CIA_CR_PBXON) {res = (res & ~0x40) | (cia->reg[11] & 0x40);}
			if (cia->timerB.flags & CIA_CR_PBXON) {res = (res & ~0x80) | (cia->reg[11] & 0x80);}
			break;
		case 0x02: res = cia->portA_mask; break;
		case 0x03: res = cia->portB_mask; break;
		case 0x04: res = cia->timerA.vall; break;
		case 0x05: res = cia->timerA.valh; break;
		case 0x06: res = cia->timerB.vall; break;
		case 0x07: res = cia->timerB.valh; break;
		case 0x08: res = toBCD(cia->time.tenth); break;
		case 0x09: res = toBCD(cia->time.sec); break;
		case 0x0a: res = toBCD(cia->time.min); break;
		case 0x0b: if (cia->time.hour < 12) {
				res = toBCD(cia->time.hour);
			} else {
				res = toBCD(cia->time.hour - 12) | 0x80;
			}
			break;
		case 0x0c: res = cia->ssr; break;
		case 0x0d:
			res = cia->intrq;
			cia->intrq = 0;
			break;		// cia1:INT; cia2:NMI bits
		case 0x0e: res = cia->timerA.flags; break;
		case 0x0f: res = cia->timerB.flags; break;
	}
	return res;
}

void c64_cia_wr(CIA* cia, int adr, int val) {
	switch (adr & 0x0f) {
		case 0x00: if (cia->pawr) cia->pawr(adr, val, cia->xptr);
			break;
		case 0x01: if (cia->pbwr) cia->pbwr(adr, val, cia->xptr);
			break;
		case 0x02: cia->portA_mask = val & 0xff; break;
		case 0x03: cia->portB_mask = val & 0xff; break;
		case 0x04: cia->timerA.inil = val & 0xff; break;
		case 0x05:
			cia->timerA.inih = val & 0xff;
			if (!(cia->timerA.flags & CIA_CR_START))
				cia->timerA.value = cia->timerA.inival;
			break;
		case 0x06: cia->timerB.inil = val & 0xff; break;
		case 0x07:
			cia->timerB.inih = val;
			if (!(cia->timerB.flags & CIA_CR_START))
				cia->timerB.value = cia->timerB.inival;
			break;
		case 0x08: if (cia->timerB.flags & 0x80) {cia->alarm.tenth = val & 0xff;} else {cia->time.tenth = val & 0xff;} break;
		case 0x09: if (cia->timerB.flags & 0x80) {cia->alarm.sec = val & 0xff;} else {cia->time.sec = val & 0xff;} break;
		case 0x0a: if (cia->timerB.flags & 0x80) {cia->alarm.min = val & 0xff;} else {cia->time.min = val & 0xff;} break;
		case 0x0b: if (cia->timerB.flags & 0x80) {cia->alarm.hour = val & 0xff;} else {cia->time.hour = val & 0xff;} break;
		case 0x0c: cia->ssr = val & 0xff; break;
		case 0x0d:
			if (val & 0x80) {
				cia->inten |= (val & 0x7f);		// 1 - set state bits 0..6 where val bits is 1
			} else {
				cia->inten &= ~val;			// 0 - same but reset bits 0..6
			}
			break;
		case 0x0e: cia->timerA.flags = val & 0xff;
			if (val & CIA_CR_RELOAD)
				cia->timerA.value = cia->timerA.inival;
			break;
		case 0x0f: cia->timerB.flags = val & 0xff;
			if (val & CIA_CR_RELOAD)
				cia->timerB.value = cia->timerB.inival;
			break;
	}
}

void cia_sync_time(ciaTime* xt, int ns) {
	xt->ns += ns;
	while (xt->ns >= 1e8) {		// 1/10 sec
		xt->ns -= 1e8;
		xt->tenth++;
		if (xt->tenth > 9) {
			xt->tenth = 0;
			xt->sec++;
			if (xt->sec > 59) {
				xt->sec = 0;
				xt->min++;
				if (xt->min > 59) {
					xt->min = 0;
					xt->hour++;
					if (xt->hour > 23) {
						xt->hour = 0;
					}
				}
			}
		}
	}
}

void cia_timer_tick(ciaTimer* tmr, int mod) {
	if (!(tmr->flags & CIA_CR_START)) return;	// timer stoped
	switch (mod & 3) {
		case 0:	tmr->value--;	// CLK
			break;
		case 1:			// CNT input (TODO)
			break;
		case 2:	if (mod & 0x80)	// TB: TA overflows
				tmr->value--;
			break;
		case 3:			// TB: TA overflows while CNT is high (TODO)
			break;
	}
	if (tmr->value == 0xffff) {				// overflow
		tmr->overflow = 1;
		if (tmr->flags & CIA_CR_ONESHOT)		// one-shot - stop timer
			tmr->flags &= ~CIA_CR_START;
		if (tmr->flags & CIA_CR_RELOAD)
			tmr->value = tmr->inival;
	}
}

// ~1MHz ticks
void cia_sync(CIA* cia, int ns, int nspt) {
	cia_sync_time(&cia->time, ns);
	cia->ns += ns;
	while (cia->ns >= nspt) {
		cia->ns -= nspt;
		int mod = ((cia->timerB.flags & 0x60) >> 5);

		cia_timer_tick(&cia->timerA, (cia->timerA.flags & 0x20) >> 5);
		if ((cia->timerA.flags & (CIA_CR_PBXON | CIA_CR_TOGGLE)) == CIA_CR_PBXON)	// reset b6, reg b if it was set by timer A in 1-pulse mode
			cia->reg[11] &= ~0x40;
		if (cia->timerA.overflow) {
			mod |= 0x80;
			cia->timerA.overflow = 0;
			if (cia->timerA.flags & CIA_CR_PBXON) {			// overflow appears in bit 6 of reg B
				if (cia->timerA.flags & CIA_CR_TOGGLE) {	// 1: inverse bit 6, 0:set 1 bit but reset it on next cycle
					cia->reg[11] ^= 0x40;
				} else {
					cia->reg[11] |= 0x40;
				}
			}
			cia_irq(cia, CIA_IRQ_TIMA);
		}

		cia_timer_tick(&cia->timerB, mod);
		if ((cia->timerB.flags & (CIA_CR_PBXON | CIA_CR_TOGGLE)) == CIA_CR_PBXON)
			cia->reg[11] &= ~0x80;
		if (cia->timerB.overflow) {
			cia->timerB.overflow = 0;
			if (cia->timerB.flags & CIA_CR_PBXON) {
				if (cia->timerB.flags & CIA_CR_TOGGLE) {
					cia->reg[11] ^= 0x80;
				} else {
					cia->reg[11] |= 0x80;
				}
			}
			cia_irq(cia, CIA_IRQ_TIMB);
		}
	}
}
