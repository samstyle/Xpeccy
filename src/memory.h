#ifndef _MEMOR_H
#define _MEMOR_H

#include <string>
#include <vector>
#include <stdint.h>

#define TYP_SNA		0
#define TYP_Z80 	1
#define TYP_RZX 	2

#define MEM_ZX	0
#define MEM_GS	1

struct RomSet {
	std::string name;
	struct {
		std::string path;
		uint8_t part;
	} roms[8];
};

struct RZXFrame {
	int fetches;		// fetches till next int
	std::vector<uint8_t> in;
};

class Memory {
	public:
	Memory(int32_t);
	int type;
	uint8_t ram[64][16384];
	uint8_t rom[8][16384];
	uint8_t *pt0,*pt1,*pt2,*pt3;
	uint8_t cram,crom;
	uint8_t prt0;		// 7ffd value
	uint8_t prt1;		// extend port value
	uint8_t res;		// rompart active after reset
	int32_t	mask;
	uint8_t rd(uint16_t);
	void wr(uint16_t, uint8_t);
	void load(std::string,int32_t);
	void parse(std::ifstream*,int32_t);
	void save(std::string,int32_t,bool);
//	std::vector<RomSet> rsetlist;
	RomSet *romset;
	void loadromset(std::string);
//	void setromptr(std::string);
	void setram(uint8_t);
	void setrom(uint8_t);
};

#endif
