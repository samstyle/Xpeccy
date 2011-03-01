#ifndef _MEMOR_H
#define _MEMOR_H

#include <string>
#include <vector>

#define TYP_SNA	0
#define TYP_Z80 1

#define MEM_ZX	0
#define MEM_GS	1

struct RomSet {
	std::string name;
	struct {
		std::string path;
		unsigned char part;
	} roms[8];
};

struct Memory {
	Memory(int);
	int type;
	unsigned char ram[64][16384];
	unsigned char rom[8][16384];
	unsigned char *pt0,*pt1,*pt2,*pt3;
//	unsigned char curram,currom;
	unsigned char cram,crom;
	unsigned char prt0;		// 7ffd value
	unsigned char prt1;		// extend port value
//	unsigned char vtmp;
	unsigned char res;		// rompart active after reset
	unsigned char rd(unsigned short);
	void wr(unsigned short,unsigned char);
//	unsigned char* getptr(unsigned short);
	void load(std::string,int);
	void parse(std::ifstream*,int);
	void save(std::string,int,bool);
	std::vector<RomSet> rsetlist;
	RomSet *romset;
	void loadromset();
	void setromptr(std::string);
	void setram(unsigned char);
	void setrom(unsigned char);
};

//extern Memory *mem;

#endif
