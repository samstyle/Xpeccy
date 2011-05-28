// NOTE: prefix fetch not included in ticks (ZB: rl b took 8 ticks, but here is only last 4, because lead 4 is CB-fetch)

void cbp00(ZXBase* p) {p->cpu->f = flag[p->cpu->b].rlc.f; p->cpu->b = flag[p->cpu->b].rlc.r;}
void cbp01(ZXBase* p) {p->cpu->f = flag[p->cpu->c].rlc.f; p->cpu->c = flag[p->cpu->c].rlc.r;}
void cbp02(ZXBase* p) {p->cpu->f = flag[p->cpu->d].rlc.f; p->cpu->d = flag[p->cpu->d].rlc.r;}
void cbp03(ZXBase* p) {p->cpu->f = flag[p->cpu->e].rlc.f; p->cpu->e = flag[p->cpu->e].rlc.r;}
void cbp04(ZXBase* p) {p->cpu->f = flag[p->cpu->h].rlc.f; p->cpu->h = flag[p->cpu->h].rlc.r;}
void cbp05(ZXBase* p) {p->cpu->f = flag[p->cpu->l].rlc.f; p->cpu->l = flag[p->cpu->l].rlc.r;}
void cbp06(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->hl); p->cpu->f = flag[p->cpu->x].rlc.f; p->mem->wr(p->cpu->hl,flag[p->cpu->x].rlc.r);}
void cbp07(ZXBase* p) {p->cpu->f = flag[p->cpu->a].rlc.f; p->cpu->a = flag[p->cpu->a].rlc.r;}

void cbp08(ZXBase* p) {p->cpu->f = flag[p->cpu->b].rrc.f; p->cpu->b = flag[p->cpu->b].rrc.r;}
void cbp09(ZXBase* p) {p->cpu->f = flag[p->cpu->c].rrc.f; p->cpu->c = flag[p->cpu->c].rrc.r;}
void cbp0A(ZXBase* p) {p->cpu->f = flag[p->cpu->d].rrc.f; p->cpu->d = flag[p->cpu->d].rrc.r;}
void cbp0B(ZXBase* p) {p->cpu->f = flag[p->cpu->e].rrc.f; p->cpu->e = flag[p->cpu->e].rrc.r;}
void cbp0C(ZXBase* p) {p->cpu->f = flag[p->cpu->h].rrc.f; p->cpu->h = flag[p->cpu->h].rrc.r;}
void cbp0D(ZXBase* p) {p->cpu->f = flag[p->cpu->l].rrc.f; p->cpu->l = flag[p->cpu->l].rrc.r;}
void cbp0E(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->hl); p->cpu->f = flag[p->cpu->x].rrc.f; p->mem->wr(p->cpu->hl,flag[p->cpu->x].rrc.r);}
void cbp0F(ZXBase* p) {p->cpu->f = flag[p->cpu->a].rrc.f; p->cpu->a = flag[p->cpu->a].rrc.r;}

