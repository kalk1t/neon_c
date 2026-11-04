#include "raylib.h"
#include "rr_race_ui.h"
#include "rr_css_font_metrics.h"
#include <stdio.h>
#include <string.h>
typedef struct BoxCase {float w,h;RRUIRect timer,speed,children[6];} BoxCase;
typedef struct FaceCase {const char *file;int weight,base,units,ascent,descent;} FaceCase;
typedef struct WidthCase {int face;float size,tracking;const char *text;double expected;int chrome;} WidthCase;
#include "hud_readouts_fixture.h"
static int failures;static double worst_width,old_width_error;
static void rect(RRUIRect a,RRUIRect b,double tolerance){if(fabs(a.x-b.x)>tolerance||fabs(a.y-b.y)>tolerance||fabs(a.w-b.w)>tolerance||fabs(a.h-b.h)>tolerance)failures++;}
int main(int argc,char **argv){if(argc!=2)return 1;SetTraceLogLevel(LOG_WARNING);
for(size_t i=0;i<sizeof(box_cases)/sizeof(box_cases[0]);i++){
 const BoxCase *c=&box_cases[i];RRUIHudLayout l=rr_ui_hud_layout(c->w,c->h);rect(l.timer,c->timer,.02);rect(l.speed,c->speed,.02);RRUIHudBoxes b=rr_ui_hud_boxes(l);RRUIRect got[]={b.lap_row,b.time,b.checkpoint_row,b.speed_label,b.speed_value,b.unit};
 for(int j=0;j<6;j++){RRUIRect parent=j<3?l.timer:l.speed,expected=c->children[j];expected=(RRUIRect){parent.x+expected.x,parent.y+expected.y,expected.w,expected.h};rect(got[j],expected,.001);}
}
int cps[95];for(int i=0;i<95;i++)cps[i]=32+i;
for(size_t f=0;f<sizeof(face_cases)/sizeof(face_cases[0]);f++){
 const FaceCase *c=&face_cases[f];char path[1024];snprintf(path,sizeof(path),"%s/%s",argv[1],c->file);int bytes=0,count=0;unsigned char *data=LoadFileData(path,&bytes);if(!data)return 1;GlyphInfo *glyphs=LoadFontData(data,bytes,c->base,cps,95,FONT_DEFAULT,&count);if(!glyphs)return 1;
 Rectangle recs[95]={0};for(int i=0;i<count&&i<95;i++)recs[i]=(Rectangle){0,0,(float)glyphs[i].image.width,(float)glyphs[i].image.height};
 /* Dummy texture ID only satisfies the pure MeasureTextEx guard. No window,
  * atlas texture, upload, font drawing or GPU initialization is performed. */
 Font font={0};font.baseSize=c->base;font.glyphCount=count;font.glyphs=glyphs;font.recs=recs;font.texture.id=1;
 double factor=(double)(c->ascent-c->descent)/c->units;if(fabs(rr_css_font_ratio(c->weight)-factor)>1e-12)failures++;
 for(size_t i=0;i<sizeof(width_cases)/sizeof(width_cases[0]);i++){
   const WidthCase *v=&width_cases[i];if(v->face!=(int)f)continue;float size=(float)rr_css_font_size(v->size,c->weight);double got=MeasureTextEx(font,v->text,size,v->tracking).x,old=MeasureTextEx(font,v->text,v->size,v->tracking).x;
   double error=fabs(got-v->expected),bound=strlen(v->text)*(double)size/c->base+.05;if(error>worst_width)worst_width=error;if(v->chrome&&fabs(old-v->expected)>old_width_error)old_width_error=fabs(old-v->expected);
   if(!isfinite(got)||error>bound||error>=fabs(old-v->expected)){if(failures<5)fprintf(stderr,"Face%zu width%zu %.9g expected%.9g bound%.9g old%.9g\n",f,i,got,v->expected,bound,old);failures++;}
   double baseline=rr_css_line_baseline(60,50,v->size,c->weight),origin=rr_css_raylib_origin(baseline,v->size,c->weight,c->base),ascent=floor((double)c->base*c->ascent/(c->ascent-c->descent));
   if(fabs(origin+ascent*rr_css_font_size(v->size,c->weight)/c->base-baseline)>1e-10)failures++;
 }
 UnloadFontData(glyphs,count);UnloadFileData(data);
}
printf("HUD readouts: %zu reused Chrome panel states, four Canvas widths and %zu raw-font widths; worst corrected width%.9g px, old Chrome error%.9g px; nominal baselines, %d failures; no window/GPU\n",sizeof(box_cases)/sizeof(box_cases[0]),sizeof(width_cases)/sizeof(width_cases[0])-4,worst_width,old_width_error,failures);return failures?1:0;}
