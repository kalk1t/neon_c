#include "rr_results_tracks_draw.h"
RRResultsNameStatus rr_ui_results_tracks_draw(RRUI *ui,RRResultsArt *art,const RRResultsBoundView *view,const RRResultsBodyInput *profile,const double other[2],int scale,RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row)
{
 if(!profile||!work||!rr_results_flow_prepare(view,binding))return RR_NAME_INVALID;
 RRResultsTracks tracks;RRResultsNameStatus status=rr_results_tracks(binding->rows[0],binding->counts[0],binding->rows[1],binding->counts[1],other,&work->name_characters,&tracks);if(status!=RR_NAME_READY)return status;
 RRResultsBodyInput input=*profile;input.left_width=tracks.width[0];input.right_width=tracks.width[1];input.standings=binding->rows[0];input.coins=binding->rows[1];input.standings_count=binding->counts[0];input.coin_count=binding->counts[1];
 return rr_ui_results_flow_draw(ui,art,&input,binding->player[0],binding->player[1],scale,work,row);
}