void cbp10(ZXBase* p) {p->cpu->x = p->cpu->b; p->cpu->b = flag[p->cpu->x].rl[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rl[p->cpu->f & FC].f;}
void cbp11(ZXBase* p) {p->cpu->x = p->cpu->c; p->cpu->c = flag[p->cpu->x].rl[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rl[p->cpu->f & FC].f;}
void cbp12(ZXBase* p) {p->cpu->x = p->cpu->d; p->cpu->d = flag[p->cpu->x].rl[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rl[p->cpu->f & FC].f;}
void cbp13(ZXBase* p) {p->cpu->x = p->cpu->e; p->cpu->e = flag[p->cpu->x].rl[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rl[p->cpu->f & FC].f;}
void cbp14(ZXBase* p) {p->cpu->x = p->cpu->h; p->cpu->h = flag[p->cpu->x].rl[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rl[p->cpu->f & FC].f;}
void cbp15(ZXBase* p) {p->cpu->x = p->cpu->l; p->cpu->l = flag[p->cpu->x].rl[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rl[p->cpu->f & FC].f;}
void cbp16(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->hl); p->mem->wr(p->cpu->hl,flag[p->cpu->x].rl[p->cpu->f & FC].r); p->cpu->f = flag[p->cpu->x].rl[p->cpu->f & FC].f;}
void cbp17(ZXBase* p) {p->cpu->x = p->cpu->a; p->cpu->a = flag[p->cpu->x].rl[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rl[p->cpu->f & FC].f;}

void cbp18(ZXBase* p) {p->cpu->x = p->cpu->b; p->cpu->b = flag[p->cpu->x].rr[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rr[p->cpu->f & FC].f;}
void cbp19(ZXBase* p) {p->cpu->x = p->cpu->c; p->cpu->c = flag[p->cpu->x].rr[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rr[p->cpu->f & FC].f;}
void cbp1A(ZXBase* p) {p->cpu->x = p->cpu->d; p->cpu->d = flag[p->cpu->x].rr[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rr[p->cpu->f & FC].f;}
void cbp1B(ZXBase* p) {p->cpu->x = p->cpu->e; p->cpu->e = flag[p->cpu->x].rr[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rr[p->cpu->f & FC].f;}
void cbp1C(ZXBase* p) {p->cpu->x = p->cpu->h; p->cpu->h = flag[p->cpu->x].rr[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rr[p->cpu->f & FC].f;}
void cbp1D(ZXBase* p) {p->cpu->x = p->cpu->l; p->cpu->l = flag[p->cpu->x].rr[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rr[p->cpu->f & FC].f;}
void cbp1E(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->hl); p->mem->wr(p->cpu->hl,flag[p->cpu->x].rr[p->cpu->f & FC].r); p->cpu->f = flag[p->cpu->x].rr[p->cpu->f & FC].f;}
void cbp1F(ZXBase* p) {p->cpu->x = p->cpu->a; p->cpu->a = flag[p->cpu->x].rr[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->x].rr[p->cpu->f & FC].f;}

void cbp20(ZXBase* p) {p->cpu->f = flag[p->cpu->b].rl[0].f; p->cpu->b = flag[p->cpu->b].rl[0].r;}
void cbp21(ZXBase* p) {p->cpu->f = flag[p->cpu->c].rl[0].f; p->cpu->c = flag[p->cpu->c].rl[0].r;}
void cbp22(ZXBase* p) {p->cpu->f = flag[p->cpu->d].rl[0].f; p->cpu->d = flag[p->cpu->d].rl[0].r;}
void cbp23(ZXBase* p) {p->cpu->f = flag[p->cpu->e].rl[0].f; p->cpu->e = flag[p->cpu->e].rl[0].r;}
void cbp24(ZXBase* p) {p->cpu->f = flag[p->cpu->h].rl[0].f; p->cpu->h = flag[p->cpu->h].rl[0].r;}
void cbp25(ZXBase* p) {p->cpu->f = flag[p->cpu->l].rl[0].f; p->cpu->l = flag[p->cpu->l].rl[0].r;}
void cbp26(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->hl); p->cpu->f = flag[p->cpu->x].rl[0].f; p->mem->wr(p->cpu->hl,flag[p->cpu->x].rl[0].r);}
void cbp27(ZXBase* p) {p->cpu->f = flag[p->cpu->a].rl[0].f; p->cpu->a = flag[p->cpu->a].rl[0].r;}

void cbp28(ZXBase* p) {p->cpu->f = flag[p->cpu->b].sra.f; p->cpu->b = flag[p->cpu->b].sra.r;}
void cbp29(ZXBase* p) {p->cpu->f = flag[p->cpu->c].sra.f; p->cpu->c = flag[p->cpu->c].sra.r;}
void cbp2A(ZXBase* p) {p->cpu->f = flag[p->cpu->d].sra.f; p->cpu->d = flag[p->cpu->d].sra.r;}
void cbp2B(ZXBase* p) {p->cpu->f = flag[p->cpu->e].sra.f; p->cpu->e = flag[p->cpu->e].sra.r;}
void cbp2C(ZXBase* p) {p->cpu->f = flag[p->cpu->h].sra.f; p->cpu->h = flag[p->cpu->h].sra.r;}
void cbp2D(ZXBase* p) {p->cpu->f = flag[p->cpu->l].sra.f; p->cpu->l = flag[p->cpu->l].sra.r;}
void cbp2E(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->hl); p->cpu->f = flag[p->cpu->x].sra.f; p->mem->wr(p->cpu->hl,flag[p->cpu->x].sra.r);}
void cbp2F(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sra.f; p->cpu->a = flag[p->cpu->a].sra.r;}

void cbp30(ZXBase* p) {p->cpu->f = flag[p->cpu->b].rl[1].f; p->cpu->b = flag[p->cpu->b].rl[1].r;}
void cbp31(ZXBase* p) {p->cpu->f = flag[p->cpu->c].rl[1].f; p->cpu->c = flag[p->cpu->c].rl[1].r;}
void cbp32(ZXBase* p) {p->cpu->f = flag[p->cpu->d].rl[1].f; p->cpu->d = flag[p->cpu->d].rl[1].r;}
void cbp33(ZXBase* p) {p->cpu->f = flag[p->cpu->e].rl[1].f; p->cpu->e = flag[p->cpu->e].rl[1].r;}
void cbp34(ZXBase* p) {p->cpu->f = flag[p->cpu->h].rl[1].f; p->cpu->h = flag[p->cpu->h].rl[1].r;}
void cbp35(ZXBase* p) {p->cpu->f = flag[p->cpu->l].rl[1].f; p->cpu->l = flag[p->cpu->l].rl[1].r;}
void cbp36(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->hl); p->cpu->f = flag[p->cpu->x].rl[1].f; p->mem->wr(p->cpu->hl,flag[p->cpu->x].rl[1].r);}
void cbp37(ZXBase* p) {p->cpu->f = flag[p->cpu->a].rl[1].f; p->cpu->a = flag[p->cpu->a].rl[1].r;}

void cbp38(ZXBase* p) {p->cpu->f = flag[p->cpu->b].rr[0].f; p->cpu->b = flag[p->cpu->b].rr[0].r;}
void cbp39(ZXBase* p) {p->cpu->f = flag[p->cpu->c].rr[0].f; p->cpu->c = flag[p->cpu->c].rr[0].r;}
void cbp3A(ZXBase* p) {p->cpu->f = flag[p->cpu->d].rr[0].f; p->cpu->d = flag[p->cpu->d].rr[0].r;}
void cbp3B(ZXBase* p) {p->cpu->f = flag[p->cpu->e].rr[0].f; p->cpu->e = flag[p->cpu->e].rr[0].r;}
void cbp3C(ZXBase* p) {p->cpu->f = flag[p->cpu->h].rr[0].f; p->cpu->h = flag[p->cpu->h].rr[0].r;}
void cbp3D(ZXBase* p) {p->cpu->f = flag[p->cpu->l].rr[0].f; p->cpu->l = flag[p->cpu->l].rr[0].r;}
void cbp3E(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->hl); p->cpu->f = flag[p->cpu->x].rr[0].f; p->mem->wr(p->cpu->hl,flag[p->cpu->x].rr[0].r);}
void cbp3F(ZXBase* p) {p->cpu->f = flag[p->cpu->a].rr[0].f; p->cpu->a = flag[p->cpu->a].rr[0].r;}
// bit n,reg
// bit n,(hl): b3,5 = memptr b3,5
void cbp40(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->b].bit[0];}
void cbp41(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->c].bit[0];}
void cbp42(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->d].bit[0];}
void cbp43(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->e].bit[0];}
void cbp44(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->h].bit[0];}
void cbp45(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->l].bit[0];}
void cbp46(ZXBase* p) {p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->hl)].bit[0]) & (~(F5 | F3))) | (p->cpu->hptr & (F5 | F3));}
void cbp47(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->a].bit[0];}

