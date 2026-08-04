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
// 000	op0->op1->op2->op3->out
// 001	(op0+op2)->op1->op3->out
// 010	(op0+(op2->op1))->op3->out
// 011	((op0->op2)+op1)->op3)->out
// 100	op0->op2->out, op1->op3->out
// 101	op0->(op1,op2,op3)->out
// 110	op0->op2->out, op1->out, op3->out
// 111	op0->out, op1->out, op2->out, op3->out

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

#define SIN_BITS 10
#define SIN_LEN (1 << SIN_BITS)
#define SIN_MASK (SIN_LEN - 1)
#define SIN_SHIFT (10 - SIN_BITS)

static float sin_tab[SIN_LEN];
static int att_sin_log_tab[256];
static int pow2_tab[256];
static float pow2_m[1024];

void init_sin_tab() {
	int idx = 0;
	float ang = 0;
	while (idx < SIN_LEN) {
		sin_tab[idx] = sin(ang);
		ang += 2 * M_PI / SIN_LEN;
		idx++;
	}
	for (idx = 0; idx < 256; idx++) {
		att_sin_log_tab[idx] = round((-log2(sin((2 * idx + 1) / 512 * M_PI / 2))) * 256.0);
		pow2_tab[idx] = round(pow(2, (-(idx + 1) / 256.0)) * 2048.0);		// 0.11 [0]~1 to [255]~.5 NOTE:in fact, pretty linear
	}
	for (idx = 0; idx < 1024; idx++) {
		pow2_m[idx] = pow(2, -1.0*idx/64.0);
	}
}

static int dt_tab[8] = {0,1,2,3,0,-3,-2,-1};

