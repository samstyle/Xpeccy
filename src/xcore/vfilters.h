#pragma once

enum {
	AF_2C_ADAPTIVE = 0,
	AF_3C_ADAPTIVE,
	AF_2C_FULL,
    AF_3C_FULL
};

void scrMix(unsigned char *src, unsigned char *p0, int size, double mass, float gamma, int mode);
