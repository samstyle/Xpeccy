#include <QColor>

// convert <size> bytes (by 3) on <ptr> from color-RGB to gray-RGB
void scrGray(unsigned char* ptr, int size) {
	unsigned char gray;
	while (size > 0) {
		gray = qGray(*ptr, *(ptr+1), *(ptr+2)) & 0xff;
		*(ptr++) = gray;
		*(ptr++) = gray;
		*(ptr++) = gray;
		size -= 3;
	}
}

// mix prev <size> bytes from <src> to <dst> 50/50 and copy unmixed <dst> to <src>
void scrMix(unsigned char* src, unsigned char* dst, int size, double mass) {
	unsigned char cur;
	while (size > 0) {
		cur = *dst;
		*dst = (*src * (1.0 - mass) + cur * mass);
		*src = cur;
		src++;
		dst++;
		size--;
	}
}
