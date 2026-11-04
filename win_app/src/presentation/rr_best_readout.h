#ifndef RR_BEST_READOUT_H
#define RR_BEST_READOUT_H
#include "rr_race_ui.h"
#include "rr_road_picker.h"
/* Read-only view of the host's existing finite, positive per-road lap cache.
 * No persistence, account/profile import or progression mutation. */
static inline double rr_ui_best_value(const RRRoadPicker *picker,int road)
{return picker&&road>=1&&road<=RR_ROAD_COUNT?picker->rows[road-1].best_lap:0;}
static inline void rr_ui_best_clock(char out[32],const RRRoadPicker *picker,int road)
{double best=rr_ui_best_value(picker,road);rr_ui_time(out,best>0?best:NAN);}
static inline void rr_ui_best_row(char out[64],const RRRoadPicker *picker,int road)
{
    if(picker&&road>picker->open_count){snprintf(out,64,"WIN ROAD %d IN 1ST",road-1);return;}
    if(rr_ui_best_value(picker,road)>0){char clock[32];rr_ui_best_clock(clock,picker,road);snprintf(out,64,"BEST LAP %s",clock);}
    else snprintf(out,64,"NO LAP YET");
}
#endif
