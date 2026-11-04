#ifndef RR_SPEED_UNITS_H
#define RR_SPEED_UNITS_H
#include <math.h>
#include <stdbool.h>
typedef enum RRSpeedUnit { RR_SPEED_KMH,RR_SPEED_MPH } RRSpeedUnit;
static inline double rr_speed_factor(RRSpeedUnit u){return u==RR_SPEED_MPH?2.23694:3.6;}
static inline const char *rr_speed_label(RRSpeedUnit u){return u==RR_SPEED_MPH?"mph":"km/h";}
static inline int rr_speed_readout(double meters_per_second,RRSpeedUnit u){return isfinite(meters_per_second)?(int)fmin(99999,floor(fabs(meters_per_second)*rr_speed_factor(u)+.5)):0;}
/* Native two-option adapter: Home/negative chooses first, End/positive last;
 * command3 is an explicit toggle. Device-local/session-only, no account calls. */
static inline RRSpeedUnit rr_speed_select(RRSpeedUnit u,int command){return command==3?(u==RR_SPEED_MPH?RR_SPEED_KMH:RR_SPEED_MPH):command<0?RR_SPEED_KMH:command>0?RR_SPEED_MPH:u;}
#endif
