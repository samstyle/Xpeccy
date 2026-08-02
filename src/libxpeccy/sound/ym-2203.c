#include "ayym.h"

#include <math.h>
#include <stdio.h>

// YM2203 = YM2149 + FM

// 3 channels
// 4 operators for each channel

// basic: F = A * sin (wC*t + I * sin(wM*t))
// multiple: F = A * sin(wC*t + I1 * sin(wM1*t + I2 * sin(wM2 * t)))
// feedback: F = A * sin(wC*t + b*F)
// A = amplitude
// I = modulation index (amplitude?)
// wC = carrier angular freq (phase step)
// wM = modulator angular freq (mod)
// b = feedback

// 21	0
// 24	timerA (bits 2-9)
// 25	timerA (b0,1)		// tA = 72 * (1024 - N) / Fclk		inc each 72 ticks, max counter is (0-1023)	0 is longest period
// 26	timerB (8 bits)		// tB = 1152 * (256 - N) / Fclk		inc each 1152 ticks, max counter is (0-255)
// 27	b0,1: load A,B; b2,3:enable A,B; b4,5:reset A,B; b6,7:ch3 mode
// ch3 mode:	00 - as ch1,ch2
//		01 - separate freq for every op, key on/off by timerA
//		1x - separate freq for every op

// 28	key on-off - b0,1 channel num (except 11b), b2,3 not used (for chips with 3+ channels), b4..8 operator key on/off

// modify divider: x0:1/2, 01:1/6, 11:1/3
// 2d	write anything: pre-scaler (1/3 or 1/6, depends on 2e/2f/reset) (modify divider x1)
// 2e	write anything: pre-scaler 1/3 for 2d (modify divider 1x)
// 2f	write anything: pre-scaler 1/2, select 1/6 for 2d (modify divider 00)
//	reset: select pre-scaler 1/6 (modify divider 01)

// 30+ regs: b0,1 of reg num = channel (0-2); b2,3 - operator (0-3); channel 3 is special
// 3x	b0..3 multiple; b4..6 detune
// 4x	b0..6 total level
// 5x	b0..4 attack rate; b6,7 key scale
// 6x	b0..4 decay rate
// 7x	b0..4 sustain rate
// 8x	b0..3 release rate; b4..7 sustain level
// 9x	b0..3 envelope control

// a0-2	F-num (low 8 bits)
// a4-6 b0..2 F-num (high 3 bits); b3..5 block (octave)
// a8-a 3ch * F-num (low)
// ac-e b0..2 3ch * F-num (hi); b3..5 3ch * block (octave)
// b0-2 b0..2 algoritm, b5..7 self-feedback level (op1 only)

// algoritms
// 000	op1->op2->op3->op4->out
// 001	(op1+op2)->op3->op4->out
// 010	((op2->op3)+op1)->op4->out
// 011	(op1->op2)+(op3->op4)->out
// 100	op1->op2->out, op3->op4->out
// 101	op1->(op2,op3,op4)->out
// 110	op1->op2->out, op3->out, op4->out
// 111	op1->out, op2->out, op3->out, op4->out

/*
 envelope form (ADSR):
 attack: speed of rising signal from min to max (0:fastest)
 decay: speed of lowing signal from max to (sustain.lev)
 sustain.lev: constant level while note is on
 sustain.rate: slightly lowing amp if sr!=0
 release: lowing signal from (sustain.lev) to 0
 _/atk \decay _sustain \release_
	 /\
	/  \______
    ___/	   \___
*/

// block = freq shifting
// F.1 F.2 = 11bit freq

// operator freq: f = (F * 2 ^ B * Fclk) / (2 ^ 20)
// F = freq; B = block (octave); Fclk = base freq (up to 4.2MHz)
// The 20-bit phase accumulator is incremented by F*2^B each internal clock cycle

