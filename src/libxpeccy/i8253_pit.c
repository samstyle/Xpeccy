// Programmable Interval Timer (PIT) i8253

#include <stddef.h>
#include <stdio.h>

#include "i8253_pit.h"

void pit_ch_reset(pitChan* ch) {
	ch->wdiv = 1;
	ch->wgat = 1;
	ch->gate = 1;
	ch->out = 1;
	ch->lout = 1;
}

void pit_reset(PIT* pit) {
	pit_ch_reset(&pit->ch0);
	pit_ch_reset(&pit->ch1);
	pit_ch_reset(&pit->ch2);
}

int pit_ch_rd(pitChan* ch) {
	int res = -1;
	if (ch->latch > 0) {
		ch->latch--;
		res = ch->clat & 0xff;
		ch->clat >>= 8;
	} else {
		switch(ch->acmod) {
			case 1:			// lsb
				res = ch->cnt & 0xff;
				ch->half = 0;
				break;
			case 2:			// msb
				res = (ch->cnt >> 8) & 0xff;
				ch->half = 0;
				break;
			case 3:			// lsb,msb
				if (ch->half) {
					res = (ch->cnt >> 8) & 0xff;
				} else {
					res = ch->cnt & 0xff;
				}
				ch->half ^= 1;
				break;
		}
	}
	return res & 0xff;
}

int pit_rd(PIT* pit, int adr) {
	int res = -1;
	switch(adr & 7) {
		case 0: res = pit_ch_rd(&pit->ch0); break;
		case 1: res = pit_ch_rd(&pit->ch1); break;
		case 2: res = pit_ch_rd(&pit->ch2); break;
	}
	return res & 0xff;
}

// mod0: wr will stop counter until full divider is set
// mod1: wr not affect counter
// mod2: wr not affect counter
// mod3: wr not affect counter
// mod4: when full divider is set, it will reload counter
// mod5: wr not affect counter
void pit_ch_wr(pitChan* ch, int val) {
	switch(ch->acmod) {
		case 1: ch->div = (ch->div & 0xff00) | (val & 0xff);
			ch->wdiv = 0;
			ch->half = 0;
			break;
		case 2: ch->div = (ch->div & 0xff) | ((val << 8) & 0xff00);
			ch->wdiv = 0;
			ch->half = 0;
			break;
		case 3: if (ch->half) {		// high byte
				ch->div = (ch->div & 0x00ff) | ((val << 8) & 0xff00);
				ch->wdiv = 0;
			} else {		// low byte
				ch->div = (ch->div & 0xff00) | (val & 0xff);
				ch->wdiv = 1;
			}
			ch->half ^= 1;
			break;
	}
	if (!ch->wdiv) {			// divider is loaded
		if (ch->cb->wr)
			ch->cb->wr(ch);
	}
}

//	on wr div	on timer			on cmd			gate
// m0	cnt=div		out=1, SIG			out=0, wait div
// m1	wgat		out=1				out=1, wait div		0->1: if div is set, out=0,cnt=div
// m2	-		out1-0-1,SIG,cnt=div		out=1, wait div		0:stop, 0->1: cnt=div,start
// m3	-		out^=1, SIG if 0-1,cnt=div	out=1, wait div		0:stop, 0->1: cnt=div,start
// m4	cnt=div		out1-0-1,SIG,(no reload)	out=1, wait div		0:stop until div is loaded
// m5	wgat		out=0				out=1, wait div		0->1: cnt=div,start
// wait div = write low or hi byte (am=1,2). if am=3, writing low byte will stop counter, and it's continues on writing high byte

// actions on divider loading
void pch_wr_m0(pitChan* ch) {ch->cnt = ch->div;}
void pch_wr_m4(pitChan* ch) {ch->cnt = ch->div;}

// actions on counter==0
void pch_tm_m0(pitChan* ch) {ch->lout = ch->out; ch->out = 1;}
void pch_tm_m1(pitChan* ch) {ch->lout = ch->out; ch->out = 1;}
void pch_tm_m2(pitChan* ch) {ch->lout = 0; ch->out = 1; ch->cnt = ch->div;}
void pch_tm_m3(pitChan* ch) {ch->lout = ch->out; ch->out ^= 1; ch->cnt = ch->div;}
void pch_tm_m4(pitChan* ch) {ch->lout = 0; ch->out = 1;}
void pch_tm_m5(pitChan* ch) {ch->out = 0;}

// actions on gate input
void pch_gt_m0(pitChan* ch, int gt) {ch->gate = gt;}		// doesn't affect in mode0

void pch_gt_m1(pitChan* ch, int gt) {
	if (!ch->wdiv) {
		if (!ch->gate && gt) {		// 0->1
			ch->lout = ch->out;
			ch->out = 0;
			ch->cnt = ch->div;
		}
	}
	ch->gate = gt;
}

void pch_gt_m2(pitChan* ch, int gt) {
	if (!gt) {			// 0
		ch->wgat = 1;
		ch->lout = ch->out;
		ch->out = 1;
	} else if (!ch->gate) {		// 0->1
		ch->cnt = ch->div;
		ch->wgat = 0;
	}
	ch->gate = gt;
}

void pch_gt_m3(pitChan* ch, int gt) {
	if (!gt) {			// x->0
		ch->wgat = 1;
	} else if (!ch->gate) {		// 0->1
		ch->cnt = ch->div;
		ch->wgat = 0;
	}
	ch->gate = gt;
}

