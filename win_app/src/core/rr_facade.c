#include "rr_facade.h"
#include <string.h>
static double random99(uint32_t *state)
{
    uint32_t t=(*state+=0x6d2b79f5u);
    t=(t^(t>>15))*(1u|t);t^=t+(t^(t>>7))*(61u|t);
    return (double)(t^(t>>14))/4294967296.0;
}
bool rr_window_pixels(uint8_t *pixels,size_t bytes)
{
    if(!pixels || bytes<RR_WINDOW_BYTES)return false;
    uint32_t seed=99;const int palette[5][3]={{255,196,150},{110,225,255},{130,120,255},{200,210,255},{255,80,220}};
    memset(pixels,0,RR_WINDOW_BYTES);for(size_t i=3;i<RR_WINDOW_BYTES;i+=4)pixels[i]=255;
    for(int row=0;row<32;row++){
        bool row_lit=random99(&seed)<.08;double density=random99(&seed)<.3?.05:.28;
        for(int col=0;col<16;col++){
            bool lit=row_lit || random99(&seed)<density;uint8_t rgb[3]={11,9,30};
            if(lit){int p;if(random99(&seed)<.14)p=0;else if(random99(&seed)<.86)p=1+(int)(random99(&seed)*3);else p=4;
                double k=.45+random99(&seed)*.55;for(int j=0;j<3;j++)rgb[j]=(uint8_t)(palette[p][j]*k);}
            for(int y=row*16+3;y<row*16+14;y++)for(int x=col*16+3;x<col*16+13;x++){
                size_t at=((size_t)y*RR_WINDOW_WIDTH+(size_t)x)*4;memcpy(pixels+at,rgb,3);
            }
        }
    }
    return true;
}
