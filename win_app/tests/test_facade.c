#include "rr_facade.h"
#include <stdio.h>
#include <stdlib.h>
#include "generated_facade_reference.h"
int main(void)
{
    unsigned char *p=malloc(RR_WINDOW_BYTES+16);int failures=0;
    if(!p)return 2;
    for(size_t i=RR_WINDOW_BYTES;i<RR_WINDOW_BYTES+16;i++)p[i]=0x5a;
    if(rr_window_pixels(NULL,RR_WINDOW_BYTES)||rr_window_pixels(p,RR_WINDOW_BYTES-1)||!rr_window_pixels(p,RR_WINDOW_BYTES))failures++;
    uint32_t hash=2166136261u;for(size_t i=0;i<RR_WINDOW_BYTES;i++)hash=(hash^p[i])*16777619u;
    if(hash!=RR_WINDOW_REFERENCE_FNV)failures++;
    for(int row=0;row<32;row++)for(int col=0;col<16;col++)for(int y=0;y<16;y++)for(int x=0;x<16;x++){
        size_t at=((size_t)(row*16+y)*256+(size_t)(col*16+x))*4;
        for(int c=0;c<3;c++){unsigned char expected=(x>=3&&x<13&&y>=3&&y<14)?window_reference_colors[row*16+col][c]:0;if(p[at+c]!=expected)failures++;}
        if(p[at+3]!=255)failures++;
    }
    for(size_t i=RR_WINDOW_BYTES;i<RR_WINDOW_BYTES+16;i++)if(p[i]!=0x5a)failures++;
    printf("Browser facade: 131072 RGBA pixels,512 source window colors/rectangles, short-buffer/null/guard checks, FNV=%u; %d failures.\n",hash,failures);
    free(p);return failures?1:0;
}
