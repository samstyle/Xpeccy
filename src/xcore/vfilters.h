#pragma once

enum {
	AF_2C_FULL = 0,
	AF_2C_ADAPTIVE,
    AF_3C_FULL,
	AF_3C_ADAPTIVE
};

void scrMix(unsigned char *src, unsigned char *p0, int size, double mass, float gamma, int mode);
