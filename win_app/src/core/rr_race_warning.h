#ifndef RR_RACE_WARNING_H
#define RR_RACE_WARNING_H
#include "rr_race.h"
enum {RR_RACE_MISSED_GATE_WARNING=32};
#define RR_MISSED_GATE_SECONDS 1.4
/* Browser raceRules notification only. Ordered native progress equals passed.size.
 * Evaluate after the race step; accepted laps reset progress and do not warn. */
static inline bool rr_race_missed_gate(const RRRace *race,int checkpoint_count,
    const RRCar *car,RRVec3 previous,double race_time)
{
    return !race->finished&&race->mode!=RR_RACE_FREE_ROAM&&race_time>5&&
        race->next_checkpoint>0&&race->next_checkpoint<checkpoint_count&&
        previous.z>0&&car->pos.z<=0&&fabs(car->pos.x)<23&&car->pos.y>109&&car->pos.y<142;
}
#endif
