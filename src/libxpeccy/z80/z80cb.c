// 00..07	rlc	4 [4rd 3wr]
void cb00(Z80CPU* cpu) {RLC(cpu->b);}
void cb01(Z80CPU* cpu) {RLC(cpu->c);}
void cb02(Z80CPU* cpu) {RLC(cpu->d);}
void cb03(Z80CPU* cpu) {RLC(cpu->e);}
void cb04(Z80CPU* cpu) {RLC(cpu->h);}
void cb05(Z80CPU* cpu) {RLC(cpu->l);}
void cb06(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,4); RLC(cpu->tmpb); MEMWR(cpu->hl,cpu->tmpb,3);}
void cb07(Z80CPU* cpu) {RLC(cpu->a);}
// 08..0f	rrc
void cb08(Z80CPU* cpu) {RRC(cpu->b);}
void cb09(Z80CPU* cpu) {RRC(cpu->c);}
void cb0A(Z80CPU* cpu) {RRC(cpu->d);}
void cb0B(Z80CPU* cpu) {RRC(cpu->e);}
void cb0C(Z80CPU* cpu) {RRC(cpu->h);}
void cb0D(Z80CPU* cpu) {RRC(cpu->l);}
void cb0E(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,4); RRC(cpu->tmpb); MEMWR(cpu->hl,cpu->tmpb,3);}
void cb0F(Z80CPU* cpu) {RRC(cpu->a);}
// 10..17	rl
void cb10(Z80CPU* cpu) {RL(cpu->b);}
void cb11(Z80CPU* cpu) {RL(cpu->c);}
void cb12(Z80CPU* cpu) {RL(cpu->d);}
void cb13(Z80CPU* cpu) {RL(cpu->e);}
void cb14(Z80CPU* cpu) {RL(cpu->h);}
void cb15(Z80CPU* cpu) {RL(cpu->l);}
void cb16(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,4); RL(cpu->tmpb); MEMWR(cpu->hl,cpu->tmpb,3);}
void cb17(Z80CPU* cpu) {RL(cpu->a);}
// 18..1f	rr
void cb18(Z80CPU* cpu) {RR(cpu->b);}
void cb19(Z80CPU* cpu) {RR(cpu->c);}
void cb1A(Z80CPU* cpu) {RR(cpu->d);}
void cb1B(Z80CPU* cpu) {RR(cpu->e);}
void cb1C(Z80CPU* cpu) {RR(cpu->h);}
void cb1D(Z80CPU* cpu) {RR(cpu->l);}
void cb1E(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,4); RR(cpu->tmpb); MEMWR(cpu->hl,cpu->tmpb,3);}
void cb1F(Z80CPU* cpu) {RR(cpu->a);}
// 20..27	sla
void cb20(Z80CPU* cpu) {SLA(cpu->b);}
void cb21(Z80CPU* cpu) {SLA(cpu->c);}
void cb22(Z80CPU* cpu) {SLA(cpu->d);}
void cb23(Z80CPU* cpu) {SLA(cpu->e);}
void cb24(Z80CPU* cpu) {SLA(cpu->h);}
void cb25(Z80CPU* cpu) {SLA(cpu->l);}
void cb26(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,4); SLA(cpu->tmpb); MEMWR(cpu->hl,cpu->tmpb,3);}
void cb27(Z80CPU* cpu) {SLA(cpu->a);}
// 28..2f	sra
void cb28(Z80CPU* cpu) {SRA(cpu->b);}
void cb29(Z80CPU* cpu) {SRA(cpu->c);}
void cb2A(Z80CPU* cpu) {SRA(cpu->d);}
void cb2B(Z80CPU* cpu) {SRA(cpu->e);}
void cb2C(Z80CPU* cpu) {SRA(cpu->h);}
void cb2D(Z80CPU* cpu) {SRA(cpu->l);}
void cb2E(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,4); SRA(cpu->tmpb); MEMWR(cpu->hl,cpu->tmpb,3);}
void cb2F(Z80CPU* cpu) {SRA(cpu->a);}
// 30..37	sll
void cb30(Z80CPU* cpu) {SLL(cpu->b);}
void cb31(Z80CPU* cpu) {SLL(cpu->c);}
void cb32(Z80CPU* cpu) {SLL(cpu->d);}
void cb33(Z80CPU* cpu) {SLL(cpu->e);}
void cb34(Z80CPU* cpu) {SLL(cpu->h);}
void cb35(Z80CPU* cpu) {SLL(cpu->l);}
void cb36(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,4); SLL(cpu->tmpb); MEMWR(cpu->hl,cpu->tmpb,3);}
void cb37(Z80CPU* cpu) {SLL(cpu->a);}
// 38..3f	srl
void cb38(Z80CPU* cpu) {SRL(cpu->b);}
void cb39(Z80CPU* cpu) {SRL(cpu->c);}
void cb3A(Z80CPU* cpu) {SRL(cpu->d);}
void cb3B(Z80CPU* cpu) {SRL(cpu->e);}
void cb3C(Z80CPU* cpu) {SRL(cpu->h);}
void cb3D(Z80CPU* cpu) {SRL(cpu->l);}
void cb3E(Z80CPU* cpu) {cpu->tmpb = MEMRD(cpu->hl,4); SRL(cpu->tmpb); MEMWR(cpu->hl,cpu->tmpb,3);}
void cb3F(Z80CPU* cpu) {SRL(cpu->a);}

