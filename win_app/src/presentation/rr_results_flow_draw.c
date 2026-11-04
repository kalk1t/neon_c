#include "rr_results_flow_draw.h"
#include "rr_results_fallback_glyphs.h"
#include "rr_css_font_metrics.h"
#include <float.h>
#include <string.h>
typedef struct FlowSink {Font font;const RRResultsBodyInput *input;const bool *player[2];const RRResultsBodyLayout *body;RRResultsRow *row;RRResultsPaintSink sink;void *context;int scale,kind;bool ready;} FlowSink;
static void no_paint(void *context,const RRResultsPaint *paint){(void)context;(void)paint;}
static double pen(const RRResultsCharacters *c,size_t index){return index==c->count?c->advance:c->ranges[index].pen;}
static bool fragment(FlowSink *s,const char *text,const RRResultsCharacters *chars,size_t first,size_t count,double x,double top,bool value,bool player)
{
 RRResultsRow *row=s->row;
 if(!count||first>chars->count||count>chars->count-first||!isfinite(x)||!isfinite(top)||fabs(x)>FLT_MAX||fabs(top)>FLT_MAX||!rr_results_text_build(s->font,text,s->scale,&row->name)||row->name.count!=chars->count||!rr_results_text_build(s->font,"",s->scale,&row->value))return false;
 double origin=pen(chars,first);
 for(size_t j=0;j<count;j++){
  RRResultsToken token=row->name.tokens[first+j];if(token.codepoint!=chars->ranges[first+j].codepoint)return false;
  token.pen=(float)(pen(chars,first+j)-origin);token.advance=(float)(pen(chars,first+j+1)-pen(chars,first+j));row->name.tokens[j]=token;
 }
 row->name.count=count;row->name.width=(float)(pen(chars,first+count)-origin);
 row->left=(float)x;row->value_x=(float)x;row->baseline=(float)rr_css_line_baseline(top,21.296875,15,700);row->shadow=player;
 row->name_color=value?(s->kind?(Color){255,210,90,255}:player?(Color){255,43,214,255}:(Color){153,187,221,255}):player?(Color){255,255,255,255}:s->kind?(Color){243,224,176,255}:(Color){207,239,255,255};
 row->value_color=row->name_color;row->shadow_color=s->kind?(Color){255,160,18,255}:(Color){255,43,214,255};
 return rr_results_row_emit(row,s->sink,s->context);
}
static void row_flow(void *context,const RRResultsListRowView *v)
{
 FlowSink *s=context;if(!s->ready)return;const RRResultsFlowText *rows=s->kind?s->input->coins:s->input->standings;const RRUIRect box=s->body->list_boxes[s->kind];bool player=s->player[s->kind][v->index];
 for(size_t j=0;j<v->name->count;j++){
  const RRResultsNameLine *line=v->name->lines+j;
  if(!fragment(s,rows[v->index].name,v->name_characters,line->first,line->count,(double)box.x+26+line->x,(double)box.y+v->y+line->y-1,false,player)){s->ready=false;return;}
 }
 s->ready=fragment(s,rows[v->index].value,v->value_characters,0,v->value_characters->count,(double)box.x+26+v->value.x,(double)box.y+v->value.y,true,player);
}
RRResultsNameStatus rr_results_flow_emit(Font font,const RRResultsBodyInput *input,const bool *standings_player,const bool *coin_player,int scale,RRResultsListWork *work,RRResultsRow *row,RRResultsPaintSink sink,void *context)
{
 if(!input||!work||!row||!sink||scale<1||scale>3||(input->standings_count&&!standings_player)||(input->coin_count&&!coin_player))return RR_NAME_INVALID;
 RRResultsBodyLayout body;RRResultsNameStatus status=rr_results_body_flow(input,work,&body);if(status!=RR_NAME_READY)return status;
 for(int pass=0;pass<2;pass++){
  FlowSink s={font,input,{standings_player,coin_player},&body,row,pass?sink:no_paint,pass?context:NULL,scale,0,true};
  for(int k=0;k<2;k++){
   s.kind=k;RRResultsListSummary summary;
   status=rr_results_list_flow(k?RR_FLOW_COINS:RR_FLOW_STANDINGS,k?input->coins:input->standings,k?input->coin_count:input->standings_count,k?input->right_width-26:input->left_width-26,work,row_flow,&s,&summary);
   if(status!=RR_NAME_READY)return status;if(!s.ready)return RR_NAME_INVALID;
  }
 }
 return RR_NAME_READY;
}
typedef struct NativeFlow {Font font;RRResultsArt *art;} NativeFlow;
static void native_paint(void *context,const RRResultsPaint *p)
{NativeFlow *s=context;if(p->fallback<0)DrawTextCodepoint(s->font,(int)p->codepoint,p->origin,p->font_size,p->color);else DrawTexturePro(s->art->glyphs[p->fallback],p->source,p->destination,(Vector2){0},0,p->color);}
RRResultsNameStatus rr_ui_results_flow_draw(RRUI *ui,RRResultsArt *art,const RRResultsBodyInput *input,const bool *standings_player,const bool *coin_player,int scale,RRResultsListWork *work,RRResultsRow *row)
{
 if(!ui||!ui->loaded||!ui->bold.texture.id||!art||!art->ready)return RR_NAME_INVALID;
 for(int i=0;i<RR_RESULTS_ART_COUNT;i++)if(!art->glyphs[i].id||art->glyphs[i].width!=rr_results_fallback_glyphs[i].width_pixels||art->glyphs[i].height!=rr_results_fallback_glyphs[i].height_pixels)return RR_NAME_INVALID;
 NativeFlow sink={ui->bold,art};return rr_results_flow_emit(ui->bold,input,standings_player,coin_player,scale,work,row,native_paint,&sink);
}