void cbp48(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->b].bit[1];}
void cbp49(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->c].bit[1];}
void cbp4A(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->d].bit[1];}
void cbp4B(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->e].bit[1];}
void cbp4C(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->h].bit[1];}
void cbp4D(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->l].bit[1];}
void cbp4E(ZXBase* p) {p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->hl)].bit[1]) & (~(F5 | F3))) | (p->cpu->hptr & (F5 | F3));}
void cbp4F(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->a].bit[1];}

void cbp50(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->b].bit[2];}
void cbp51(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->c].bit[2];}
void cbp52(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->d].bit[2];}
void cbp53(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->e].bit[2];}
void cbp54(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->h].bit[2];}
void cbp55(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->l].bit[2];}
void cbp56(ZXBase* p) {p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->hl)].bit[2]) & (~(F5 | F3))) | (p->cpu->hptr & (F5 | F3));}
void cbp57(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->a].bit[2];}

void cbp58(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->b].bit[3];}
void cbp59(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->c].bit[3];}
void cbp5A(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->d].bit[3];}
void cbp5B(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->e].bit[3];}
void cbp5C(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->h].bit[3];}
void cbp5D(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->l].bit[3];}
void cbp5E(ZXBase* p) {p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->hl)].bit[3]) & (~(F5 | F3))) | (p->cpu->hptr & (F5 | F3));}
void cbp5F(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->a].bit[3];}

void cbp60(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->b].bit[4];}
void cbp61(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->c].bit[4];}
void cbp62(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->d].bit[4];}
void cbp63(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->e].bit[4];}
void cbp64(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->h].bit[4];}
void cbp65(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->l].bit[4];}
void cbp66(ZXBase* p) {p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->hl)].bit[4]) & (~(F5 | F3))) | (p->cpu->hptr & (F5 | F3));}
void cbp67(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->a].bit[4];}

void cbp68(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->b].bit[5];}
void cbp69(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->c].bit[5];}
void cbp6A(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->d].bit[5];}
void cbp6B(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->e].bit[5];}
void cbp6C(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->h].bit[5];}
void cbp6D(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->l].bit[5];}
void cbp6E(ZXBase* p) {p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->hl)].bit[5]) & (~(F5 | F3))) | (p->cpu->hptr & (F5 | F3));}
void cbp6F(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->a].bit[5];}

void cbp70(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->b].bit[6];}
void cbp71(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->c].bit[6];}
void cbp72(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->d].bit[6];}
void cbp73(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->e].bit[6];}
void cbp74(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->h].bit[6];}
void cbp75(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->l].bit[6];}
void cbp76(ZXBase* p) {p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->hl)].bit[6]) & (~(F5 | F3))) | (p->cpu->hptr & (F5 | F3));}
void cbp77(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->a].bit[6];}

void cbp78(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->b].bit[7];}
void cbp79(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->c].bit[7];}
void cbp7A(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->d].bit[7];}
void cbp7B(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->e].bit[7];}
void cbp7C(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->h].bit[7];}
void cbp7D(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->l].bit[7];}
void cbp7E(ZXBase* p) {p->cpu->f = (((p->cpu->f & FC) | flag[p->mem->rd(p->cpu->hl)].bit[7]) & (~(F5 | F3))) | (p->cpu->hptr & (F5 | F3));}
void cbp7F(ZXBase* p) {p->cpu->f = (p->cpu->f & FC) | flag[p->cpu->a].bit[7];}

void cbp80(ZXBase* p) {p->cpu->b &= 0xfe;}
void cbp81(ZXBase* p) {p->cpu->c &= 0xfe;}
void cbp82(ZXBase* p) {p->cpu->d &= 0xfe;}
void cbp83(ZXBase* p) {p->cpu->e &= 0xfe;}
void cbp84(ZXBase* p) {p->cpu->h &= 0xfe;}
void cbp85(ZXBase* p) {p->cpu->l &= 0xfe;}
void cbp86(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) & 0xfe);}
void cbp87(ZXBase* p) {p->cpu->a &= 0xfe;}

void cbp88(ZXBase* p) {p->cpu->b &= 0xfd;}
void cbp89(ZXBase* p) {p->cpu->c &= 0xfd;}
void cbp8A(ZXBase* p) {p->cpu->d &= 0xfd;}
void cbp8B(ZXBase* p) {p->cpu->e &= 0xfd;}
void cbp8C(ZXBase* p) {p->cpu->h &= 0xfd;}
void cbp8D(ZXBase* p) {p->cpu->l &= 0xfd;}
void cbp8E(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) & 0xfd);}
void cbp8F(ZXBase* p) {p->cpu->a &= 0xfd;}

void cbp90(ZXBase* p) {p->cpu->b &= 0xfb;}
void cbp91(ZXBase* p) {p->cpu->c &= 0xfb;}
void cbp92(ZXBase* p) {p->cpu->d &= 0xfb;}
void cbp93(ZXBase* p) {p->cpu->e &= 0xfb;}
void cbp94(ZXBase* p) {p->cpu->h &= 0xfb;}
void cbp95(ZXBase* p) {p->cpu->l &= 0xfb;}
void cbp96(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) & 0xfb);}
void cbp97(ZXBase* p) {p->cpu->a &= 0xfb;}

