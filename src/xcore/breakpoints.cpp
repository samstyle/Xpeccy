#include "xcore.h"

typedef struct {
	int idx;
	xBrkPoint* ptr;
} xbpIndex;

xbpIndex brkFind(xBrkPoint brk) {
	xbpIndex res;
	res.ptr = NULL;
	res.idx = -1;
	xBrkPoint* tbrk;
	int i;
	int max = conf.prof.cur->brkList.size();
	for (i = 0; (i < max) && !res.ptr; i++) {
		tbrk = &conf.prof.cur->brkList[i];
		if ((tbrk->type == brk.type) && (tbrk->adr == brk.adr)) {
			if (brk.type == BRK_IOPORT) {
				if (tbrk->mask == brk.mask) {
					res.ptr = tbrk;
					res.idx = i;
				}
			} else if (brk.type == BRK_CPUADR) {
				if (tbrk->eadr == brk.eadr) {
					res.ptr = tbrk;
					res.idx = i;
				}
			} else {
				res.ptr = tbrk;
				res.idx = i;
			}
		}
//		if (res.ptr) break;
	}
	return res;
}

// create breakpoint
// type		BRK_MEMCELL	memory cell
//		BRK_CPUADR	cpu address
//		BRK_IOPORT	io address
//		BRK_IRQ		interrupt
// flag		MEM_BRK_ROM | MEM_BRK_RAM | MEM_BRK_SLT | 0 : memory type for BRK_MEMCELL
//		BRK_FETCH || BRK_RD || BRK_WR		: break conditions
// adr		0000..FFFF for BRK_CPUADR
//		full memory address for BRK_MEMCELL
// mask		address mask for BRK_IOADR
xBrkPoint brkCreate(int type, int flag, int adr, int mask) {
	xBrkPoint brk;
	if (type == BRK_MEMCELL) {
		switch(flag  & MEM_BRK_TMASK) {
			case MEM_BRK_ROM: brk.type = BRK_MEMROM; break;
			case MEM_BRK_RAM: brk.type = BRK_MEMRAM; break;
			case MEM_BRK_SLT: brk.type = BRK_MEMSLT; break;
			default: brk.type = BRK_MEMEXT; break;
		}
		brk.adr = adr;
		brk.eadr = brk.adr;
	} else if (type == BRK_CPUADR) {
		brk.type = type;
		brk.adr = adr & 0xffff;
		if (mask > adr) {
			brk.eadr = mask & 0xffff;
		} else if (mask >= 0) {
			brk.adr = mask & 0xffff;
			brk.eadr = adr & 0xffff;
		} else {
			brk.eadr = brk.adr;
		}
		mask = -1;
	} else {
		brk.type = type;
		brk.adr = adr & 0xffff;
		brk.eadr = brk.adr;
	}
	brk.off = 0;
	brk.fetch = (flag & MEM_BRK_FETCH) ? 1 : 0;
	brk.read = (flag & MEM_BRK_RD) ? 1 : 0;
	brk.write = (flag & MEM_BRK_WR) ? 1 : 0;
	brk.temp = 0;
	brk.mask = mask;
	return brk;
}

void brkInstall(xBrkPoint brk, int del) {
	unsigned char* ptr = NULL;
	Computer* comp = conf.prof.cur->zx;
	unsigned char msk = 0;
	int cnt = 1;
	if (!brk.off) {
		if (brk.temp) msk |= MEM_BRK_TFETCH;
		if (brk.fetch) msk |= MEM_BRK_FETCH;
		if (brk.read) msk |= MEM_BRK_RD;
		if (brk.write) msk |= MEM_BRK_WR;
	}
	switch(brk.type) {
		case BRK_IOPORT:
			for (int adr = 0; adr < 0x10000; adr++) {
				if ((adr & brk.mask) == (brk.adr & brk.mask)) {
					comp->brkIOMap[adr] = 0;
					if (!brk.off) {
						if (brk.read) comp->brkIOMap[adr] |= MEM_BRK_RD;
						if (brk.write) comp->brkIOMap[adr] |= MEM_BRK_WR;
					}
				}
			}
			break;
		case BRK_CPUADR:
			ptr = comp->brkAdrMap + (brk.adr & 0xffff);
			cnt = brk.eadr - brk.adr + 1;
			break;
		case BRK_MEMRAM: ptr = comp->brkRamMap + (brk.adr & 0x3fffff); break;
		case BRK_MEMROM: ptr = comp->brkRomMap + (brk.adr & 0x7ffff); break;
		case BRK_MEMSLT:
			if (!comp->slot->brkMap) break;
			ptr = comp->slot->brkMap + (brk.adr & comp->slot->memMask);
			break;
		case BRK_IRQ:
			comp->brkirq = !brk.off;
			break;
	}
	if (!ptr) return;
	while (cnt > 0) {
		*ptr &= 0xf0;
		*ptr |= (msk & 0x0f);
		ptr++;
		cnt--;
	}
	if (!del || brk.fetch || brk.read || brk.write || brk.temp) return;
	brkDelete(brk);
}

void brkAdd(xBrkPoint brk) {
	xbpIndex idx = brkFind(brk);
	if (idx.ptr) {
		idx.ptr->fetch = brk.fetch;
		idx.ptr->read = brk.read;
		idx.ptr->write = brk.write;
		brk = *idx.ptr;
	} else {
		conf.prof.cur->brkList.push_back(brk);
	}
	// brkInstall(brk, 0);
	brkInstallAll();
}

void brkSet(int type, int flag, int adr, int mask) {
	xBrkPoint brk = brkCreate(type, flag, adr, mask);
	brkAdd(brk);
}

void brkXor(int type, int flag, int adr, int mask, int del) {
	xBrkPoint brk = brkCreate(type, flag, adr, mask);
	xbpIndex idx = brkFind(brk);
	if (idx.ptr) {
		idx.ptr->fetch ^= brk.fetch;
		idx.ptr->read ^= brk.read;
		idx.ptr->write ^= brk.write;
		brk = *idx.ptr;
		if (del && !brk.fetch && !brk.read && !brk.write && !brk.temp) {
			conf.prof.cur->brkList.erase(conf.prof.cur->brkList.begin() + idx.idx);
		}
	} else {
		conf.prof.cur->brkList.push_back(brk);
	}
	brkInstallAll();
	// brkInstall(brk, del);		// delete if inactive
}

void brkDelete(xBrkPoint dbrk) {
	int idx = brkFind(dbrk).idx;
	if (idx < 0) return;
	if (idx >= (int)conf.prof.cur->brkList.size()) return;
//	xBrkPoint brk = conf.prof.cur->brkList[idx];
//	brk.off = 1;
//	brk.fetch = 1;
//	brkInstall(brk, 0);
	conf.prof.cur->brkList.erase(conf.prof.cur->brkList.begin() + idx);
	brkInstallAll();
}

void clearMap(unsigned char* ptr, int siz) {
	while (siz > 0) {
		siz--;
		ptr[siz] &= 0xf0;
	}
}

void brkInstallAll() {
	Computer* comp = conf.prof.cur->zx;
	memset(comp->brkAdrMap, 0x00, 0x10000);
	memset(comp->brkIOMap, 0x00, 0x10000);
	clearMap(comp->brkRamMap, 0x400000);
	clearMap(comp->brkRomMap, 0x80000);
	if (comp->slot->brkMap)
		clearMap(comp->slot->brkMap, comp->slot->memMask + 1);
	foreach(xBrkPoint brk, conf.prof.cur->brkList) {
		brkInstall(brk, 0);
	}
}