// [adsr eff.rate][(ecount >> shift) & 7]
int att_inc[64][8] = {
	{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{1,0,1,0,1,0,1,0},{1,0,1,0,1,0,1,0},  // 0-3    (0x00-0x03)
	{1,0,1,0,1,0,1,0},{1,0,1,0,1,0,1,0},{1,1,1,0,1,1,1,0},{1,1,1,0,1,1,1,0},  // 4-7    (0x04-0x07)
	{1,0,1,0,1,0,1,0},{1,0,1,1,1,0,1,0},{1,1,1,0,1,1,1,0},{1,1,1,1,1,1,1,0},  // 8-11   (0x08-0x0B)
	{1,0,1,0,1,0,1,0},{1,0,1,1,1,0,1,0},{1,1,1,0,1,1,1,0},{1,1,1,1,1,1,1,0},  // 12-15  (0x0C-0x0F)
	{1,0,1,0,1,0,1,0},{1,0,1,1,1,0,1,0},{1,1,1,0,1,1,1,0},{1,1,1,1,1,1,1,0},  // 16-19  (0x10-0x13)
	{1,0,1,0,1,0,1,0},{1,0,1,1,1,0,1,0},{1,1,1,0,1,1,1,0},{1,1,1,1,1,1,1,0},  // 20-23  (0x14-0x17)
	{1,0,1,0,1,0,1,0},{1,0,1,1,1,0,1,0},{1,1,1,0,1,1,1,0},{1,1,1,1,1,1,1,0},  // 24-27  (0x18-0x1B)
	{1,0,1,0,1,0,1,0},{1,0,1,1,1,0,1,0},{1,1,1,0,1,1,1,0},{1,1,1,1,1,1,1,0},  // 28-31  (0x1C-0x1F)
	{1,0,1,0,1,0,1,0},{1,0,1,1,1,0,1,0},{1,1,1,0,1,1,1,0},{1,1,1,1,1,1,1,0},  // 32-35  (0x20-0x23)
	{1,0,1,0,1,0,1,0},{1,0,1,1,1,0,1,0},{1,1,1,0,1,1,1,0},{1,1,1,1,1,1,1,0},  // 36-39  (0x24-0x27)
	{1,0,1,0,1,0,1,0},{1,0,1,1,1,0,1,0},{1,1,1,0,1,1,1,0},{1,1,1,1,1,1,1,0},  // 40-43  (0x28-0x2B)
	{1,0,1,0,1,0,1,0},{1,0,1,1,1,0,1,0},{1,1,1,0,1,1,1,0},{1,1,1,1,1,1,1,0},  // 44-47  (0x2C-0x2F)
	{1,1,1,1,1,1,1,1},{2,1,1,1,2,1,1,1},{2,1,2,1,2,1,2,1},{2,2,2,1,2,2,2,1},  // 48-51  (0x30-0x33)
	{2,2,2,2,2,2,2,2},{4,2,2,2,4,2,2,2},{4,2,4,2,4,2,4,2},{4,4,4,2,4,4,4,2},  // 52-55  (0x34-0x37)
	{4,4,4,4,4,4,4,4},{8,4,4,4,8,4,4,4},{8,4,8,4,8,4,8,4},{8,8,8,4,8,8,8,4},  // 56-59  (0x38-0x3B)
	{8,8,8,8,8,8,8,8},{8,8,8,8,8,8,8,8},{8,8,8,8,8,8,8,8},{8,8,8,8,8,8,8,8}   // 60-63  (0x3C-0x3F)
};

// for every effecive rate (2*R + Rks)
int shift_tab[64] = {
	11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8,
	 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4,
	 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
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
// TODO: pre-calc rate,shift,sel,eg_inc[] pointer
void ym2203_eg_tick(fmOper* op, int ecount) {
	int shift;
	int rate;
	int inc;
	switch(op->state) {
		case OPST_OFF:
			break;
		case OPST_ATK:
			rate = 2 * op->eg.atkrate + op->eg.kscale;
			if (rate > 63) rate = 63;
			shift = shift_tab[rate]; // (rate < 44) ? (11 - (rate >> 2)) : 0;
			if (!(ecount & ((1 << shift) - 1))) {
				inc = (~op->eg.att * att_inc[rate][(ecount >> shift) & 7]) >> 4;	// negative value
				op->eg.att += inc;
				if (op->eg.att <= 0) {
					op->eg.att = 0;
					op->state = OPST_DEC;
				}
			}
			break;
		case OPST_DEC:
			rate = 2 * op->eg.decrate + op->eg.kscale;
			if (rate > 63) rate = 63;
			shift = shift_tab[rate]; // (rate < 44) ? (11 - (rate >> 2)) : 0;
			if (!(ecount & ((1 << shift) - 1))) {
				inc = att_inc[rate][(ecount >> shift) & 7];
				op->eg.att += inc;
				if (op->eg.att >= op->eg.suslev) {
					op->state = OPST_SUS;
				}
			}
			break;
		case OPST_SUS:
			rate = 2 * op->eg.susrate + op->eg.kscale;
			if (rate > 63) rate = 63;
			shift = shift_tab[rate]; //(rate < 44) ? (11 - (rate >> 2)) : 0;
			if (!(ecount & ((1 << shift) - 1))) {
				inc = att_inc[rate][(ecount >> shift) & 7];
				op->eg.att += inc;
				if (op->eg.att >= 1023) {
					op->eg.att = 1023;
					op->state = OPST_OFF;
				}
			}
			break;
		case OPST_REL:
			rate = 4 * op->eg.relrate + 2 + op->eg.kscale;
			if (rate > 63) rate = 63;
			shift = shift_tab[rate]; // (rate < 44) ? (11 - (rate >> 2)) : 0;
			if (!(ecount & ((1 << shift) - 1))) {
				inc = att_inc[rate][(ecount >> shift) & 7];
				op->eg.att += inc;
				if (op->eg.att >= 1023) {
					op->eg.att = 1023;
					op->state = OPST_OFF;
				}
			}
			break;
	}
	if (op->eg.envflag & 8) {
		// b0,1: 00 - repeat
		//	01 - hold volume at end of 1st loop
		//	10 - repeat with volume inversion at end of each loop
		//	11 - hold inverted volume at end of 1st loop
		// b2 - invert volume in the beninging
	}
	if (op->state == OPST_OFF) return;
	op->eg.out = op->eg.att; // + op->tlev;
	if (op->eg.out > 1023) op->eg.out = 1023;
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
	if (st && !op->key) {	// key on
		op->state = OPST_ATK;
		op->pg.phase = 0;
		op->eg.att = 1023;
		op->key = 1;
	} else if (!st && op->key) {	// key off
		op->state = OPST_REL;
		op->key = 0;
	}
}

void ym2203_op_swkey(fmOper* op) {
	ym2203_op_key(op, !op->key);
}

// update phase generator only (mod will be applied later)
// phase generator output is higher 10 bits of op->phase
void ym2203_fmop_tick(fmOper* op) {
	int step = op->pg.pstep;
	step += op->pg.detune;			// detune before multiple
	step = (step * op->pg.mult) / 2;		// op->mult is scaled x2: 1,2,4,6,8,...
	op->pg.phase += step;
}

// apply modulator and calculate output
void ym2203_fmop_exec(fmOper* op, int mod) {
	if (op->state == OPST_OFF) {
		op->out = 0;
	} else {
// op->phase is 20bits(10.10), phase is higher 10 bits of it, modulator applied to this value
// b0 of modulator has no effect, using bits 1-10 -> mod>>1
		int phase = ((op->pg.phase >> 10) + (mod >> 1)) & 0x3ff;		// result phase is 10 bits
#if 0
// sign = phase.b9
// idx = (phase.b8 ? (1ff - (phase & 1ff)) : (phase & ff)
		int psign = phase & (1 << 9);
		int idx = (phase & (1 << 8)) ? (0x1ff - (phase & 0x1ff)) : (phase & 0xff);	// sin_tab[0..1024], [0..256] is 1st quarter
// x = ((2 * idx + 1) / 512) * pi/2
// att = -log2(sin(x))
// att = (att << 8).round()	<- (4.8) 12bit attenuation : TODO: store this value in att_sin_log_table ?
		unsigned short att = att_sin_log_tab[idx];		// 4.8
// N = att + (env_att << 2)	<- (5.8) 13-bit, env_att is envelope op->amp (10 bits as 4.6, see above, shifting to make it 4.8 as att)
		att += ((op->eg.att + op->tlev) << 2);			// 5.8
// chip computes 2^(-N) as 2^(-I) * 2^(-F) where I is integer part (5 bits), F is fractal part (8 bits). 2^(-F) from table: T[i] = (2^(-(N+1)/256) << 11) : 11 bits (0.11)
//	so result is: fract=N&FF, intgr=N>>8, result=T[fract] >> intgr : 13 bit value; if (intgr>13),result=0
		int res = (pow2_tab[att & 0xff] << 2) >> (att >> 8);
		res &= ((1 << 13) - 1);
// apply sign to result: this is 14-bit output value (sign + 13 bits: if this is modulator, it adds value in range [-8pi;+8pi] to next operator)
		if (psign) res = -res;
		res >>= 3;	// to 10 bit signed
		op->out = res;
#else
// 2^(-(att + (eg.att << 2)) = 2^(-(-log2(sin(x)) + eg.att)) = 2^(log2(sin(x))*2^(-eg.att) = 2^(-eg.att)*sin(x)
		//op->out = 1024.0 * pow(2, -op->eg.out / 64.0) * sin_tab[(phase >> SIN_SHIFT) & SIN_MASK];
		op->out = 1024.0 * pow2_m[op->eg.out] * sin_tab[(phase >> SIN_SHIFT) & SIN_MASK];
#endif
//		op->out += op->tlev;
	}
}

// update phase generator for all operators
void ym2203_fmchan_tick(fmChan* ch) {
	ym2203_fmop_tick(&ch->op[0]);
	ym2203_fmop_tick(&ch->op[1]);
	ym2203_fmop_tick(&ch->op[2]);
	ym2203_fmop_tick(&ch->op[3]);
	// not connected yet, connect when ym2203_vol to get output volume
	// fully calculate op0 for proper feedback
	if (ch->op[0].feedback & 7) {
		int shift = 7 - (ch->op[0].feedback & 7);
		int mod = (ch->op[0].out + ch->op[0].outp) >> shift;	// out is previous, outp is pre-previous
		ch->op[0].outp = ch->op[0].out;		// previous is pre-previous now
		ym2203_fmop_exec(&ch->op[0], mod);	// generate new out (will be previous @ next step until new generation)
	} else {
		ch->op[0].outp = ch->op[0].out;
		ym2203_fmop_exec(&ch->op[0], 0);
	}
}

// calculate operators output + connect operators
void ym2203_fmchan_connect(fmChan* ch) {
/*
	if (ch->op[0].feedback & 7) {
		int shift = 7 - (ch->op[0].feedback & 7);
		int mod = (ch->op[0].out + ch->op[0].outp) >> shift;	// out is previous, outp is pre-previous
		ch->op[0].outp = ch->op[0].out;		// previous is pre-previous now
		ym2203_fmop_exec(&ch->op[0], mod);	// generate new out (will be previous @ next step until new generation)
	} else {
		ch->op[0].outp = ch->op[0].out;
		ym2203_fmop_exec(&ch->op[0], 0);
	}
*/
	switch(ch->algo & 7) {
		case 0:		// op0->op1->op2->op3->out
			ym2203_fmop_exec(&ch->op[1], ch->op[0].out);
			ym2203_fmop_exec(&ch->op[2], ch->op[1].out);
			ym2203_fmop_exec(&ch->op[3], ch->op[2].out);
			ch->out = ch->op[3].out;
			break;
		case 1:		// (op0+op2)->op1->op3->out
			ym2203_fmop_exec(&ch->op[2], 0);
			ym2203_fmop_exec(&ch->op[1], ch->op[0].out + ch->op[2].out);
			ym2203_fmop_exec(&ch->op[3], ch->op[2].out);
			ch->out = ch->op[3].out;
			break;
		case 2:		// (op0+(op2->op1))->op3->out
			ym2203_fmop_exec(&ch->op[2], 0);
			ym2203_fmop_exec(&ch->op[1], ch->op[2].out);
			ym2203_fmop_exec(&ch->op[3], ch->op[0].out + ch->op[1].out);
			ch->out = ch->op[3].out;
			break;
		case 3:		// ((op0->op2)+op1)->op3)->out
			ym2203_fmop_exec(&ch->op[1], 0);
			ym2203_fmop_exec(&ch->op[2], ch->op[0].out);
			ym2203_fmop_exec(&ch->op[3], ch->op[2].out + ch->op[1].out);
			ch->out = ch->op[3].out;
			break;
		case 4:		// op0->op2->out, op1->op3->out
			ym2203_fmop_exec(&ch->op[1], 0);
			ym2203_fmop_exec(&ch->op[2], ch->op[0].out);
			ym2203_fmop_exec(&ch->op[3], ch->op[1].out);
			ch->out = (ch->op[2].out + ch->op[3].out) / 2;
			break;
		case 5:		// op0->(op1,op2,op3)->out
			ym2203_fmop_exec(&ch->op[1], ch->op[0].out);
			ym2203_fmop_exec(&ch->op[2], ch->op[0].out);
			ym2203_fmop_exec(&ch->op[3], ch->op[0].out);
			ch->out = (ch->op[1].out + ch->op[2].out + ch->op[3].out) / 3;
			break;
		case 6:		// op0->op2->out, op1->out, op3->out
			ym2203_fmop_exec(&ch->op[1], 0);
			ym2203_fmop_exec(&ch->op[2], ch->op[0].out);
			ym2203_fmop_exec(&ch->op[3], 0);
			ch->out = (ch->op[1].out + ch->op[2].out + ch->op[3].out) / 3;
			break;
		case 7:		// op0->out, op1->out, op2->out, op3->out
			ym2203_fmop_exec(&ch->op[1], 0);
			ym2203_fmop_exec(&ch->op[2], 0);
			ym2203_fmop_exec(&ch->op[3], 0);
			ch->out = (ch->op[0].out + ch->op[1].out + ch->op[2].out + ch->op[3].out) / 4;
			break;
	}
	if (ch->off) ch->out = 0;
	// ch->out [-1024;1024]
	ch->out += 1024;
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

// NOTE: chip->per is half-period
void ym2203_sync(aymChip* chip, int ns) {
	if (chip->per < 1) return;

	chip->cnt -= ns;

	while (chip->cnt < 0) {
		chip->cnt += chip->per;
		chip->pscnt++;
		// temporary: fix ssg clock by 1/2 of master clock
		if (chip->pscnt & 1) {
			ay_tick(chip);
		}
		if (chip->pscnt >= chip->fmdiv) {	// pre-scaler
			chip->pscnt = 0;
			chip->fmcnt++;
			if (chip->fmcnt >= 24) {
				chip->fmcnt = 0;
				// update fm channels (op phase generators)
				ym2203_fmchan_tick(&chip->chanFM[0]);
				ym2203_fmchan_tick(&chip->chanFM[1]);
				ym2203_fmchan_tick(&chip->chanFM[2]);
				// eg update each 3 fm ticks
				chip->eg_timer++;
				if (chip->eg_timer >= 3) {
					chip->eg_timer = 0;
					chip->eg_cnt++;
					if (chip->eg_cnt == 0) chip->eg_cnt = 1;
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
									chip->reg[0xff] |= 2;
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
									chip->reg[0xff] |= 1;
								}
							}
						}
					}
				}
			}
		}
	}
}