// 40..47	bit 0,r		4 [4rd]
void cb40(Z80CPU* cpu) {BIT(0,cpu->b);}
void cb41(Z80CPU* cpu) {BIT(0,cpu->c);}
void cb42(Z80CPU* cpu) {BIT(0,cpu->d);}
void cb43(Z80CPU* cpu) {BIT(0,cpu->e);}
void cb44(Z80CPU* cpu) {BIT(0,cpu->h);}
void cb45(Z80CPU* cpu) {BIT(0,cpu->l);}
void cb46(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); BIT(0,cpu->tmp);}
void cb47(Z80CPU* cpu) {BIT(0,cpu->a);}
// 48..4f	bit 1,r
void cb48(Z80CPU* cpu) {BIT(1,cpu->b);}
void cb49(Z80CPU* cpu) {BIT(1,cpu->c);}
void cb4A(Z80CPU* cpu) {BIT(1,cpu->d);}
void cb4B(Z80CPU* cpu) {BIT(1,cpu->e);}
void cb4C(Z80CPU* cpu) {BIT(1,cpu->h);}
void cb4D(Z80CPU* cpu) {BIT(1,cpu->l);}
void cb4E(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); BIT(1,cpu->tmp);}
void cb4F(Z80CPU* cpu) {BIT(1,cpu->a);}
// 50..57	bit 2,r
void cb50(Z80CPU* cpu) {BIT(2,cpu->b);}
void cb51(Z80CPU* cpu) {BIT(2,cpu->c);}
void cb52(Z80CPU* cpu) {BIT(2,cpu->d);}
void cb53(Z80CPU* cpu) {BIT(2,cpu->e);}
void cb54(Z80CPU* cpu) {BIT(2,cpu->h);}
void cb55(Z80CPU* cpu) {BIT(2,cpu->l);}
void cb56(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); BIT(2,cpu->tmp);}
void cb57(Z80CPU* cpu) {BIT(2,cpu->a);}
// 58..5f	bit 3,r
void cb58(Z80CPU* cpu) {BIT(3,cpu->b);}
void cb59(Z80CPU* cpu) {BIT(3,cpu->c);}
void cb5A(Z80CPU* cpu) {BIT(3,cpu->d);}
void cb5B(Z80CPU* cpu) {BIT(3,cpu->e);}
void cb5C(Z80CPU* cpu) {BIT(3,cpu->h);}
void cb5D(Z80CPU* cpu) {BIT(3,cpu->l);}
void cb5E(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); BIT(3,cpu->tmp);}
void cb5F(Z80CPU* cpu) {BIT(3,cpu->a);}
// 60..67	bit 4,r
void cb60(Z80CPU* cpu) {BIT(4,cpu->b);}
void cb61(Z80CPU* cpu) {BIT(4,cpu->c);}
void cb62(Z80CPU* cpu) {BIT(4,cpu->d);}
void cb63(Z80CPU* cpu) {BIT(4,cpu->e);}
void cb64(Z80CPU* cpu) {BIT(4,cpu->h);}
void cb65(Z80CPU* cpu) {BIT(4,cpu->l);}
void cb66(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); BIT(4,cpu->tmp);}
void cb67(Z80CPU* cpu) {BIT(4,cpu->a);}
// 68..6f	bit 5,r
void cb68(Z80CPU* cpu) {BIT(5,cpu->b);}
void cb69(Z80CPU* cpu) {BIT(5,cpu->c);}
void cb6A(Z80CPU* cpu) {BIT(5,cpu->d);}
void cb6B(Z80CPU* cpu) {BIT(5,cpu->e);}
void cb6C(Z80CPU* cpu) {BIT(5,cpu->h);}
void cb6D(Z80CPU* cpu) {BIT(5,cpu->l);}
void cb6E(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); BIT(5,cpu->tmp);}
void cb6F(Z80CPU* cpu) {BIT(5,cpu->a);}
// 70..77	bit 6,r
void cb70(Z80CPU* cpu) {BIT(6,cpu->b);}
void cb71(Z80CPU* cpu) {BIT(6,cpu->c);}
void cb72(Z80CPU* cpu) {BIT(6,cpu->d);}
void cb73(Z80CPU* cpu) {BIT(6,cpu->e);}
void cb74(Z80CPU* cpu) {BIT(6,cpu->h);}
void cb75(Z80CPU* cpu) {BIT(6,cpu->l);}
void cb76(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); BIT(6,cpu->tmp);}
void cb77(Z80CPU* cpu) {BIT(6,cpu->a);}
// 78..7f	bit 7,r
void cb78(Z80CPU* cpu) {BIT(7,cpu->b);}
void cb79(Z80CPU* cpu) {BIT(7,cpu->c);}
void cb7A(Z80CPU* cpu) {BIT(7,cpu->d);}
void cb7B(Z80CPU* cpu) {BIT(7,cpu->e);}
void cb7C(Z80CPU* cpu) {BIT(7,cpu->h);}
void cb7D(Z80CPU* cpu) {BIT(7,cpu->l);}
void cb7E(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); BIT(7,cpu->tmp);}
void cb7F(Z80CPU* cpu) {BIT(7,cpu->a);}

