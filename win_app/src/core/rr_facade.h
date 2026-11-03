#ifndef RR_FACADE_H
#define RR_FACADE_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#define RR_WINDOW_WIDTH 256
#define RR_WINDOW_HEIGHT 512
#define RR_WINDOW_BYTES (RR_WINDOW_WIDTH*RR_WINDOW_HEIGHT*4)
/* Canvas top-row-first RGBA8, exact browser materials.js windowTexture seed99. */
bool rr_window_pixels(uint8_t *pixels,size_t bytes);
#endif
