#include "rr_arrow_art.h"
void rr_arrow_art_unload(RRArrowArt *art)
{if(art->texture.id)UnloadTexture(art->texture);*art=(RRArrowArt){0};}
bool rr_arrow_art_load(RRArrowArt *art)
{
    if(art->ready)return true;
    const char *path="assets/ui-guidance/edge-arrow@1x.png";int bytes=0;unsigned char *data=LoadFileData(path,&bytes);
    if(!rr_arrow_png_header(data,bytes)){if(data)UnloadFileData(data);TraceLog(LOG_ERROR,"UI_ARROW: bounded browser PNG unavailable; vector placeholder remains unqualified");return false;}
    Image image=LoadImageFromMemory(".png",data,bytes);UnloadFileData(data);
    if(!image.data||image.width!=RR_ARROW_WIDTH||image.height!=RR_ARROW_HEIGHT||image.format!=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8){if(image.data)UnloadImage(image);TraceLog(LOG_ERROR,"UI_ARROW: PNG decode mismatch; placeholder unqualified");return false;}
    Texture2D texture=LoadTextureFromImage(image);UnloadImage(image);
    if(!texture.id||texture.width!=RR_ARROW_WIDTH||texture.height!=RR_ARROW_HEIGHT){if(texture.id)UnloadTexture(texture);TraceLog(LOG_ERROR,"UI_ARROW: texture upload failed; placeholder unqualified");return false;}
    art->texture=texture;art->ready=true;SetTextureFilter(art->texture,TEXTURE_FILTER_BILINEAR);
    TraceLog(LOG_INFO,"UI_ARROW: browser reference170x188, one RGBA8 texture127840 bytes; native pixels unqualified");return true;
}
bool rr_arrow_art_draw(const RRArrowArt *art,double left,double top,double zoom,double radians)
{
    if(!art->ready||!isfinite(left)||!isfinite(top)||!isfinite(zoom)||!isfinite(radians)||zoom<=0)return false;
    RRArrowLayout a=rr_arrow_layout(left,top,zoom,radians);
    DrawTexturePro(art->texture,(Rectangle){0,0,RR_ARROW_WIDTH,RR_ARROW_HEIGHT},(Rectangle){(float)a.cx,(float)a.cy,(float)a.w,(float)a.h},(Vector2){(float)a.origin_x,(float)a.origin_y},(float)a.degrees,WHITE);return true;
}
