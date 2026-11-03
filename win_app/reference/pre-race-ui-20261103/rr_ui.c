#include "rr_ui.h"
#include <math.h>
#include <stddef.h>
#include <string.h>
static const Color cyan={25,230,255,255},magenta={255,43,214,255};
static Rectangle rec(RRUIRect r){return (Rectangle){r.x,r.y,r.w,r.h};}
bool rr_ui_load(RRUI *ui)
{
    const char *paths[]={"assets/ui-theme/Orbitron-500.ttf","assets/ui-theme/Orbitron-700.ttf","assets/ui-theme/Orbitron-900.ttf","assets/ui-theme/Rajdhani-Medium.ttf"};
    Font *fonts[]={&ui->regular,&ui->bold,&ui->heavy,&ui->body};ui->loaded=true;
    /* One bounded ASCII atlas per weight. No scene/world allocation or postprocess. */
    for(int i=0;i<4;i++){
        if(!FileExists(paths[i])){ui->loaded=false;break;}
        *fonts[i]=LoadFontEx(paths[i],i==3?32:64,NULL,95);
        if(fonts[i]->texture.id==GetFontDefault().texture.id||!fonts[i]->texture.id){ui->loaded=false;break;}
        SetTextureFilter(fonts[i]->texture,TEXTURE_FILTER_BILINEAR);
        TraceLog(LOG_INFO,"UI_FONT: %s atlas=%dx%d",paths[i],fonts[i]->texture.width,fonts[i]->texture.height);
    }
    if(!ui->loaded){TraceLog(LOG_ERROR,"UI_FONT: exact theme font unavailable; fallback explicitly unqualified");rr_ui_unload(ui);}
    return ui->loaded;
}
void rr_ui_unload(RRUI *ui)
{
    Font *fonts[]={&ui->regular,&ui->bold,&ui->heavy,&ui->body};
    for(int i=0;i<4;i++){if(fonts[i]->texture.id&&fonts[i]->texture.id!=GetFontDefault().texture.id)UnloadFont(*fonts[i]);*fonts[i]=(Font){0};}
    ui->loaded=false;
}
void rr_ui_text(RRUI *ui,const char *s,float x,float y,float size,float tracking,int weight,Color c,bool glow)
{
    Font f=ui->loaded?(weight==900?ui->heavy:weight==700?ui->bold:weight==0?ui->body:ui->regular):GetFontDefault();
    if(glow)for(int i=0;i<4;i++){float dx=i==0?-2.f:i==1?2.f:0.f,dy=i==2?-2.f:i==3?2.f:0.f;DrawTextEx(f,s,(Vector2){x+dx,y+dy},size,tracking,Fade(c,.12f));}
    DrawTextEx(f,s,(Vector2){x,y},size,tracking,c);
}
void rr_ui_text_fit(RRUI *ui,const char *s,float x,float y,float width,float size,float tracking,int weight,Color c)
{
    Font f=ui->loaded?(weight==700?ui->bold:ui->regular):GetFontDefault();char text[256];
    size_t n=strlen(s);if(n>250)n=250;memcpy(text,s,n);text[n]=0;
    if(MeasureTextEx(f,text,size,tracking).x>width){
        while(n>0){n--;text[n]=0;memcpy(text+n,"...",4);if(MeasureTextEx(f,text,size,tracking).x<=width)break;text[n]=0;}
    }
    rr_ui_text(ui,text,x,y,size,tracking,weight,c,false);
}
static void chamfer(RRUIRect r,Color a,Color b)
{
    /* Vertical strips clip the gradient without a render texture/stencil. */
    int count=(int)ceilf(r.w/4);
    for(int i=0;i<count;i++){
        float x=(float)i*4.f,sw=fminf(4,r.w-x),top=fmaxf(0,14-x),bottom=fmaxf(0,14-(r.w-x-sw));
        DrawRectangleGradientV((int)(r.x+x),(int)(r.y+top),(int)ceilf(sw),(int)(r.h-top-bottom),a,b);
    }
}
static void outline(RRUIRect r,Color c)
{
    Vector2 p[7]={{r.x+14,r.y},{r.x+r.w,r.y},{r.x+r.w,r.y+r.h-14},{r.x+r.w-14,r.y+r.h},{r.x,r.y+r.h},{r.x,r.y+14},{r.x+14,r.y}};
    for(int i=0;i<6;i++)DrawLineEx(p[i],p[i+1],2,c);
}
void rr_ui_row(RRUIRect r,Color edge,float opacity,bool highlighted)
{
    chamfer(r,Fade((Color){6,14,46,230},opacity),Fade((Color){6,14,46,230},opacity));
    if(highlighted)chamfer(r,Fade(cyan,.12f),Fade(cyan,.12f));
    outline(r,Fade(edge,opacity));
}
void rr_ui_panel(RRUIRect r,Color border)
{
    DrawRectangleGradientV((int)r.x,(int)r.y,(int)r.w,(int)r.h,(Color){8,20,60,235},(Color){3,6,24,242});
    DrawRectangleLinesEx(rec(r),2,border);
    for(int i=1;i<=3;i++)DrawRectangleLinesEx((Rectangle){r.x-(float)i,r.y-(float)i,r.w+(float)i*2,r.h+(float)i*2},1,Fade(border,.04f));
}
bool rr_ui_button(RRUI *ui,RRUIRect r,const char *label,bool primary,bool enabled,bool focus,int slot)
{
    Vector2 m=GetMousePosition();bool hover=rr_ui_hit(r,m.x,m.y,14);float t=hover||focus?1.f:0.f;
    if(slot>=0&&slot<8){float dt=fminf(GetFrameTime(),.05f);ui->hover[slot]+=fmaxf(-dt/.15f,fminf(dt/.15f,t-ui->hover[slot]));t=ui->hover[slot];}
    float alpha=enabled?1:.4f;Color edge=primary?magenta:cyan;
    chamfer(r,Fade(primary?(Color){96,16,95,235}:(Color){6,14,46,230},alpha),Fade((Color){6,14,46,230},alpha));
    if(t>0)chamfer(r,Fade(cyan,.12f*t*alpha),Fade(cyan,.06f*t*alpha));
    outline(r,Fade(edge,alpha));rr_ui_text(ui,label,r.x+22+10*t,r.y+(r.h-18)*.5f,18,3,700,Fade((Color){221,255,255,255},alpha),false);
    return enabled&&hover&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
void rr_ui_pips(float x,float y,int on,float alpha)
{
    for(int i=0;i<9;i++){
        Color a=i<on?(Color){255,210,90,255}:Fade(cyan,.18f),b=i<on?magenta:a;
        /* Source skewX(-15deg), 6x12 with 3px gap. */
        for(int j=0;j<12;j++){float t=j/11.f;Color c={(unsigned char)(a.r+(b.r-a.r)*t),(unsigned char)(a.g+(b.g-a.g)*t),(unsigned char)(a.b+(b.b-a.b)*t),(unsigned char)(a.a*alpha)};DrawRectangle((int)(x+i*9+(6-j)*.268f),(int)y+j,6,1,c);}
    }
}
