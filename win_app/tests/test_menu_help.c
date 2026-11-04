#include "raylib.h"
#include "rr_menu_footer.h"
#include "rr_ui_layout.h"
#include <stdio.h>
#include <string.h>
typedef struct KeySource {const char *text;float w,h;} KeySource;
typedef struct HelpWidthCase {int face;const char *text;float size;double expected;} HelpWidthCase;
#include "menu_help_fixture.h"
static int failures;static double worst_width,worst_row;
#define CHECK(x) do{if(!(x)){if(failures<8)fprintf(stderr,"menu help line%d\n",__LINE__);failures++;}}while(0)
static double ideal_text(const char *s){for(size_t i=0;i<sizeof(help_widths)/sizeof(help_widths[0]);i++)if(help_widths[i].face==0&&!strcmp(help_widths[i].text,s))return help_widths[i].expected;CHECK(0);return 0;}
int main(int argc,char **argv)
{
    if(argc!=2)return 1;SetTraceLogLevel(LOG_WARNING);
    for(int i=0;i<11;i++){const RRHelpKey *k=rr_menu_help_key(i);const KeySource *s=&key_sources[i];RRHelpBox b=rr_menu_help_key_box(i,100,200,1000);CHECK(!strcmp(k->text,s->text)&&fabs(b.w-s->w)<.001&&fabs(b.h-s->h)<.001&&b.x==102);CHECK(fabs((b.y+3+rr_css_line_baseline(0,15,12,700))-200)<.0001);}
    CHECK(!rr_menu_help_key(-1)&&!rr_menu_help_key(11)&&!rr_menu_help_run(3,0)&&!rr_menu_help_run(0,4));
    Font fonts[2]={{0}};Rectangle recs[2][95]={0};unsigned char *data[2]={0};
    for(int face=0;face<2;face++){
        char path[1024];snprintf(path,sizeof(path),"%s/%s",argv[1],face==0?"Rajdhani-Medium.ttf":"Orbitron-700.ttf");int bytes=0,count=0,cps[95];for(size_t i=0;i<sizeof(cps)/sizeof(cps[0]);i++)cps[i]=32+(int)i;data[face]=LoadFileData(path,&bytes);if(!data[face])return 1;int base=face==0?32:64;GlyphInfo *g=LoadFontData(data[face],bytes,base,cps,95,FONT_DEFAULT,&count);if(!g)return 1;
        fonts[face]=(Font){0};fonts[face].baseSize=base;fonts[face].glyphCount=count;fonts[face].glyphs=g;fonts[face].texture.id=1;fonts[face].recs=recs[face];for(int i=0;i<count&&i<95;i++)recs[face][i]=(Rectangle){0,0,(float)g[i].image.width,(float)g[i].image.height};
        for(size_t i=0;i<sizeof(help_widths)/sizeof(help_widths[0]);i++){
            const HelpWidthCase *c=&help_widths[i];if(c->face!=face)continue;float pixels=(float)rr_css_font_size(c->size,face==0?0:700);double got=MeasureTextEx(fonts[face],c->text,pixels,0).x,error=fabs(got-c->expected);if(error>worst_width)worst_width=error;CHECK(error<strlen(c->text)*(double)pixels/base+.05);
        }
    }
    for(int row=0;row<3;row++){
        double native=0,ideal=0,bound=0;
        for(int j=0;j<rr_menu_help_count(row);j++){
            const RRHelpRun *r=rr_menu_help_run(row,j);if(r->key>=0){native+=rr_menu_help_advance(r,0);ideal+=rr_menu_help_advance(r,0);}else{double w=MeasureTextEx(fonts[0],r->text,(float)rr_css_font_size(15,0),0).x;native+=rr_menu_help_advance(r,w);ideal+=ideal_text(r->text);bound+=strlen(r->text)*rr_css_font_size(15,0)/32+.05;}
        }
        double e=fabs(native-ideal);if(e>worst_row)worst_row=e;CHECK(fabs(ideal-ideal_row_widths[row])<.001&&native<470&&e<bound);
    }
    const float views[][2]={{960,720},{1280,720},{1600,1000},{1920,1080},{1440,960},{1180,820},{1280,821},{960,1200}};
    for(size_t i=0;i<sizeof(views)/sizeof(views[0]);i++){
        float w=views[i][0],h=views[i][1];RRUIMenuLayout l=rr_ui_menu_layout(w,h);CHECK(l.y>=16&&l.footer_y+rr_menu_footer_height(h)<=h);
        for(int row=0;row<3;row++){double top=l.footer_y+61+row*rr_menu_help_line_height(h),base=rr_menu_help_baseline(top,rr_menu_help_line_height(h));RRHelpBox b=rr_menu_help_key_box(0,l.x,(float)base,h);CHECK(b.y>=top&&b.y+b.h<=top+rr_menu_help_line_height(h));CHECK(b.h==(h<=820?17:21));}
    }
    for(int face=0;face<2;face++){UnloadFontData(fonts[face].glyphs,fonts[face].glyphCount);UnloadFileData(data[face]);}
    printf("Menu help:11 reused browser keycaps,21 independent TTF widths,3 supported-control rows,8 footer views; worst width %.9g, row %.9g, %d failures; no GPU\n",worst_width,worst_row,failures);return failures?1:0;
}