// 80..87	res 0,r		4 [4rd 3wr]
void cb80(Z80CPU* cpu) {cpu->b &= 0xfe;}
void cb81(Z80CPU* cpu) {cpu->c &= 0xfe;}
void cb82(Z80CPU* cpu) {cpu->d &= 0xfe;}
void cb83(Z80CPU* cpu) {cpu->e &= 0xfe;}
void cb84(Z80CPU* cpu) {cpu->h &= 0xfe;}
void cb85(Z80CPU* cpu) {cpu->l &= 0xfe;}
void cb86(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp &= 0xfe; MEMWR(cpu->hl,cpu->tmp,3);}
void cb87(Z80CPU* cpu) {cpu->a &= 0xfe;}
// 88..8f	res 1,r
void cb88(Z80CPU* cpu) {cpu->b &= 0xfd;}
void cb89(Z80CPU* cpu) {cpu->c &= 0xfd;}
void cb8A(Z80CPU* cpu) {cpu->d &= 0xfd;}
void cb8B(Z80CPU* cpu) {cpu->e &= 0xfd;}
void cb8C(Z80CPU* cpu) {cpu->h &= 0xfd;}
void cb8D(Z80CPU* cpu) {cpu->l &= 0xfd;}
void cb8E(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp &= 0xfd; MEMWR(cpu->hl,cpu->tmp,3);}
void cb8F(Z80CPU* cpu) {cpu->a &= 0xfd;}
// 90..97	res 2,r
void cb90(Z80CPU* cpu) {cpu->b &= 0xfb;}
void cb91(Z80CPU* cpu) {cpu->c &= 0xfb;}
void cb92(Z80CPU* cpu) {cpu->d &= 0xfb;}
void cb93(Z80CPU* cpu) {cpu->e &= 0xfb;}
void cb94(Z80CPU* cpu) {cpu->h &= 0xfb;}
void cb95(Z80CPU* cpu) {cpu->l &= 0xfb;}
void cb96(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp &= 0xfb; MEMWR(cpu->hl,cpu->tmp,3);}
void cb97(Z80CPU* cpu) {cpu->a &= 0xfb;}
// 98..9f	res 3,r
void cb98(Z80CPU* cpu) {cpu->b &= 0xf7;}
void cb99(Z80CPU* cpu) {cpu->c &= 0xf7;}
void cb9A(Z80CPU* cpu) {cpu->d &= 0xf7;}
void cb9B(Z80CPU* cpu) {cpu->e &= 0xf7;}
void cb9C(Z80CPU* cpu) {cpu->h &= 0xf7;}
void cb9D(Z80CPU* cpu) {cpu->l &= 0xf7;}
void cb9E(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp &= 0xf7; MEMWR(cpu->hl,cpu->tmp,3);}
void cb9F(Z80CPU* cpu) {cpu->a &= 0xf7;}
// a0..a7	res 4,r
void cbA0(Z80CPU* cpu) {cpu->b &= 0xef;}
void cbA1(Z80CPU* cpu) {cpu->c &= 0xef;}
void cbA2(Z80CPU* cpu) {cpu->d &= 0xef;}
void cbA3(Z80CPU* cpu) {cpu->e &= 0xef;}
void cbA4(Z80CPU* cpu) {cpu->h &= 0xef;}
void cbA5(Z80CPU* cpu) {cpu->l &= 0xef;}
void cbA6(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp &= 0xef; MEMWR(cpu->hl,cpu->tmp,3);}
void cbA7(Z80CPU* cpu) {cpu->a &= 0xef;}
// a8..af	res 5,r
void cbA8(Z80CPU* cpu) {cpu->b &= 0xdf;}
void cbA9(Z80CPU* cpu) {cpu->c &= 0xdf;}
void cbAA(Z80CPU* cpu) {cpu->d &= 0xdf;}
void cbAB(Z80CPU* cpu) {cpu->e &= 0xdf;}
void cbAC(Z80CPU* cpu) {cpu->h &= 0xdf;}
void cbAD(Z80CPU* cpu) {cpu->l &= 0xdf;}
void cbAE(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp &= 0xdf; MEMWR(cpu->hl,cpu->tmp,3);}
void cbAF(Z80CPU* cpu) {cpu->a &= 0xdf;}
// b0..b7	res 6,r
void cbB0(Z80CPU* cpu) {cpu->b &= 0xbf;}
void cbB1(Z80CPU* cpu) {cpu->c &= 0xbf;}
void cbB2(Z80CPU* cpu) {cpu->d &= 0xbf;}
void cbB3(Z80CPU* cpu) {cpu->e &= 0xbf;}
void cbB4(Z80CPU* cpu) {cpu->h &= 0xbf;}
void cbB5(Z80CPU* cpu) {cpu->l &= 0xbf;}
void cbB6(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp &= 0xbf; MEMWR(cpu->hl,cpu->tmp,3);}
void cbB7(Z80CPU* cpu) {cpu->a &= 0xbf;}
// b8..bf	res 7,r
void cbB8(Z80CPU* cpu) {cpu->b &= 0x7f;}
void cbB9(Z80CPU* cpu) {cpu->c &= 0x7f;}
void cbBA(Z80CPU* cpu) {cpu->d &= 0x7f;}
void cbBB(Z80CPU* cpu) {cpu->e &= 0x7f;}
void cbBC(Z80CPU* cpu) {cpu->h &= 0x7f;}
void cbBD(Z80CPU* cpu) {cpu->l &= 0x7f;}
void cbBE(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp &= 0x7f; MEMWR(cpu->hl,cpu->tmp,3);}
void cbBF(Z80CPU* cpu) {cpu->a &= 0x7f;}

