#include <QColor>
#include <math.h>
#include "vfilters.h"

// Linear->sRGB conversion table
static unsigned char linear_to_srgb[256];
// sRGB->Linear conversion table
static unsigned char srgb_to_linear[256];

// helper for clamping values
static inline float clampf(float x, float lo, float hi) {
    return fminf(fmaxf(x, lo), hi);
}

#define RING_FRAMES 5
float last_gamma = 0;
uint32_t *ring_base = NULL;
static int ring_head = 0; // last frame idx

static void ring_rotate(void) {
	ring_head = (ring_head + RING_FRAMES - 1) % RING_FRAMES;
}

static uint32_t *ring_get_frame(int i, int size) { // i = 0..4, 0 = last, 1 = last-1, ...
	return ring_base + ((ring_head + i) % RING_FRAMES) * size;
}

static void rebuild_gamma_lut(float gamma) {

    gamma = fmaxf(1.0, gamma);	// gamma <= 1 == linear blending

    const float igamma = 1.0f / gamma;
    const float maxvalue = (float)(256 - 1);

	// generating sRGB colorspace conversion tables
    for (int i = 0; i < 256; i++) {
        const float component = (float)i / maxvalue;

        // building Linear->sRGB conversion table
        float v_fwd = component <= 0.0031308f ? 12.92f * component : (1.055f * powf(component, igamma) - 0.055f) * maxvalue;
        linear_to_srgb[i] = (unsigned char)(clampf(v_fwd, 0.0f, maxvalue) + 0.5f);

        // building sRGB->Linear conversion table
        float v_rev = component <= 0.04045f ? component / 12.92f : powf((component + 0.055f) / 1.055f, gamma) * maxvalue;
        srgb_to_linear[i] = (unsigned char)(clampf(v_rev, 0.0f, maxvalue) + 0.5f);
    }
	last_gamma = gamma;
}

// - Blending functions --------------------------------------------------------
// Gigascreen blending via LUTs
static inline uint32_t blend_2c(uint32_t p0, uint32_t p1, float ratio) {
	const double ratio_rev = 1.0 - ratio;

	// Extract RGB pixel components for current frame (RGBA packed format)
	unsigned char frame0_r = srgb_to_linear[p0 & 0xFF];
	unsigned char frame0_b = srgb_to_linear[(p0 >> 16) & 0xFF];
	unsigned char frame0_g = srgb_to_linear[(p0 >> 8) & 0xFF];

	// Extract RGB pixel components for previous frame (RGBA packed format)
	unsigned char frame1_r = srgb_to_linear[p1 & 0xFF];
	unsigned char frame1_g = srgb_to_linear[(p1 >> 8) & 0xFF];
	unsigned char frame1_b = srgb_to_linear[(p1 >> 16) & 0xFF];

	// Look up precomputed blended components in encoded (8-bit) space
	return 	linear_to_srgb[int(frame1_r * ratio + frame0_r * ratio_rev)] |
			linear_to_srgb[int(frame1_g * ratio + frame0_g * ratio_rev)] << 8 |
			linear_to_srgb[int(frame1_b * ratio + frame0_b * ratio_rev)] << 16;
}

// 3-Color blending in linear light using LUTs
static inline uint32_t blend_3c(uint32_t p0, uint32_t p1, uint32_t p2, float ratio) {
	const double ratio_rev = 1.0 - ratio;

	// Decode RGB components from RGBA encoded space to linear colorspace (sRGB -> linear)
	unsigned char frame0_r = srgb_to_linear[p0 & 0xFF];
	unsigned char frame0_b = srgb_to_linear[(p0 >> 16) & 0xFF];
	unsigned char frame0_g = srgb_to_linear[(p0 >> 8) & 0xFF];

	unsigned char frame1_r = srgb_to_linear[p1 & 0xFF];
	unsigned char frame1_g = srgb_to_linear[(p1 >> 8) & 0xFF];
	unsigned char frame1_b = srgb_to_linear[(p1 >> 16) & 0xFF];

	unsigned char frame2_r = srgb_to_linear[p2 & 0xFF];
	unsigned char frame2_g = srgb_to_linear[(p2 >> 8) & 0xFF];
	unsigned char frame2_b = srgb_to_linear[(p2 >> 16) & 0xFF];

	// Encode averaged linear components back to RGBA encoded space (linear -> sRGB)
	return 	linear_to_srgb[int((frame0_r + frame1_r + frame2_r) / 3 * ratio + frame0_r * ratio_rev)] |
			linear_to_srgb[int((frame0_g + frame1_g + frame2_g) / 3 * ratio + frame0_g * ratio_rev)] << 8 |
			linear_to_srgb[int((frame0_b + frame1_b + frame2_b) / 3 * ratio + frame0_b * ratio_rev)] << 16;
}

