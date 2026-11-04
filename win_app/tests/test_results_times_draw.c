#include "rr_results_times_draw.h"
#include "rr_ui_codepoints.h"
#include "rr_css_font_metrics.h"
#include "results_times_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,calls,total_calls;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);failures++;}}while(0)
typedef struct Paint {char text[64];float x,y,size,tracking;int weight;Color color;bool center,glow;} Paint;
static Paint paints[3];
static void record(const char *s,float x,float y,float size,float tracking,int weight,Color color,bool glow,bool center){CHECK(calls<3);if(calls>=3)return;Paint *p=paints+calls++;snprintf(p->text,sizeof(p->text),"%s",s);p->x=x;p->y=y;p->size=size;p->tracking=tracking;p->weight=weight;p->color=color;p->glow=glow;p->center=center;total_calls++;}
/* Only the actual native text-paint endpoints are recording stubs. Native
 * component, actual font decoding/atlas and MeasureTextEx all execute on CPU. */
void rr_ui_text_css(RRUI *ui,const char *s,float x,float y,float size,float tracking,int weight,Color c,bool glow){(void)ui;record(s,x,y,size,tracking,weight,c,glow,false);}
void rr_ui_text_center_css_glow(RRUI *ui,const char *s,float x,float y,float size,float tracking,int weight,Color c,bool glow){(void)ui;record(s,x,y,size,tracking,weight,c,glow,true);}
static bool near(double a,double b){return fabs(a-b)<.001;}
int main(int argc,char **argv){
 CHECK(argc==2);if(argc!=2)return 1;SetTraceLogLevel(LOG_WARNING);
 int bytes=0,count=0,cps[RR_UI_MAX_CODEPOINTS];unsigned char *data=LoadFileData(argv[1],&bytes);CHECK(data&&bytes>1000);if(!data)return 1;
 int requested=rr_ui_codepoints(cps,700);GlyphInfo *glyphs=LoadFontData(data,bytes,64,cps,requested,FONT_DEFAULT,&count);CHECK(glyphs&&count==96);if(!glyphs){UnloadFileData(data);return 1;}
 Rectangle *rects=NULL;Image atlas=GenImageFontAtlas(glyphs,&rects,count,64,4,0);CHECK(atlas.data&&rects&&atlas.width<=2048&&atlas.height<=2048);if(!rects){UnloadFontData(glyphs,count);UnloadFileData(data);return 1;}
 /* MeasureTextEx checks this metadata field before reading CPU advances.
  * Nonzero marker only: no texture is allocated/bound/uploaded or destroyed,
  * and paint endpoints below remain recording stubs. */
 Font font={0};font.texture.id=1;font.baseSize=64;font.glyphCount=count;font.glyphs=glyphs;font.recs=rects;
 for(int i=0;i<requested;i++){int found=GetGlyphIndex(font,cps[i]);CHECK(found>=0&&found<count);if(found>=0&&found<count){if(cps[i]==94)CHECK(glyphs[found].value!=94);else CHECK(glyphs[found].value==cps[i]);}}
 int dash=GetGlyphIndex(font,8212);CHECK(dash>=0&&glyphs[dash].value==8212&&glyphs[dash].advanceX>0);
 RRUI ui={0};ui.loaded=true;ui.bold=font;
 for(size_t i=0;i<sizeof(times_cases)/sizeof(times_cases[0]);i++){
  const struct TimesCase *c=times_cases+i;RRResultsTimes text;CHECK(rr_results_times_build(c->laps,c->count,&text));calls=0;
  CHECK(rr_ui_results_times_draw(&ui,&text,800,100));CHECK(calls==1+c->count);
  CHECK(!strcmp(paints[0].text,c->total)&&paints[0].center&&paints[0].glow&&paints[0].size==32&&paints[0].weight==900&&paints[0].tracking==0&&paints[0].color.r==255&&paints[0].color.g==255&&paints[0].color.b==255&&paints[0].color.a==255);
  CHECK(near(paints[0].x,800)&&near(paints[0].y,100+107.296875+(40-32*1.254)/2+32*1.011));
  float width[2]={0};double group=0;for(size_t j=0;j<c->count;j++){width[j]=MeasureTextEx(font,c->labels[j],(float)rr_css_font_size(16,700),0).x;CHECK(width[j]>0);group+=width[j];}if(c->count==2)group+=28;double left=800-group/2;
  for(size_t j=0;j<c->count;j++){
   const Paint *v=paints+j+1;CHECK(!strcmp(v->text,c->labels[j])&&!v->center&&!v->glow&&v->size==16&&v->weight==700&&v->tracking==0&&v->color.a==255);CHECK(near(v->x,left)&&near(v->y,100+156.296875+(20-16*1.254)/2+16*1.011));left+=width[j]+28;
   if((int)j==c->best)CHECK(v->color.r==255&&v->color.g==43&&v->color.b==214);else CHECK(v->color.r==207&&v->color.g==239&&v->color.b==255);
  }
 }
 RRResultsTimes text;double laps[2]={30,40};CHECK(rr_results_times_build(laps,2,&text));calls=0;ui.loaded=false;CHECK(!rr_ui_results_times_draw(&ui,&text,800,100)&&!calls);ui.loaded=true;
 ui.bold.glyphCount=dash;CHECK(!rr_ui_results_times_draw(&ui,&text,800,100)&&!calls);ui.bold.glyphCount=count;
 CHECK(!rr_ui_results_times_draw(&ui,&text,NAN,100)&&!calls);CHECK(!rr_ui_results_times_draw(NULL,&text,800,100)&&!calls);CHECK(!rr_ui_results_times_draw(&ui,NULL,800,100)&&!calls);
 printf("RESULTS_DRAW_SUMMARY cases=%zu paint_commands=%u glyphs=%d dash_advance=%d atlas=%dx%d rgba_bytes=%zu failures=%u paint_recorded_not_rendered=true\n",sizeof(times_cases)/sizeof(times_cases[0]),total_calls,count,glyphs[dash].advanceX,atlas.width,atlas.height,(size_t)atlas.width*atlas.height*4,failures);
 UnloadImage(atlas);MemFree(rects);UnloadFontData(glyphs,count);UnloadFileData(data);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