// c0..c7	set 0,r		4 [4rd 3wr]
void cbC0(Z80CPU* cpu) {cpu->b |= 0x01;}
void cbC1(Z80CPU* cpu) {cpu->c |= 0x01;}
void cbC2(Z80CPU* cpu) {cpu->d |= 0x01;}
void cbC3(Z80CPU* cpu) {cpu->e |= 0x01;}
void cbC4(Z80CPU* cpu) {cpu->h |= 0x01;}
void cbC5(Z80CPU* cpu) {cpu->l |= 0x01;}
void cbC6(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp |= 0x01; MEMWR(cpu->hl,cpu->tmp,3);}
void cbC7(Z80CPU* cpu) {cpu->a |= 0x01;}
// c8..cf	set 1,r
void cbC8(Z80CPU* cpu) {cpu->b |= 0x02;}
void cbC9(Z80CPU* cpu) {cpu->c |= 0x02;}
void cbCA(Z80CPU* cpu) {cpu->d |= 0x02;}
void cbCB(Z80CPU* cpu) {cpu->e |= 0x02;}
void cbCC(Z80CPU* cpu) {cpu->h |= 0x02;}
void cbCD(Z80CPU* cpu) {cpu->l |= 0x02;}
void cbCE(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp |= 0x02; MEMWR(cpu->hl,cpu->tmp,3);}
void cbCF(Z80CPU* cpu) {cpu->a |= 0x02;}
// d0..d7	set 2,r
void cbD0(Z80CPU* cpu) {cpu->b |= 0x04;}
void cbD1(Z80CPU* cpu) {cpu->c |= 0x04;}
void cbD2(Z80CPU* cpu) {cpu->d |= 0x04;}
void cbD3(Z80CPU* cpu) {cpu->e |= 0x04;}
void cbD4(Z80CPU* cpu) {cpu->h |= 0x04;}
void cbD5(Z80CPU* cpu) {cpu->l |= 0x04;}
void cbD6(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp |= 0x04; MEMWR(cpu->hl,cpu->tmp,3);}
void cbD7(Z80CPU* cpu) {cpu->a |= 0x04;}
// d8..df	set 3,r
void cbD8(Z80CPU* cpu) {cpu->b |= 0x08;}
void cbD9(Z80CPU* cpu) {cpu->c |= 0x08;}
void cbDA(Z80CPU* cpu) {cpu->d |= 0x08;}
void cbDB(Z80CPU* cpu) {cpu->e |= 0x08;}
void cbDC(Z80CPU* cpu) {cpu->h |= 0x08;}
void cbDD(Z80CPU* cpu) {cpu->l |= 0x08;}
void cbDE(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp |= 0x08; MEMWR(cpu->hl,cpu->tmp,3);}
void cbDF(Z80CPU* cpu) {cpu->a |= 0x08;}
// e0..e7	set 4,r
void cbE0(Z80CPU* cpu) {cpu->b |= 0x10;}
void cbE1(Z80CPU* cpu) {cpu->c |= 0x10;}
void cbE2(Z80CPU* cpu) {cpu->d |= 0x10;}
void cbE3(Z80CPU* cpu) {cpu->e |= 0x10;}
void cbE4(Z80CPU* cpu) {cpu->h |= 0x10;}
void cbE5(Z80CPU* cpu) {cpu->l |= 0x10;}
void cbE6(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp |= 0x10; MEMWR(cpu->hl,cpu->tmp,3);}
void cbE7(Z80CPU* cpu) {cpu->a |= 0x10;}
// e8..ef	set 5,r
void cbE8(Z80CPU* cpu) {cpu->b |= 0x20;}
void cbE9(Z80CPU* cpu) {cpu->c |= 0x20;}
void cbEA(Z80CPU* cpu) {cpu->d |= 0x20;}
void cbEB(Z80CPU* cpu) {cpu->e |= 0x20;}
void cbEC(Z80CPU* cpu) {cpu->h |= 0x20;}
void cbED(Z80CPU* cpu) {cpu->l |= 0x20;}
void cbEE(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp |= 0x20; MEMWR(cpu->hl,cpu->tmp,3);}
void cbEF(Z80CPU* cpu) {cpu->a |= 0x20;}
// f0..f7	set 6,r
void cbF0(Z80CPU* cpu) {cpu->b |= 0x40;}
void cbF1(Z80CPU* cpu) {cpu->c |= 0x40;}
void cbF2(Z80CPU* cpu) {cpu->d |= 0x40;}
void cbF3(Z80CPU* cpu) {cpu->e |= 0x40;}
void cbF4(Z80CPU* cpu) {cpu->h |= 0x40;}
void cbF5(Z80CPU* cpu) {cpu->l |= 0x40;}
void cbF6(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp |= 0x40; MEMWR(cpu->hl,cpu->tmp,3);}
void cbF7(Z80CPU* cpu) {cpu->a |= 0x40;}
// f8..ff	set 7,r
void cbF8(Z80CPU* cpu) {cpu->b |= 0x80;}
void cbF9(Z80CPU* cpu) {cpu->c |= 0x80;}
void cbFA(Z80CPU* cpu) {cpu->d |= 0x80;}
void cbFB(Z80CPU* cpu) {cpu->e |= 0x80;}
void cbFC(Z80CPU* cpu) {cpu->h |= 0x80;}
void cbFD(Z80CPU* cpu) {cpu->l |= 0x80;}
void cbFE(Z80CPU* cpu) {cpu->tmp = MEMRD(cpu->hl,4); cpu->tmp |= 0x80; MEMWR(cpu->hl,cpu->tmp,3);}
void cbFF(Z80CPU* cpu) {cpu->a |= 0x80;}

