#include "rr_results_body_flow.h"
#include <math.h>
#include <float.h>
static void shift(RRUIRect *r,double y){r->y=(float)((double)r->y+y);}
RRResultsNameStatus rr_results_body_flow(const RRResultsBodyInput *i,RRResultsListWork *w,RRResultsBodyLayout *out)
{
 if(!i||!w||!out||!isfinite(i->width)||!isfinite(i->height)||i->width<=0||i->height<=0||i->width>FLT_MAX||i->height>FLT_MAX||!isfinite(i->scroll)||!isfinite(i->left_width)||!isfinite(i->right_width)||!isfinite(i->caption_height)||i->left_width<=26||i->right_width<=26||i->caption_height<=0||i->caption_height>FLT_MAX)return RR_NAME_INVALID;
 if(floor(i->width)!=i->width||floor(i->height)!=i->height||!i->coin_count)return RR_NAME_NEEDS_FLOW;
 RRResultsBodyLayout value={0};RRResultsNameStatus status=rr_results_list_flow(RR_FLOW_STANDINGS,i->standings,i->standings_count,i->left_width-26,w,NULL,NULL,&value.lists[0]);if(status!=RR_NAME_READY)return status;
 status=rr_results_list_flow(RR_FLOW_COINS,i->coins,i->coin_count,i->right_width-26,w,NULL,NULL,&value.lists[1]);if(status!=RR_NAME_READY)return status;
 double bank_y=25+value.lists[1].height+8;
 value.metrics=(RRResultsGridMetrics){i->left_width,i->right_width,19+value.lists[0].height,fmax(bank_y+i->caption_height,25+value.lists[1].float_bottom)};
 RRResultsCardContent content=i->other;content.heights[RR_CARD_COLUMNS]=fmax(value.metrics.left_natural,value.metrics.right_natural);
 if(!rr_results_card_flow_intrinsic(i->width,i->height,&content,&value.metrics,0,&value.card))return RR_NAME_INVALID;
 /* Chromium's measured integer scrollHeight and 1/64 flex positioning. Keep
  * the legacy continuous card solver intact for its existing callers. */
 value.card.max_scroll=fmax(0,floor(value.card.content_height+32+.5)-i->height);
 value.card.scroll=fmin(value.card.max_scroll,fmax(0,i->scroll));
 double top=floor(fmax(16,(i->height-value.card.content_height)*.5)*64)/64-value.card.scroll;
 double delta=top-value.card.panel.y;if(!isfinite(top)||fabs(top)>FLT_MAX)return RR_NAME_INVALID;
 shift(&value.card.panel,delta);shift(&value.card.left,delta);shift(&value.card.right,delta);
 for(int slot=0;slot<RR_CARD_SLOTS;slot++)if(value.card.visible&(1u<<slot))shift(value.card.boxes+slot,delta);
 value.labels[0]=(RRUIRect){value.card.left.x,value.card.left.y,value.card.left.w,19};
 value.labels[1]=(RRUIRect){value.card.right.x,value.card.right.y,value.card.right.w,19};
 value.list_boxes[0]=(RRUIRect){value.card.left.x,(float)(value.card.left.y+19),value.card.left.w,(float)value.lists[0].height};
 value.list_boxes[1]=(RRUIRect){value.card.right.x,(float)(value.card.right.y+25),value.card.right.w,(float)value.lists[1].height};
 value.caption=(RRUIRect){value.card.right.x,(float)(value.card.right.y+bank_y),value.card.right.w,(float)i->caption_height};
 *out=value;return RR_NAME_READY;
}
