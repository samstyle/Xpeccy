#include <QColor>

// convert <size> bytes (by 3) on <ptr> from color-RGB to gray-RGB
void scrGray(unsigned char* ptr, int size) {
	int gray;
	while (size > 0) {
		gray = qGray(*ptr, *(ptr+1), *(ptr+2));
		*(ptr++) = gray & 0xff;
		*(ptr++) = gray & 0xff;
		*(ptr++) = gray & 0xff;
		size -= 3;
	}
}

// mix prev <size> bytes from <src> to <dst> 50/50 and copy unmixed <dst> to <src>
void scrMix(unsigned char* src, unsigned char* dst, int size, double mass) {
	unsigned char cur;
	while (size > 0) {
		cur = *dst;
		*dst = (*src * (1.0 - mass) + cur * mass);
		// *dst = (*src + cur) >> 1;
		*src = cur;
		src++;
		dst++;
		size--;
	}
}

/*
// make image dim
void scrDim(unsigned char* src, int size) {
	while(size > 0) {
		*src >>= 2;
		src++;
		size--;
	}
}
*/

// TODO: pixel filters like eps, scale2x... etc

/*
void colEPS(unsigned char* ptr, int linSize) {
	int cp,ca,cb,cc,cd,c1,c2,c3,c4;
	cp = *ptr; ca = *(ptr - linSize); cb = *(ptr + 3); cc = *(ptr - 3); cd = *(ptr + linSize);
	c1 = *(ptr - linSize - 3); c2 = ca; c3 = cc; c4 = cp;
	if (((ca == cb) && (cb == cc)) || ((ca == cb) && (cb == cd)) || ((ca == cc) && (cc == cd)) || ((cb == cc) && (cc == cd))) {
		c1 = c2 = c3 = c4 = cp;
	} else {
		if (ca == cc) c1 = ca;
		if (ca == cb) c2 = cb;
		if (cc == cd) c3 = cc;
		if (cb == cd) c4 = cd;
	}
	*(ptr - linSize - 3) = c1;
	*(ptr - linSize) = c2;
	*(ptr - 3) = c3;
	*ptr = c4;
}

void scrEPS(unsigned char* src, int wid, int hig) {
#if 0
	int x,y;
	int linSize = wid * 3;
	unsigned char* sptr = src + linSize + 3;	// (1,1) pixel
	unsigned char* ptr;
	for (y = 1; y < hig - 1; y += 2) {
		ptr = sptr;
		for (x = 1; x < wid - 1; x += 2) {
			colEPS(ptr++, linSize);	// r
			colEPS(ptr++, linSize);	// g
			colEPS(ptr++, linSize);	// b
			ptr += 3;
		}
		sptr += linSize * 2;
	}
#else
	int x,y;
	int linSize = wid * 3;
	unsigned char* sptr = src + 3;
	unsigned char* ptr;
	for (y = 1; y < hig - 1; y++) {
		ptr = sptr;
		for (x = 1; x < wid -1; x++) {
			*ptr = (*(ptr - 3) >> 2) + (*ptr >> 1) + (*(ptr + 3) >> 2); ptr++;
			*ptr = (*(ptr - 3) >> 2) + (*ptr >> 1) + (*(ptr + 3) >> 2); ptr++;
			*ptr = (*(ptr - 3) >> 2) + (*ptr >> 1) + (*(ptr + 3) >> 2); ptr++;
		}
		sptr += linSize;
	}
#endif
}
*/
