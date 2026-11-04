#include "rr_subtext.h"
#include "rr_subtext_alpha.h"
#include <stdio.h>
uint32_t rr_sub_next(const char *s,int *bytes)
{
    const unsigned char *p=(const unsigned char *)s;*bytes=0;if(!p[0])return 0;
    *bytes=1;if(p[0]<128)return p[0];
    int n=p[0]>=0xc2&&p[0]<=0xdf?2:p[0]>=0xe0&&p[0]<=0xef?3:p[0]>=0xf0&&p[0]<=0xf4?4:0;
    if(!n)return 0xfffd;
    for(int i=1;i<n;i++)if(!p[i]||(p[i]&0xc0)!=0x80)return 0xfffd;
    if((p[0]==0xe0&&p[1]<0xa0)||(p[0]==0xed&&p[1]>=0xa0)||(p[0]==0xf0&&p[1]<0x90)||(p[0]==0xf4&&p[1]>=0x90))return 0xfffd;
    uint32_t cp=p[0]&((1u<<(7-n))-1u);for(int i=1;i<n;i++)cp=(cp<<6)|(p[i]&63);*bytes=n;return cp;
}
Rectangle rr_sub_glyph_quad(int index,float pen,float baseline,float scale)
{
    const RRSubGlyph *g=&rr_sub_glyphs[index];return (Rectangle){pen+g->x*scale,baseline+g->y*scale,g->w*scale,g->h*scale};
}
static float ascii_advance(Font f,int cp,float scale)
{
    int i=GetGlyphIndex(f,cp);return (f.glyphs[i].advanceX?(float)f.glyphs[i].advanceX:f.recs[i].width)*(26*scale*RR_SUB_FONT_HEIGHT_TO_EM/(float)f.baseSize);
}
float rr_subtext_measure(Font font,const char *text,float scale)
{
    float width=0;for(int n=0;*text;text+=n){uint32_t cp=rr_sub_next(text,&n);int k=rr_sub_glyph_index(cp);width+=k>=0?rr_sub_glyphs[k].advance*scale:ascii_advance(font,(int)cp,scale);}return width;
}
void rr_sub_art_unload(RRSubArt *art)
{for(int i=0;i<2;i++){if(art->glyphs[i].id)UnloadTexture(art->glyphs[i]);art->glyphs[i]=(Texture2D){0};}art->ready=false;}
bool rr_sub_art_load(RRSubArt *art)
{
    for(int i=0;i<2;i++){
        char path[256];snprintf(path,sizeof(path),"assets/ui-fallback/%s",rr_sub_glyphs[i].file);art->glyphs[i]=LoadTexture(path);
        if(!art->glyphs[i].id||art->glyphs[i].width!=rr_sub_glyphs[i].width||art->glyphs[i].height!=rr_sub_glyphs[i].height){rr_sub_art_unload(art);TraceLog(LOG_ERROR,"UI_FALLBACK: glyph unavailable; native Unicode notice appearance unqualified");return false;}
        SetTextureFilter(art->glyphs[i],TEXTURE_FILTER_BILINEAR);
    }
    art->ready=true;TraceLog(LOG_INFO,"UI_FALLBACK: two browser white-core crops ready; RGBA8 bound27060 bytes");return true;
}
void rr_ui_subtext(RRSubArt *art,RRUI *ui,const char *text,float cx,float baseline,float scale)
{rr_ui_subtext_opacity(art,ui,text,cx,baseline,scale,1);}
void rr_ui_subtext_opacity(RRSubArt *art,RRUI *ui,const char *text,float cx,float baseline,float scale,double opacity)
{
    Font font=ui->loaded?ui->bold:GetFontDefault();float size=26*scale*RR_SUB_FONT_HEIGHT_TO_EM;
    float x=cx-rr_subtext_measure(font,text,scale)*.5f;const char *start=text;
    for(int pass=0;pass<5;pass++){
        text=start;float dx=pass==0?-2*scale:pass==1?2*scale:0,dy=pass==2?-2*scale:pass==3?2*scale:0;
        Color color={25,230,255,rr_subtext_alpha(opacity,pass<4)};float pen=x;
        for(int n=0;*text;text+=n){uint32_t cp=rr_sub_next(text,&n);int k=rr_sub_glyph_index(cp);
            if(k>=0){
                if(art->ready){Rectangle dst=rr_sub_glyph_quad(k,pen+dx,baseline+dy,scale);DrawTexturePro(art->glyphs[k],(Rectangle){0,0,(float)rr_sub_glyphs[k].width,(float)rr_sub_glyphs[k].height},dst,(Vector2){0},0,color);}
                pen+=rr_sub_glyphs[k].advance*scale;
            }else{DrawTextCodepoint(font,(int)cp,(Vector2){pen+dx,baseline-RR_SUB_FONT_RASTER_BASELINE*size/(float)font.baseSize+dy},size,color);pen+=ascii_advance(font,(int)cp,scale);}
        }
    }
}

float rr_subtext_measure_spaced(Font font,const char *text,float css_size,float spacing)
{
    float width=rr_subtext_measure(font,text,css_size/26);int bytes=0;
    for(const char *s=text;*s;s+=bytes){rr_sub_next(s,&bytes);width+=spacing;}
    return width;
}
void rr_ui_subtext_spaced(RRSubArt *art,RRUI *ui,const char *text,float cx,float baseline,float css_size,float spacing,Color color)
{
    Font font=ui->loaded?ui->bold:GetFontDefault();float scale=css_size/26,size=css_size*RR_SUB_FONT_HEIGHT_TO_EM;
    float pen=cx-rr_subtext_measure_spaced(font,text,css_size,spacing)*.5f;
    for(int bytes=0;*text;text+=bytes){
        uint32_t cp=rr_sub_next(text,&bytes);int k=rr_sub_glyph_index(cp);
        if(k>=0){
            if(art->ready){Rectangle dst=rr_sub_glyph_quad(k,pen,baseline,scale);DrawTexturePro(art->glyphs[k],(Rectangle){0,0,(float)rr_sub_glyphs[k].width,(float)rr_sub_glyphs[k].height},dst,(Vector2){0},0,color);}
            pen+=rr_sub_glyphs[k].advance*scale;
        }else{DrawTextCodepoint(font,(int)cp,(Vector2){pen,baseline-RR_SUB_FONT_RASTER_BASELINE*size/(float)font.baseSize},size,color);pen+=ascii_advance(font,(int)cp,scale);}
        pen+=spacing;
    }
}
