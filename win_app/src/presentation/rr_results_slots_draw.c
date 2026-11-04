#include "rr_results_slots_draw.h"
RRResultsNameStatus rr_ui_results_empty_profile_draw(RRUI *ui,RRResultsArt *art,const RRResultsBoundView *view,double width,double height,double scroll,int scale,RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row,RRResultsCaption *caption)
{
 if(!view||!view->results)return RR_NAME_INVALID;RRResultsCardContent slots;
 RRResultsNameStatus status=rr_results_empty_slots(view->results,&slots);if(status!=RR_NAME_READY)return status;
 RRResultsBodyInput profile={width,height,scroll,NAN,NAN,NAN,NULL,NULL,0,0,slots};
 return rr_ui_results_caption_flow_draw(ui,art,view,&profile,scale,binding,work,row,caption);
}
