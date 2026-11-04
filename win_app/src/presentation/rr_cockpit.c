#include "rr_cockpit.h"
#include <string.h>
static const char *paths[RR_COCKPIT_LAYER_COUNT]={"assets/cockpit-roadster/base.png","assets/cockpit-roadster/control.png","assets/cockpit-roadster/boost.png","assets/cockpit-roadster/attack.png","assets/cockpit-roadster/warning.png"};
static const int dims[RR_COCKPIT_LAYER_COUNT][2]={{1672,941},{900,492},{128,134},{128,110},{128,111}};
void rr_cockpit_unload(RRCockpitArt *art)
{
    for(int i=0;i<RR_COCKPIT_LAYER_COUNT;i++)if(art->layers[i].id)UnloadTexture(art->layers[i]);
    memset(art,0,sizeof(*art));
}
bool rr_cockpit_load_next(RRCockpitArt *art)
{
    if(art->failed)return false;if(art->loaded==RR_COCKPIT_LAYER_COUNT)return true;
    int slot=art->loaded;Image image=LoadImage(paths[slot]);
    if(!image.data||image.width!=dims[slot][0]||image.height!=dims[slot][1]){
        if(image.data)UnloadImage(image);rr_cockpit_unload(art);art->failed=true;TraceLog(LOG_ERROR,"COCKPIT: image validation failed; exterior retained");return false;
    }
    Texture2D texture=LoadTextureFromImage(image);UnloadImage(image);
    if(!texture.id){rr_cockpit_unload(art);art->failed=true;return false;}
    SetTextureFilter(texture,TEXTURE_FILTER_BILINEAR);art->layers[slot]=texture;art->loaded++;
    TraceLog(LOG_INFO,"COCKPIT: candidate upload %d/%d, isolated menu/pause frame",art->loaded,RR_COCKPIT_LAYER_COUNT);return art->loaded==RR_COCKPIT_LAYER_COUNT;
}
static void texture_box(Texture2D t,RRUIRect r,Vector2 origin,float rotation,Color tint)
{DrawTexturePro(t,(Rectangle){0,0,(float)t.width,(float)t.height},(Rectangle){r.x+origin.x,r.y+origin.y,r.w,r.h},origin,rotation,tint);}
void rr_cockpit_draw(const RRCockpitArt *art,RRUI *ui,int width,int height,const RRCKReadout *v,double boost,bool combat_connected)
{
    if(art->failed||art->loaded!=RR_COCKPIT_LAYER_COUNT)return;
    RRCKFit fit=rr_cockpit_fit(width,height);RRCKLayout layout=rr_cockpit_layout(0);float em=(float)fit.font;
    BeginScissorMode(0,0,width,height);
    texture_box(art->layers[0],(RRUIRect){(float)fit.x,(float)fit.y,(float)fit.w,(float)fit.h},(Vector2){0,0},0,WHITE);
    RRUIRect speed=rr_cockpit_box(fit,layout.panel[0]),bo=rr_cockpit_box(fit,layout.panel[1]),at=rr_cockpit_box(fit,layout.panel[2]),race=rr_cockpit_box(fit,layout.panel[3]);
    Color ink={246,249,255,255},cyan={54,231,255,255};
    rr_ui_text_center(ui,v->speed,speed.x+speed.w*.5f,speed.y+speed.h*.5f-1.8f*em,3*em,.105f*em,900,ink,false);
    rr_ui_text_center(ui,v->unit,speed.x+speed.w*.5f,speed.y+speed.h*.5f+1.1f*em,.9f*em,.126f*em,700,ink,false);
    float be=em*1.2f;
    RRUIRect icon={bo.x+bo.w*.5f-2*be,bo.y+bo.h*.5f-1.4f*be,.8f*be,.84f*be};texture_box(art->layers[2],icon,(Vector2){0,0},0,WHITE);
    rr_ui_text_center(ui,"BOOST",bo.x+bo.w*.5f+.6f*be,bo.y+bo.h*.5f-1.4f*be,.72f*be,.058f*be,700,ink,false);
    rr_ui_text_center(ui,v->boost,bo.x+bo.w*.5f,bo.y+bo.h*.5f-.4f*be,1.45f*be,0,900,GetColor(rr_cockpit_boost_rgba(v)),false);
    DrawRectangle((int)(bo.x+bo.w*.125f),(int)(bo.y+bo.h*.5f+1.2f*be),(int)(bo.w*.75f),(int)fmaxf(3,.42f*be),(Color){20,32,44,255});
    DrawRectangle((int)(bo.x+bo.w*.125f),(int)(bo.y+bo.h*.5f+1.2f*be),(int)(bo.w*.75f*rr_clamp(boost,0,1)),(int)fmaxf(3,.42f*be),cyan);
    float ae=em*1.1f;
    if(combat_connected){
        RRUIRect ai={at.x+at.w*.5f-.75f*ae,at.y+at.h*.5f-1.9f*ae,1.5f*ae,1.3f*ae};texture_box(art->layers[v->hot?4:3],ai,(Vector2){0,0},0,WHITE);
    }
    rr_ui_text_center(ui,"PULSE",at.x+at.w*.5f,at.y+at.h*.5f-.3f*ae,.72f*ae,.058f*ae,700,ink,false);
    rr_ui_text_center(ui,combat_connected?v->heat:"NOT CONNECTED",at.x+at.w*.5f,at.y+at.h*.5f+.7f*ae,.65f*ae,0,700,combat_connected?GetColor(rr_cockpit_heat_rgba(v)):(Color){170,180,195,255},false);
    float re=em*1.15f;
    rr_ui_text_center(ui,v->lap,race.x+race.w*.27f,race.y+(race.h-.84f*re)*.5f,.84f*re,0,700,ink,false);
    rr_ui_text_center(ui,v->time,race.x+race.w*.74f,race.y+(race.h-.84f*re)*.5f,.84f*re,0,700,ink,false);
    /* Source z-order: controls cover art/readouts, with exact percent pivot. */
    RRUIRect control=rr_cockpit_box(fit,layout.control[0]);Vector2 origin={(float)(control.w*layout.pivot_x),(float)(control.h*layout.pivot_y)};
    texture_box(art->layers[1],control,origin,(float)v->angle,WHITE);EndScissorMode();
}
