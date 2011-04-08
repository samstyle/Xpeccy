#ifndef _IOSYS_H
#define _IOSYS_H

#include <string>
#include <vector>

#define	IO_WAIT	1
#define WAIT_ON	2

#define IO_ZX	0
#define	IO_GS	1

class HardWare {
	public:
		std::string name;
		int mask;		// mem size mask (0:128, 1:256, 2:512, 3:1024); =0 for 48K
		int flags;
		int type;
//		int (*getport)(int);
//		void (*out)(int,uint8_t);
//		uint8_t (*in)(int);
//		void (*setrom)();
};

class IOSys {
	public:
//		IOSys(uint8_t(*)(int),void(*)(int,uint8_t));
		IOSys(int);
//		std::vector<HardWare> hwlist;
//		void addhardware(std::string,int(*)(int),void(*)(int,uint8_t),uint8_t(*)(int),void(*)(),int,int);
//		void setmacptr(std::string);
		bool block7ffd;
		bool resafter;
//		int32_t mask;		// rampage mask (0x00,0x0f,0x1f,0x3f)
//		int32_t flags;
		int32_t type;
		void iostdout(int,uint8_t);
		uint8_t iostdin(int);
		void out7ffd(uint8_t);
		void out(int32_t,uint8_t);
		uint8_t in(int32_t);
};

#endif
