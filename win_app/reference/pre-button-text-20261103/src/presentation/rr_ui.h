#ifndef RR_UI_H
#define RR_UI_H
#include "raylib.h"
#include "rr_ui_layout.h"
typedef struct RRUI { Font regular,bold,heavy,body; bool loaded; float hover[8]; } RRUI;
bool rr_ui_load(RRUI *ui);
void rr_ui_unload(RRUI *ui);
void rr_ui_text(RRUI *ui,const char *s,float x,float y,float size,float tracking,int weight,Color c,bool glow);
/* CSS em sizing, explicit nominal baseline; only qualified readout call sites. */
void rr_ui_text_css(RRUI *ui,const char *s,float x,float baseline,float size,float tracking,int weight,Color c,bool glow);
void rr_ui_text_right_css(RRUI *ui,const char *s,float right,float baseline,float size,float tracking,int weight,Color c,bool glow);
void rr_ui_text_center_css_glow(RRUI *ui,const char *s,float cx,float baseline,float size,float tracking,int weight,Color c,bool glow);
void rr_ui_text_center_css(RRUI *ui,const char *s,float cx,float baseline,float size,float tracking,int weight,Color c);
void rr_ui_text_fit(RRUI *ui,const char *s,float x,float y,float width,float size,float tracking,int weight,Color c);
void rr_ui_row(RRUIRect r,Color edge,float opacity,bool highlighted);
void rr_ui_panel(RRUIRect r,Color border);
bool rr_ui_button(RRUI *ui,RRUIRect r,const char *label,bool primary,bool enabled,bool focus,int slot);
/* Text size/inset variant; existing call sites retain their18px convention. */
bool rr_ui_button_ex(RRUI *ui,RRUIRect r,const char *label,bool primary,bool enabled,bool focus,int slot,float font_size,float inset);
void rr_ui_pause_panel(RRUIRect r,Color border);
void rr_ui_text_center(RRUI *ui,const char *s,float cx,float y,float size,float tracking,int weight,Color c,bool glow);
void rr_ui_hud_panel(RRUIRect r,Color border,float scale);
void rr_ui_pips(float x,float y,int on,float alpha);
#endif
