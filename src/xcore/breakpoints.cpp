#include "xcore.h"

typedef struct {
	int idx;
	xBrkPoint* ptr;
} xbpIndex;

xbpIndex brkFind(xBrkPoint* brk) {
	xbpIndex res;
	res.ptr = NULL;
	res.idx = -1;
	xBrkPoint* tbrk;
	int i;
	int max = conf.prof.cur->brkList.size();
	for (i = 0; (i < max) && !res.ptr; i++) {
		tbrk = &conf.prof.cur->brkList[i];
		if ((tbrk->type == brk->type) && (tbrk->adr == brk->adr)) {
			if (brk->type == BRK_IOPORT) {
				if (tbrk->mask == brk->mask) {
					res.ptr = tbrk;
					res.idx = i;
				}
			} else if (brk->type == BRK_CPUADR) {
				if (tbrk->eadr == brk->eadr) {
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

xBrkPoint* brk_find(int t, int adr) {
	xBrkPoint* ptr = NULL;
	if (t == BRK_IRQ) {
		for (auto it = conf.prof.cur->brkList.begin(); it != conf.prof.cur->brkList.end(); it++) {
			if (!it->off && (it->type == t)) {
				ptr = &(*it);
			}
		}
	} else {
		std::map<int, xBrkPoint*>* map = NULL;
		if (conf.prof.cur->brkMap.count(t) > 0) {
			map = &conf.prof.cur->brkMap[t];
		}
		if (map) {
			std::map<int, xBrkPoint*>::iterator it;
			it = map->find(adr);
			if (it != map->end()) {
				ptr = map->at(adr);
			}
		}
	}
	return ptr;
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
//		end address for BRK_MEMCELL/BRK_CPUADR (-1 if single address)
xBrkPoint brkCreate(int type, int flag, int adr, int mask, int act = BRK_ACT_DBG) {
	xBrkPoint brk;
	int adrmask = -1;
	if (type == BRK_MEMCELL) {
		switch(flag  & MEM_BRK_TMASK) {
			case MEM_BRK_ROM: brk.type = BRK_MEMROM; adrmask = conf.prof.cur->zx->mem->romMask; break;
			case MEM_BRK_RAM: brk.type = BRK_MEMRAM; adrmask = conf.prof.cur->zx->mem->ramMask; break;
			case MEM_BRK_SLT: brk.type = BRK_MEMSLT; adrmask = conf.prof.cur->zx->slot->memMask; break;
			default: brk.type = BRK_MEMEXT; break;
		}
		adr &= adrmask;
		if (mask > 0) mask &= adrmask;
		brk.adr = adr;
		if (mask < 0) {
			brk.eadr = brk.adr;
		} else if (mask > adr) {
			brk.eadr = mask;
		} else {
			brk.adr = mask;
			brk.eadr = adr;
		}
		mask = -1;
	} else if (type == BRK_CPUADR) {
		brk.type = BRK_CPUADR;
		adr &= 0xffff;		// mem->busmask
		if (mask > 0) mask &= 0xffff;
		brk.adr = adr;
		if (mask > adr) {
			brk.eadr = mask;
		} else if (mask >= 0) {
			brk.adr = mask;
			brk.eadr = adr;
		} else {
			brk.eadr = brk.adr;
		}
		mask = -1;
	} else {
		brk.type = type;
		brk.adr = adr;
		brk.eadr = brk.adr;
	}
	brk.off = 0;
	brk.fetch = !!(flag & MEM_BRK_FETCH);
	brk.read = !!(flag & MEM_BRK_RD);
	brk.write = !!(flag & MEM_BRK_WR);
	brk.temp = 0;
	brk.mask = mask;
	brk.count = 0;
	brk.action = act;
	return brk;
}

bool brk_compare(xBrkPoint& bp1, xBrkPoint& bp2) {return (bp1.adr < bp2.adr);}
void brkSort() {std::sort(conf.prof.cur->brkList.begin(), conf.prof.cur->brkList.end(), brk_compare);}

void brkAdd(xBrkPoint brk) {
	xbpIndex idx = brkFind(&brk);
	if (idx.ptr) {
		idx.ptr->fetch = brk.fetch;
		idx.ptr->read = brk.read;
		idx.ptr->write = brk.write;
		brk = *idx.ptr;
	} else {
		conf.prof.cur->brkList.push_back(brk);
	}
	brkSort();
	brkInstallAll();
}

void brkSet(int type, int flag, int adr, int mask) {
	xBrkPoint brk = brkCreate(type, flag, adr, mask);
	brkAdd(brk);
}

void brkXor(int type, int flag, int adr, int mask, int del) {
	xBrkPoint brk = brkCreate(type, flag, adr, mask);
	xbpIndex idx = brkFind(&brk);
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
	brkSort();
	brkInstallAll();
	// brkInstall(brk, del);		// delete if inactive
}

void brkDelete(xBrkPoint dbrk) {
	int idx = brkFind(&dbrk).idx;
	if (idx < 0) return;
	if (idx >= (int)conf.prof.cur->brkList.size()) return;
	conf.prof.cur->brkList.erase(conf.prof.cur->brkList.begin() + idx);
	brkSort();
	brkInstallAll();
}

void brk_clear_tmp(Computer* comp) {
	int i;
	for (i = 0; i < MEM_4M; i++) {
		comp->brkRamMap[i] &= ~MEM_BRK_TFETCH;
	}
	for (i = 0; i < MEM_512K; i++) {
		comp->brkRomMap[i] &= ~MEM_BRK_TFETCH;
	}
	for (i = 0; i < MEM_64K; i++) {
		comp->brkAdrMap[i] &= ~MEM_BRK_TFETCH;
	}
}

void clearMap(unsigned char* ptr, int siz) {
	while (siz > 0) {
		*ptr &= 0xf0;
		ptr++;
		siz--;
	}
}

void brkInstall(xBrkPoint* brk, int del) {
	if (del) {
		brkDelete(*brk);
	} else {
		unsigned char* ptr = NULL;
		Computer* comp = conf.prof.cur->zx;
		unsigned char msk = 0;
		std::map<int, xBrkPoint*>* map = NULL;
		int cnt = 1;
		int adr = -1;
		if (!brk->off) {
			if (brk->temp) msk |= MEM_BRK_TFETCH;
			if (brk->fetch) msk |= MEM_BRK_FETCH;
			if (brk->read) msk |= MEM_BRK_RD;
			if (brk->write) msk |= MEM_BRK_WR;
		}
		if (brk->type != BRK_IRQ)
			map = &conf.prof.cur->brkMap[brk->type];
		switch(brk->type) {
			case BRK_IOPORT:
				for (adr = 0; adr < 0x10000; adr++) {
					if ((adr & brk->mask) == (brk->adr & brk->mask)) {
						comp->brkIOMap[adr] = 0;
						if (!brk->off) {
							if (brk->read) comp->brkIOMap[adr] |= MEM_BRK_RD;
							if (brk->write) comp->brkIOMap[adr] |= MEM_BRK_WR;
						}
					}
					if (comp->brkIOMap[adr]) {
						conf.prof.cur->brkMap[BRK_IOPORT][adr] = brk;
						map = NULL;
					}
				}
				break;
			case BRK_CPUADR:
				ptr = comp->brkAdrMap + (brk->adr & 0xffff);
				cnt = brk->eadr - brk->adr + 1;
				break;
			case BRK_MEMRAM:
				ptr = comp->brkRamMap + (brk->adr & comp->mem->ramMask);
				cnt = brk->eadr - brk->adr + 1;
				break;
			case BRK_MEMROM:
				ptr = comp->brkRomMap + (brk->adr & comp->mem->romMask);
				cnt = brk->eadr - brk->adr + 1;
				break;
			case BRK_MEMSLT:
				if (!comp->slot->brkMap) break;
				ptr = comp->slot->brkMap + (brk->adr & comp->slot->memMask);
				cnt = brk->eadr - brk->adr + 1;
				break;
			case BRK_IRQ:
				comp->brkirq = !brk->off;
				map = NULL;
				break;
		}
		if (ptr) {
			adr = brk->adr;
			while (cnt > 0) {
				*ptr &= 0xf0;
				*ptr |= (msk & 0x0f);
				ptr++;
				if (map) (*map)[adr++] = brk;
				cnt--;
			}
		}
	}
}

void brkInstallAll() {
	xProfile* prf = conf.prof.cur;
	Computer* comp = prf->zx;
	memset(comp->brkAdrMap, 0x00, MEM_64K);
	memset(comp->brkIOMap, 0x00, MEM_64K);
	clearMap(comp->brkRamMap, MEM_4M);
	clearMap(comp->brkRomMap, MEM_512K);
	if (comp->slot->brkMap)
		clearMap(comp->slot->brkMap, comp->slot->memMask + 1);
	prf->brkMap.clear();
	comp->brkirq = 0;
	for(auto it = prf->brkList.begin(); it != prf->brkList.end(); it++) {
		brkInstall(&(*it), 0);
	}
//	foreach(xBrkPoint brk, conf.prof.cur->brkList) {
//		brkInstall(brk, 0);
//	}
}
