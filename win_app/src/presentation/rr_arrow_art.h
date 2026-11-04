#ifndef RR_ARROW_ART_H
#define RR_ARROW_ART_H
#include "raylib.h"
#include <math.h>
#include <string.h>
#define RR_ARROW_WIDTH 170
#define RR_ARROW_HEIGHT 188
#define RR_ARROW_RGBA_BYTES 127840
#define RR_ARROW_MAX_FILE_BYTES 65536
/* One verified prepared browser component; no font redistribution. */
typedef struct RRArrowArt {Texture2D texture;bool ready;} RRArrowArt;
typedef struct RRArrowLayout {double cx,cy,w,h,origin_x,origin_y,degrees;} RRArrowLayout;
static inline bool rr_arrow_png_header(const unsigned char *data,int size)
{
    static const unsigned char signature[8]={137,80,78,71,13,10,26,10};
    if(!data||size<33||size>RR_ARROW_MAX_FILE_BYTES||memcmp(data,signature,8)||memcmp(data+12,"IHDR",4))return false;
    return data[8]==0&&data[9]==0&&data[10]==0&&data[11]==13&&data[16]==0&&data[17]==0&&data[18]==0&&data[19]==170&&data[20]==0&&data[21]==0&&data[22]==0&&data[23]==188&&data[24]==8&&data[25]==6&&data[26]==0&&data[27]==0&&data[28]==0;
}
static inline RRArrowLayout rr_arrow_layout(double left,double top,double zoom,double radians)
{
    /* Source box42x60, transform-origin21/30, PNG64px transparent padding.
     * HUD's existing -20/-25 positioning offsets are deliberately retained. */
    return (RRArrowLayout){left+21*zoom,top+30*zoom,170*zoom,188*zoom,85*zoom,94*zoom,radians*180/3.14159265358979323846};
}
bool rr_arrow_art_load(RRArrowArt *art);
void rr_arrow_art_unload(RRArrowArt *art);
bool rr_arrow_art_draw(const RRArrowArt *art,double left,double top,double zoom,double radians);
#endif
