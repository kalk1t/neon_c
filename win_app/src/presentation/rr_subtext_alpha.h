#ifndef RR_SUBTEXT_ALPHA_H
#define RR_SUBTEXT_ALPHA_H
#include <math.h>
/* Tint alpha for the existing four-offset shadow and shared ASCII/image core.
 * Per-pass approximation; CSS isolated group compositing remains unqualified. */
static inline unsigned char rr_subtext_alpha(double opacity,int shadow)
{
    if(!isfinite(opacity))opacity=0;
    opacity=fmax(0,fmin(1,opacity));return (unsigned char)floor((shadow?31:255)*opacity+.5);
}
#endif
