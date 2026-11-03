#include "raylib.h"
#include <stdio.h>
#include <stdint.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(int argc,char **argv)
{
    CHECK(argc==2);SetTraceLogLevel(LOG_WARNING);const char *names[]={"base.png","control.png","boost.png","attack.png","warning.png"};int dims[5][2]={{1672,941},{900,492},{128,134},{128,110},{128,111}};size_t total=0,peak=0;
    for(int i=0;i<5;i++){
        char path[1024];snprintf(path,sizeof(path),"%s/%s",argv[1],names[i]);Image im=LoadImage(path);CHECK(im.data&&im.width==dims[i][0]&&im.height==dims[i][1]);Color *pixels=LoadImageColors(im);CHECK(pixels);size_t count=(size_t)im.width*(size_t)im.height,transparent=0,nonzero=0;uint64_t sum=0;
        for(size_t j=0;j<count;j++){transparent+=pixels[j].a==0;nonzero+=pixels[j].a>0;sum+=pixels[j].a;}
        CHECK(transparent>0&&nonzero>0);size_t bytes=count*4;total+=bytes;if(bytes>peak)peak=bytes;
        printf("%s %dx%d RGBA8=%zu alpha_sum=%llu\n",names[i],im.width,im.height,bytes,(unsigned long long)sum);UnloadImageColors(pixels);UnloadImage(im);
    }
    CHECK(total<=9u*1024u*1024u);printf("CPU cockpit PNG decode passed; resident RGBA8=%zu, largest single image=%zu; no window/GPU initialized.\n",total,peak);return 0;
}