// Check if RGBA pixel has more than one color component
static inline bool rgb_has_multi_component(uint32_t c) {

    // Components
    uint32_t r = c & 0x000000FF; // Red
    uint32_t g = c & 0x0000FF00; // Green
    uint32_t b = c & 0x00FF0000; // Blue

    // More than one non-zero component?
    return ((r != 0) + (g != 0) + (b != 0)) > 1;
}

// blend src into dst with weight `mass` in linear space (sRGB-correct),
// then store original dst back to src for the next frame.
// sRGB colorspace in Gigascreen reference: https://hype.retroscene.org/blog/graphics/808.html
void scrMix(unsigned char* src, unsigned char* dst, int size, double ratio, float gamma, int mode) {
	const double ratio_x2 = ratio * 2.0;

	// Init ring buffer pointer if not set yet
	if (ring_base == NULL) { ring_base = (uint32_t *)src; }
	// Rebuild gamma LUTs if Gamma value has changed
	if (last_gamma != gamma) { rebuild_gamma_lut(gamma); }

	// screen is in GL_RGBA 32-bit format: Red,Green,Blue,Alpha
	size /= 4;
	// re-cast pointer to uint32_t* as we're going to process RGB-data at once
	uint32_t *p0 = reinterpret_cast<uint32_t*>(dst);
	uint32_t *p1 = ring_get_frame(0, size);
	uint32_t *p2 = ring_get_frame(1, size);
	uint32_t *p3 = ring_get_frame(2, size);
	uint32_t *p4 = ring_get_frame(3, size);
	uint32_t *p5 = ring_get_frame(4, size);
	ring_rotate();

	while (size > 0) {
		uint32_t current_color = *p0;
		uint32_t output_color = current_color;
		bool multi_components;

		switch (mode) {
		// 2C+3C (adaptive)
		case AF_3C_ADAPTIVE:
			// 3Color simple check
			multi_components =	rgb_has_multi_component(*p0) ||
								rgb_has_multi_component(*p1) ||
								rgb_has_multi_component(*p2);

			if (!multi_components && *p0 == *p3 && *p1 == *p4 && *p2 == *p5) {
				output_color = blend_3c(*p0, *p1, *p2, ratio_x2);
			} else {
			if (*p0 == *p2 && *p0 != *p1)
				output_color = blend_2c(*p0, *p1, ratio);
			}
			break;

		// 2C only (adaptive)
		case AF_2C_ADAPTIVE:
			if (*p0 == *p2 && *p0 != *p1)
				output_color = blend_2c(*p0, *p1, ratio);
			break;

		// 2C only (fullscreen)
		case AF_2C_FULL:
			output_color = blend_2c(*p0, *p1, ratio);
			break;

		// 2C+3C (fullscreen)
		case AF_3C_FULL:
			if (*p0 == *p3 && *p1 == *p4 && *p2 == *p5) {
				output_color = blend_3c(*p0, *p1, *p2, ratio_x2);
			} else {
				output_color = blend_2c(*p0, *p1, ratio);
			}
			break;

		default:
			break;
		}

		*p5++ = current_color;	// replace most expired buffer with fresh data
		*p0++ = output_color;	// update current screen buffer with calculated color
		p1++;
		p2++;
		p3++;
		p4++;
		size--;
	}
}
