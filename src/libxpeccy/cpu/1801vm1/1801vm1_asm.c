#include <string.h>
#include <stdio.h>

#include "1801vm1.h"

// asm

static const char* ptr;
static unsigned short code;
static int src = -1;
static int srce = -1;
static int dst = -1;
static int dste = -1;
static int tmpe = -1;
static xAsmScan res;
static opCode pdpopc;

extern xPdpDasm pdp11_dasm_tab[];

typedef struct {
	const char* name;
	int idx;
} pdpRegNums;

// NOTE: # is already replaced by 0x
pdpRegNums pdpadrs[] = {
	{"rN",0},{"@rN",1},{"(rN)+",2},{"@(rN)+",3},
	{"-(rN)",4},{"@-(rN)",5},{"E(rN)",6},{"@E(rN)",7},
	{"E",8},{"0xE",8},
	{"(rN+E)",6},{"(E+rN)",6},		// something special (alternative addressation)
	{"@(rN+E)",7},{"@(E+rN)",7},
	{"@E",9},{"@0xE",9},{"(E)",9},
	{NULL,-1}
};

int pdpCheckE(const char** pp) {
	const char* p = *pp;
	// check 1st symbol (is number)
	if (*p < '0') return -1;
	if (*p > '7') return -1;
	int r = 0;
	while ((*p >= '0') && (*p <= '7')) {
		r <<= 3;
		r |= (*p - '0');
		p++;
	}
	if (r > 0xffff) {
		r = -1;
	}
	*pp = p;
	return r;
}

int pdpCheckEShift(const char** pp, int adr) {
	const char* p = *pp;
	int r = -1;
	if (*p == '-') {
		p++;
		r = pdpCheckE(&p);
		if (r < 0) {
			r = 0x10000;		// overflow
		} else {
			r = -r;
		}
	} else if (*p == '+') {
		p++;
		r = pdpCheckE(&p);
		if (r < 0) {
			r = 0x10000;
		}
	} else {
		r = pdpCheckE(&p);
		if (r < 0) {
			r = 0x10000;
		} else {
			r = r - (adr + 2);
		}
	}
	*pp = p;
	return r;
}

int pdpGetAdrIdx() {
	int i = 0;
	int r = -1;
	int n = 0;
	int e = 0;
	int w;
	const char* p;
	const char* s;
	while ((pdpadrs[i].name != NULL) && (r < 0)) {
		p = ptr;
		s = pdpadrs[i].name;
		w = 1;
		tmpe = -1;
		while (w) {
			switch (*s) {
				case 0x00:
					w = 0;
					r = pdpadrs[i].idx;
					switch(r) {
						case 8: r = 0x97 | (tmpe << 8); break;
						case 9: r = 0x9f | (tmpe << 8); break;
						default: r = (r << 3) | (n & 7); break;
					}
					ptr = p;
					break;
				case 'N':				// check number 0..7 and remember it
					if ((*p >= '0') && (*p <= '7')) {
						n = *p - '0';
						s++;
						p++;
					} else {
						w = 0;
					}
					break;
				case 'E':				// check if variable len substring is number 0..65535 (base:oct?)
					s++;
					e = pdpCheckE(&p);		// p is changing
					if (e < 0) {
						w = 0;			// not number or overflow
					} else {
						tmpe = e;
					}
					break;
				default:				// other symbols must match
					if (*p == *s) {
						p++;
						s++;
					} else {
						w = 0;
					}
					break;
			}
		}
		i++;
	}
	return r;
}

// check if string on ptr match addressation
int pdpasm_check_adr() {
	int r = -1;		// result: on succes = (e<<8) | (0x80 if e exists) | (t << 3) | n; on error = -1;

	// replace PC->R7, SP->R6 (unification)
	char* p;
	do {
		p = strstr(ptr,"sp");
		if (p) {*(p++) = 'r'; *p = '6';}
	} while (p);
	do {
		p = strstr(ptr,"pc");
		if (p) {*(p++) = 'r'; *p = '7';}
	} while (p);

	r = pdpGetAdrIdx();
	if ((r >= 0) && (tmpe >= 0)) {
		r |= 0x80;		// set 'there is data'
		r |= (tmpe << 8);	// store data in result
	}
	return r;
}

int pdpasm_check_src() {
	int r = pdpasm_check_adr();
	if (r < 0) return 0;
	src = r & 0x3f;
	if (r & 0x80) {
		srce = (r >> 8) & 0xffff;
	} else {
		srce = -1;
	}
	return 1;
}

