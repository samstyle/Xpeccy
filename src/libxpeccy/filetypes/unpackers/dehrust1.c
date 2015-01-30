#include "unpackers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEHRUST_PC_VERSION	"1.0i"

/* depacker */

WORD dehrust( BYTE* source, BYTE* dest ) {

	BYTE *from = source;
	bbStream strm;
	bbStream* bs = &strm;
	bsInit(bs, from, PAK_HRS);
	BYTE* to = dest;

	*(to++) = bsGetByte(bs);

	BYTE noBits = 2;
	BYTE mask[] = { 0, 0, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0 };

	while( !bs->eof )
	{
		while( bsGetBit(bs) ) {
			*to = bsGetByte(bs);
			to++;
		}

		WORD len = 0;
		BYTE bb;
		do {
			bb = bsGetBits(bs, 2);
			len += bb;
		} while((bb == 0x03) && (len != 0x0f));

		short offset = 0;

		if( len == 0 ) {
			offset = 0xfff8 + bsGetBits(bs, 3);
			*to = to[offset]; to++;
			continue;
		}

		if( len == 1 ) {
			BYTE code = bsGetBits(bs, 2);

			if( code == 2 ) {
				BYTE b = bsGetByte(bs);
				if( b >= 0xe0 ) {
					b <<= 1; ++b; // rlca
					b ^= 2;       // xor c

					if( b == 0xff ) { ++noBits; continue; }

					offset = 0xff00 + b - 0x0f;

					*to = to[offset]; to++;
					*to = bsGetByte(bs); to++;
					*to = to[offset]; to++;
					continue;
				}
				offset = 0xff00 + b;
			}

			if( code == 0 || code == 1 ) {
				offset = bsGetByte(bs);
				offset += 256*(code ? 0xfe : 0xfd );
			}
			if( code == 3 ) offset = 0xffe0 + bsGetBits(bs, 5);

			for( BYTE i = 0; i < 2; ++i ) {
				*to = to[offset]; to++;
			}
			continue;
		}

		if( len == 3 ) {
			if (bsGetBit(bs)) {
				offset = 0xfff0 + bsGetBits(bs, 4);
				*to = to[offset]; to++;
				*to = bsGetByte(bs); to++;
				*to = to[offset]; to++;
				continue;
			}

			if( bsGetBit(bs) ) {
				BYTE noBytes = 6 + bsGetBits(bs, 4);
				for( BYTE i = 0; i < 2*noBytes; ++i ) *(to++) = bsGetByte(bs);
				continue;
			}

			len = bsGetBits(bs, 7);
			if( len == 0x0f ) break; // EOF
			if( len <  0x0f ) len = 256*len + bsGetByte(bs);
		}

		if( len == 2 ) ++len;

		BYTE code = bsGetBits(bs, 2);

		if( code == 1 ) {
			BYTE b = bsGetByte(bs);

			if( b >= 0xe0 ) {
				if( len > 3 ) return 0;

				b <<= 1; ++b; // rlca
				b ^= 3;       // xor c

				offset = 0xff00 + b - 0x0f;

				*to = to[offset]; to++;
				*to = bsGetByte(bs); to++;
				*to = to[offset]; to++;
				continue;
			}
			offset = 0xff00 + b;
		}

		if( code == 0 ) offset = 0xfe00 + bsGetByte(bs);
		if( code == 2 ) offset = 0xffe0 + bsGetBits(bs, 5);
		if( code == 3 ) {
			offset  = 256*( mask[noBits] + bsGetBits(bs, noBits) );
			offset += bsGetByte(bs);
		}

		for( WORD i = 0; i < len; ++i ) {
			*to = to[offset]; to++;
		}
	}

	return to-dest;
}