void pch_gt_m4(pitChan* ch, int gt) {
	if (!gt) ch->wdiv = 1;
	ch->gate = gt;
}

void pch_gt_m5(pitChan* ch, int gt) {
	if (!ch->gate && gt) {			// 0->1
		ch->cnt = ch->div;
		ch->wgat = 0;
	}
	ch->gate = gt;
}

// mode0: wait for divider, then start (once)
// mode1: wait for divider, then gate 0->1, then start (once)
// mode2: wait for divider, out=1, on cnt 2->1 short out 1-0-1, reload counter. gate=0 will stop counter, out=1; gate 0->1 reload and start counter
// mode3: wait for divider, div&=~1, then start (const, 2 decrements every time), out is changing every cnt==0. gate=0 stops counter, gate=1 reload it and starts
// mode4: wait for divider, then start (const, w/o reloading). reloading divider (full) will reload counter, gate 0->1 will reload value too
// mode5: wait for divider, then gate 0->1, then start in mode4. gate=0 stops
// mode6 = mode2
// mode7 = mode3

pchCore pit_mode_tab[8] = {
	{pch_wr_m0, pch_tm_m0, pch_gt_m0},	// m0
	{NULL, pch_tm_m1, pch_gt_m1},		// m1
	{NULL, pch_tm_m2, pch_gt_m2},		// m2
	{NULL, pch_tm_m3, pch_gt_m3},		// m3
	{pch_wr_m4, pch_tm_m4, pch_gt_m4},	// m4
	{NULL, pch_tm_m5, pch_gt_m5},		// m5
	{NULL, pch_tm_m2, pch_gt_m2},		// m6 = m2
	{NULL, pch_tm_m3, pch_gt_m3},		// m7 = m3
};

void pch_set_mod(pitChan* ch, int mod) {
	ch->opmod = mod & 7;
	ch->cb = &pit_mode_tab[ch->opmod];
}

void pch_fix(pitChan* ch, int flag) {
	ch->clat = 0;
	ch->latch = 0;
	if (!(flag & 0x20)) {		// fix counter
		if (ch->acmod & 2) {	// msb
			ch->latch++;
			ch->clat |= (ch->cnt >> 8) & 0xff;
		}
		if (ch->acmod & 1) {	// lsb
			ch->latch++;
			ch->clat <<= 8;
			ch->clat |= ch->cnt & 0xff;
		}
	}
	if (!(flag & 0x10)) {		// state
		ch->clat <<= 8;
		ch->latch++;
		ch->clat = ch->state & 0x3f;
		if (ch->out) ch->clat |= 0x80;
		if (ch->wdiv) ch->clat |= 0x40;
	}
}

void pit_wr(PIT* pit, int adr, int val) {
	pitChan* ch;
	printf("pit_wr %.2X %.2X\n",adr,val);
	switch(adr & 7) {
		case 0: pit_ch_wr(&pit->ch0, val); break;
		case 1: pit_ch_wr(&pit->ch1, val); break;
		case 2: pit_ch_wr(&pit->ch2, val); break;
		case 3:
			ch = NULL;
			switch(val & 0xc0) {
				case 0x00: ch = &pit->ch0; break;
				case 0x40: ch = &pit->ch1; break;
				case 0x80: ch = &pit->ch2; break;
				case 0xc0:
					if (val & 2) pch_fix(&pit->ch0, val);
					if (val & 4) pch_fix(&pit->ch1, val);
					if (val & 8) pch_fix(&pit->ch2, val);
					break;
			}
			if (ch != NULL) {
				ch->state = val & 0x3f;
				if (val & 0x30) {
					ch->acmod = (val & 0x30) >> 4;
				} else {		// latch counter
					pch_fix(ch, 0x10);
				}
				pch_set_mod(ch, (val >> 1) & 7);
				ch->out = (ch->opmod == 0) ? 0 : 1;
				ch->wdiv = 1;
				ch->wgat = 0;
				ch->bcd = val & 1;
			}
			break;
	}
}

// mod0: if out=0, send signal and set out=1; if out=1 do nothing. out=0 on command/mode wr or divider reloading. counter will continue
// mod1: =mod0
void pit_ch_tick(pitChan* ch) {
	if (ch->wdiv || ch->wgat) return;	// wait divider or wait gate = stop counter
	ch->cnt--;
	if ((ch->opmod == 3) && ch->cnt) ch->cnt--;		// 2 times in mode 3
	if (ch->bcd) {
		// correct to bcd
		if ((ch->cnt & 0x000f) > 0x0009) ch->cnt -= 0x0006;
		if ((ch->cnt & 0x00f0) > 0x0090) ch->cnt -= 0x0060;
		if ((ch->cnt & 0x0f00) > 0x0900) ch->cnt -= 0x0600;
		if ((ch->cnt & 0xf000) > 0x9000) ch->cnt -= 0x6000;
	}
	if (ch->cnt == 0) {
		if (ch->cb->tm)
			ch->cb->tm(ch);
	}
}

void pit_sync(PIT* pit, int ns) {
	pit->ns -= ns;
	while (pit->ns < 0) {
		pit->ns += 838;		// 838ns, ~1.1933MHz
		pit_ch_tick(&pit->ch0);
		pit_ch_tick(&pit->ch1);
		pit_ch_tick(&pit->ch2);
	}
}