// process (ChatGPT):
// 1)
// phasestep = F * (2 ^ block) = (F << block)
// apply multi (step *= multi)
// apply detune (step += detune)
// [apply lfo] <- not for 2203
// 2)
// phase += phasestep: 20bit counter, overflow = 2pi, full circle
// 3)
// add fm modulation:
// for carrier: O = phase + mod (mod is modulator operator output)
// for modulator: O = phase
// for feedback: O = phase + feedback
// 4) operator work
// out = envelope x sin(O)
// ---
// each operator have its own phase accumulator (phase)
// phase = 0 on KeyOn
// simpled:
// 1) phase = phase + ((F << block) * multi + detune)
// 4) output = envelope * sin(phase + mod)

static float sin_tab[256];

void init_sin_tab() {
	int idx = 0;
	float ang = 0;
	while (idx < 256) {
		sin_tab[idx] = (sin(ang) + 1) / 2;	// [0;1]
		ang += 2 * M_PI / 256.0;
		idx++;
	}
}

static int dt_tab[8] = {0,1,2,3,0,-3,-2,-1};

static const int eg_inc[52][8] = {
	{0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,1,1},
	{0,0,0,0,1,0,1,1},
	{0,0,0,0,1,1,1,1},

	{0,0,0,1,0,0,0,1},
	{0,0,0,1,0,0,1,1},
	{0,0,0,1,1,0,1,1},
	{0,0,0,1,1,1,1,1},

	{0,0,1,1,0,0,1,1},
	{0,0,1,1,1,0,1,1},
	{0,0,1,1,1,1,1,1},
	{0,1,1,1,0,1,1,1},

	{0,1,1,1,1,1,1,1},
	{1,1,1,1,0,1,1,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,2,1,1,1,2},

	{1,2,1,2,1,2,1,2},
	{2,2,2,2,2,2,2,2},
	{2,2,2,4,2,2,2,4},
	{2,4,2,4,2,4,2,4},

	{4,4,4,4,4,4,4,4},
	{4,4,4,8,4,4,4,8},
	{4,8,4,8,4,8,4,8},
	{8,8,8,8,8,8,8,8},

	{8,8,8,16,8,8,8,16},
	{8,16,8,16,8,16,8,16},
	{16,16,16,16,16,16,16,16},

	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},

	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},

	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},

	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},

	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},

	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0}
};

/*
// note.b0 = frq.bit11
// note.b1 = frq.((b11 & (b10 | b9 | b8)) | (!b11 & b10 & b11 & b12) -> 0111 | (!1000 & 1xxx)
// ([(block << 2) | note]) | (ks << 5)
static int keyscale_tab[32 * 4] = {
	0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,
	0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,
	0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,
	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
};
*/

// update envelope generator for operator
// TODO: envelope looping (reg 9x)
void ym2203_eg_tick(fmOper* op, int ecount) {
	int shift;
	int sel;
	int rate;
	int inc;
	switch(op->state) {
		case OPST_OFF:
			break;
		case OPST_ATK:
			rate = 2 * op->atkrate + op->kscale;
			if (rate > 63) rate = 63;
			shift = (rate < 44) ? (11 - (rate >> 2)) : 0;
			if (!(ecount & ((1 << shift) - 1))) {
				inc = op->amp >> rate;
				op->amp -= inc;
				if ((op->amp <= op->tlev) || (inc == 0)) {		// tlev 0:max
					op->amp = 0;
					op->state = OPST_DEC;
				}
			}
			break;
		case OPST_DEC:
			rate = 2 * op->decrate + op->kscale;
			if (rate > 63) rate = 63;
			shift = (rate < 44) ? (11 - (rate >> 2)) : 0;
			if (!(ecount & ((1 << shift) - 1))) {
				sel = (rate < 12) ? (rate & 3) : (rate - 12);
				inc = eg_inc[sel][(ecount >> shift) & 7];
				op->amp += inc;
				if (op->amp >= op->suslev) {
					op->state = OPST_SUS;
				}
			}
			break;
		case OPST_SUS:
			rate = 2 * op->susrate + op->kscale;
			if (rate > 63) rate = 63;
			shift = (rate < 44) ? (11 - (rate >> 2)) : 0;
			if (!(ecount & ((1 << shift) - 1))) {
				sel = (rate < 12) ? (rate & 3) : (rate - 12);
				inc = eg_inc[sel][(ecount >> shift) & 7];
				inc = rate;
				op->amp += inc;
				if (op->amp >= 1023) {
					op->amp = 1023;
					op->state = OPST_OFF;
				}
			}
			break;
		case OPST_REL:
			rate = 2 * op->relrate + op->kscale;
			if (rate > 63) rate = 63;
			shift = (rate < 44) ? (11 - (rate >> 2)) : 0;
			if (!(ecount & ((1 << shift) - 1))) {
				sel = (rate < 12) ? (rate & 3) : (rate - 12);
				inc = eg_inc[sel][(ecount >> shift) & 7];
				op->amp += inc;
				if (op->amp >= 1023) {
					op->amp = 1023;
					op->state = OPST_OFF;
				}
			}
			break;
	}
}

