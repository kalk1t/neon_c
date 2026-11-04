#include "rr_spin_readout.h"
#include "rr_race_ui.h"
#include "rr_subtext.h"
#include <stdio.h>
#include <string.h>
typedef struct SpinBoxCase {double w,h,zoom,right,top,height,cx,cy,key_top,key_h;} SpinBoxCase;
typedef struct WidthCase {const char *text;double size,spacing,ideal;int ascii_count,codepoints;} WidthCase;
#include "spin_readout_fixture.h"
static int failures;static double worst_width;
#define CHECK(v) do{if(!(v)){if(failures<5)fprintf(stderr,"line%d: %s\n",__LINE__,#v);failures++;}}while(0)
int main(int argc,char **argv){if(argc!=2)return 1;CHECK(strcmp(RR_SPIN_KEY_TEXT,source_key)==0);
for(size_t i=0;i<sizeof(box_cases)/sizeof(box_cases[0]);i++){
 const SpinBoxCase *c=&box_cases[i];RRUIHudLayout l=rr_ui_hud_layout((float)c->w,(float)c->h);RRSpinReadout r=rr_spin_readout(l.scale);CHECK(fabs(l.spin.y-c->top)<.002&&fabs(l.spin.h-c->height)<.002&&fabs(l.spin.x+l.spin.w-c->right)<.002);CHECK(fabs(l.spin.x+l.spin.w-r.right_inset-c->cx)<.002);CHECK(fabs(l.spin.y+r.center_y-c->cy)<.002);CHECK(fabs(l.spin.y+r.key_top-c->key_top)<.002);
 double baseline=rr_css_line_baseline(r.key_top,c->key_h,r.key_font,700);CHECK(fabs(r.key_baseline-baseline)<.00001);CHECK(fabs(r.key_font-9*c->zoom)<1e-6&&fabs(r.key_spacing-1.5*c->zoom)<1e-6);
}
SetTraceLogLevel(LOG_WARNING);int bytes=0,count=0,cps[96];for(int i=0;i<95;i++)cps[i]=32+i;cps[95]=215;unsigned char *data=LoadFileData(argv[1],&bytes);if(!data)return 1;GlyphInfo *glyphs=LoadFontData(data,bytes,64,cps,96,FONT_DEFAULT,&count);if(!glyphs){UnloadFileData(data);return 1;}Rectangle recs[96]={0};for(int i=0;i<count&&i<96;i++)recs[i]=(Rectangle){0,0,(float)glyphs[i].image.width,(float)glyphs[i].image.height};Font font={0};font.baseSize=64;font.glyphCount=count;font.glyphs=glyphs;font.recs=recs;font.texture.id=1;
for(size_t i=0;i<sizeof(width_cases)/sizeof(width_cases[0]);i++){
 const WidthCase *c=&width_cases[i];float got=rr_subtext_measure_spaced(font,c->text,(float)c->size,(float)c->spacing);double error=fabs(got-c->ideal),bound=c->ascii_count*c->size*1.254/64+.03;if(error>worst_width)worst_width=error;
 if(error>bound){if(failures<5)fprintf(stderr,"Width row%zu got%.12g expected%.12g bound%.12g\n",i,(double)got,c->ideal,bound);failures++;}
 float no_space=rr_subtext_measure_spaced(font,c->text,(float)c->size,0);CHECK(fabs((got-no_space)-c->codepoints*c->spacing)<.00003);
 if(c->size==26&&c->spacing==0)CHECK(fabs(got-rr_subtext_measure(font,c->text,1))<1e-6);
}
UnloadFontData(glyphs,count);UnloadFileData(data);
printf("SPIN readout: two reused Chrome group boxes, %zu independent font/fallback width cases; CSS em sizing and codepoint tracking; worst quantized width error%.12g, %d failures; no window/GPU\n",sizeof(width_cases)/sizeof(width_cases[0]),worst_width,failures);return failures?1:0;}
