#include "rr_recovery.h"
unsigned rr_recovery_conditions(RRRecovery *s,double along,double off,
    double height,double floor_height,double dt,double message_time,bool roam)
{
    unsigned events=0;
    s->wrong_way=along < -15?s->wrong_way+dt:0;
    if(s->wrong_way>1.2 && message_time<=0 && !roam)events|=RR_RECOVERY_WRONG_WAY;
    if(!roam && (height<floor_height || off>650)){
        s->lost_time+=dt;
        if(s->lost_time>0.05 && s->lost_time-dt<=0.05)events|=RR_RECOVERY_SIGNAL_LOST;
        if(s->lost_time>1.1){events|=RR_RECOVERY_RESPAWN;s->lost_time=0;}
    }else s->lost_time=0;
    return events;
}
void rr_recovery_rebase(RRRecovery *s,const RRTrack *track,RRVec3 pos)
{
    s->sample=rr_track_nearest(track,pos);s->lost_time=0;
}
void rr_recovery_init(RRRecovery *s,const RRTrack *track,RRVec3 pos)
{
    *s=(RRRecovery){0};rr_recovery_rebase(s,track,pos);
}
unsigned rr_recovery_update(RRRecovery *s,const RRTrack *track,RRVec3 pos,
    RRVec3 velocity,double floor_height,double dt,double message_time,bool roam)
{
    s->sample=rr_track_nearest_local(track,pos,s->sample);
    double along=rr_dot(velocity,track->tangents[s->sample]);
    double off=rr_length(rr_sub(pos,track->samples[s->sample]));
    return rr_recovery_conditions(s,along,off,pos.y,floor_height,dt,message_time,roam);
}
