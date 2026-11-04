#ifndef RR_INPUT_MIX_H
#define RR_INPUT_MIX_H
#include "rr_flight.h"
/* Browser Input.state digital keys, before platform mouse aiming. */
enum { RR_IN_W,RR_IN_S,RR_IN_D,RR_IN_RIGHT,RR_IN_A,RR_IN_LEFT,
       RR_IN_UP,RR_IN_DOWN,RR_IN_E,RR_IN_Q,RR_IN_SHIFT_LEFT,RR_IN_SHIFT_RIGHT };
typedef struct RRInputPad {bool connected,boost;double x,y,throttle,brake;} RRInputPad;
static inline RRInput rr_input_mix_capture(unsigned keys,RRInputPad pad,bool captured)
{
    RRInput in={0};
#define RR_HELD(bit) ((keys>>(bit))&1u)
    in.throttle=RR_HELD(RR_IN_W);in.brake=RR_HELD(RR_IN_S);
    in.steer=(double)RR_HELD(RR_IN_RIGHT)-RR_HELD(RR_IN_LEFT)+(captured?0:(double)RR_HELD(RR_IN_D)-RR_HELD(RR_IN_A));
    in.strafe=captured?(double)RR_HELD(RR_IN_D)-RR_HELD(RR_IN_A):0;
    in.climb=(double)RR_HELD(RR_IN_UP)-RR_HELD(RR_IN_DOWN)+RR_HELD(RR_IN_E)-RR_HELD(RR_IN_Q);
    in.boost=RR_HELD(RR_IN_SHIFT_LEFT)||RR_HELD(RR_IN_SHIFT_RIGHT);
#undef RR_HELD
    if(pad.connected){
        double x=fabs(pad.x)<.15?0:pad.x,y=fabs(pad.y)<.15?0:-pad.y;
        if(x)in.steer=x;if(y)in.climb=y;
        in.throttle=fmax(in.throttle,pad.throttle);in.brake=fmax(in.brake,pad.brake);
        in.boost=in.boost||pad.boost;
    }
    in.steer=rr_clamp(in.steer,-1,1);in.climb=rr_clamp(in.climb,-1,1);
    return in;
}
/* Legacy offline/AI fixtures have no captured mouse. */
static inline RRInput rr_input_mix(unsigned keys,RRInputPad pad){return rr_input_mix_capture(keys,pad,false);}
#endif
