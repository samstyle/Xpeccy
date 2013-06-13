//#include <windows.h>

#include "packers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
#pragma hdrstop
#pragma argsused
*/
#define DEHRUST_PC_VERSION	"1.0i"

/* BBStream class */

class BBStream {
private:
	BYTE* base;
	BYTE* p;
	int   idx;
	int   len;
	bool  eof;
	WORD  bits;
public:
	BBStream( BYTE* from, int blockSize ) {
		base = p = from;

		len = blockSize;
		idx = 0;
		eof = false;

		bits  = getByte();
		bits += 256*getByte();
	}

	BYTE getByte( void ) {
		if( p - base == len ) { eof = true; return 0; }
		return *p++;
	}

	BYTE getBit() {
		WORD mask[]  = { 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100, 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1 };

		BYTE bit = ( bits & mask[idx] ) ? 1 : 0;
		if( idx == 15 )
		{
			bits  = getByte();
			bits += 256*getByte();
		}

		idx = (idx + 1) & 0x0f; // % 16;
		return bit;
	}

	BYTE getBits( int n ) {
		BYTE r = 0;
		do { r = 2*r + getBit(); } while( --n );
		return r;
	}

	bool error( void ) { return eof; }
};

/* depacker */

WORD dehrust( BYTE* source, BYTE* dest )
{

	BYTE *from = source;
	BBStream s( from /* + 12 , from[4] + 256*from[5] */,0xffff);
	BYTE* to = dest;

	*(to++) = s.getByte();

	BYTE noBits = 2;
	BYTE mask[] = { 0, 0, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0 };

	while( !s.error() )
	{
		while( s.getBit() ) {
			*to = s.getByte();
			to++;
		}

		WORD len = 0;
		BYTE bb /* = 0*/;
		do
		{
			bb = s.getBits( 2 );
			len += bb;
		} while( bb == 0x03 && len != 0x0f );

		short offset = 0;

		if( len == 0 )
		{
			offset = 0xfff8 + s.getBits( 3 );
			*to = to[offset]; to++;
			continue;
		}

		if( len == 1 )
		{
			BYTE code = s.getBits(2);

			if( code == 2 )
			{
				BYTE b = s.getByte();
				if( b >= 0xe0 )
				{
					b <<= 1; ++b; // rlca
					b ^= 2;       // xor c

					if( b == 0xff ) { ++noBits; continue; }

					offset = 0xff00 + b - 0x0f;

					*to = to[offset]; to++;
					*to = s.getByte(); to++;
					*to = to[offset]; to++;
					continue;
				}
				offset = 0xff00 + b;
			}

			if( code == 0 || code == 1 )
			{
				offset = s.getByte();
				offset += 256*(code ? 0xfe : 0xfd );
			}
			if( code == 3 ) offset = 0xffe0 + s.getBits( 5 );

			for( BYTE i = 0; i < 2; ++i ) {
				*to = to[offset]; to++;
			}
			continue;
		}

		if( len == 3 )
		{
			if( s.getBit() )
			{
				offset = 0xfff0 + s.getBits( 4 );
				*to = to[offset]; to++;
				*to = s.getByte(); to++;
				*to = to[offset]; to++;
				continue;
			}

			if( s.getBit() )
			{
				BYTE noBytes = 6 + s.getBits(4);
				for( BYTE i = 0; i < 2*noBytes; ++i ) *(to++) = s.getByte();
				continue;
			}

			len = s.getBits( 7 );
			if( len == 0x0f ) break; // EOF
			if( len <  0x0f ) len = 256*len + s.getByte();
		}

		if( len == 2 ) ++len;

		BYTE code = s.getBits( 2 );

		if( code == 1 )
		{
			BYTE b = s.getByte();

			if( b >= 0xe0 )
			{
				if( len > 3 ) return false;

				b <<= 1; ++b; // rlca
				b ^= 3;       // xor c

				offset = 0xff00 + b - 0x0f;

				*to = to[offset]; to++;
				*to = s.getByte(); to++;
				*to = to[offset]; to++;
				continue;
			}
			offset = 0xff00 + b;
		}

		if( code == 0 ) offset = 0xfe00 + s.getByte();
		if( code == 2 ) offset = 0xffe0 + s.getBits( 5 );
		if( code == 3 )
		{
			offset  = 256*( mask[noBits] + s.getBits(noBits) );
			offset += s.getByte();
		}

		for( WORD i = 0; i < len; ++i ) {
			*to = to[offset]; to++;
		}
	}

	for( int i = 0; i < 6; ++i ) *(to++) = from[6+i];

	return to-dest;
}




/*


const clr_default	= 0x07;
const clr_info		= 0x0a;
const clr_info_		= 0x0e;
const clr_warning	= 0x0c;
const clr_done  	= 0x0f;

void set_color(WORD ink)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ink);
}





int main(int argc, char* argv[])
{
    BYTE src[0xffff],*ss;
    BYTE dst[0xffff];

    int i;
    char *nameIn, *nameOut, *p;
    FILE *pfi, *pfo;

    nameIn = NULL;
    nameOut = NULL;

    for (i = 1; i < argc; i++)
    {
	p = argv[i];
	if (*p != '-')
	  if (!nameIn) nameIn = p;
	  else if (!nameOut) nameOut = p;
    }
    if (!nameIn || !nameOut)
    {
	set_color(clr_info);
	printf("\nHrust 1 depacker, version " DEHRUST_PC_VERSION "\n");
	set_color(clr_default);
	printf("Original depacker by Hrumer, C++ version by Hrumer & HalfElf,\n"
	       "console version by psb.\n\n");
	set_color(clr_info);
	printf("Usage: dehrust1 <packed.bin> <depacked.bin>\n\n");
	set_color(clr_default);
	return 1;
    }

    pfi = fopen(nameIn, "rb");
    if (!pfi)
    {
	set_color(clr_warning);
	printf("DeHrust 1: Can't open '%s' for reading.\n", nameIn);
	set_color(clr_default);
	return 2;
    }

    pfo = fopen(nameOut, "wb");
    if (!pfo)
    {
	fclose(pfi);
	set_color(clr_warning);
	printf("DeHrust 1: Can't open '%s' for writting.\n", nameOut);
	set_color(clr_default);
	return 2;
    }

  i=fread(src, 1, 0xffff, pfi);
  if (src[0]=='H' && src[1]=='R' && i>=15)
    ss=src;
  else if (src[0+0x103]=='H' && src[1+0x103]=='R' && i>=15)
	 ss=src+0x103;
       else
	 {
	   fclose(pfi); fclose(pfo);
	   set_color(clr_warning);
	   printf("DeHrust 1: '%s' is not hrust1 file.\n", nameIn);
	   set_color(clr_default);
	   return 2;
	 }


  i=dehrust(ss, dst);
  fwrite(dst, 1, i, pfo);
  fclose(pfi); fclose(pfo);
  if (i!=ss[2]+256*ss[3])
  {
    set_color(clr_warning);
    printf("DeHrust 1: Warning! Length of unpacked file '%s' is not equal length in hrust header!\n",nameIn);
    set_color(clr_default);
    return 3;
  }
  set_color(clr_done);
  printf("DeHrust 1: File '%s' depacked.\n",nameIn);
  set_color(clr_default);

  return 0;
}



*/
