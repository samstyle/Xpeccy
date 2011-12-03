#ifndef _BDI_H
#define _BDI_H

#include <string>
#include <vector>
#include <stdint.h>

#define TRACKLEN 6250

#define	ERR_OK		0
#define	ERR_MANYFILES	1
#define	ERR_NOSPACE	2
#define	ERR_SHIT	3

struct TrackIMG {
	uint8_t byte[TRACKLEN];
	uint8_t field[TRACKLEN];
};

class Sector {
	public:
	Sector();
	Sector(uint8_t,uint8_t,uint8_t,uint8_t,uint8_t*);
	uint8_t cyl;
	uint8_t side;
	uint8_t sec;
	uint8_t len;
	uint8_t* data;
	uint8_t type;
	int32_t crc;
};

struct TRFile {
	uint8_t name[8];
	uint8_t ext;
	uint8_t lst,hst;
	uint8_t llen,hlen;
	uint8_t slen;
	uint8_t sec;
	uint8_t trk;
};

class Floppy {
	public:
	Floppy();
	bool insert;
	bool protect;
	bool trk80;
	bool dblsid;
	bool index;
	bool motor;
	bool head;
	bool changed;
	bool side;
	uint8_t id;
	uint8_t iback;
	uint8_t trk,rtrk;
	uint8_t field;
	int32_t pos;
	uint32_t ti;
	std::string path;
	TrackIMG data[256];
	void next(bool,uint32_t);
	void step(bool);
	void format();
	void formtrdtrack(uint8_t, uint8_t*);
	void formtrack(uint8_t, std::vector<Sector>);
	void nulltrack(uint8_t);
	void fillfields(uint8_t, bool);
	void load(std::string, uint8_t);
	void save(std::string, uint8_t);
	void loaduditrack(std::ifstream*, uint8_t, bool);
	void getudibitfield(uint8_t, uint8_t*);
	void wr(uint8_t);
	bool savecha();
	bool eject();
	uint8_t getfield();
	uint8_t rd();
	uint16_t getcrc(uint8_t*, int32_t);
	std::vector<Sector> getsectors(uint8_t);
	std::string getString();
	void setString(std::string);
	int getDiskType();
	int createFile(TRFile*);
	std::vector<TRFile> getTRCatalog();
	uint8_t* getSectorDataPtr(uint8_t,uint8_t);
	bool getSectorData(uint8_t, uint8_t, uint8_t*, int);
	bool putSectorData(uint8_t, uint8_t, uint8_t*, int);
	bool getSectorsData(uint8_t, uint8_t, uint8_t*, int);
};

class VG93 {
	public:
		VG93();
		bool turbo;
		bool idle;
		bool mr;
		bool crchi;
		bool block,mfm,irq,drq,sdir;
		bool idxold,idx,strb;
		uint8_t com,cop;
		uint8_t trk,sec,side,data,flag,bus;
		uint8_t buf[6];
		uint8_t dpos;
		uint8_t mode,sc;
		uint8_t *wptr, *sp;
		uint16_t ic;
		uint16_t crc,fcrc;
		int32_t count;
		uint32_t t;
		uint32_t tf;
		Floppy* fptr;
		void tick();
		void command(uint8_t);
		void setmr(bool);
		void addcrc(uint8_t);
		uint8_t getflag();
};

class BDI {
	public:
		BDI();
		bool enable;
		bool active;
		bool pcatch;
		uint32_t tab;
		Floppy flop[4];
		VG93 vg93;
		void sync(int);
		bool out(int32_t, uint8_t);
		bool in(int32_t, uint8_t*);
		int32_t getport(int);
};

typedef void(*VGOp)(VG93*);

#endif
