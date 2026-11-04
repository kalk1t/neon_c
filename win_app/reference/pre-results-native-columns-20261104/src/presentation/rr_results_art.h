#ifndef RR_RESULTS_ART_H
#define RR_RESULTS_ART_H
#include "rr_results_text.h"
typedef struct RRResultsArt {Texture2D glyphs[RR_RESULTS_ART_COUNT];bool ready;} RRResultsArt;
/* Six15px masks, three source scales. TotalRGBA24072; plus old dot/star27060
 * remains51132<=65536. Loader/unloader must only run in an authorized graphics
 * context. No production full-pack load/activation occurs in this slice. */
bool rr_results_art_load(RRResultsArt *art);
void rr_results_art_unload(RRResultsArt *art);
/* Prepared immutable row and same loaded ui.bold arrays; preflight all required
 * glyph textures before emitting either name or value. Not a graphics test. */
bool rr_ui_results_row_draw(RRUI *ui,RRResultsArt *art,const RRResultsRow *row);
#endif
