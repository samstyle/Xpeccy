#ifndef _IOSYS_H
#define _IOSYS_H

#include <string>
#include <vector>

#define	IO_WAIT	1

struct Machine {
	std::string name;
	int mask;		// mem size mask (0:128, 1:256, 2:512, 3:1024); =0 for 48K
	int flags;
	int (*getport)(int);
	void (*out)(int,unsigned char);
	unsigned char (*in)(int);
	void (*setrom)();
};

struct IOSys {
	IOSys(unsigned char(*)(int),void(*)(int,unsigned char));
	std::vector<Machine> machlist;
	void addmachine(std::string,int(*)(int),void(*)(int,unsigned char),unsigned char(*)(int),void(*)(),int,int);
	void setmacptr(std::string);
	bool block7ffd;
	bool resafter;
	int mask;		// rampage mask (0x00,0x0f,0x1f,0x3f)
	int flags;
	void iostdout(int,unsigned char);
	unsigned char iostdin(int);
	void out7ffd(unsigned char);
	void (*out)(int,unsigned char);
	unsigned char (*in)(int);
};

//extern IOSys *iosys;
extern Machine *machine;

#endif
