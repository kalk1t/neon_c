#include "rr_results_bind.h"
#include <string.h>
bool rr_results_bind_view(const RRResultsBinding *b,RRResultsBoundView *out)
{
 if(!b||!out||!b->owner||!b->ready||b->active>1||!b->owner->visible||!b->owner->has_snapshot||b->owner->error)return false;
 const RRResultsIdentities *ids=b->buffers+b->active;
 if(ids->revision!=b->owner->revision||!ids->count||ids->count>RR_LOCAL_MAX_CARS)return false;
 RRResultsBoundView v={0};v.results=&b->owner->snapshot;v.count=ids->count;v.revision=ids->revision;v.player=0;
 for(size_t i=0;i<v.count;i++)v.names[i]=ids->names[i];*out=v;return true;
}
typedef struct BindFrame {RRResultsBinding *binding;RRResultsTimer *timer;const RRRunCurrent *run;RRResultsNameLookup names;void *names_context;RRResultsBoundDraw draw;void *draw_context;} BindFrame;
static bool publish(void *context,const RRResultsCurrent *snapshot,size_t revision)
{
 BindFrame *f=context;RRResultsBinding *b=f->binding;
 if(snapshot!=&f->timer->snapshot||revision!=f->timer->revision){b->status=RR_RESULTS_BIND_CORE_ERROR;return false;}
 if(revision!=b->attempted_revision){
  b->attempted_revision=revision;b->ready=false;
  size_t count=snapshot->coin_count;
  if(!f->names||!count||count>RR_LOCAL_MAX_CARS||!f->run->host.battle){b->status=RR_RESULTS_BIND_IDENTITY_ERROR;return false;}
  unsigned stage=1u-b->active;RRResultsIdentities *ids=b->buffers+stage;
  for(size_t i=0;i<count;i++){
   const RRLocalRacer *r=f->run->host.battle->roster[i];if(!r){b->status=RR_RESULTS_BIND_IDENTITY_ERROR;return false;}
   const char *name=f->names(f->names_context,i,r);if(!name){b->status=RR_RESULTS_BIND_IDENTITY_ERROR;return false;}
   size_t n=0;while(n<RR_RESULTS_NAME_BYTES&&name[n])n++;
   if(n==RR_RESULTS_NAME_BYTES){b->status=RR_RESULTS_BIND_IDENTITY_ERROR;return false;}
   memcpy(ids->names[i],name,n+1);
  }
  ids->count=count;ids->revision=revision;b->active=stage;b->ready=true;
 }
 RRResultsBoundView view;
 if(!rr_results_bind_view(b,&view)){b->status=RR_RESULTS_BIND_IDENTITY_ERROR;return false;}
 if(!f->draw||!f->draw(f->draw_context,&view)){b->status=RR_RESULTS_BIND_DRAW_ERROR;return false;}
 b->status=RR_RESULTS_BIND_READY;return true;
}
bool rr_results_bind_frame(RRResultsBinding *b,RRResultsTimer *timer,const RRRunCurrent *run,const RRRunEvents *tick,double now,bool hide,RRResultsNameLookup names,void *names_context,RRResultsBoundDraw draw,void *draw_context,RRResultsPump *events)
{
 if(!events)return false;*events=(RRResultsPump){0};if(!b||!timer||!run)return false;
 if((b->owner&&b->owner!=timer)||b->active>1||timer->revision<b->attempted_revision){b->status=RR_RESULTS_BIND_OWNER_ERROR;return false;}
 b->owner=timer;b->status=RR_RESULTS_BIND_IDLE;
 BindFrame frame={b,timer,run,names,names_context,draw,draw_context};
 bool ok=rr_results_ui_frame(timer,run,tick,now,hide,publish,&frame,events);
 if(!ok&&b->status==RR_RESULTS_BIND_IDLE)b->status=RR_RESULTS_BIND_CORE_ERROR;return ok;
}