// ===

opCode cbTab[256]={
	{0,4,0,0,0,0,&cb00,"rlc b"},
	{0,4,0,0,0,0,&cb01,"rlc c"},
	{0,4,0,0,0,0,&cb02,"rlc d"},
	{0,4,0,0,0,0,&cb03,"rlc e"},
	{0,4,0,0,0,0,&cb04,"rlc h"},
	{0,4,0,0,0,0,&cb05,"rlc l"},
	{0,4,4,3,0,0,&cb06,"rlc (hl)"},
	{0,4,0,0,0,0,&cb07,"rlc a"},

	{0,4,0,0,0,0,&cb08,"rrc b"},
	{0,4,0,0,0,0,&cb09,"rrc c"},
	{0,4,0,0,0,0,&cb0A,"rrc d"},
	{0,4,0,0,0,0,&cb0B,"rrc e"},
	{0,4,0,0,0,0,&cb0C,"rrc h"},
	{0,4,0,0,0,0,&cb0D,"rrc l"},
	{0,4,4,3,0,0,&cb0E,"rrc (hl)"},
	{0,4,0,0,0,0,&cb0F,"rrc a"},

	{0,4,0,0,0,0,&cb10,"rl b"},
	{0,4,0,0,0,0,&cb11,"rl c"},
	{0,4,0,0,0,0,&cb12,"rl d"},
	{0,4,0,0,0,0,&cb13,"rl e"},
	{0,4,0,0,0,0,&cb14,"rl h"},
	{0,4,0,0,0,0,&cb15,"rl l"},
	{0,4,4,3,0,0,&cb16,"rl (hl)"},
	{0,4,0,0,0,0,&cb17,"rl a"},

	{0,4,0,0,0,0,&cb18,"sla b"},
	{0,4,0,0,0,0,&cb19,"sla c"},
	{0,4,0,0,0,0,&cb1A,"sla d"},
	{0,4,0,0,0,0,&cb1B,"sla e"},
	{0,4,0,0,0,0,&cb1C,"sla h"},
	{0,4,0,0,0,0,&cb1D,"sla l"},
	{0,4,4,3,0,0,&cb1E,"sla (hl)"},
	{0,4,0,0,0,0,&cb1F,"sla a"},

	{0,4,0,0,0,0,&cb20,"sla b"},
	{0,4,0,0,0,0,&cb21,"sla c"},
	{0,4,0,0,0,0,&cb22,"sla d"},
	{0,4,0,0,0,0,&cb23,"sla e"},
	{0,4,0,0,0,0,&cb24,"sla h"},
	{0,4,0,0,0,0,&cb25,"sla l"},
	{0,4,4,3,0,0,&cb26,"sla (hl)"},
	{0,4,0,0,0,0,&cb27,"sla a"},

	{0,4,0,0,0,0,&cb28,"sra b"},
	{0,4,0,0,0,0,&cb29,"sra c"},
	{0,4,0,0,0,0,&cb2A,"sra d"},
	{0,4,0,0,0,0,&cb2B,"sra e"},
	{0,4,0,0,0,0,&cb2C,"sra h"},
	{0,4,0,0,0,0,&cb2D,"sra l"},
	{0,4,4,3,0,0,&cb2E,"sra (hl)"},
	{0,4,0,0,0,0,&cb2F,"sra a"},

	{0,4,0,0,0,0,&cb30,"sli b"},
	{0,4,0,0,0,0,&cb31,"sli c"},
	{0,4,0,0,0,0,&cb32,"sli d"},
	{0,4,0,0,0,0,&cb33,"sli e"},
	{0,4,0,0,0,0,&cb34,"sli h"},
	{0,4,0,0,0,0,&cb35,"sli l"},
	{0,4,4,3,0,0,&cb36,"sli (hl)"},
	{0,4,0,0,0,0,&cb37,"sli a"},

	{0,4,0,0,0,0,&cb38,"srl b"},
	{0,4,0,0,0,0,&cb39,"srl c"},
	{0,4,0,0,0,0,&cb3A,"srl d"},
	{0,4,0,0,0,0,&cb3B,"srl e"},
	{0,4,0,0,0,0,&cb3C,"srl h"},
	{0,4,0,0,0,0,&cb3D,"srl l"},
	{0,4,4,3,0,0,&cb3E,"srl (hl)"},
	{0,4,0,0,0,0,&cb3F,"srl a"},

	{0,4,0,0,0,0,&cb40,"bit 0,b"},
	{0,4,0,0,0,0,&cb41,"bit 0,c"},
	{0,4,0,0,0,0,&cb42,"bit 0,d"},
	{0,4,0,0,0,0,&cb43,"bit 0,e"},
	{0,4,0,0,0,0,&cb44,"bit 0,h"},
	{0,4,0,0,0,0,&cb45,"bit 0,l"},
	{0,4,4,0,0,0,&cb46,"bit 0,(hl)"},
	{0,4,0,0,0,0,&cb47,"bit 0,a"},

	{0,4,0,0,0,0,&cb48,"bit 1,b"},
	{0,4,0,0,0,0,&cb49,"bit 1,c"},
	{0,4,0,0,0,0,&cb4A,"bit 1,d"},
	{0,4,0,0,0,0,&cb4B,"bit 1,e"},
	{0,4,0,0,0,0,&cb4C,"bit 1,h"},
	{0,4,0,0,0,0,&cb4D,"bit 1,l"},
	{0,4,4,0,0,0,&cb4E,"bit 1,(hl)"},
	{0,4,0,0,0,0,&cb4F,"bit 1,a"},

	{0,4,0,0,0,0,&cb50,"bit 2,b"},
	{0,4,0,0,0,0,&cb51,"bit 2,c"},
	{0,4,0,0,0,0,&cb52,"bit 2,d"},
	{0,4,0,0,0,0,&cb53,"bit 2,e"},
	{0,4,0,0,0,0,&cb54,"bit 2,h"},
	{0,4,0,0,0,0,&cb55,"bit 2,l"},
	{0,4,4,0,0,0,&cb56,"bit 2,(hl)"},
	{0,4,0,0,0,0,&cb57,"bit 2,a"},

	{0,4,0,0,0,0,&cb58,"bit 3,b"},
	{0,4,0,0,0,0,&cb59,"bit 3,c"},
	{0,4,0,0,0,0,&cb5A,"bit 3,d"},
	{0,4,0,0,0,0,&cb5B,"bit 3,e"},
	{0,4,0,0,0,0,&cb5C,"bit 3,h"},
	{0,4,0,0,0,0,&cb5D,"bit 3,l"},
	{0,4,4,0,0,0,&cb5E,"bit 3,(hl)"},
	{0,4,0,0,0,0,&cb5F,"bit 3,a"},

	{0,4,0,0,0,0,&cb60,"bit 4,b"},
	{0,4,0,0,0,0,&cb61,"bit 4,c"},
	{0,4,0,0,0,0,&cb62,"bit 4,d"},
	{0,4,0,0,0,0,&cb63,"bit 4,e"},
	{0,4,0,0,0,0,&cb64,"bit 4,h"},
	{0,4,0,0,0,0,&cb65,"bit 4,l"},
	{0,4,4,0,0,0,&cb66,"bit 4,(hl)"},
	{0,4,0,0,0,0,&cb67,"bit 4,a"},

	{0,4,0,0,0,0,&cb68,"bit 5,b"},
	{0,4,0,0,0,0,&cb69,"bit 5,c"},
	{0,4,0,0,0,0,&cb6A,"bit 5,d"},
	{0,4,0,0,0,0,&cb6B,"bit 5,e"},
	{0,4,0,0,0,0,&cb6C,"bit 5,h"},
	{0,4,0,0,0,0,&cb6D,"bit 5,l"},
	{0,4,4,0,0,0,&cb6E,"bit 5,(hl)"},
	{0,4,0,0,0,0,&cb6F,"bit 5,a"},

	{0,4,0,0,0,0,&cb70,"bit 6,b"},
	{0,4,0,0,0,0,&cb71,"bit 6,c"},
	{0,4,0,0,0,0,&cb72,"bit 6,d"},
	{0,4,0,0,0,0,&cb73,"bit 6,e"},
	{0,4,0,0,0,0,&cb74,"bit 6,h"},
	{0,4,0,0,0,0,&cb75,"bit 6,l"},
	{0,4,4,0,0,0,&cb76,"bit 6,(hl)"},
	{0,4,0,0,0,0,&cb77,"bit 6,a"},

	{0,4,0,0,0,0,&cb78,"bit 7,b"},
	{0,4,0,0,0,0,&cb79,"bit 7,c"},
	{0,4,0,0,0,0,&cb7A,"bit 7,d"},
	{0,4,0,0,0,0,&cb7B,"bit 7,e"},
	{0,4,0,0,0,0,&cb7C,"bit 7,h"},
	{0,4,0,0,0,0,&cb7D,"bit 7,l"},
	{0,4,4,0,0,0,&cb7E,"bit 7,(hl)"},
	{0,4,0,0,0,0,&cb7F,"bit 7,a"},

	{0,4,0,0,0,0,&cb80,"res 0,b"},
	{0,4,0,0,0,0,&cb81,"res 0,c"},
	{0,4,0,0,0,0,&cb82,"res 0,d"},
	{0,4,0,0,0,0,&cb83,"res 0,e"},
	{0,4,0,0,0,0,&cb84,"res 0,h"},
	{0,4,0,0,0,0,&cb85,"res 0,l"},
	{0,4,4,3,0,0,&cb86,"res 0,(hl)"},
	{0,4,0,0,0,0,&cb87,"res 0,a"},

	{0,4,0,0,0,0,&cb88,"res 1,b"},
	{0,4,0,0,0,0,&cb89,"res 1,c"},
	{0,4,0,0,0,0,&cb8A,"res 1,d"},
	{0,4,0,0,0,0,&cb8B,"res 1,e"},
	{0,4,0,0,0,0,&cb8C,"res 1,h"},
	{0,4,0,0,0,0,&cb8D,"res 1,l"},
	{0,4,4,3,0,0,&cb8E,"res 1,(hl)"},
	{0,4,0,0,0,0,&cb8F,"res 1,a"},

	{0,4,0,0,0,0,&cb90,"res 2,b"},
	{0,4,0,0,0,0,&cb91,"res 2,c"},
	{0,4,0,0,0,0,&cb92,"res 2,d"},
	{0,4,0,0,0,0,&cb93,"res 2,e"},
	{0,4,0,0,0,0,&cb94,"res 2,h"},
	{0,4,0,0,0,0,&cb95,"res 2,l"},
	{0,4,4,3,0,0,&cb96,"res 2,(hl)"},
	{0,4,0,0,0,0,&cb97,"res 2,a"},

	{0,4,0,0,0,0,&cb98,"res 3,b"},
	{0,4,0,0,0,0,&cb99,"res 3,c"},
	{0,4,0,0,0,0,&cb9A,"res 3,d"},
	{0,4,0,0,0,0,&cb9B,"res 3,e"},
	{0,4,0,0,0,0,&cb9C,"res 3,h"},
	{0,4,0,0,0,0,&cb9D,"res 3,l"},
	{0,4,4,3,0,0,&cb9E,"res 3,(hl)"},
	{0,4,0,0,0,0,&cb9F,"res 3,a"},

	{0,4,0,0,0,0,&cbA0,"res 4,b"},
	{0,4,0,0,0,0,&cbA1,"res 4,c"},
	{0,4,0,0,0,0,&cbA2,"res 4,d"},
	{0,4,0,0,0,0,&cbA3,"res 4,e"},
	{0,4,0,0,0,0,&cbA4,"res 4,h"},
	{0,4,0,0,0,0,&cbA5,"res 4,l"},
	{0,4,4,3,0,0,&cbA6,"res 4,(hl)"},
	{0,4,0,0,0,0,&cbA7,"res 4,a"},

	{0,4,0,0,0,0,&cbA8,"res 5,b"},
	{0,4,0,0,0,0,&cbA9,"res 5,c"},
	{0,4,0,0,0,0,&cbAA,"res 5,d"},
	{0,4,0,0,0,0,&cbAB,"res 5,e"},
	{0,4,0,0,0,0,&cbAC,"res 5,h"},
	{0,4,0,0,0,0,&cbAD,"res 5,l"},
	{0,4,4,3,0,0,&cbAE,"res 5,(hl)"},
	{0,4,0,0,0,0,&cbAF,"res 5,a"},

	{0,4,0,0,0,0,&cbB0,"res 6,b"},
	{0,4,0,0,0,0,&cbB1,"res 6,c"},
	{0,4,0,0,0,0,&cbB2,"res 6,d"},
	{0,4,0,0,0,0,&cbB3,"res 6,e"},
	{0,4,0,0,0,0,&cbB4,"res 6,h"},
	{0,4,0,0,0,0,&cbB5,"res 6,l"},
	{0,4,4,3,0,0,&cbB6,"res 6,(hl)"},
	{0,4,0,0,0,0,&cbB7,"res 6,a"},

	{0,4,0,0,0,0,&cbB8,"res 7,b"},
	{0,4,0,0,0,0,&cbB9,"res 7,c"},
	{0,4,0,0,0,0,&cbBA,"res 7,d"},
	{0,4,0,0,0,0,&cbBB,"res 7,e"},
	{0,4,0,0,0,0,&cbBC,"res 7,h"},
	{0,4,0,0,0,0,&cbBD,"res 7,l"},
	{0,4,4,3,0,0,&cbBE,"res 7,(hl)"},
	{0,4,0,0,0,0,&cbBF,"res 7,a"},

	{0,4,0,0,0,0,&cbC0,"set 0,b"},
	{0,4,0,0,0,0,&cbC1,"set 0,c"},
	{0,4,0,0,0,0,&cbC2,"set 0,d"},
	{0,4,0,0,0,0,&cbC3,"set 0,e"},
	{0,4,0,0,0,0,&cbC4,"set 0,h"},
	{0,4,0,0,0,0,&cbC5,"set 0,l"},
	{0,4,4,3,0,0,&cbC6,"set 0,(hl)"},
	{0,4,0,0,0,0,&cbC7,"set 0,a"},

	{0,4,0,0,0,0,&cbC8,"set 1,b"},
	{0,4,0,0,0,0,&cbC9,"set 1,c"},
	{0,4,0,0,0,0,&cbCA,"set 1,d"},
	{0,4,0,0,0,0,&cbCB,"set 1,e"},
	{0,4,0,0,0,0,&cbCC,"set 1,h"},
	{0,4,0,0,0,0,&cbCD,"set 1,l"},
	{0,4,4,3,0,0,&cbCE,"set 1,(hl)"},
	{0,4,0,0,0,0,&cbCF,"set 1,a"},

	{0,4,0,0,0,0,&cbD0,"set 2,b"},
	{0,4,0,0,0,0,&cbD1,"set 2,c"},
	{0,4,0,0,0,0,&cbD2,"set 2,d"},
	{0,4,0,0,0,0,&cbD3,"set 2,e"},
	{0,4,0,0,0,0,&cbD4,"set 2,h"},
	{0,4,0,0,0,0,&cbD5,"set 2,l"},
	{0,4,4,3,0,0,&cbD6,"set 2,(hl)"},
	{0,4,0,0,0,0,&cbD7,"set 2,a"},

	{0,4,0,0,0,0,&cbD8,"set 3,b"},
	{0,4,0,0,0,0,&cbD9,"set 3,c"},
	{0,4,0,0,0,0,&cbDA,"set 3,d"},
	{0,4,0,0,0,0,&cbDB,"set 3,e"},
	{0,4,0,0,0,0,&cbDC,"set 3,h"},
	{0,4,0,0,0,0,&cbDD,"set 3,l"},
	{0,4,4,3,0,0,&cbDE,"set 3,(hl)"},
	{0,4,0,0,0,0,&cbDF,"set 3,a"},

	{0,4,0,0,0,0,&cbE0,"set 4,b"},
	{0,4,0,0,0,0,&cbE1,"set 4,c"},
	{0,4,0,0,0,0,&cbE2,"set 4,d"},
	{0,4,0,0,0,0,&cbE3,"set 4,e"},
	{0,4,0,0,0,0,&cbE4,"set 4,h"},
	{0,4,0,0,0,0,&cbE5,"set 4,l"},
	{0,4,4,3,0,0,&cbE6,"set 4,(hl)"},
	{0,4,0,0,0,0,&cbE7,"set 4,a"},

	{0,4,0,0,0,0,&cbE8,"set 5,b"},
	{0,4,0,0,0,0,&cbE9,"set 5,c"},
	{0,4,0,0,0,0,&cbEA,"set 5,d"},
	{0,4,0,0,0,0,&cbEB,"set 5,e"},
	{0,4,0,0,0,0,&cbEC,"set 5,h"},
	{0,4,0,0,0,0,&cbED,"set 5,l"},
	{0,4,4,3,0,0,&cbEE,"set 5,(hl)"},
	{0,4,0,0,0,0,&cbEF,"set 5,a"},

	{0,4,0,0,0,0,&cbF0,"set 6,b"},
	{0,4,0,0,0,0,&cbF1,"set 6,c"},
	{0,4,0,0,0,0,&cbF2,"set 6,d"},
	{0,4,0,0,0,0,&cbF3,"set 6,e"},
	{0,4,0,0,0,0,&cbF4,"set 6,h"},
	{0,4,0,0,0,0,&cbF5,"set 6,l"},
	{0,4,4,3,0,0,&cbF6,"set 6,(hl)"},
	{0,4,0,0,0,0,&cbF7,"set 6,a"},

	{0,4,0,0,0,0,&cbF8,"set 7,b"},
	{0,4,0,0,0,0,&cbF9,"set 7,c"},
	{0,4,0,0,0,0,&cbFA,"set 7,d"},
	{0,4,0,0,0,0,&cbFB,"set 7,e"},
	{0,4,0,0,0,0,&cbFC,"set 7,h"},
	{0,4,0,0,0,0,&cbFD,"set 7,l"},
	{0,4,4,3,0,0,&cbFE,"set 7,(hl)"},
	{0,4,0,0,0,0,&cbFF,"set 7,a"},
};
