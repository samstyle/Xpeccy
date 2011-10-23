#ifndef _BDI_H
#define _BDI_H

#include <string>
#include <vector>
#include <stdint.h>

#define TRACKLEN 6250

#define INSTIME		1			// на будущее - время смены дискеты

#define TYPE_TRD	0
#define TYPE_SCL	1
#define TYPE_FDI	2
#define TYPE_UDI	3
#define TYPE_HOBETA	4

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
	uint8_t len;	// 0..6 = 128,256,512,1024,2048,4096
	uint8_t* data;
	uint8_t type;	// f8 / fb
	int32_t crc;	// data crc (-1 = calculated)
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
	bool insert;		// disk inserted
	bool protect;		// write protected
	bool trk80;		// drive have 80 tracks
	bool dblsid;		// drive have 2 heads (DS)
	bool index;		// index pulse
	bool motor;		// motor on/off
	bool head;		// head load/unload
	bool changed;		// disk changed
	bool side;		// side signal from FDC
	uint8_t id;	// 0..3 = A..D
	uint8_t iback;	// countdown for index (250);
	uint8_t trk,rtrk;	// physic, logic track number
	uint8_t field; //,fnext;	// текущее и следующее поле (0:none; 1:address; 2:data-FB; 3:data-F8
	int32_t pos;		// 0..TRACKLEN-1	current byte position
	uint32_t ti;
	std::string path;		// путь к файлу
	TrackIMG data[256];		// disk data (full)
	void next(bool,uint32_t);		// NEW: move to next byte
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
};

class VG93 {
	public:
		VG93();
		bool turbo;		// турбо
		bool idle;
		bool mr;			// master reset
		bool crchi;
		bool block,mfm,irq,drq,sdir;
		bool idxold,idx,strb;
		uint8_t com,cop; //,fld;//,lop
		uint8_t trk,sec,side,data,flag,bus;	// ,drv
		uint8_t buf[6];
		uint8_t dpos;
		uint8_t mode,sc;
		uint8_t *wptr, *sp;
		uint16_t ic;
		uint16_t crc,fcrc;
		int32_t count;			// задержка;
		uint32_t t;
		uint32_t tf;			// tf: countdown for ticks @ floppy byte;
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
		bool enable;		// есть-нет
		bool active;		// активен дос
		bool pcatch;		// порт перехвачен
		uint32_t tab;		// tab: ticks @ floppy byte
		Floppy flop[4];
		VG93 vg93;
		void sync(uint32_t);
		bool out(int32_t, uint8_t);
		bool in(int32_t, uint8_t*);
		int32_t getport(int);
};

// extern BDI *bdi;
typedef void(*VGOp)(VG93*);

#endif
