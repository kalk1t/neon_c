#include "rr_current_empty_host.h"
bool rr_current_empty_host(RRResultsEmptyFrame *results,RRRunInputState *inputs,RRRunCurrent *run,const RRCurrentEmptyRequest *request,const struct RRWorld *world,RRShotWorld segment,void *segment_context,RRResultsNameLookup names,void *names_context,RRUI *ui,RRResultsArt *art,RRCurrentEmptyEvents *events)
{
 if(!events)return false;*events=(RRCurrentEmptyEvents){0};if(!results||!inputs||!run||!request)return false;
 results->draw_attempted=false;results->caption_ready=false;results->draw_status=RR_NAME_INVALID;
 if(request->simulate){events->tick_attempted=true;events->tick_ok=rr_run_input_tick(inputs,run,request->input,request->mouse,request->dt,world,segment,segment_context,&events->tick);if(!events->tick_ok)return false;}
 events->presentation_attempted=true;
 events->presentation_ok=rr_results_empty_frame(results,run,request->simulate?&events->tick:NULL,request->wall_seconds,request->hide,names,names_context,ui,art,request->width,request->height,request->scroll,request->device_scale,&events->results);
 return events->presentation_ok;
}
