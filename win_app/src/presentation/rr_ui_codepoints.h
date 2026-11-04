#ifndef RR_UI_CODEPOINTS_H
#define RR_UI_CODEPOINTS_H
/* Existing ASCII atlas, plus source multiply and results em dash in bold.
 * Both additions are present in the supplied unmodified Orbitron font. */
enum {RR_UI_MAX_CODEPOINTS=97};
static inline int rr_ui_codepoints(int cps[RR_UI_MAX_CODEPOINTS],int weight)
{for(int i=0;i<95;i++)cps[i]=32+i;cps[95]=215;cps[96]=8212;return weight==700?97:95;}
#endif
