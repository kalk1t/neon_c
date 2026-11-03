#ifndef RR_SUBTEXT_DATA_H
#define RR_SUBTEXT_DATA_H
#include <stdint.h>
typedef struct RRSubGlyph {uint32_t codepoint;int width,height;const char *file;float x,y,w,h,advance;} RRSubGlyph;
/* Exact browser handoff crop metrics at26CSSpx,3x white core. No whole fonts. */
static const RRSubGlyph rr_sub_glyphs[]={
{183u,30,28,"native-glyphs/dot-white@3x.png",-0.666666667f,-14.000000000f,10.000000000f,9.333333333f,8.671875000f},
{9733u,79,75,"native-glyphs/star-white@3x.png",-2.333333333f,-21.666666667f,26.333333333f,25.000000000f,21.671875000f},
};
static inline int rr_sub_glyph_index(uint32_t cp){return cp==183?0:cp==9733?1:-1;}
/* Orbitron700 hhea1011/-243, units1000; raylib loads64px pixel-height.
 * Only the new submessage channel maps that raster to CSS-em sizing. */
#define RR_SUB_FONT_HEIGHT_TO_EM 1.254f
#define RR_SUB_FONT_RASTER_BASELINE 51.f
#endif