// TODO: normal mixer
sndPair ym2203_vol(aymChip* chip) {
	sndPair v = ym_vol(chip);
	int fmv = 0;
	for (int i = 0; i < 3; i++) {
		ym2203_fmchan_connect(&chip->chanFM[i]);
		fmv += chip->chanFM[i].out * 4;
	}
	v.left += fmv / 3;
	v.right += fmv / 3;
	return v;
}

int ym2203_rd(aymChip* chip, int adr) {
	int res = -1;
	if (chip->curReg < 0x10) {
		res = ym_rd(chip, adr);
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
	chip->fmcnt <<= 1;	// halfperiod -> period
}

int calc_kscale(int frq, int blk, int ks) {
	int nte = (frq & 0x400) ? 2 : 0;
	if (((frq & 0x780) == 0x380) || ((frq & 0x400) && (frq & 0x380))) nte |= 1;
	int idx = (nte & 3) | (blk << 2);
	return idx >> (3 - ks);
}

void op_update_freq(fmChan* ch, int opn, unsigned char regl, unsigned char regh) {
	int reg = regl | (regh << 8);
	int frq = reg & 0x7ff;
	int blk = (regh >> 3) & 7;
	int stp = (frq << blk) >> 1;		// F * (2 ^ (B - 1))
	int all = !!(opn < 0);
	if (all) opn = 0;
	fmOper* op;
	do {
		op = &ch->op[opn];
		op->pg.freq = frq;
		op->pg.block = blk;
		op->pg.pstep = stp;
		op->eg.kscale = calc_kscale(frq, blk, op->eg.ks);
		opn++;
	} while (all && (opn < 4));
}

void ch_update_ch3_frq(aymChip* chip) {
	fmChan* ch3 = &chip->chanFM[2];
	if (chip->reg[0x27] & 0x40) {	// special mode
		op_update_freq(ch3, 0, chip->reg[0xa9], chip->reg[0xad]);
		op_update_freq(ch3, 1, chip->reg[0xaa], chip->reg[0xae]);
		op_update_freq(ch3, 2, chip->reg[0xa8], chip->reg[0xac]);
		op_update_freq(ch3, 3, chip->reg[0xa2], chip->reg[0xa6]);
	} else {			// common mode
		op_update_freq(ch3, -1, chip->reg[0xa2], chip->reg[0xa6]);
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
					// chip->ta_value = (chip->reg[0x24] << 2) | (chip->reg[0x25] & 3);
					chip->ta_value = chip->reg[0x24] | ((chip->reg[0x25] & 3) << 8);
					break;
				case 0x25:
					break;
				case 0x26:
					// reg[0x26] = tb_value;
					break;
				case 0x27:
					if (val & 1) {chip->ta_cnt = chip->ta_value;}
					if (val & 2) {chip->tb_cnt = chip->reg[0x26];}
					if (val & 0x10) {chip->reg[0xff] &= ~2;}	// reset state of timerA
					if (val & 0x20) {chip->reg[0xff] &= ~1;}	// reset state of timerB
					ch_update_ch3_frq(chip);
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
						op->pg.mult = (val & 0x0f) << 1;	// b0..3: multiple (x2)
						if (!op->pg.mult) op->pg.mult = 1;	// 0 -> 1/2
						op->pg.detune = dt_tab[(val >> 4) & 7];	// b4..6: detune
						break;
					case 0x40:
						op->tlev = (val & 0x7f) << 3;	// b0..5 total level
						break;
					case 0x50:
						op->eg.atkrate = val & 0x1f;	// b0..4 atk rate (0 - slow, 1f - fast)
						op->eg.ks = (val >> 6) & 3;	// b6,7 key scale (rate scale)
						op->eg.kscale = calc_kscale(op->pg.freq, op->pg.block, op->eg.ks);
						break;
					case 0x60:
						op->eg.decrate = val & 0x1f;	// b0..4 decay rate
						break;
					case 0x70:
						op->eg.susrate = val & 0x1f;	// b0..4 sustain rate
						break;
					case 0x80:
						op->eg.relrate = val & 0x0f;	// b0..3 release rate
						op->eg.suslev = (val & 0xf0) << 2;	// b4..7 sustain level
						break;
					case 0x90:
						op->eg.envflag = val & 0x0f;	// b0..3 envelope control (TODO)
						break;
					case 0xa0:
						switch (chip->curReg & 0x0c) {
							case 0x00:			// a0..a3 (ch3 op3 for special mode)
								op_update_freq(ch, -1, val, chip->reg[chip->curReg + 4]);
								ch_update_ch3_frq(chip);
								break;
							case 0x04:			// a4..a7 (high byte of a0..a3)
								break;
							case 0x08:
								ch_update_ch3_frq(chip);
								break;
							case 0x0c:
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

void ym2203_op_reset(fmOper* op) {
	op->state = OPST_OFF;
	op->eg.att = 1023;
}

void ym2203_ch_reset(fmChan* ch) {
	ym2203_op_reset(&ch->op[0]);
	ym2203_op_reset(&ch->op[1]);
	ym2203_op_reset(&ch->op[2]);
	ym2203_op_reset(&ch->op[3]);
}

void ym2203_reset(aymChip* chip) {
	ay_reset(chip);
	for (int i = 0x10; i < 256; i++) {
		chip->curReg = i;
		ym2203_wr(chip, 0, 0);
	}
	chip->divmode = 1;
	ym2203_divmode(chip);
	chip->pscnt = 0;
	chip->fmcnt = 0;
	chip->eg_cnt = 0;
	chip->eg_timer = 0;
	chip->sg_cnt = 0;
	chip->blk_fm = 0;
	ym2203_ch_reset(&chip->chanFM[0]);
	ym2203_ch_reset(&chip->chanFM[1]);
	ym2203_ch_reset(&chip->chanFM[2]);
}
