#include <QColor>
#include <math.h>

// Linear->sRGB conversion table
static unsigned char srgb_to_linear[256];
// sRGB->Linear conversion table
static unsigned char linear_to_srgb[256];

// helper for clamping values
static inline float clampf(float x, float lo, float hi) {
    return fminf(fmaxf(x, lo), hi);
}

float last_gamma = 0;

static void rebuild_gamma_lut(float gamma) {

    gamma = fmaxf(1.0, gamma);	// gamma <= 1 == linear blending

    const float igamma = 1.0f / gamma;
    const float maxvalue = (float)(256 - 1);

	// generating sRGB colorspace conversion tables
    for (int i = 0; i < 256; i++) {
        const float component = (float)i / maxvalue;

        // building Linear->sRGB conversion table
        float v_fwd = (1.055f * powf(component, igamma) - 0.055f) * maxvalue;
        srgb_to_linear[i] = (unsigned char)(clampf(v_fwd, 0.0f, maxvalue) + 0.5f);

        // building sRGB->Linear conversion table
        float v_rev = powf((component + 0.055f) / 1.055f, gamma) * maxvalue;
        linear_to_srgb[i] = (unsigned char)(clampf(v_rev, 0.0f, maxvalue) + 0.5f);
    }
	last_gamma = gamma;
}

// blend src into dst with weight `mass` in linear space (sRGB-correct),
// then store original dst back to src for the next frame.
// sRGB colorspace in Gigascreen reference: https://hype.retroscene.org/blog/graphics/808.html
void scrMix(unsigned char* src, unsigned char* dst, int size, double mass, float gamma) {
	unsigned char cur;
	// Rebuild gamma LUTs if Gamma value has changed
	if (last_gamma != gamma) { rebuild_gamma_lut(gamma); }

	while (size > 0) {
		cur = *dst;
		*dst = srgb_to_linear[int(linear_to_srgb[*src] * mass + linear_to_srgb[*dst] * (1.0 - mass))];
		*src = cur;
		src++;
		dst++;
		size--;
	}
}