// update eg for all channel operators
void ym2203_cheg_tick(fmChan* ch, int ecount) {
	ym2203_eg_tick(&ch->op[0], ecount);
	ym2203_eg_tick(&ch->op[1], ecount);
	ym2203_eg_tick(&ch->op[2], ecount);
	ym2203_eg_tick(&ch->op[3], ecount);
}

// press/release key for operator
void ym2203_op_key(fmOper* op, int st) {
	if (st) {	// key on
		if (!op->key) {		// don't press twice
			op->state = OPST_ATK;
			op->phase = 0;
			op->amp = 1023;
			op->key = 1;
		}
	} else if (op->key) {		// key off (if not already off)
		op->state = OPST_REL;
		op->key = 0;
	}
}

void ym2203_op_swkey(fmOper* op) {
	ym2203_op_key(op, !op->key);
}

// update phase generator only (mod will be applied later)
void ym2203_fmop_tick(fmOper* op, int step) {
	step = (step * op->mult) / 2;			// op->mult is scaled x2: 1,2,4,6,8,...
	step += op->detune;
	op->phase += step;
}

// update phase generator for all operators
void ym2203_fmchan_tick(fmChan* ch) {
	ym2203_fmop_tick(&ch->op[0], ch->step);
	ym2203_fmop_tick(&ch->op[1], ch->step);
	ym2203_fmop_tick(&ch->op[2], ch->step);
	ym2203_fmop_tick(&ch->op[3], ch->step);
	// not connected yet, connect when ym2203_vol to get output volume
}

// special ch3 tick
void ym2203_fmch3_tick(fmChan* ch) {
	ym2203_fmop_tick(&ch->op[0], ch->spcstp[0]);
	ym2203_fmop_tick(&ch->op[1], ch->spcstp[1]);
	ym2203_fmop_tick(&ch->op[2], ch->spcstp[2]);
	ym2203_fmop_tick(&ch->op[3], ch->step);
}

// apply modulator and calculate output
void ym2203_fmop_exec(fmOper* op, int mod) {
	if (op->state == OPST_OFF) {
		op->out = 0;
	} else {
		int phase = (op->phase + mod) & ((1 << 20) - 1);
		// TODO: cut phase to highest 8/9/10 bits and take sin from table
		// op->out = (1024 - op->amp) * sin(phase * 3.1415 / (1 << 19));		// 2^20 of phase is 2pi
		op->out = (1024 - op->amp) * sin_tab[(phase >> 12) & 0xff];			// [-1024;1024] on full sin, [0;1024] on lifted sin
	}
}

