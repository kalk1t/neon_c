#ifndef RR_SPIN_CONTROL_H
#define RR_SPIN_CONTROL_H
#include "rr_arsenal.h"
#include "rr_flight.h"
typedef struct RRSpinResult {bool accepted,denied;int side;unsigned random_draws;} RRSpinResult;
/* Isolated bridge to the already source-tested arsenal's spin rule. Other
 * weapon inputs are zero and no production weapon resources are activated.
 * Call after movement/race rules; a finish frame skips cooldown and the press. */
static inline RRSpinResult rr_spin_control_step(double *cooldown,bool active,double dt,bool hit,double steer,RRArsenalRandom random,void *context)
{
    RRSpinResult result={0};if(!active)return result;
    RRArsenal resources={0};resources.spin_cd=*cooldown;
    RRArsenalInput input={0};input.spin_hit=hit;input.steer=steer;
    RRArsenalEvents events={0};rr_arsenal_step(&resources,dt,input,random,context,&events);
    *cooldown=resources.spin_cd;result.side=events.spin_side;result.random_draws=events.random_draws;
    for(unsigned i=0;i<events.count;i++){if(events.order[i]==RR_ARSENAL_SPIN)result.accepted=true;if(events.order[i]==RR_ARSENAL_DENY)result.denied=true;}
    return result;
}
static inline void rr_spin_control_apply(RRSpinResult result,RRCar *car,double *kick)
{if(result.accepted){rr_car_dodge(car,result.side);*kick=fmax(*kick,.6);}}
static inline void rr_spin_control_apply_current(RRSpinResult result,RRCar *car,double *kick)
{if(result.accepted){rr_car_dodge_current(car,result.side);*kick=fmax(*kick,.6);}}
#endif