int pdpasm_check_dst() {
	int r = pdpasm_check_adr();
	if (r < 0) return 0;
	dst = r & 0x3f;
	if (r & 0x80) {
		dste = (r >> 8) & 0xffff;
	} else {
		dste = -1;
	}
	return 1;
}

xAsmScan pdp11_asm(int adr, const char* mnm, char* buf) {
	res.match = 0;
	int idx = 0;
	const char* mp;
	char ch;
	int work;
	int e;
	xPdpDasm* ent = &pdp11_dasm_tab[0];
	while ((ent->mask != 0) && !res.match) {
		ptr = mnm;
		res.ptr = buf;
		mp = ent->mnem;
		src = srce = dst = dste = -1;
		work = 1;
		code = ent->code;			// base value, modify it with src/dst/etc
		while (work) {
			if (*mp == ':') {		// special symbol
				mp++;
				ch = *(mp++);		// type of symbol
				switch (ch) {
					case 's':
						if (pdpasm_check_src()) {	// check addressation on ptr & set src/srce (bit 6..11) & return !0 on success
							code &= ~(0x3f << 6);
							code |= ((src & 0x3f) << 6);
						} else {
							work = 0;
						}
						break;
					case 'd':
						if (pdpasm_check_dst()) {	// same as check_src, but set dst/dste on success (bit 0..5)
							code &= ~0x3f;
							code |= (dst & 0x3f);
						} else {
							work = 0;
						}
						break;
					case '0':				// 0..7 as bit 0..2 in code
						if ((*ptr < '0') || (*ptr > '7')) {
							work = 0;
						} else {
							code &= ~7;
							code |= ((*ptr - '0') & 7);
							ptr++;
						}
						break;
					case '6':				// 0..7 as bit 6..8 in code
						if ((*ptr < '0') || (*ptr > '7')) {
							work = 0;
						} else {
							code &= ~(7 << 6);
							code |= (((*ptr - '0') & 7) << 6);
							ptr++;
						}
						break;
					case 'x':				// lower 8 bits of opcode, number 00..FF
						e = pdpCheckE(&ptr);
						if ((e < 0) || (e > 0xff)) {
							work = 0;
						} else {
							code &= ~0xff;
							code |= (e & 0xff);
						}
						break;
					case 'f':
						e = 0;
						while (!(e & 0x80) && work) {
							switch(*ptr) {
								case 0x00: e |= 0x80; break;
								case 'c': e |= 1; break;
								case 'v': e |= 2; break;
								case 'z': e |= 4; break;
								case 'n': e |= 8; break;
								default: work = 0; break;
							}
							if (!(e & 0x80)) ptr++;
						}
						if (e & 0x80) {
							code |= (e & 0x0f);
						} else {
							work = 0;
						}
						break;
					case 'e':				// b0..7 = shift/2 (-254..+254)
						e = pdpCheckEShift(&ptr, adr);
						if (!(e & 1) && (e > -255) && (e < 255)) {
							code &= ~0xff;
							code |= ((e / 2) & 0xff);
						} else {
							work = 0;
						}
						break;
					case 'j':				// b0..6 = shift back/2 (-126..0)
						e = pdpCheckEShift(&ptr, adr);
						if (!(e & 1) && (e > -127) && (e < 1)) {
							code &= ~0x3f;
							code |= (((-e) / 2) & 0x3f);
						} else {
							work = 0;
						}
						break;
					default:
						work = 0;
						break;
				}
			} else if (*mp == *ptr) {	// still matched
				if (*mp == 0) {		// end of mnem
					res.match = 1;	// found
					work = 0;	// stop checking
					*(res.ptr++) = code & 0xff;
					*(res.ptr++) = (code >> 8) & 0xff;
					if (srce >= 0) {
						*(res.ptr++) = srce & 0xff;
						*(res.ptr++) = (srce >> 8) & 0xff;
					}
					if (dste >= 0) {
						*(res.ptr++) = dste & 0xff;
						*(res.ptr++) = (dste >> 8) & 0xff;
					}
				} else {		// to next symbol
					mp++;
					ptr++;
				}
			} else {			// not matched
				work = 0;
			}
		}
		idx++;
		ent = &pdp11_dasm_tab[idx];
	}

	res.op = &pdpopc;
	pdpopc.flag = 0;
	pdpopc.mnem = NULL;

	return res;
}