// calculate operators output + connect operators
void ym2203_fmchan_connect(fmChan* ch) {
	if (ch->op[0].feedback & 7) {
		int shift = 7 - (ch->op[0].feedback & 7);
		int mod = (ch->op[0].out + ch->op[0].outp) >> shift;	// out is previous, outp is pre-previous
		ch->op[0].outp = ch->op[0].out;		// previous is pre-previous now
		ym2203_fmop_exec(&ch->op[0], mod);	// generate new out (will be previous @ next step until new generation)
	} else {
		ch->op[0].outp = ch->op[0].out;
		ym2203_fmop_exec(&ch->op[0], 0);
	}
	switch(ch->algo & 7) {
		case 0:		// op0->op1->op2->op3->out
			ym2203_fmop_exec(&ch->op[1], ch->op[0].out);
			ym2203_fmop_exec(&ch->op[2], ch->op[1].out);
			ym2203_fmop_exec(&ch->op[3], ch->op[2].out);
			ch->out = ch->op[3].out << 2;
			break;
		case 1:		// (op0+op1)->op2->op3->out
			ym2203_fmop_exec(&ch->op[1], 0);
			ym2203_fmop_exec(&ch->op[2], ch->op[0].out + ch->op[1].out);
			ym2203_fmop_exec(&ch->op[3], ch->op[2].out);
			ch->out = ch->op[3].out << 2;
			break;
		case 2:		// (op0+(op1->op2))->op3->out
			ym2203_fmop_exec(&ch->op[1], 0);
			ym2203_fmop_exec(&ch->op[2], ch->op[1].out);
			ym2203_fmop_exec(&ch->op[3], ch->op[0].out + ch->op[2].out);
			ch->out = ch->op[3].out << 2;
			break;
		case 3:		// (op0->op1)+(op2->op3)->out
			ym2203_fmop_exec(&ch->op[1], ch->op[0].out);
			ym2203_fmop_exec(&ch->op[2], 0);
			ym2203_fmop_exec(&ch->op[3], ch->op[2].out);
			ch->out = (ch->op[1].out + ch->op[3].out) << 1;
			break;
		case 4:		// op0->op1->out, op2->op3->out
			ym2203_fmop_exec(&ch->op[1], ch->op[0].out);
			ym2203_fmop_exec(&ch->op[2], 0);
			ym2203_fmop_exec(&ch->op[3], ch->op[2].out);
			ch->out = (ch->op[1].out + ch->op[3].out) << 1;
			break;
		case 5:		// op0->(op1,op2,op3)->out
			ym2203_fmop_exec(&ch->op[1], ch->op[0].out);
			ym2203_fmop_exec(&ch->op[2], ch->op[0].out);
			ym2203_fmop_exec(&ch->op[3], ch->op[0].out);
			ch->out = ch->op[1].out + ch->op[2].out + ch->op[3].out;
			break;
		case 6:		// op0->op1->out, op2->out, op3->out
			ym2203_fmop_exec(&ch->op[1], ch->op[0].out);
			ym2203_fmop_exec(&ch->op[2], 0);
			ym2203_fmop_exec(&ch->op[3], 0);
			ch->out = ch->op[1].out + ch->op[2].out + ch->op[3].out;
			break;
		case 7:		// op0->out, op1->out, op2->out, op3->out
			ym2203_fmop_exec(&ch->op[1], 0);
			ym2203_fmop_exec(&ch->op[2], 0);
			ym2203_fmop_exec(&ch->op[3], 0);
			ch->out = ch->op[0].out + ch->op[1].out + ch->op[2].out + ch->op[3].out;
			break;
	}
	// ch->out [-4096;4096] / [0..4096]
	// ch->out = (ch->out >> 1) + 2048;	// TODO: silense must be 0
	// if (ch->out < 0) ch->out = 0;
}

