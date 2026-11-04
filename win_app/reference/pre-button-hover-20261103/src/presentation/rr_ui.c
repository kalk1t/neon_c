#include "rr_ui.h"
#include "rr_hud_panel_shape.h"
#include "rr_button_surface.h"
#include "rr_button_text.h"
#include "rlgl.h"
#include "rr_pause_style.h"
#include "rr_css_font_metrics.h"
#include <math.h>
#include <stddef.h>
#include <string.h>
static const Color cyan={25,230,255,255},magenta={255,43,214,255};
static Rectangle rec(RRUIRect r){return (Rectangle){r.x,r.y,r.w,r.h};}
bool rr_ui_load(RRUI *ui)
{
    const char *paths[]={"assets/ui-theme/Orbitron-500.ttf","assets/ui-theme/Orbitron-700.ttf","assets/ui-theme/Orbitron-900.ttf","assets/ui-theme/Rajdhani-Medium.ttf"};
    Font *fonts[]={&ui->regular,&ui->bold,&ui->heavy,&ui->body};ui->loaded=true;
    /* One bounded atlas per weight; bold also includes source multiplication U+00D7. */
    int cps[96];for(int i=0;i<95;i++)cps[i]=32+i;cps[95]=215;
    for(int i=0;i<4;i++){
        if(!FileExists(paths[i])){ui->loaded=false;break;}
        *fonts[i]=LoadFontEx(paths[i],i==3?32:64,cps,i==1?96:95);
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
static void css_text(RRUI *ui,const char *s,float x,float baseline,float size,float tracking,int weight,Color color,bool glow,bool right)
{
    Font font=ui->loaded?(weight==900?ui->heavy:weight==700?ui->bold:weight==0?ui->body:ui->regular):GetFontDefault();
    float pixels=ui->loaded?(float)rr_css_font_size(size,weight):size;
    float y=ui->loaded?(float)rr_css_raylib_origin(baseline,size,weight,font.baseSize):baseline-size;
    if(right)x-=MeasureTextEx(font,s,pixels,tracking).x;
    rr_ui_text(ui,s,x,y,pixels,tracking,weight,color,glow);
}
void rr_ui_text_css(RRUI *ui,const char *s,float x,float baseline,float size,float tracking,int weight,Color c,bool glow)
{css_text(ui,s,x,baseline,size,tracking,weight,c,glow,false);}
void rr_ui_text_right_css(RRUI *ui,const char *s,float right,float baseline,float size,float tracking,int weight,Color c,bool glow)
{css_text(ui,s,right,baseline,size,tracking,weight,c,glow,true);}
void rr_ui_text_center_css_glow(RRUI *ui,const char *s,float cx,float baseline,float size,float tracking,int weight,Color c,bool glow)
{
    Font font=ui->loaded?(weight==900?ui->heavy:weight==700?ui->bold:weight==0?ui->body:ui->regular):GetFontDefault();
    float pixels=ui->loaded?(float)rr_css_font_size(size,weight):size;
    /* CSS letter-spacing also contributes the trailing character advance.
     * Native advances/kerning remain approximate, as in prior HUD readouts. */
    float width=MeasureTextEx(font,s,pixels,tracking).x+tracking;
    rr_ui_text_css(ui,s,cx-width*.5f,baseline,size,tracking,weight,c,glow);
}
void rr_ui_text_center_css(RRUI *ui,const char *s,float cx,float baseline,float size,float tracking,int weight,Color c)
{rr_ui_text_center_css_glow(ui,s,cx,baseline,size,tracking,weight,c,false);}
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
    for(int i=0;i<6;i++)if(i!=2&&i!=5)DrawLineEx(p[i],p[i+1],2,c);
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
void rr_ui_pause_panel(RRUIRect r,Color border)
{
    uint32_t rgba=rr_pause_panel_rgba();Color fill={(unsigned char)(rgba>>24),(unsigned char)(rgba>>16),(unsigned char)(rgba>>8),(unsigned char)rgba};
    DrawRectangleRec(rec(r),fill);DrawRectangleLinesEx(rec(r),2,border);
    /* Existing bounded native shadow approximation; source30px blur unqualified. */
    for(int i=1;i<=3;i++)DrawRectangleLinesEx((Rectangle){r.x-(float)i,r.y-(float)i,r.w+(float)i*2,r.h+(float)i*2},1,Fade(border,.04f));
}
static void draw_button_surface(RRUIRect r,bool primary,bool enabled,float hover)
{
    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);rlBegin(RL_TRIANGLES);
    for(int pass=0;pass<(hover>0?2:1);pass++)for(int triangle=0;triangle<(pass?6:14);triangle++){
        RRButtonVertex p[3];int kind=rr_button_surface_triangle(r,triangle,p);if(!kind)continue;
        for(int i=0;i<3;i++){
            RRButtonRGBA c=rr_button_surface_rgba(primary,enabled,kind,p[i].gradient);
            if(pass){/* Retained native hover wash, not source shadow/transition parity. */
                double a=(.12-.06*(p[i].y-r.y)/r.h)*hover*(enabled?1:.4);c=(RRButtonRGBA){25*a,230*a,255*a,255*a};
            }
            rlColor4ub((unsigned char)lround(c.r),(unsigned char)lround(c.g),(unsigned char)lround(c.b),(unsigned char)lround(c.a));
            rlVertex2f(p[i].x,p[i].y);
        }
    }
    rlEnd();EndBlendMode();
}

bool rr_ui_button(RRUI *ui,RRUIRect r,const char *label,bool primary,bool enabled,bool focus,int slot)
{return rr_ui_button_ex(ui,r,label,primary,enabled,focus,slot,20,24);}
bool rr_ui_button_ex(RRUI *ui,RRUIRect r,const char *label,bool primary,bool enabled,bool focus,int slot,float font_size,float inset)
{
    Vector2 m=GetMousePosition();bool hover=rr_ui_hit(r,m.x,m.y,14);float t=hover||focus?1.f:0.f;
    if(slot>=0&&slot<8){float dt=fminf(GetFrameTime(),.05f);ui->hover[slot]+=fmaxf(-dt/.15f,fminf(dt/.15f,t-ui->hover[slot]));t=ui->hover[slot];}
    RRButtonText text=rr_button_text(r,font_size,inset,t,enabled);
    Color color={(unsigned char)(text.rgba>>24),(unsigned char)(text.rgba>>16),(unsigned char)(text.rgba>>8),(unsigned char)text.rgba};
    draw_button_surface(r,primary,enabled,t);
    rr_ui_text_css(ui,label,(float)text.x,(float)text.baseline,(float)text.size,(float)text.tracking,700,color,false);
    return enabled&&IsWindowFocused()&&hover&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
void rr_ui_pips(float x,float y,int on,float alpha)
{
    for(int i=0;i<9;i++){
        Color a=i<on?(Color){255,210,90,255}:Fade(cyan,.18f),b=i<on?magenta:a;
        /* Source skewX(-15deg), 6x12 with 3px gap. */
        for(int j=0;j<12;j++){float t=j/11.f;Color c={(unsigned char)(a.r+(b.r-a.r)*t),(unsigned char)(a.g+(b.g-a.g)*t),(unsigned char)(a.b+(b.b-a.b)*t),(unsigned char)(a.a*alpha)};DrawRectangle((int)(x+i*9+(6-j)*.268f),(int)y+j,6,1,c);}
    }
}

void rr_ui_text_center(RRUI *ui,const char *s,float cx,float y,float size,float tracking,int weight,Color c,bool glow)
{
    Font f=ui->loaded?(weight==900?ui->heavy:weight==700?ui->bold:weight==0?ui->body:ui->regular):GetFontDefault();
    rr_ui_text(ui,s,cx-MeasureTextEx(f,s,size,tracking).x*.5f,y,size,tracking,weight,c,glow);
}
void rr_ui_hud_panel(RRUIRect r,Color border,float scale)
{
    /* Same untextured RL_TRIANGLES path as pinned DrawCircleGradient.
     * Global vertical color field, straight bevels and inside clipped border. */
    rlBegin(RL_TRIANGLES);
    for(int triangle=0;triangle<16;triangle++){
        RRPanelVertex points[3];int kind=rr_hud_panel_triangle(r,scale,triangle,points);if(!kind)continue;
        for(int i=0;i<3;i++){
            Color color=border;
            if(kind==1){float t=points[i].gradient;color=(Color){(unsigned char)lroundf(8-5*t),(unsigned char)lroundf(20-14*t),(unsigned char)lroundf(60-36*t),199};}
            rlColor4ub(color.r,color.g,color.b,color.a);rlVertex2f(points[i].x,points[i].y);
        }
    }
    rlEnd();
}
