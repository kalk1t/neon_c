/* Metadata-only handoff; no native integration or visual acceptance claimed. */
#ifndef RR_RESULTS_FALLBACK_GLYPHS_H
#define RR_RESULTS_FALLBACK_GLYPHS_H
#include <stdint.h>
#include <stddef.h>
typedef struct RRResultsFallbackGlyph { uint32_t codepoint; int font_size_css,device_scale; const char*png; int width_pixels,height_pixels; float offset_x,offset_y,draw_width,draw_height,advance,source_tracking; } RRResultsFallbackGlyph;
static const RRResultsFallbackGlyph rr_results_fallback_glyphs[]={
    { 8776u, 15, 1, "native-glyphs/approx-results-15@1x.png", 14, 10, -2.000000f, -10.000000f, 14.000000f, 10.000000f, 9.906250f, 0.000000f },
    { 8776u, 15, 2, "native-glyphs/approx-results-15@2x.png", 26, 22, -1.500000f, -11.000000f, 13.000000f, 11.000000f, 9.906250f, 0.000000f },
    { 8776u, 15, 3, "native-glyphs/approx-results-15@3x.png", 39, 33, -1.666667f, -11.000000f, 13.000000f, 11.000000f, 9.906250f, 0.000000f },
    { 9819u, 15, 1, "native-glyphs/queen-results-15@1x.png", 19, 17, -2.000000f, -14.000000f, 19.000000f, 17.000000f, 15.000000f, 0.000000f },
    { 9819u, 15, 2, "native-glyphs/queen-results-15@2x.png", 36, 32, -1.500000f, -13.500000f, 18.000000f, 16.000000f, 15.000000f, 0.000000f },
    { 9819u, 15, 3, "native-glyphs/queen-results-15@3x.png", 53, 48, -1.333333f, -13.666667f, 17.666667f, 16.000000f, 15.000000f, 0.000000f },
    { 9819u, 20, 1, "native-glyphs/queen-menu-20@1x.png", 24, 21, -2.000000f, -18.000000f, 24.000000f, 21.000000f, 20.000000f, 3.000000f },
    { 9819u, 20, 2, "native-glyphs/queen-menu-20@2x.png", 44, 40, -1.000000f, -17.500000f, 22.000000f, 20.000000f, 20.000000f, 3.000000f },
    { 9819u, 20, 3, "native-glyphs/queen-menu-20@3x.png", 64, 59, -0.666667f, -17.333333f, 21.333333f, 19.666667f, 20.000000f, 3.000000f },
    { 9819u, 18, 1, "native-glyphs/queen-menu-18@1x.png", 22, 19, -2.000000f, -16.000000f, 22.000000f, 19.000000f, 18.000000f, 3.000000f },
    { 9819u, 18, 2, "native-glyphs/queen-menu-18@2x.png", 40, 37, -1.000000f, -16.000000f, 20.000000f, 18.500000f, 18.000000f, 3.000000f },
    { 9819u, 18, 3, "native-glyphs/queen-menu-18@3x.png", 60, 55, -1.000000f, -16.000000f, 20.000000f, 18.333333f, 18.000000f, 3.000000f }
};
static inline const RRResultsFallbackGlyph*rr_find_results_fallback_glyph(uint32_t codepoint,int font_size_css,int device_scale){
 size_t i;for(i=0;i<sizeof(rr_results_fallback_glyphs)/sizeof(rr_results_fallback_glyphs[0]);++i){const RRResultsFallbackGlyph*g=&rr_results_fallback_glyphs[i];if(g->codepoint==codepoint&&g->font_size_css==font_size_css&&g->device_scale==device_scale)return g;}return NULL;
}
#endif
