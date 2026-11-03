#include "rr_subtext.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"subtext check line%d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(int argc,char **argv){CHECK(argc==3);SetTraceLogLevel(LOG_WARNING);
    const char *strings[]={"A","\xc2\xb7","\xe2\x98\x85","\xf0\x9f\x98\x80","\xc0\xaf","\xed\xa0\x80","\xe2\x98"};
    const uint32_t codes[]={65,183,9733,128512,65533,65533,65533};const int lengths[]={1,2,3,4,1,1,1};
    for(int i=0;i<7;i++){int n=0;CHECK(rr_sub_next(strings[i],&n)==codes[i]&&n==lengths[i]);}
    size_t rgba=0;for(int i=0;i<2;i++){
        char path[1024];snprintf(path,sizeof(path),"%s/%s",argv[2],rr_sub_glyphs[i].file);Image image=LoadImage(path);
        CHECK(image.data&&image.width==rr_sub_glyphs[i].width&&image.height==rr_sub_glyphs[i].height);Color *pixels=LoadImageColors(image);CHECK(pixels);
        int ink=0,clear=0;for(int j=0;j<image.width*image.height;j++){
            if(pixels[j].a){CHECK(pixels[j].r==255&&pixels[j].g==255&&pixels[j].b==255);ink++;}else clear++;
        }
        CHECK(ink>0&&clear>0);rgba+=(size_t)image.width*(size_t)image.height*4;UnloadImageColors(pixels);UnloadImage(image);
        for(int k=0;k<3;k++){float scale=(float[]){.8f,1.f,1.5f}[k];Rectangle q=rr_sub_glyph_quad(i,100,200,scale);
            CHECK(fabsf(q.x-(100+rr_sub_glyphs[i].x*scale))<1e-5f&&fabsf(q.y-(200+rr_sub_glyphs[i].y*scale))<1e-5f);
            CHECK(q.width>0&&q.height>0&&rr_sub_glyphs[i].advance<rr_sub_glyphs[i].w);
        }
    }CHECK(rgba==27060&&rgba<=65536);
    char fontpath[1024];snprintf(fontpath,sizeof(fontpath),"%s/Orbitron-700.ttf",argv[1]);int bytes=0;unsigned char *data=LoadFileData(fontpath,&bytes);CHECK(data);
    int cps[96];for(int i=0;i<95;i++)cps[i]=32+i;cps[95]=215;int count=0;GlyphInfo *glyphs=LoadFontData(data,bytes,64,cps,96,FONT_DEFAULT,&count);CHECK(glyphs&&count>=94);
    Rectangle *rects=NULL;Image atlas=GenImageFontAtlas(glyphs,&rects,count,64,4,0);CHECK(atlas.data&&rects);
    Font font={0};font.baseSize=64;font.glyphCount=count;font.glyphs=glyphs;font.recs=rects;
    CHECK(GetGlyphIndex(font,183)==GetGlyphIndex(font,'?')&&GetGlyphIndex(font,9733)==GetGlyphIndex(font,'?'));
    CHECK(count>=95&&GetGlyphIndex(font,215)!=GetGlyphIndex(font,'?')&&font.glyphs[GetGlyphIndex(font,215)].value==215);
    {int n=0;CHECK(rr_sub_next("\xc3\x97",&n)==215&&n==2);CHECK(rr_subtext_measure(font,"SPIN \xc2\xb7 1.5\xc3\x97 SPEED",1)>0);}
    CHECK(fabsf(rr_subtext_measure(font,"\xc2\xb7",1)-8.671875f)<1e-6f);
    CHECK(fabsf(rr_subtext_measure(font,"\xe2\x98\x85",1)-21.671875f)<1e-6f);
    for(int i=0;i<3;i++){
        float scale=(float[]){.8f,1.f,1.5f}[i];float total=rr_subtext_measure(font,"CHECKPOINT 1/8 \xc2\xb7 00:42.18",scale);
        float parts=rr_subtext_measure(font,"CHECKPOINT 1/8 ",scale)+8.671875f*scale+rr_subtext_measure(font," 00:42.18",scale);
        CHECK(fabsf(total-parts)<.0001f);CHECK(rr_subtext_measure(font,"\xe2\x98\x85 BEST LAP 00:42.18",scale)>21.671875f*scale);
    }
    UnloadImage(atlas);MemFree(rects);UnloadFontData(glyphs,count);UnloadFileData(data);
    printf("Fallback glyphs:2 lossless white-core decodes,27060 RGBA bytes; UTF-8, fractional advances, bearings and missing-primary bypass passed; no GPU/window.\n");return 0;
}
