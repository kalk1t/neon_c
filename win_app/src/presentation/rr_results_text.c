#include "rr_results_text.h"
#include "rr_results_label.h"
#include "rr_results_fallback_glyphs.h"
#include "rr_subtext.h"
#include "rr_race_ui.h"
#include "rr_css_font_metrics.h"
#include <float.h>
#include <string.h>
static bool font_ready(Font f){return f.baseSize>0&&f.glyphCount>0&&f.glyphs&&f.recs;}
static int fallback_index(uint32_t cp,int scale){for(int i=0;i<RR_RESULTS_ART_COUNT;i++){const RRResultsFallbackGlyph *g=rr_results_fallback_glyphs+i;if(g->codepoint==cp&&g->font_size_css==15&&g->device_scale==scale)return i;}return -1;}
bool rr_results_text_build(Font font,const char *text,int scale,RRResultsText *out)
{
 if(!out||!text||!font_ready(font)||scale<1||scale>3)return false;
 size_t bytes=0;while(bytes<RR_RESULTS_TEXT_BYTES&&text[bytes])bytes++;if(bytes==RR_RESULTS_TEXT_BYTES)return false;
 RRResultsText value={0};value.font=font;value.device_scale=scale;
 for(const char *s=text;*s;){
  if(value.count==RR_RESULTS_TEXT_TOKENS)return false;int n=0;uint32_t cp=rr_sub_next(s,&n);
  if(n<=0||cp<32||cp==65533)return false;s+=n;
  int k=fallback_index(cp,scale);float advance=0;
  if(k>=0){advance=rr_results_fallback_glyphs[k].advance;value.fallback=true;}
  else{int j=GetGlyphIndex(font,(int)cp);if(j<0||j>=font.glyphCount||font.glyphs[j].value!=(int)cp)return false;
   advance=(font.glyphs[j].advanceX>0?(float)font.glyphs[j].advanceX:font.recs[j].width+(float)font.glyphs[j].offsetX)*(float)rr_css_font_size(15,700)/(float)font.baseSize;}
  if(!isfinite(advance)||advance<0||!isfinite(value.width+advance))return false;
  value.tokens[value.count++]=(RRResultsToken){cp,value.width,advance,k};value.width+=advance;
 }
 *out=value;return true;
}
RRResultsRowStatus rr_results_row_build(Font font,const RRResultsCurrent *s,RRResultsRowKind kind,size_t row,const char *const *names,size_t count,size_t player,int scale,float left,float right,float baseline,RRResultsRow *out)
{
 if(!out||!s||!names||!count||count>RR_LOCAL_MAX_CARS||player>=count||!isfinite(left)||!isfinite(right)||!isfinite(baseline)||right<left)return RR_RESULTS_ROW_INVALID;
 RRResultsLabel label;if(!rr_results_label_build(s,kind==RR_RESULTS_STANDINGS?RR_LABEL_STANDINGS:kind==RR_RESULTS_COINS?RR_LABEL_COINS:(RRResultsLabelKind)2,row,names,count,player,&label))return RR_RESULTS_ROW_INVALID;
 RRResultsRow v={0};if(!rr_results_text_build(font,label.name,scale,&v.name)||!rr_results_text_build(font,label.value,scale,&v.value))return RR_RESULTS_ROW_INVALID;
 if((double)v.name.width+v.value.width>(double)right-left)return RR_RESULTS_ROW_NEEDS_WRAP;
 double vx=(double)right-v.value.width;if(fabs(vx)>FLT_MAX)return RR_RESULTS_ROW_INVALID;
 v.left=left;v.value_x=(float)vx;v.baseline=baseline;v.shadow=label.player;
 v.name_color=v.shadow?(Color){255,255,255,255}:kind==RR_RESULTS_COINS?(Color){243,224,176,255}:(Color){207,239,255,255};
 v.value_color=kind==RR_RESULTS_COINS?(Color){255,210,90,255}:v.shadow?(Color){255,43,214,255}:(Color){153,187,221,255};
 v.shadow_color=kind==RR_RESULTS_COINS?(Color){255,160,18,255}:(Color){255,43,214,255};*out=v;return RR_RESULTS_ROW_READY;
}
static bool preflight(const RRResultsText *text,float x,float baseline)
{
 if(!text||!font_ready(text->font)||text->count>RR_RESULTS_TEXT_TOKENS||text->device_scale<1||text->device_scale>3||!isfinite(x)||!isfinite(baseline))return false;
 for(size_t i=0;i<text->count;i++){
  const RRResultsToken *t=text->tokens+i;double px=(double)x+t->pen,py=rr_css_raylib_origin(baseline,15,700,text->font.baseSize);
  if(t->fallback>=0){if(t->fallback>=RR_RESULTS_ART_COUNT)return false;const RRResultsFallbackGlyph *g=rr_results_fallback_glyphs+t->fallback;px+=g->offset_x;py=(double)baseline+g->offset_y;}
  if(!isfinite(px)||!isfinite(py)||fabs(px)>FLT_MAX-4.0||fabs(py)>FLT_MAX-4.0)return false;
 }
 return true;
}
static void emit_text(const RRResultsText *text,float x,float baseline,Color core,Color shadow,bool has_shadow,RRResultsPaintSink sink,void *ctx)
{
 for(int pass=has_shadow?0:4;pass<5;pass++){
  float dx=pass==0?-2.f:pass==1?2.f:0.f,dy=pass==2?-2.f:pass==3?2.f:0.f;Color color=pass<4?shadow:core;if(pass<4)color.a=31;
  for(size_t i=0;i<text->count;i++){
   const RRResultsToken *t=text->tokens+i;RRResultsPaint p={0};p.codepoint=t->codepoint;p.fallback=t->fallback;p.color=color;p.shadow=pass<4;p.font_size=(float)rr_css_font_size(15,700);
   p.origin=(Vector2){x+t->pen+dx,(float)rr_css_raylib_origin(baseline,15,700,text->font.baseSize)+dy};
   if(t->fallback>=0){const RRResultsFallbackGlyph *g=rr_results_fallback_glyphs+t->fallback;p.source=(Rectangle){0,0,(float)g->width_pixels,(float)g->height_pixels};p.destination=(Rectangle){x+t->pen+g->offset_x+dx,baseline+g->offset_y+dy,g->draw_width,g->draw_height};}
   sink(ctx,&p);
  }
 }
}
bool rr_results_row_emit(const RRResultsRow *row,RRResultsPaintSink sink,void *ctx)
{
 if(!row||!sink||!preflight(&row->name,row->left,row->baseline)||!preflight(&row->value,row->value_x,row->baseline))return false;
 emit_text(&row->name,row->left,row->baseline,row->name_color,row->shadow_color,row->shadow,sink,ctx);
 emit_text(&row->value,row->value_x,row->baseline,row->value_color,row->shadow_color,row->shadow,sink,ctx);return true;
}
