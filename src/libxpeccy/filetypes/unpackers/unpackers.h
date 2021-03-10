#pragma once

#define	PAK_HRS	1
#define	PAK_MLZ	2

typedef unsigned short WORD;
typedef unsigned char BYTE;

/* BBStream class */

typedef struct {
	BYTE* base;
	BYTE* p;
	int   idx;
	int   len;
	int pType;
	int  eof;
	WORD  bits;
} bbStream;

void bsInit(bbStream*, BYTE*, int);
BYTE bsGetByte(bbStream*);
BYTE bsGetBit(bbStream*);
BYTE bsGetBits(bbStream*, int);

/*
class BBStream {
private:
	BYTE* base;
	BYTE* p;
	int   idx;
	int   len;
	int pType;
	bool  eof;
	WORD  bits;
public:
	BBStream( BYTE* from, int type) {
		base = p = from;

		pType = type;
		len = 0xffff;
		eof = false;

		if (pType == PAK_HRS) {
			idx = 0;
			bits  = getByte();
			bits += (getByte() << 8);
		} else if (pType == PAK_MLZ) {
			idx = 16;
		}
	}

	BYTE getByte( void ) {
		if( p - base == len ) { eof = true; return 0; }
		return *p++;
	}

	BYTE getBit() {
		WORD mask[]  = { 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100, 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1 };

		if ((idx > 15) && (pType == PAK_MLZ)) {
			bits = getByte();
			idx = 8;
		}
		BYTE bit = ( bits & mask[idx] ) ? 1 : 0;
		if ((idx == 15) && (pType == PAK_HRS)) {
			bits = getByte();
			bits += (getByte() << 8);
			idx = -1;
		}
		idx++;
		return bit;
	}

	BYTE getBits( int n ) {
		BYTE r = 0;
		do { r = (r << 1) | getBit(); } while( --n );
		return r;
	}

	bool error( void ) { return eof; }
};
*/

WORD dehrust(BYTE*, BYTE*);
WORD demegalz(BYTE*, BYTE*);
