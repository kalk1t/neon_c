#include "raylib.h"
#include "rr_button_text.h"
#include <stdio.h>
#include <string.h>
typedef struct ButtonStyleCase {double size,tracking,inset;int hover,enabled,r,g,b,a;} ButtonStyleCase;
typedef struct ButtonWidthCase {const char *text;float size;double expected;} ButtonWidthCase;
#include "button_text_fixture.h"
static int failures;static double worst_width,old_width;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"button text line%d\n",__LINE__);failures++;}}while(0)
int main(int argc,char **argv)
{
    if(argc!=2)return 1;SetTraceLogLevel(LOG_WARNING);
    for(size_t i=0;i<sizeof(style_cases)/sizeof(style_cases[0]);i++){
        const ButtonStyleCase *c=&style_cases[i];RRButtonText t=rr_button_text((RRUIRect){100,200,316,61},c->size,24,c->hover,c->enabled!=0);
        CHECK(t.size==c->size&&t.tracking==c->tracking&&fabs(t.x-100-c->inset)<1e-10);
        CHECK((int)(t.rgba>>24)==c->r&&(int)((t.rgba>>16)&255)==c->g&&(int)((t.rgba>>8)&255)==c->b&&(int)(t.rgba&255)==c->a);
    }
    CHECK(rr_menu_button_css_size(720)==18&&rr_menu_button_css_size(820)==18&&rr_menu_button_css_size(820.0001)==20&&rr_menu_button_css_size(1080)==20);
    int bytes=0,count=0;unsigned char *data=LoadFileData(argv[1],&bytes);if(!data)return 1;int cps[95];for(int i=0;i<95;i++)cps[i]=32+i;
    GlyphInfo *glyphs=LoadFontData(data,bytes,64,cps,95,FONT_DEFAULT,&count);if(!glyphs){UnloadFileData(data);return 1;}
    Rectangle rects[95]={0};for(int i=0;i<count&&i<95;i++)rects[i]=(Rectangle){0,0,(float)glyphs[i].image.width,(float)glyphs[i].image.height};
    /* CPU font only: dummy ID for MeasureTextEx guard, never uploaded/drawn. */
    Font font={0};font.baseSize=64;font.glyphCount=count;font.glyphs=glyphs;font.recs=rects;font.texture.id=1;
    for(size_t i=0;i<sizeof(width_cases)/sizeof(width_cases[0]);i++){
        const ButtonWidthCase *c=&width_cases[i];float pixels=(float)rr_css_font_size(c->size,700);double got=MeasureTextEx(font,c->text,pixels,3).x,error=fabs(got-c->expected),old=fabs(MeasureTextEx(font,c->text,c->size,3).x-c->expected);
        if(error>worst_width)worst_width=error;if(old>old_width)old_width=old;
        double bound=strlen(c->text)*(double)pixels/64+.05;CHECK(error<=bound&&error<old);
        double width=strcmp(c->text,"QUIT")==0?144:strcmp(c->text,"RESUME")==0||strcmp(c->text,"RESTART")==0||strcmp(c->text,"MAIN MENU")==0?316:470;
        CHECK(c->expected+3+34+24<width); /* includes trailing CSS tracking */
        const float heights[]={47,50,52,60,61};
        for(size_t j=0;j<sizeof(heights)/sizeof(heights[0]);j++){
            RRButtonText t=rr_button_text((RRUIRect){10,20,470,heights[j]},c->size,24,.5,true);double origin=rr_css_raylib_origin(t.baseline,t.size,700,64),ascent=floor(64*1.011/1.254);
            CHECK(t.x==39&&fabs(origin+ascent*pixels/64-t.baseline)<1e-5);CHECK(t.baseline>20&&t.baseline<20+heights[j]);
        }
    }
    UnloadFontData(glyphs,count);UnloadFileData(data);
    printf("Button typography:6 reused Chrome styles,22 independent TTF widths,110 nominal baseline/fit checks; worst width %.9g, prior raster-unit model %.9g, %d failures; no GPU\n",worst_width,old_width,failures);return failures?1:0;
}
