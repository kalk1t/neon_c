#include "rr_results_caption_draw.h"
RRResultsNameStatus rr_ui_results_caption_flow_draw(RRUI *ui,RRResultsArt *art,const RRResultsBoundView *view,const RRResultsBodyInput *profile,int scale,RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row,RRResultsCaption *out)
{
 if(!view||!view->results||!profile||!out)return RR_NAME_INVALID;
 const RRResultsCurrent *s=view->results;RRResultsCaption caption;RRResultsNameStatus status=rr_results_caption_build(s,258.25,&caption);if(status!=RR_NAME_READY)return status;
 if(s->coins[s->player_coin_rank-1].racer<0||(size_t)s->coins[s->player_coin_rank-1].racer!=view->player)return RR_NAME_INVALID;
 RRResultsBodyInput input=*profile;input.caption_height=caption.height;const double bounds[2]={258.25,258.25};
 status=rr_ui_results_tracks_draw(ui,art,view,&input,bounds,scale,binding,work,row);if(status==RR_NAME_READY)*out=caption;return status;
}
