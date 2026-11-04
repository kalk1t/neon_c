#include "rr_targeting.h"
void rr_lock_init(RRLocalLock *state){if(state)*state=(RRLocalLock){-1,0};}
bool rr_target_angle(RRVec3 player,RRVec3 aim,const RRLocalTarget *target,double max_distance,double *angle)
{
    if(!target||!angle||!target->member||!target->alive||target->finished||!isfinite(max_distance)||max_distance<0)return false;
    RRVec3 delta=rr_sub(target->pos,player);double distance=rr_length(delta);if(!isfinite(distance)||distance>max_distance||distance<8)return false;
    double value=acos(rr_clamp(rr_dot(delta,aim)/distance,-1,1));if(!isfinite(value))return false;*angle=value;return true;
}
unsigned rr_lock_angles(RRLocalLock *state,const double *acquire,int count,bool keep_valid,double keep_angle,double dt)
{
    if(!state||(count&&!acquire)||count<0||count>RR_AI_RIVALS||!isfinite(dt)||dt<0)return 0;unsigned events=0;
    bool keep=state->target>=0&&state->target<count&&keep_valid&&keep_angle<.5;
    if(!keep){int best=-1;double ba=.3;for(int i=0;i<count;i++){double a=acquire[i];if(a>=0&&a<ba){ba=a;best=i;}}
        if(best!=state->target){state->target=best;state->charge=0;if(best>=0)events|=RR_LOCK_ACQUIRE;}}
    if(state->target<0){state->charge=0;return events;}double was=state->charge;state->charge=fmin(1,state->charge+dt/.5);if(was<1&&state->charge>=1)events|=RR_LOCK_READY;return events;
}
unsigned rr_lock_update(RRLocalLock *state,RRVec3 player,RRVec3 aim,const RRLocalTarget *targets,int count,double dt)
{
    if(!state||(count&&!targets)||count<0||count>RR_AI_RIVALS)return 0;double acquire[RR_AI_RIVALS];for(int i=0;i<count;i++)if(!rr_target_angle(player,aim,&targets[i],400,&acquire[i]))acquire[i]=-1;
    double keep_angle=0;bool keep=state->target>=0&&state->target<count&&rr_target_angle(player,aim,&targets[state->target],460,&keep_angle);return rr_lock_angles(state,acquire,count,keep,keep_angle,dt);
}
bool rr_target_lead(RRVec3 from,const RRLocalTarget *target,double speed,RRVec3 *direction)
{
    if(!target||!direction||!isfinite(speed)||speed<=0)return false;double time=rr_length(rr_sub(from,target->pos))/speed;*direction=rr_normalize(rr_sub(rr_add(target->pos,rr_scale(target->vel,time*.9)),from));return isfinite(direction->x)&&isfinite(direction->y)&&isfinite(direction->z);
}
