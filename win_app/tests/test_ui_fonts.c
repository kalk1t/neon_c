#include "raylib.h"
#include "rr_ui_codepoints.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"Font check failed line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(int argc,char **argv)
{
    CHECK(argc==2);SetTraceLogLevel(LOG_WARNING);
    const char *names[]={"Orbitron-500.ttf","Orbitron-700.ttf","Orbitron-900.ttf","Rajdhani-Medium.ttf"};
    unsigned long long sums[4]={0};int cps[RR_UI_MAX_CODEPOINTS];
    size_t total=0;
    for(int k=0;k<4;k++){
        char path[1024];snprintf(path,sizeof(path),"%s/%s",argv[1],names[k]);int bytes=0;
        unsigned char *data=LoadFileData(path,&bytes);CHECK(data&&bytes>1000);
        int size=k==3?32:64,count=0,requested=rr_ui_codepoints(cps,k==1?700:500);GlyphInfo *g=LoadFontData(data,bytes,size,cps,requested,FONT_DEFAULT,&count);CHECK(g&&count>=94);
        for(int i=0;i<count;i++)if(g[i].value!=' '){
            CHECK(g[i].image.data&&g[i].image.width>0&&g[i].image.height>0&&g[i].advanceX>0);
            unsigned char *pixels=g[i].image.data;for(int j=0;j<g[i].image.width*g[i].image.height;j++)sums[k]+=pixels[j];
        }
        if(k==1){bool dash=false;for(int j=0;j<count;j++)if(g[j].value==8212&&g[j].advanceX>0&&g[j].image.data)dash=true;CHECK(dash);bool multiply=false;for(int j=0;j<count;j++)if(g[j].value==215&&g[j].advanceX>0&&g[j].image.data)multiply=true;CHECK(multiply);}
        Rectangle *rects=NULL;Image atlas=GenImageFontAtlas(g,&rects,count,size,4,0);
        CHECK(atlas.data&&rects&&atlas.width<=2048&&atlas.height<=2048);size_t rgba=(size_t)atlas.width*(size_t)atlas.height*4;total+=rgba;
        printf("%s CPU atlas %dx%d, conservative RGBA8=%zu bytes, glyph coverage=%llu\n",names[k],atlas.width,atlas.height,rgba,sums[k]);
        UnloadImage(atlas);MemFree(rects);UnloadFontData(g,count);UnloadFileData(data);
    }
    CHECK(sums[0]<sums[1]&&sums[1]<sums[2]);CHECK(total<=16*1024*1024);
    printf("CPU font rasterization/atlas passed; RGBA8 total=%zu; no window/GPU initialized.\n",total);return 0;
}
