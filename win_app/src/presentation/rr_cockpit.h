#ifndef RR_COCKPIT_H
#define RR_COCKPIT_H
#include "rr_ui.h"
#include "rr_cockpit_plan.h"
#define RR_COCKPIT_LAYER_COUNT 5
#define RR_COCKPIT_RGBA_LIMIT (9u*1024u*1024u)
typedef struct RRCockpitArt {Texture2D layers[RR_COCKPIT_LAYER_COUNT];int loaded;bool failed;} RRCockpitArt;
/* Candidate-only Roadster pack. One upload per menu/paused frame, never alongside city uploads. */
bool rr_cockpit_load_next(RRCockpitArt *art);
void rr_cockpit_unload(RRCockpitArt *art);
void rr_cockpit_draw(const RRCockpitArt *art,RRUI *ui,int width,int height,const RRCKReadout *values,double boost,bool combat_connected);
#endif