// timings (chatgpt):
// pre-scaler: Fclk / (2/3/6)
// each 12 pre-scaler ticks - update FM (1/2:24, 1/3:36, 1/6:72 of Fclk)
// eg_timer++ each FM-sample
// each 3 FM ticks -> update EG:
//	eg_clock() {eg_cnt++; update_eg_for_each_op();}		// eg_cnt as counter for ADSR steps (if eg_cnt && ((1 << shift) - 1) ...)
// each 6 FM ticks -> update timerA
// each 16 timerA ticks (32 eg ticks) -> update timerB

void ay_tick(aymChip*);

void ym2203_sync(aymChip* chip, int ns) {
	if (chip->per < 1) return;

	chip->cnt -= ns;

	while (chip->cnt < 0) {
		chip->cnt += chip->per;

		chip->sg_cnt--;				// psg(ssg) divider control
		if (chip->sg_cnt <= 0) {
			chip->sg_cnt = chip->sgdiv;
			ay_tick(chip);
		}

		chip->pscnt++;
		if (chip->pscnt >= chip->fmdiv) {	// pre-scaler
			chip->pscnt = 0;
			chip->fmcnt++;
			if (chip->fmcnt >= 12) {	// fm sampling (12 psticks)
				chip->fmcnt = 0;
				// update fm channels (op phase generators)
				ym2203_fmchan_tick(&chip->chanFM[0]);
				ym2203_fmchan_tick(&chip->chanFM[1]);
				if (chip->reg[0x27] & 0xc0) {
					ym2203_fmch3_tick(&chip->chanFM[2]);
				} else {
					ym2203_fmchan_tick(&chip->chanFM[2]);
				}
				// eg update each 3 fm ticks
				chip->eg_timer++;
				if (chip->eg_timer >= 3) {
					chip->eg_timer = 0;
					chip->eg_cnt++;
					ym2203_cheg_tick(&chip->chanFM[0], chip->eg_cnt);
					ym2203_cheg_tick(&chip->chanFM[1], chip->eg_cnt);
					ym2203_cheg_tick(&chip->chanFM[2], chip->eg_cnt);
					if (chip->eg_cnt & 1) {				// /2: each 6 fm ticks, update timerA (6 * 12 = 72 psticks)
						if (chip->reg[0x27] & 1) {		// running
							chip->ta_cnt--;
							if (chip->ta_cnt <= 0) {	// overflow
								chip->ta_cnt = (1024 - chip->ta_value);
								if (chip->reg[0x27] & 4) {	// irq enabled
									// irq
									// switch ch3 op keys
									if ((chip->reg[0x27] & 0xc0) == 0x40) {		// special mode, change keys state
										ym2203_op_swkey(&chip->chanFM[2].op[0]);
										ym2203_op_swkey(&chip->chanFM[2].op[1]);
										ym2203_op_swkey(&chip->chanFM[2].op[2]);
										ym2203_op_swkey(&chip->chanFM[2].op[3]);
									}
								}
							}
						}
					}
					if (!(chip->eg_cnt & 0x1f)) {			// /32: each 96 fm ticks update timerB (96 * 12 = 1152 psticks)
						if (chip->reg[0x27] & 2) {		// running
							chip->tb_cnt--;
							if (chip->tb_cnt <= 0) {	// overflow
								chip->tb_cnt = (256 - chip->reg[0x26]);
								if (chip->reg[0x27] & 8) {	// irq enabled
									// irq
								}
							}
						}
					}
				}
			}
		}
	}
}

sndPair ym2203_vol(aymChip* chip) {
	sndPair v = ym_vol(chip);
	int fmv = 0;
	for (int i = 0; i < 3; i++) {
		ym2203_fmchan_connect(&chip->chanFM[i]);
		fmv += chip->chanFM[i].out << 2;
	}
	v.left += fmv;
	v.right += fmv;
	return v;
}

int ym2203_rd(aymChip* chip, int adr) {
	int res = -1;
	if (chip->curReg < 0x10) {
		res = ym_rd(chip, 1);
	}
	return res;
}

