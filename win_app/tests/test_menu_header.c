#include "raylib.h"
#include "rr_menu_header.h"
#include <stdio.h>
#include <string.h>
typedef struct MenuSource {float w,h,brand,line,track_y,track_h;RRUIRect actions[5];} MenuSource;
typedef struct MenuWidthCase {int face;const char *text;float size,tracking;double expected;} MenuWidthCase;
#include "menu_header_fixture.h"
static int failures;static double worst,old_worst;
#define CHECK(x) do{if(!(x)){if(failures<6)fprintf(stderr,"menu header line%d\n",__LINE__);failures++;}}while(0)
static void close_value(double a,double b){CHECK(fabs(a-b)<.002);}
int main(int argc,char **argv)
{
    if(argc!=2)return 1;SetTraceLogLevel(LOG_WARNING);
    for(size_t i=0;i<sizeof(menu_sources)/sizeof(menu_sources[0]);i++){
        const MenuSource *s=&menu_sources[i];RRUIMenuLayout l=rr_ui_menu_layout(s->w,s->h);close_value(l.brand,s->brand);close_value(l.brand_line_h,s->line);close_value(l.track_y-l.y,s->track_y);close_value(l.track_h,s->track_h);
        for(int j=0;j<5;j++){RRUIRect r=l.actions[j],v=s->actions[j];close_value(r.x-l.x,v.x);close_value(r.y-l.y,v.y);close_value(r.w,v.w);close_value(r.h,v.h);}
    }
    const float views[][2]={{960,720},{1280,720},{1600,1000},{1920,1080},{1440,960},{1180,820},{1280,821},{960,1200}};
    for(size_t i=0;i<sizeof(views)/sizeof(views[0]);i++){
        float w=views[i][0],h=views[i][1];RRUIMenuLayout l=rr_ui_menu_layout(w,h);CHECK(l.y>=16&&l.x>=0&&l.x+l.width<=w);CHECK(l.footer_y+146<=h-16+.002);
        close_value(l.y,fmax(16,(h-l.content_h)*.5));CHECK(l.track_y>l.y+2*l.brand_line_h&&l.actions[0].y>l.track_y+l.track_h);
        for(int j=1;j<5;j++)close_value(l.actions[j].y-l.actions[j-1].y-l.actions[j-1].h,l.gap);
        CHECK(rr_menu_brand_baseline(l,0)<rr_menu_brand_baseline(l,1)&&rr_menu_track_baseline(l)>l.track_y&&rr_menu_best_baseline(l)>l.best_y);
    }
    for(int face=0;face<2;face++){
        char path[1024];snprintf(path,sizeof(path),"%s/%s",argv[1],face==0?"Orbitron-900.ttf":"Orbitron-700.ttf");int bytes=0,count=0,cps[95];for(int i=0;i<95;i++)cps[i]=32+i;unsigned char *data=LoadFileData(path,&bytes);if(!data)return 1;GlyphInfo *g=LoadFontData(data,bytes,64,cps,95,FONT_DEFAULT,&count);if(!g){UnloadFileData(data);return 1;}Rectangle rects[95]={0};for(int i=0;i<count&&i<95;i++)rects[i]=(Rectangle){0,0,(float)g[i].image.width,(float)g[i].image.height};Font f={0};f.baseSize=64;f.glyphCount=count;f.glyphs=g;f.recs=rects;f.texture.id=1;
        for(size_t i=0;i<sizeof(menu_widths)/sizeof(menu_widths[0]);i++){
            const MenuWidthCase *c=&menu_widths[i];if(c->face!=face)continue;float pixels=(float)rr_css_font_size(c->size,face==0?900:700);double got=MeasureTextEx(f,c->text,pixels,c->tracking).x,e=fabs(got-c->expected),old=fabs(MeasureTextEx(f,c->text,c->size,c->tracking).x-c->expected);if(e>worst)worst=e;if(old>old_worst)old_worst=old;CHECK(e<strlen(c->text)*(double)pixels/64+.05&&e<old);if(c->size==13)CHECK(c->expected<470);
        }
        UnloadFontData(g,count);UnloadFileData(data);
    }
    printf("Menu header:2 reused relative browser layouts,8 native footer-containment views,15 independent TTF widths; worst width %.9g, prior raster-unit model %.9g, %d failures; no GPU\n",worst,old_worst,failures);return failures?1:0;
}