void cbp98(ZXBase* p) {p->cpu->b &= 0xf7;}
void cbp99(ZXBase* p) {p->cpu->c &= 0xf7;}
void cbp9A(ZXBase* p) {p->cpu->d &= 0xf7;}
void cbp9B(ZXBase* p) {p->cpu->e &= 0xf7;}
void cbp9C(ZXBase* p) {p->cpu->h &= 0xf7;}
void cbp9D(ZXBase* p) {p->cpu->l &= 0xf7;}
void cbp9E(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) & 0xf7);}
void cbp9F(ZXBase* p) {p->cpu->a &= 0xf7;}

void cbpA0(ZXBase* p) {p->cpu->b &= 0xef;}
void cbpA1(ZXBase* p) {p->cpu->c &= 0xef;}
void cbpA2(ZXBase* p) {p->cpu->d &= 0xef;}
void cbpA3(ZXBase* p) {p->cpu->e &= 0xef;}
void cbpA4(ZXBase* p) {p->cpu->h &= 0xef;}
void cbpA5(ZXBase* p) {p->cpu->l &= 0xef;}
void cbpA6(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) & 0xef);}
void cbpA7(ZXBase* p) {p->cpu->a &= 0xef;}

void cbpA8(ZXBase* p) {p->cpu->b &= 0xdf;}
void cbpA9(ZXBase* p) {p->cpu->c &= 0xdf;}
void cbpAA(ZXBase* p) {p->cpu->d &= 0xdf;}
void cbpAB(ZXBase* p) {p->cpu->e &= 0xdf;}
void cbpAC(ZXBase* p) {p->cpu->h &= 0xdf;}
void cbpAD(ZXBase* p) {p->cpu->l &= 0xdf;}
void cbpAE(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) & 0xdf);}
void cbpAF(ZXBase* p) {p->cpu->a &= 0xdf;}

void cbpB0(ZXBase* p) {p->cpu->b &= 0xbf;}
void cbpB1(ZXBase* p) {p->cpu->c &= 0xbf;}
void cbpB2(ZXBase* p) {p->cpu->d &= 0xbf;}
void cbpB3(ZXBase* p) {p->cpu->e &= 0xbf;}
void cbpB4(ZXBase* p) {p->cpu->h &= 0xbf;}
void cbpB5(ZXBase* p) {p->cpu->l &= 0xbf;}
void cbpB6(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) & 0xbf);}
void cbpB7(ZXBase* p) {p->cpu->a &= 0xbf;}

void cbpB8(ZXBase* p) {p->cpu->b &= 0x7f;}
void cbpB9(ZXBase* p) {p->cpu->c &= 0x7f;}
void cbpBA(ZXBase* p) {p->cpu->d &= 0x7f;}
void cbpBB(ZXBase* p) {p->cpu->e &= 0x7f;}
void cbpBC(ZXBase* p) {p->cpu->h &= 0x7f;}
void cbpBD(ZXBase* p) {p->cpu->l &= 0x7f;}
void cbpBE(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) & 0x7f);}
void cbpBF(ZXBase* p) {p->cpu->a &= 0x7f;}

void cbpC0(ZXBase* p) {p->cpu->b |= 0x01;}
void cbpC1(ZXBase* p) {p->cpu->c |= 0x01;}
void cbpC2(ZXBase* p) {p->cpu->d |= 0x01;}
void cbpC3(ZXBase* p) {p->cpu->e |= 0x01;}
void cbpC4(ZXBase* p) {p->cpu->h |= 0x01;}
void cbpC5(ZXBase* p) {p->cpu->l |= 0x01;}
void cbpC6(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) | 0x01);}
void cbpC7(ZXBase* p) {p->cpu->a |= 0x01;}

void cbpC8(ZXBase* p) {p->cpu->b |= 0x02;}
void cbpC9(ZXBase* p) {p->cpu->c |= 0x02;}
void cbpCA(ZXBase* p) {p->cpu->d |= 0x02;}
void cbpCB(ZXBase* p) {p->cpu->e |= 0x02;}
void cbpCC(ZXBase* p) {p->cpu->h |= 0x02;}
void cbpCD(ZXBase* p) {p->cpu->l |= 0x02;}
void cbpCE(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) | 0x02);}
void cbpCF(ZXBase* p) {p->cpu->a |= 0x02;}

void cbpD0(ZXBase* p) {p->cpu->b |= 0x04;}
void cbpD1(ZXBase* p) {p->cpu->c |= 0x04;}
void cbpD2(ZXBase* p) {p->cpu->d |= 0x04;}
void cbpD3(ZXBase* p) {p->cpu->e |= 0x04;}
void cbpD4(ZXBase* p) {p->cpu->h |= 0x04;}
void cbpD5(ZXBase* p) {p->cpu->l |= 0x04;}
void cbpD6(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) | 0x04);}
void cbpD7(ZXBase* p) {p->cpu->a |= 0x04;}

void cbpD8(ZXBase* p) {p->cpu->b |= 0x08;}
void cbpD9(ZXBase* p) {p->cpu->c |= 0x08;}
void cbpDA(ZXBase* p) {p->cpu->d |= 0x08;}
void cbpDB(ZXBase* p) {p->cpu->e |= 0x08;}
void cbpDC(ZXBase* p) {p->cpu->h |= 0x08;}
void cbpDD(ZXBase* p) {p->cpu->l |= 0x08;}
void cbpDE(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) | 0x08);}
void cbpDF(ZXBase* p) {p->cpu->a |= 0x08;}

void cbpE0(ZXBase* p) {p->cpu->b |= 0x10;}
void cbpE1(ZXBase* p) {p->cpu->c |= 0x10;}
void cbpE2(ZXBase* p) {p->cpu->d |= 0x10;}
void cbpE3(ZXBase* p) {p->cpu->e |= 0x10;}
void cbpE4(ZXBase* p) {p->cpu->h |= 0x10;}
void cbpE5(ZXBase* p) {p->cpu->l |= 0x10;}
void cbpE6(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) | 0x10);}
void cbpE7(ZXBase* p) {p->cpu->a |= 0x10;}

