struct _res {
	unsigned char r;
	unsigned char f;
};

struct _flg {
	unsigned char inc;
	unsigned char dec;
	unsigned char orf;		// or & xor have same flags
	unsigned char andf;
	unsigned char sub[256][2];
	unsigned char cp[256];
	unsigned char add[256][2];
	unsigned char bit[8];
	unsigned char in;
	_res rlc;
	_res rrc;
	_res rl[2];			// sla = rl[0], sli = rl[1]
	_res rr[2];			// srl = rr[0]
	_res sra;
};

_flg flag[256];

bool parity(unsigned char op) {
	bool res = true;
	do {
		if (op & 0x01) res=!res;
		op >>= 1;
	} while (op!=0);
	return res;
}

void filltabs() {
	unsigned char rs;
	int c,i,r2,ti = 0;
	unsigned int r1;
	do {
		flag[ti].orf = (ti & (FS | F5 | F3)) | ((ti == 0)?FZ:0) | (parity(ti)?FP:0);
		flag[ti].andf = flag[ti].orf | FH;
		flag[ti].inc = (ti & (FS | F5 | F3)) | ((ti == 0)?FZ:0) | (((ti & 15)==0)?FH:0) | ((ti == 0x80)?FP:0);
		flag[ti].dec = (ti & (FS | F5 | F3)) | ((ti == 0)?FZ:0) | (((ti & 15)==15)?FH:0) | FN | ((ti == 0x7f)?FP:0);
		for (i=0; i<256; i++) {
			rs = ti - i; flag[ti].sub[i][0] = (rs & (FS | F5 | F3)) | ((rs == 0)?FZ:0) | (((ti & 15) < (i & 15))?FH:0) | (((ti > 0x7f) && (rs < 0x80))?FP:0) | FN | ((ti < i)?FC:0);
			rs = ti - i - 1; flag[ti].sub[i][1] = (rs & (FS | F5 | F3)) | ((rs == 0)?FZ:0) | (((ti & 15) - (i & 15) < 1)?FH:0) | (((ti > 0x7f) && (rs < 0x80))?FP:0) | FN | ((ti < (i+1))?FC:0);
			for (c=0; c<2; c++) {
				r1 = ti + i + c; r2 = (signed char)ti + (signed char)i + c;
				flag[ti].add[i][c] = (r1 & (FS | F5 | F3)) | ((r1 & 0xff)?0:FZ) | ((r1 > 0xff)?FC:0) | ((((ti & 15) + (i & 15) + c) & 0x10)?FH:0) | (((r2 > 0x7f) || (r2 < -0x80))?FP:0);
				r1 = ti - i - c; r2 = (signed char)ti - (signed char)i - c;
				flag[ti].sub[i][c] = (r1 & (FS | F5 | F3)) | ((r1 & 0xff)?0:FZ) | ((r1 & 0x10000)?FC:0) | ((((ti & 15) - (i & 15) - c) & 0x10)?FH:0) | FN | (((r2 > 0x7f) || (r2 < -0x80))?FP:0);
			}
			flag[ti].cp[i] = (flag[ti].sub[i][0] & ~(F5 | F3)) | (i & (F5 | F3));
			
//			rs = ti + i; flag[ti].add[i][0] = (rs & (FS | F5 | F3)) | ((rs == 0)?FZ:0) | ((((ti & 15) + (i & 15)) > 15)?FH:0) | (((ti < 0x80) && (rs > 0x7f))?FP:0) | (((ti + i) > 0xff)?FC:0);
			
//			r1++; r2++;
//			flag[ti].add[i][1] = (((r1 & 0xff) == 0)?FZ:0) | (r1 & (FS | F5 | F3)) | ((r1 > 0xff)?FC:0) | (((ti & 15) + (i & 15) + 1 > 15)?FH:0) | (((r2 > 0x80) || (r2 < -0x7f))?FP:0);
			
//			rs = ti + i + 1; flag[ti].add[i][1] = (rs & (FS | F5 | F3)) | ((rs == 0)?FZ:0) | (((ti & 15) + (i & 15) > 14)?FH:0) | (((ti < 0x80) && (rs > 0x7f))?FP:0) | (((ti + i + 1) > 0xff)?FC:0);
		}
		for (i=0; i<8; i++) {
			rs = ti & (1 << i);
			flag[ti].bit[i] = (rs & (FS | F5 | F3)) | ((rs == 0)?(FZ | FP):0) | FH;		// b3 b5 not for IX+d
//			flag[ti].bit[i] = (ti & (FS | F5 | F3)) | ((ti & (1<<i))?0:FZ) | FH;		// old
		}
/* rlc */	rs = (ti << 1) | ((ti & 0x80)?1:0); flag[ti].rlc.r = rs; flag[ti].rlc.f = (rs & (FS | F5 | F3 | FC)) | ((rs == 0)?FZ:0) | (parity(rs)?FP:0);
/* rrc */	rs = (ti >> 1) | ((ti & 1)?0x80:0); flag[ti].rrc.r = rs; flag[ti].rrc.f = (rs & (FS | F5 | F3)) | ((rs == 0)?FZ:0) | (parity(rs)?FP:0) | ((rs & 0x80)?FC:0);
/* rl */	rs = (ti << 1);
			flag[ti].rl[0].r = rs; flag[ti].rl[0].f = (rs & (FS | F5 | F3)) | ((rs == 0)?FZ:0) | (parity(rs)?FP:0) | ((ti & 0x80)?FC:0);
			rs |= 1; flag[ti].rl[1].r = rs; flag[ti].rl[1].f = (rs & (FS | F5 | F3)) | ((rs == 0)?FZ:0) | (parity(rs)?FP:0) | ((ti & 0x80)?FC:0);
/* rr */	rs = (ti >> 1);
			flag[ti].rr[0].r = rs; flag[ti].rr[0].f = (rs & (FS | F5 | F3)) | ((rs == 0)?FZ:0) | (parity(rs)?FP:0) | ((ti & 1)?FC:0);
			rs |= 0x80; flag[ti].rr[1].r = rs; flag[ti].rr[1].f = (rs & (FS | F5 | F3)) | ((rs == 0)?FZ:0) | (parity(rs)?FP:0) | ((ti & 1)?FC:0);
/* sra */	rs = (ti >> 1) | (ti & 0x80);
			flag[ti].sra.r = rs; flag[ti].sra.f = (rs & (FS | F5 | F3)) | ((rs == 0)?FZ:0) | (parity(rs)?FP:0) | ((ti & 1)?FC:0);
		flag[ti].in = (ti & (FS | F5 | F3)) | ((ti == 0)?FZ:0) | (parity(ti)?FP:0);
		ti++;
	} while (ti < 0x100);
}
