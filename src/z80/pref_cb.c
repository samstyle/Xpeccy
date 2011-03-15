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
	ZOp(&cbp00,4,"rlc b"),
	ZOp(&cbp01,4,"rlc c"),
	ZOp(&cbp02,4,"rlc d"),
	ZOp(&cbp03,4,"rlc e"),
	ZOp(&cbp04,4,"rlc h"),
	ZOp(&cbp05,4,"rlc l"),
	ZOp(&cbp06,11,"rlc (hl)"),
	ZOp(&cbp07,4,"rlc a"),

	ZOp(&cbp08,4,"rrc b"),
	ZOp(&cbp09,4,"rrc c"),
	ZOp(&cbp0A,4,"rrc d"),
	ZOp(&cbp0B,4,"rrc e"),
	ZOp(&cbp0C,4,"rrc h"),
	ZOp(&cbp0D,4,"rrc l"),
	ZOp(&cbp0E,11,"rrc (hl)"),
	ZOp(&cbp0F,4,"rrc a"),

	ZOp(&cbp10,4,"rl b"),
	ZOp(&cbp11,4,"rl c"),
	ZOp(&cbp12,4,"rl d"),
	ZOp(&cbp13,4,"rl e"),
	ZOp(&cbp14,4,"rl h"),
	ZOp(&cbp15,4,"rl l"),
	ZOp(&cbp16,11,"rl (hl)"),
	ZOp(&cbp17,4,"rl a"),

	ZOp(&cbp18,4,"sla b"),
	ZOp(&cbp19,4,"sla c"),
	ZOp(&cbp1A,4,"sla d"),
	ZOp(&cbp1B,4,"sla e"),
	ZOp(&cbp1C,4,"sla h"),
	ZOp(&cbp1D,4,"sla l"),
	ZOp(&cbp1E,11,"sla (hl)"),
	ZOp(&cbp1F,4,"sla a"),

	ZOp(&cbp20,4,"sli b"),
	ZOp(&cbp21,4,"sli c"),
	ZOp(&cbp22,4,"sli d"),
	ZOp(&cbp23,4,"sli e"),
	ZOp(&cbp24,4,"sli h"),
	ZOp(&cbp25,4,"sli l"),
	ZOp(&cbp26,11,"sli (hl)"),
	ZOp(&cbp27,4,"sli a"),

	ZOp(&cbp28,4,"sra b"),
	ZOp(&cbp29,4,"sra c"),
	ZOp(&cbp2A,4,"sra d"),
	ZOp(&cbp2B,4,"sra e"),
	ZOp(&cbp2C,4,"sra h"),
	ZOp(&cbp2D,4,"sra l"),
	ZOp(&cbp2E,11,"sra (hl)"),
	ZOp(&cbp2F,4,"sra a"),

	ZOp(&cbp30,4,"sli b"),
	ZOp(&cbp31,4,"sli c"),
	ZOp(&cbp32,4,"sli d"),
	ZOp(&cbp33,4,"sli e"),
	ZOp(&cbp34,4,"sli h"),
	ZOp(&cbp35,4,"sli l"),
	ZOp(&cbp36,11,"sli (hl)"),
	ZOp(&cbp37,4,"sli a"),

	ZOp(&cbp38,4,"srl b"),
	ZOp(&cbp39,4,"srl c"),
	ZOp(&cbp3A,4,"srl d"),
	ZOp(&cbp3B,4,"srl e"),
	ZOp(&cbp3C,4,"srl h"),
	ZOp(&cbp3D,4,"srl l"),
	ZOp(&cbp3E,11,"srl (hl)"),
	ZOp(&cbp3F,4,"srl a"),

	ZOp(&cbp40,4,"bit 0,b"),
	ZOp(&cbp41,4,"bit 0,c"),
	ZOp(&cbp42,4,"bit 0,d"),
	ZOp(&cbp43,4,"bit 0,e"),
	ZOp(&cbp44,4,"bit 0,h"),
	ZOp(&cbp45,4,"bit 0,l"),
	ZOp(&cbp46,8,"bit 0,(hl)"),
	ZOp(&cbp47,4,"bit 0,a"),

	ZOp(&cbp48,4,"bit 1,b"),
	ZOp(&cbp49,4,"bit 1,c"),
	ZOp(&cbp4A,4,"bit 1,d"),
	ZOp(&cbp4B,4,"bit 1,e"),
	ZOp(&cbp4C,4,"bit 1,h"),
	ZOp(&cbp4D,4,"bit 1,l"),
	ZOp(&cbp4E,8,"bit 1,(hl)"),
	ZOp(&cbp4F,4,"bit 1,a"),

	ZOp(&cbp50,4,"bit 2,b"),
	ZOp(&cbp51,4,"bit 2,c"),
	ZOp(&cbp52,4,"bit 2,d"),
	ZOp(&cbp53,4,"bit 2,e"),
	ZOp(&cbp54,4,"bit 2,h"),
	ZOp(&cbp55,4,"bit 2,l"),
	ZOp(&cbp56,8,"bit 2,(hl)"),
	ZOp(&cbp57,4,"bit 2,a"),

	ZOp(&cbp58,4,"bit 3,b"),
	ZOp(&cbp59,4,"bit 3,c"),
	ZOp(&cbp5A,4,"bit 3,d"),
	ZOp(&cbp5B,4,"bit 3,e"),
	ZOp(&cbp5C,4,"bit 3,h"),
	ZOp(&cbp5D,4,"bit 3,l"),
	ZOp(&cbp5E,8,"bit 3,(hl)"),
	ZOp(&cbp5F,4,"bit 3,a"),

	ZOp(&cbp60,4,"bit 4,b"),
	ZOp(&cbp61,4,"bit 4,c"),
	ZOp(&cbp62,4,"bit 4,d"),
	ZOp(&cbp63,4,"bit 4,e"),
	ZOp(&cbp64,4,"bit 4,h"),
	ZOp(&cbp65,4,"bit 4,l"),
	ZOp(&cbp66,8,"bit 4,(hl)"),
	ZOp(&cbp67,4,"bit 4,a"),

	ZOp(&cbp68,4,"bit 5,b"),
	ZOp(&cbp69,4,"bit 5,c"),
	ZOp(&cbp6A,4,"bit 5,d"),
	ZOp(&cbp6B,4,"bit 5,e"),
	ZOp(&cbp6C,4,"bit 5,h"),
	ZOp(&cbp6D,4,"bit 5,l"),
	ZOp(&cbp6E,8,"bit 5,(hl)"),
	ZOp(&cbp6F,4,"bit 5,a"),

	ZOp(&cbp70,4,"bit 6,b"),
	ZOp(&cbp71,4,"bit 6,c"),
	ZOp(&cbp72,4,"bit 6,d"),
	ZOp(&cbp73,4,"bit 6,e"),
	ZOp(&cbp74,4,"bit 6,h"),
	ZOp(&cbp75,4,"bit 6,l"),
	ZOp(&cbp76,8,"bit 6,(hl)"),
	ZOp(&cbp77,4,"bit 6,a"),

	ZOp(&cbp78,4,"bit 7,b"),
	ZOp(&cbp79,4,"bit 7,c"),
	ZOp(&cbp7A,4,"bit 7,d"),
	ZOp(&cbp7B,4,"bit 7,e"),
	ZOp(&cbp7C,4,"bit 7,h"),
	ZOp(&cbp7D,4,"bit 7,l"),
	ZOp(&cbp7E,8,"bit 7,(hl)"),
	ZOp(&cbp7F,4,"bit 7,a"),

	ZOp(&cbp80,4,"res 0,b"),
	ZOp(&cbp81,4,"res 0,c"),
	ZOp(&cbp82,4,"res 0,d"),
	ZOp(&cbp83,4,"res 0,e"),
	ZOp(&cbp84,4,"res 0,h"),
	ZOp(&cbp85,4,"res 0,l"),
	ZOp(&cbp86,11,"res 0,(hl)"),
	ZOp(&cbp87,4,"res 0,a"),

	ZOp(&cbp88,4,"res 1,b"),
	ZOp(&cbp89,4,"res 1,c"),
	ZOp(&cbp8A,4,"res 1,d"),
	ZOp(&cbp8B,4,"res 1,e"),
	ZOp(&cbp8C,4,"res 1,h"),
	ZOp(&cbp8D,4,"res 1,l"),
	ZOp(&cbp8E,11,"res 1,(hl)"),
	ZOp(&cbp8F,4,"res 1,a"),

	ZOp(&cbp90,4,"res 2,b"),
	ZOp(&cbp91,4,"res 2,c"),
	ZOp(&cbp92,4,"res 2,d"),
	ZOp(&cbp93,4,"res 2,e"),
	ZOp(&cbp94,4,"res 2,h"),
	ZOp(&cbp95,4,"res 2,l"),
	ZOp(&cbp96,11,"res 2,(hl)"),
	ZOp(&cbp97,4,"res 2,a"),

	ZOp(&cbp98,4,"res 3,b"),
	ZOp(&cbp99,4,"res 3,c"),
	ZOp(&cbp9A,4,"res 3,d"),
	ZOp(&cbp9B,4,"res 3,e"),
	ZOp(&cbp9C,4,"res 3,h"),
	ZOp(&cbp9D,4,"res 3,l"),
	ZOp(&cbp9E,11,"res 3,(hl)"),
	ZOp(&cbp9F,4,"res 3,a"),

	ZOp(&cbpA0,4,"res 4,b"),
	ZOp(&cbpA1,4,"res 4,c"),
	ZOp(&cbpA2,4,"res 4,d"),
	ZOp(&cbpA3,4,"res 4,e"),
	ZOp(&cbpA4,4,"res 4,h"),
	ZOp(&cbpA5,4,"res 4,l"),
	ZOp(&cbpA6,11,"res 4,(hl)"),
	ZOp(&cbpA7,4,"res 4,a"),

	ZOp(&cbpA8,4,"res 5,b"),
	ZOp(&cbpA9,4,"res 5,c"),
	ZOp(&cbpAA,4,"res 5,d"),
	ZOp(&cbpAB,4,"res 5,e"),
	ZOp(&cbpAC,4,"res 5,h"),
	ZOp(&cbpAD,4,"res 5,l"),
	ZOp(&cbpAE,11,"res 5,(hl)"),
	ZOp(&cbpAF,4,"res 5,a"),

	ZOp(&cbpB0,4,"res 6,b"),
	ZOp(&cbpB1,4,"res 6,c"),
	ZOp(&cbpB2,4,"res 6,d"),
	ZOp(&cbpB3,4,"res 6,e"),
	ZOp(&cbpB4,4,"res 6,h"),
	ZOp(&cbpB5,4,"res 6,l"),
	ZOp(&cbpB6,11,"res 6,(hl)"),
	ZOp(&cbpB7,4,"res 6,a"),

	ZOp(&cbpB8,4,"res 7,b"),
	ZOp(&cbpB9,4,"res 7,c"),
	ZOp(&cbpBA,4,"res 7,d"),
	ZOp(&cbpBB,4,"res 7,e"),
	ZOp(&cbpBC,4,"res 7,h"),
	ZOp(&cbpBD,4,"res 7,l"),
	ZOp(&cbpBE,11,"res 7,(hl)"),
	ZOp(&cbpBF,4,"res 7,a"),

	ZOp(&cbpC0,4,"set 0,b"),
	ZOp(&cbpC1,4,"set 0,c"),
	ZOp(&cbpC2,4,"set 0,d"),
	ZOp(&cbpC3,4,"set 0,e"),
	ZOp(&cbpC4,4,"set 0,h"),
	ZOp(&cbpC5,4,"set 0,l"),
	ZOp(&cbpC6,11,"set 0,(hl)"),
	ZOp(&cbpC7,4,"set 0,a"),

	ZOp(&cbpC8,4,"set 1,b"),
	ZOp(&cbpC9,4,"set 1,c"),
	ZOp(&cbpCA,4,"set 1,d"),
	ZOp(&cbpCB,4,"set 1,e"),
	ZOp(&cbpCC,4,"set 1,h"),
	ZOp(&cbpCD,4,"set 1,l"),
	ZOp(&cbpCE,11,"set 1,(hl)"),
	ZOp(&cbpCF,4,"set 1,a"),

	ZOp(&cbpD0,4,"set 2,b"),
	ZOp(&cbpD1,4,"set 2,c"),
	ZOp(&cbpD2,4,"set 2,d"),
	ZOp(&cbpD3,4,"set 2,e"),
	ZOp(&cbpD4,4,"set 2,h"),
	ZOp(&cbpD5,4,"set 2,l"),
	ZOp(&cbpD6,11,"set 2,(hl)"),
	ZOp(&cbpD7,4,"set 2,a"),

	ZOp(&cbpD8,4,"set 3,b"),
	ZOp(&cbpD9,4,"set 3,c"),
	ZOp(&cbpDA,4,"set 3,d"),
	ZOp(&cbpDB,4,"set 3,e"),
	ZOp(&cbpDC,4,"set 3,h"),
	ZOp(&cbpDD,4,"set 3,l"),
	ZOp(&cbpDE,11,"set 3,(hl)"),
	ZOp(&cbpDF,4,"set 3,a"),

	ZOp(&cbpE0,4,"set 4,b"),
	ZOp(&cbpE1,4,"set 4,c"),
	ZOp(&cbpE2,4,"set 4,d"),
	ZOp(&cbpE3,4,"set 4,e"),
	ZOp(&cbpE4,4,"set 4,h"),
	ZOp(&cbpE5,4,"set 4,l"),
	ZOp(&cbpE6,11,"set 4,(hl)"),
	ZOp(&cbpE7,4,"set 4,a"),

	ZOp(&cbpE8,4,"set 5,b"),
	ZOp(&cbpE9,4,"set 5,c"),
	ZOp(&cbpEA,4,"set 5,d"),
	ZOp(&cbpEB,4,"set 5,e"),
	ZOp(&cbpEC,4,"set 5,h"),
	ZOp(&cbpED,4,"set 5,l"),
	ZOp(&cbpEE,11,"set 5,(hl)"),
	ZOp(&cbpEF,4,"set 5,a"),

	ZOp(&cbpF0,4,"set 6,b"),
	ZOp(&cbpF1,4,"set 6,c"),
	ZOp(&cbpF2,4,"set 6,d"),
	ZOp(&cbpF3,4,"set 6,e"),
	ZOp(&cbpF4,4,"set 6,h"),
	ZOp(&cbpF5,4,"set 6,l"),
	ZOp(&cbpF6,11,"set 6,(hl)"),
	ZOp(&cbpF7,4,"set 6,a"),

	ZOp(&cbpF8,4,"set 7,b"),
	ZOp(&cbpF9,4,"set 7,c"),
	ZOp(&cbpFA,4,"set 7,d"),
	ZOp(&cbpFB,4,"set 7,e"),
	ZOp(&cbpFC,4,"set 7,h"),
	ZOp(&cbpFD,4,"set 7,l"),
	ZOp(&cbpFE,11,"set 7,(hl)"),
	ZOp(&cbpFF,4,"set 7,a")
};
