#ifndef RR_MINIMAP_BACKGROUND_H
#define RR_MINIMAP_BACKGROUND_H
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
/* Source240px border-box, border2, default background-origin padding-box:
 * radial gradient reaches its farthest corner at118*sqrt(2), not radius120.
 * Native retains raylib's36-triangle fan and straight vertex interpolation.
 * Endpoint correction approximates interior premultiplied sRGB; not pixel exact. */
static inline uint32_t rr_minimap_background_vertex(bool outer)
{
    double t=outer?120/(118*sqrt(2.0)):0;
    double a=.8*(1-t)+.85*t;const double start[3]={10,20,70},end[3]={3,5,22};uint32_t packed=0;
    for(int i=0;i<3;i++){double channel=(start[i]*.8*(1-t)+end[i]*.85*t)/a;packed=(packed<<8)|(uint32_t)floor(channel+.5);}
    return (packed<<8)|(uint32_t)floor(a*255+.5);
}
#endif
