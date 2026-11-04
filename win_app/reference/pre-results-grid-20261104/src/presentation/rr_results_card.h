#ifndef RR_RESULTS_CARD_H
#define RR_RESULTS_CARD_H
#include "rr_ui_layout.h"
#include <stdint.h>
#include <float.h>
typedef enum RRResultsCardSlot {
 RR_CARD_BRAND,RR_CARD_PLACE,RR_CARD_TOTAL,RR_CARD_LAPS,RR_CARD_COLUMNS,
 RR_CARD_UNLOCK,RR_CARD_ONLINE,RR_CARD_NEXT,RR_CARD_AGAIN,RR_CARD_MENU,RR_CARD_SLOTS
} RRResultsCardSlot;
enum {RR_CARD_ALL=(1u<<RR_CARD_SLOTS)-1u,
 RR_CARD_OPTIONAL=(1u<<RR_CARD_UNLOCK)|(1u<<RR_CARD_NEXT),
 RR_CARD_REQUIRED=RR_CARD_ALL^RR_CARD_OPTIONAL};
/* Qualified intrinsic BORDER-BOX heights in logical CSS pixels. This solver
 * does not measure fonts/HTML, infer word wrapping or invent account content.
 * Empty visible blocks still get gaps; display:none slots get no gap/box.
 * Current desktop fixed-width680/source padding/border/gap rules only. */
typedef struct RRResultsCardContent {double heights[RR_CARD_SLOTS];uint32_t visible;} RRResultsCardContent;
typedef struct RRResultsCardLayout {RRUIRect panel,boxes[RR_CARD_SLOTS],left,right;double content_height,max_scroll,scroll;uint32_t visible;} RRResultsCardLayout;
static inline RRResultsCardContent rr_results_card_legacy(void)
{
 RRResultsCardContent c={{32.296875,33,40,20,113.890625,0,19,0,61,61},RR_CARD_REQUIRED};return c;
}
/* Source flex-column gap9, border2+padding22/30, flex-shrink0; fullscreen
 * overflow-y:auto/safe center with16px top/bottom padding. Scroll is logical
 * pixel input, not a guessed OS wheel/focus policy. Clamp after resize/content
 * changes without shrinking content. No implicit scroll reset on publication.
 * Finite positive viewport; invalid input leaves output untouched. Horizontal
 * overflow/touch/DPI and real scrolling/painting remain separate qualification. */
static inline bool rr_results_card_flow(double width,double height,const RRResultsCardContent *content,double requested_scroll,RRResultsCardLayout *out)
{
 if(!content||!out||!isfinite(width)||!isfinite(height)||width<=0||height<=0||width>FLT_MAX||height>FLT_MAX||!isfinite(requested_scroll))return false;
 if((content->visible&~(uint32_t)RR_CARD_ALL)||(content->visible&RR_CARD_REQUIRED)!=RR_CARD_REQUIRED)return false;
 double total=48;unsigned count=0;
 for(int i=0;i<RR_CARD_SLOTS;i++){
  if(!(content->visible&(1u<<i)))continue;
  if(!isfinite(content->heights[i])||content->heights[i]<0)return false;
  total+=content->heights[i];count++;
 }
 total+=9*(double)(count-1);if(!isfinite(total)||total>FLT_MAX)return false;
 RRResultsCardLayout value={0};value.visible=content->visible;value.content_height=total;
 value.max_scroll=fmax(0,total+32-height);value.scroll=fmin(value.max_scroll,fmax(0,requested_scroll));
 double x=(width-680)*.5,top=fmax(16,(height-total)*.5)-value.scroll;
 if(fabs(x)>FLT_MAX||fabs(top)>FLT_MAX)return false;
 value.panel=(RRUIRect){(float)x,(float)top,680,(float)total};double y=top+24;
 for(int i=0;i<RR_CARD_SLOTS;i++){
  if(!(content->visible&(1u<<i)))continue;
  if(!isfinite(y)||fabs(y)>FLT_MAX)return false;
  value.boxes[i]=(RRUIRect){(float)(x+32),(float)y,616,(float)content->heights[i]};
  y+=content->heights[i]+9;
 }
 RRUIRect columns=value.boxes[RR_CARD_COLUMNS];
 value.left=(RRUIRect){columns.x,columns.y,335.734375f,columns.h};
 value.right=(RRUIRect){columns.x+335.734375f+22,columns.y,258.25f,columns.h};
 *out=value;return true;
}
#endif
