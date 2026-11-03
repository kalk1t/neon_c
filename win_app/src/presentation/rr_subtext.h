#ifndef RR_SUBTEXT_H
#define RR_SUBTEXT_H
#include "rr_ui.h"
#include "rr_subtext_data.h"
typedef struct RRSubArt {Texture2D glyphs[2];bool ready;} RRSubArt;
/* Baseline is explicit: metrics must never be mistaken for crop-centre offsets. */
bool rr_sub_art_load(RRSubArt *art);
void rr_sub_art_unload(RRSubArt *art);
uint32_t rr_sub_next(const char *text,int *bytes);
Rectangle rr_sub_glyph_quad(int index,float pen,float baseline,float scale);
float rr_subtext_measure(Font font,const char *text,float scale);
void rr_ui_subtext(RRSubArt *art,RRUI *ui,const char *text,float cx,float baseline,float scale);
#endif
