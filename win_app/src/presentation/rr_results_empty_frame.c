#include "rr_results_empty_frame.h"
typedef struct EmptyFrameDraw {RRResultsEmptyFrame *state;RRUI *ui;RRResultsArt *art;double width,height,scroll;int scale;} EmptyFrameDraw;
static bool draw(void *context,const RRResultsBoundView *view)
{
 EmptyFrameDraw *f=context;RRResultsEmptyFrame *s=f->state;s->draw_attempted=true;
 s->draw_status=rr_ui_results_empty_profile_draw(f->ui,f->art,view,f->width,f->height,f->scroll,f->scale,&s->flow,&s->work,&s->row,&s->caption);
 s->caption_ready=s->draw_status==RR_NAME_READY;return s->caption_ready;
}
bool rr_results_empty_frame(RRResultsEmptyFrame *s,const RRRunCurrent *run,const RRRunEvents *tick,double now,bool hide,RRResultsNameLookup names,void *names_context,RRUI *ui,RRResultsArt *art,double width,double height,double scroll,int scale,RRResultsPump *events)
{
 if(!s){if(events)*events=(RRResultsPump){0};return false;}
 s->draw_attempted=false;s->caption_ready=false;s->draw_status=RR_NAME_INVALID;
 EmptyFrameDraw context={s,ui,art,width,height,scroll,scale};
 return rr_results_bind_frame(&s->identity,&s->timer,run,tick,now,hide,names,names_context,draw,&context,events);
}