void cbpE8(ZXBase* p) {p->cpu->b |= 0x20;}
void cbpE9(ZXBase* p) {p->cpu->c |= 0x20;}
void cbpEA(ZXBase* p) {p->cpu->d |= 0x20;}
void cbpEB(ZXBase* p) {p->cpu->e |= 0x20;}
void cbpEC(ZXBase* p) {p->cpu->h |= 0x20;}
void cbpED(ZXBase* p) {p->cpu->l |= 0x20;}
void cbpEE(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) | 0x20);}
void cbpEF(ZXBase* p) {p->cpu->a |= 0x20;}

void cbpF0(ZXBase* p) {p->cpu->b |= 0x40;}
void cbpF1(ZXBase* p) {p->cpu->c |= 0x40;}
void cbpF2(ZXBase* p) {p->cpu->d |= 0x40;}
void cbpF3(ZXBase* p) {p->cpu->e |= 0x40;}
void cbpF4(ZXBase* p) {p->cpu->h |= 0x40;}
void cbpF5(ZXBase* p) {p->cpu->l |= 0x40;}
void cbpF6(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) | 0x40);}
void cbpF7(ZXBase* p) {p->cpu->a |= 0x40;}

void cbpF8(ZXBase* p) {p->cpu->b |= 0x80;}
void cbpF9(ZXBase* p) {p->cpu->c |= 0x80;}
void cbpFA(ZXBase* p) {p->cpu->d |= 0x80;}
void cbpFB(ZXBase* p) {p->cpu->e |= 0x80;}
void cbpFC(ZXBase* p) {p->cpu->h |= 0x80;}
void cbpFD(ZXBase* p) {p->cpu->l |= 0x80;}
void cbpFE(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->hl) | 0x80);}
void cbpFF(ZXBase* p) {p->cpu->a |= 0x80;}

//==================