void ym2203_divmode(aymChip* chip) {
	switch (chip->divmode) {
		case 0:
		case 2:
			chip->fmdiv = 2;
			chip->sgdiv = 1;
			break;
		case 1:
			chip->fmdiv = 6;
			chip->sgdiv = 4;
			break;
		case 3:
			chip->fmdiv = 3;
			chip->sgdiv = 2;
			break;
	}
}

void ch_calc_kscale(fmChan* ch) {
	int note = (ch->freq & 0x400) ? 2 : 0;
	if (((ch->freq & 0x780) == 0x380) || ((ch->freq & 0x400) && (ch->freq & 0x380))) note |= 1;
	int idx = (note & 3) | ((ch->block & 7) << 2);
	for (int i = 0; i < 4; i++) {
		ch->op[i].kscale = idx >> (3 - ch->op[i].ks);
	}
}

extern void ay_set_reg(aymChip*, int);

void ym2203_wr(aymChip* chip, int adr, int val) {
	if (adr & 1) {
		chip->curReg = val & 0xff;
	} else {
		fmChan* ch;
		chip->reg[chip->curReg] = val & 0xff;
		if (chip->curReg < 0x10) {
			ay_set_reg(chip, val);
		} else if (chip->curReg < 0x30) {		// 20..2f
			switch (chip->curReg) {
				case 0x24:
					chip->ta_value &= 3;
					chip->ta_value |= (val << 2);
					break;
				case 0x25:
					chip->ta_value &= 0x3fc;
					chip->ta_value |= (val & 3);
					break;
				case 0x26:
					// reg[0x26] = tb_value;
					break;
				case 0x27:
					if (val & 1) {
						chip->ta_cnt = chip->ta_value;
					}
					if (val & 2) {
						chip->tb_cnt = chip->reg[0x26];
					}
					if (val & 0x10) {chip->reg[0x27] &= ~0x15;}	// reset state of timerA
					if (val & 0x20) {chip->reg[0x27] &= ~0x2a;}	// reset state of timerB
					break;
				case 0x28:
					if ((val & 3) == 3) break;
					ch = &chip->chanFM[val & 3];
					ym2203_op_key(&ch->op[0], val & 0x10);
					ym2203_op_key(&ch->op[1], val & 0x20);
					ym2203_op_key(&ch->op[2], val & 0x40);
					ym2203_op_key(&ch->op[3], val & 0x80);
					break;
				case 0x2d:
					chip->divmode |= 1;
					ym2203_divmode(chip);
					break;
				case 0x2e:
					chip->divmode |= 2;
					ym2203_divmode(chip);
					break;
				case 0x2f:
					chip->divmode = 0;
					ym2203_divmode(chip);
					break;
			}
		} else {					// 3x..bx
			int chn = chip->curReg & 3;		// channel number
			int opn = (chip->curReg >> 2) & 3;	// operator number
			if (chn != 3) {
				ch = &chip->chanFM[chn];
				fmOper* op = &ch->op[opn];
				switch (chip->curReg & 0xf0) {
					case 0x30:
						op->mult = (val & 0x0f) << 1;	// b0..3: multiple (x2)
						if (!op->mult) op->mult = 1;	// 0 -> 1/2
						op->detune = dt_tab[(val >> 4) & 7];	// b4..6: detune
						break;
					case 0x40:
						op->tlev = (val & 0x7f) << 3;	// b0..5 total level
						break;
					case 0x50:
						op->atkrate = val & 0x1f;	// b0..4 atk rate (0 - slow, 1f - fast)
						op->ks = (val >> 6) & 3;	// b6,7 key scale (rate scale)
						ch_calc_kscale(ch);
						// kscale = ((block << 2) | (note & 3)) >> (3 - v)
						break;
					case 0x60:
						op->decrate = val & 0x1f;	// b0..4 decay rate
						break;
					case 0x70:
						op->susrate = val & 0x1f;	// b0..4 sustain rate
						break;
					case 0x80:
						op->relrate = val & 0x0f;	// b0..3 release rate
						op->suslev = (val & 0xf0) << 2;	// b4..7 sustain level
						break;
					case 0x90:
						op->envflag = val & 0x0f;	// b0..3 envelope control (TODO)
						break;
					case 0xa0:
						switch (chip->curReg & 0x0c) {
							case 0x00:			// a0..a3 (ch3 op3 for special mode)
								ch->freq = (ch->freq & 0x700) | (val & 0xff);
								ch->step = (ch->freq << (ch->block & 7));
								break;
							case 0x04:			// a4..a7
								ch->freq = (ch->freq & 0xff) | ((val << 8) & 0x700);
								ch->block = (val >> 3) & 0x07;
								ch->step = (ch->freq << (ch->block & 7));
								ch_calc_kscale(ch);
								break;
							default:
								switch (chip->curReg) {
									case 0xa8:		// ch3 op2
										chip->chanFM[2].spcfrq[2] = (chip->chanFM[2].spcfrq[2] & 0x700) | (val & 0xff);
										chip->chanFM[2].spcstp[2] = (chip->chanFM[2].spcfrq[2] << (chip->chanFM[2].spcblk[2] & 7));
										break;
									case 0xa9:		// ch3 op0
										chip->chanFM[2].spcfrq[0] = (chip->chanFM[2].spcfrq[0] & 0x700) | (val & 0xff);
										chip->chanFM[2].spcstp[0] = (chip->chanFM[2].spcfrq[0] << (chip->chanFM[2].spcblk[0] & 7));
										break;
									case 0xaa:		// ch3 op1
										chip->chanFM[2].spcfrq[1] = (chip->chanFM[2].spcfrq[1] & 0x700) | (val & 0xff);
										chip->chanFM[2].spcstp[1] = (chip->chanFM[2].spcfrq[1] << (chip->chanFM[2].spcblk[1] & 7));
										break;
									case 0xac:		// ch3 op2
										chip->chanFM[2].spcfrq[2] = (chip->chanFM[2].spcfrq[2] & 0x0ff) | ((val << 8) & 0x700);
										chip->chanFM[2].spcblk[2] = (val >> 3) & 7;
										chip->chanFM[2].spcstp[2] = (chip->chanFM[2].spcfrq[2] << (chip->chanFM[2].spcblk[2] & 7));
										break;
									case 0xad:		// ch3 op0
										chip->chanFM[2].spcfrq[0] = (chip->chanFM[2].spcfrq[0] & 0x0ff) | ((val << 8) & 0x700);
										chip->chanFM[2].spcblk[0] = (val >> 3) & 7;
										chip->chanFM[2].spcstp[0] = (chip->chanFM[2].spcfrq[0] << (chip->chanFM[2].spcblk[0] & 7));
										break;
									case 0xae:		// ch3 op1
										chip->chanFM[2].spcfrq[1] = (chip->chanFM[2].spcfrq[1] & 0x0ff) | ((val << 8) & 0x700);
										chip->chanFM[2].spcblk[1] = (val >> 3) & 7;
										chip->chanFM[2].spcstp[1] = (chip->chanFM[2].spcfrq[1] << (chip->chanFM[2].spcblk[1] & 7));
										break;

								}
								break;
						}
						break;
					case 0xb0:
						ch->algo = val & 7;
						ch->op[0].feedback = (val >> 5) & 7;
						break;
				}
			}
		}
	}
}

void ym2203_reset(aymChip* chip) {
	ay_reset(chip);
	for (int i = 0x10; i < 256; i++) {
		chip->curReg = i;
		ym2203_wr(chip, 0, 0);
	}
	chip->divmode = 3;
	ym2203_divmode(chip);
	chip->pscnt = 0;
	chip->fmcnt = 0;
	chip->eg_cnt = 0;
	chip->eg_timer = 0;
	chip->sg_cnt = 0;
}