ZOp cbpref[256]={
	{4,	CND_NONE,0,0,	0,	&cbp00,	"rlc b"},
	{4,	CND_NONE,0,0,	0,	&cbp01,	"rlc c"},
	{4,	CND_NONE,0,0,	0,	&cbp02,	"rlc d"},
	{4,	CND_NONE,0,0,	0,	&cbp03,	"rlc e"},
	{4,	CND_NONE,0,0,	0,	&cbp04,	"rlc h"},
	{4,	CND_NONE,0,0,	0,	&cbp05,	"rlc l"},
	{11,	CND_NONE,0,0,	0,	&cbp06,	"rlc (hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp07,	"rlc a"},

	{4,	CND_NONE,0,0,	0,	&cbp08,	"rrc b"},
	{4,	CND_NONE,0,0,	0,	&cbp09,	"rrc c"},
	{4,	CND_NONE,0,0,	0,	&cbp0A,	"rrc d"},
	{4,	CND_NONE,0,0,	0,	&cbp0B,	"rrc e"},
	{4,	CND_NONE,0,0,	0,	&cbp0C,	"rrc h"},
	{4,	CND_NONE,0,0,	0,	&cbp0D,	"rrc l"},
	{11,	CND_NONE,0,0,	0,	&cbp0E,	"rrc (hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp0F,	"rrc a"},

	{4,	CND_NONE,0,0,	0,	&cbp10,	"rl b"},
	{4,	CND_NONE,0,0,	0,	&cbp11,	"rl c"},
	{4,	CND_NONE,0,0,	0,	&cbp12,	"rl d"},
	{4,	CND_NONE,0,0,	0,	&cbp13,	"rl e"},
	{4,	CND_NONE,0,0,	0,	&cbp14,	"rl h"},
	{4,	CND_NONE,0,0,	0,	&cbp15,	"rl l"},
	{11,	CND_NONE,0,0,	0,	&cbp16,	"rl (hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp17,	"rl a"},

	{4,	CND_NONE,0,0,	0,	&cbp18,	"sla b"},
	{4,	CND_NONE,0,0,	0,	&cbp19,	"sla c"},
	{4,	CND_NONE,0,0,	0,	&cbp1A,	"sla d"},
	{4,	CND_NONE,0,0,	0,	&cbp1B,	"sla e"},
	{4,	CND_NONE,0,0,	0,	&cbp1C,	"sla h"},
	{4,	CND_NONE,0,0,	0,	&cbp1D,	"sla l"},
	{11,	CND_NONE,0,0,	0,	&cbp1E,	"sla (hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp1F,	"sla a"},

	{4,	CND_NONE,0,0,	0,	&cbp20,	"sla b"},
	{4,	CND_NONE,0,0,	0,	&cbp21,	"sla c"},
	{4,	CND_NONE,0,0,	0,	&cbp22,	"sla d"},
	{4,	CND_NONE,0,0,	0,	&cbp23,	"sla e"},
	{4,	CND_NONE,0,0,	0,	&cbp24,	"sla h"},
	{4,	CND_NONE,0,0,	0,	&cbp25,	"sla l"},
	{11,	CND_NONE,0,0,	0,	&cbp26,	"sla (hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp27,	"sla a"},

	{4,	CND_NONE,0,0,	0,	&cbp28,	"sra b"},
	{4,	CND_NONE,0,0,	0,	&cbp29,	"sra c"},
	{4,	CND_NONE,0,0,	0,	&cbp2A,	"sra d"},
	{4,	CND_NONE,0,0,	0,	&cbp2B,	"sra e"},
	{4,	CND_NONE,0,0,	0,	&cbp2C,	"sra h"},
	{4,	CND_NONE,0,0,	0,	&cbp2D,	"sra l"},
	{11,	CND_NONE,0,0,	0,	&cbp2E,	"sra (hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp2F,	"sra a"},

	{4,	CND_NONE,0,0,	0,	&cbp30,	"sli b"},
	{4,	CND_NONE,0,0,	0,	&cbp31,	"sli c"},
	{4,	CND_NONE,0,0,	0,	&cbp32,	"sli d"},
	{4,	CND_NONE,0,0,	0,	&cbp33,	"sli e"},
	{4,	CND_NONE,0,0,	0,	&cbp34,	"sli h"},
	{4,	CND_NONE,0,0,	0,	&cbp35,	"sli l"},
	{11,	CND_NONE,0,0,	0,	&cbp36,	"sli (hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp37,	"sli a"},

	{4,	CND_NONE,0,0,	0,	&cbp38,	"srl b"},
	{4,	CND_NONE,0,0,	0,	&cbp39,	"srl c"},
	{4,	CND_NONE,0,0,	0,	&cbp3A,	"srl d"},
	{4,	CND_NONE,0,0,	0,	&cbp3B,	"srl e"},
	{4,	CND_NONE,0,0,	0,	&cbp3C,	"srl h"},
	{4,	CND_NONE,0,0,	0,	&cbp3D,	"srl l"},
	{11,	CND_NONE,0,0,	0,	&cbp3E,	"srl (hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp3F,	"srl a"},

	{4,	CND_NONE,0,0,	0,	&cbp40,	"bit 0,b"},
	{4,	CND_NONE,0,0,	0,	&cbp41,	"bit 0,c"},
	{4,	CND_NONE,0,0,	0,	&cbp42,	"bit 0,d"},
	{4,	CND_NONE,0,0,	0,	&cbp43,	"bit 0,e"},
	{4,	CND_NONE,0,0,	0,	&cbp44,	"bit 0,h"},
	{4,	CND_NONE,0,0,	0,	&cbp45,	"bit 0,l"},
	{8,	CND_NONE,0,0,	0,	&cbp46,	"bit 0,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp47,	"bit 0,a"},

	{4,	CND_NONE,0,0,	0,	&cbp48,	"bit 1,b"},
	{4,	CND_NONE,0,0,	0,	&cbp49,	"bit 1,c"},
	{4,	CND_NONE,0,0,	0,	&cbp4A,	"bit 1,d"},
	{4,	CND_NONE,0,0,	0,	&cbp4B,	"bit 1,e"},
	{4,	CND_NONE,0,0,	0,	&cbp4C,	"bit 1,h"},
	{4,	CND_NONE,0,0,	0,	&cbp4D,	"bit 1,l"},
	{8,	CND_NONE,0,0,	0,	&cbp4E,	"bit 1,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp4F,	"bit 1,a"},

	{4,	CND_NONE,0,0,	0,	&cbp50,	"bit 2,b"},
	{4,	CND_NONE,0,0,	0,	&cbp51,	"bit 2,c"},
	{4,	CND_NONE,0,0,	0,	&cbp52,	"bit 2,d"},
	{4,	CND_NONE,0,0,	0,	&cbp53,	"bit 2,e"},
	{4,	CND_NONE,0,0,	0,	&cbp54,	"bit 2,h"},
	{4,	CND_NONE,0,0,	0,	&cbp55,	"bit 2,l"},
	{8,	CND_NONE,0,0,	0,	&cbp56,	"bit 2,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp57,	"bit 2,a"},

	{4,	CND_NONE,0,0,	0,	&cbp58,	"bit 3,b"},
	{4,	CND_NONE,0,0,	0,	&cbp59,	"bit 3,c"},
	{4,	CND_NONE,0,0,	0,	&cbp5A,	"bit 3,d"},
	{4,	CND_NONE,0,0,	0,	&cbp5B,	"bit 3,e"},
	{4,	CND_NONE,0,0,	0,	&cbp5C,	"bit 3,h"},
	{4,	CND_NONE,0,0,	0,	&cbp5D,	"bit 3,l"},
	{8,	CND_NONE,0,0,	0,	&cbp5E,	"bit 3,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp5F,	"bit 3,a"},

	{4,	CND_NONE,0,0,	0,	&cbp60,	"bit 4,b"},
	{4,	CND_NONE,0,0,	0,	&cbp61,	"bit 4,c"},
	{4,	CND_NONE,0,0,	0,	&cbp62,	"bit 4,d"},
	{4,	CND_NONE,0,0,	0,	&cbp63,	"bit 4,e"},
	{4,	CND_NONE,0,0,	0,	&cbp64,	"bit 4,h"},
	{4,	CND_NONE,0,0,	0,	&cbp65,	"bit 4,l"},
	{8,	CND_NONE,0,0,	0,	&cbp66,	"bit 4,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp67,	"bit 4,a"},

	{4,	CND_NONE,0,0,	0,	&cbp68,	"bit 5,b"},
	{4,	CND_NONE,0,0,	0,	&cbp69,	"bit 5,c"},
	{4,	CND_NONE,0,0,	0,	&cbp6A,	"bit 5,d"},
	{4,	CND_NONE,0,0,	0,	&cbp6B,	"bit 5,e"},
	{4,	CND_NONE,0,0,	0,	&cbp6C,	"bit 5,h"},
	{4,	CND_NONE,0,0,	0,	&cbp6D,	"bit 5,l"},
	{8,	CND_NONE,0,0,	0,	&cbp6E,	"bit 5,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp6F,	"bit 5,a"},

	{4,	CND_NONE,0,0,	0,	&cbp70,	"bit 6,b"},
	{4,	CND_NONE,0,0,	0,	&cbp71,	"bit 6,c"},
	{4,	CND_NONE,0,0,	0,	&cbp72,	"bit 6,d"},
	{4,	CND_NONE,0,0,	0,	&cbp73,	"bit 6,e"},
	{4,	CND_NONE,0,0,	0,	&cbp74,	"bit 6,h"},
	{4,	CND_NONE,0,0,	0,	&cbp75,	"bit 6,l"},
	{8,	CND_NONE,0,0,	0,	&cbp76,	"bit 6,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp77,	"bit 6,a"},

	{4,	CND_NONE,0,0,	0,	&cbp78,	"bit 7,b"},
	{4,	CND_NONE,0,0,	0,	&cbp79,	"bit 7,c"},
	{4,	CND_NONE,0,0,	0,	&cbp7A,	"bit 7,d"},
	{4,	CND_NONE,0,0,	0,	&cbp7B,	"bit 7,e"},
	{4,	CND_NONE,0,0,	0,	&cbp7C,	"bit 7,h"},
	{4,	CND_NONE,0,0,	0,	&cbp7D,	"bit 7,l"},
	{8,	CND_NONE,0,0,	0,	&cbp7E,	"bit 7,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp7F,	"bit 7,a"},

	{4,	CND_NONE,0,0,	0,	&cbp80,	"res 0,b"},
	{4,	CND_NONE,0,0,	0,	&cbp81,	"res 0,c"},
	{4,	CND_NONE,0,0,	0,	&cbp82,	"res 0,d"},
	{4,	CND_NONE,0,0,	0,	&cbp83,	"res 0,e"},
	{4,	CND_NONE,0,0,	0,	&cbp84,	"res 0,h"},
	{4,	CND_NONE,0,0,	0,	&cbp85,	"res 0,l"},
	{11,	CND_NONE,0,0,	0,	&cbp86,	"res 0,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp87,	"res 0,a"},

	{4,	CND_NONE,0,0,	0,	&cbp88,	"res 1,b"},
	{4,	CND_NONE,0,0,	0,	&cbp89,	"res 1,c"},
	{4,	CND_NONE,0,0,	0,	&cbp8A,	"res 1,d"},
	{4,	CND_NONE,0,0,	0,	&cbp8B,	"res 1,e"},
	{4,	CND_NONE,0,0,	0,	&cbp8C,	"res 1,h"},
	{4,	CND_NONE,0,0,	0,	&cbp8D,	"res 1,l"},
	{11,	CND_NONE,0,0,	0,	&cbp8E,	"res 1,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp8F,	"res 1,a"},

	{4,	CND_NONE,0,0,	0,	&cbp90,	"res 2,b"},
	{4,	CND_NONE,0,0,	0,	&cbp91,	"res 2,c"},
	{4,	CND_NONE,0,0,	0,	&cbp92,	"res 2,d"},
	{4,	CND_NONE,0,0,	0,	&cbp93,	"res 2,e"},
	{4,	CND_NONE,0,0,	0,	&cbp94,	"res 2,h"},
	{4,	CND_NONE,0,0,	0,	&cbp95,	"res 2,l"},
	{11,	CND_NONE,0,0,	0,	&cbp96,	"res 2,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp97,	"res 2,a"},

	{4,	CND_NONE,0,0,	0,	&cbp98,	"res 3,b"},
	{4,	CND_NONE,0,0,	0,	&cbp99,	"res 3,c"},
	{4,	CND_NONE,0,0,	0,	&cbp9A,	"res 3,d"},
	{4,	CND_NONE,0,0,	0,	&cbp9B,	"res 3,e"},
	{4,	CND_NONE,0,0,	0,	&cbp9C,	"res 3,h"},
	{4,	CND_NONE,0,0,	0,	&cbp9D,	"res 3,l"},
	{11,	CND_NONE,0,0,	0,	&cbp9E,	"res 3,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbp9F,	"res 3,a"},

	{4,	CND_NONE,0,0,	0,	&cbpA0,	"res 4,b"},
	{4,	CND_NONE,0,0,	0,	&cbpA1,	"res 4,c"},
	{4,	CND_NONE,0,0,	0,	&cbpA2,	"res 4,d"},
	{4,	CND_NONE,0,0,	0,	&cbpA3,	"res 4,e"},
	{4,	CND_NONE,0,0,	0,	&cbpA4,	"res 4,h"},
	{4,	CND_NONE,0,0,	0,	&cbpA5,	"res 4,l"},
	{11,	CND_NONE,0,0,	0,	&cbpA6,	"res 4,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbpA7,	"res 4,a"},

	{4,	CND_NONE,0,0,	0,	&cbpA8,	"res 5,b"},
	{4,	CND_NONE,0,0,	0,	&cbpA9,	"res 5,c"},
	{4,	CND_NONE,0,0,	0,	&cbpAA,	"res 5,d"},
	{4,	CND_NONE,0,0,	0,	&cbpAB,	"res 5,e"},
	{4,	CND_NONE,0,0,	0,	&cbpAC,	"res 5,h"},
	{4,	CND_NONE,0,0,	0,	&cbpAD,	"res 5,l"},
	{11,	CND_NONE,0,0,	0,	&cbpAE,	"res 5,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbpAF,	"res 5,a"},

	{4,	CND_NONE,0,0,	0,	&cbpB0,	"res 6,b"},
	{4,	CND_NONE,0,0,	0,	&cbpB1,	"res 6,c"},
	{4,	CND_NONE,0,0,	0,	&cbpB2,	"res 6,d"},
	{4,	CND_NONE,0,0,	0,	&cbpB3,	"res 6,e"},
	{4,	CND_NONE,0,0,	0,	&cbpB4,	"res 6,h"},
	{4,	CND_NONE,0,0,	0,	&cbpB5,	"res 6,l"},
	{11,	CND_NONE,0,0,	0,	&cbpB6,	"res 6,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbpB7,	"res 6,a"},

	{4,	CND_NONE,0,0,	0,	&cbpB8,	"res 7,b"},
	{4,	CND_NONE,0,0,	0,	&cbpB9,	"res 7,c"},
	{4,	CND_NONE,0,0,	0,	&cbpBA,	"res 7,d"},
	{4,	CND_NONE,0,0,	0,	&cbpBB,	"res 7,e"},
	{4,	CND_NONE,0,0,	0,	&cbpBC,	"res 7,h"},
	{4,	CND_NONE,0,0,	0,	&cbpBD,	"res 7,l"},
	{11,	CND_NONE,0,0,	0,	&cbpBE,	"res 7,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbpBF,	"res 7,a"},

	{4,	CND_NONE,0,0,	0,	&cbpC0,	"set 0,b"},
	{4,	CND_NONE,0,0,	0,	&cbpC1,	"set 0,c"},
	{4,	CND_NONE,0,0,	0,	&cbpC2,	"set 0,d"},
	{4,	CND_NONE,0,0,	0,	&cbpC3,	"set 0,e"},
	{4,	CND_NONE,0,0,	0,	&cbpC4,	"set 0,h"},
	{4,	CND_NONE,0,0,	0,	&cbpC5,	"set 0,l"},
	{11,	CND_NONE,0,0,	0,	&cbpC6,	"set 0,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbpC7,	"set 0,a"},

	{4,	CND_NONE,0,0,	0,	&cbpC8,	"set 1,b"},
	{4,	CND_NONE,0,0,	0,	&cbpC9,	"set 1,c"},
	{4,	CND_NONE,0,0,	0,	&cbpCA,	"set 1,d"},
	{4,	CND_NONE,0,0,	0,	&cbpCB,	"set 1,e"},
	{4,	CND_NONE,0,0,	0,	&cbpCC,	"set 1,h"},
	{4,	CND_NONE,0,0,	0,	&cbpCD,	"set 1,l"},
	{11,	CND_NONE,0,0,	0,	&cbpCE,	"set 1,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbpCF,	"set 1,a"},

	{4,	CND_NONE,0,0,	0,	&cbpD0,	"set 2,b"},
	{4,	CND_NONE,0,0,	0,	&cbpD1,	"set 2,c"},
	{4,	CND_NONE,0,0,	0,	&cbpD2,	"set 2,d"},
	{4,	CND_NONE,0,0,	0,	&cbpD3,	"set 2,e"},
	{4,	CND_NONE,0,0,	0,	&cbpD4,	"set 2,h"},
	{4,	CND_NONE,0,0,	0,	&cbpD5,	"set 2,l"},
	{11,	CND_NONE,0,0,	0,	&cbpD6,	"set 2,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbpD7,	"set 2,a"},

	{4,	CND_NONE,0,0,	0,	&cbpD8,	"set 3,b"},
	{4,	CND_NONE,0,0,	0,	&cbpD9,	"set 3,c"},
	{4,	CND_NONE,0,0,	0,	&cbpDA,	"set 3,d"},
	{4,	CND_NONE,0,0,	0,	&cbpDB,	"set 3,e"},
	{4,	CND_NONE,0,0,	0,	&cbpDC,	"set 3,h"},
	{4,	CND_NONE,0,0,	0,	&cbpDD,	"set 3,l"},
	{11,	CND_NONE,0,0,	0,	&cbpDE,	"set 3,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbpDF,	"set 3,a"},

	{4,	CND_NONE,0,0,	0,	&cbpE0,	"set 4,b"},
	{4,	CND_NONE,0,0,	0,	&cbpE1,	"set 4,c"},
	{4,	CND_NONE,0,0,	0,	&cbpE2,	"set 4,d"},
	{4,	CND_NONE,0,0,	0,	&cbpE3,	"set 4,e"},
	{4,	CND_NONE,0,0,	0,	&cbpE4,	"set 4,h"},
	{4,	CND_NONE,0,0,	0,	&cbpE5,	"set 4,l"},
	{11,	CND_NONE,0,0,	0,	&cbpE6,	"set 4,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbpE7,	"set 4,a"},

	{4,	CND_NONE,0,0,	0,	&cbpE8,	"set 5,b"},
	{4,	CND_NONE,0,0,	0,	&cbpE9,	"set 5,c"},
	{4,	CND_NONE,0,0,	0,	&cbpEA,	"set 5,d"},
	{4,	CND_NONE,0,0,	0,	&cbpEB,	"set 5,e"},
	{4,	CND_NONE,0,0,	0,	&cbpEC,	"set 5,h"},
	{4,	CND_NONE,0,0,	0,	&cbpED,	"set 5,l"},
	{11,	CND_NONE,0,0,	0,	&cbpEE,	"set 5,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbpEF,	"set 5,a"},

	{4,	CND_NONE,0,0,	0,	&cbpF0,	"set 6,b"},
	{4,	CND_NONE,0,0,	0,	&cbpF1,	"set 6,c"},
	{4,	CND_NONE,0,0,	0,	&cbpF2,	"set 6,d"},
	{4,	CND_NONE,0,0,	0,	&cbpF3,	"set 6,e"},
	{4,	CND_NONE,0,0,	0,	&cbpF4,	"set 6,h"},
	{4,	CND_NONE,0,0,	0,	&cbpF5,	"set 6,l"},
	{11,	CND_NONE,0,0,	0,	&cbpF6,	"set 6,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbpF7,	"set 6,a"},

	{4,	CND_NONE,0,0,	0,	&cbpF8,	"set 7,b"},
	{4,	CND_NONE,0,0,	0,	&cbpF9,	"set 7,c"},
	{4,	CND_NONE,0,0,	0,	&cbpFA,	"set 7,d"},
	{4,	CND_NONE,0,0,	0,	&cbpFB,	"set 7,e"},
	{4,	CND_NONE,0,0,	0,	&cbpFC,	"set 7,h"},
	{4,	CND_NONE,0,0,	0,	&cbpFD,	"set 7,l"},
	{11,	CND_NONE,0,0,	0,	&cbpFE,	"set 7,(hl)"},
	{4,	CND_NONE,0,0,	0,	&cbpFF,	"set 7,a"},
};
