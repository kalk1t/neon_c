#include "rr_finish.h"
RRInput rr_finish_controls(const RRTrack *track,const RRCar *car,int sample)
{
    if(track->sample_count<=0)return (RRInput){0};
    int index=((sample%track->sample_count)+track->sample_count)%track->sample_count;
    RRVec3 to=rr_sub(track->samples[(index+16)%track->sample_count],car->pos);
    double dy=atan2(-to.x,-to.z)-car->yaw;dy=atan2(sin(dy),cos(dy));
    double pitch=atan2(to.y,hypot(to.x,to.z));
    return (RRInput){1,fabs(dy)>.5?1:0,rr_clamp(-dy*3,-1,1),rr_clamp(pitch/.62*1.4,-1,1),fabs(dy)<.08};
}
RRResultSummary rr_finish_summary(const double *laps,size_t count)
{
    RRResultSummary result={0,0};
    for(size_t i=0;i<count;i++){result.total+=laps[i];if(laps[i]<laps[result.best])result.best=i;}
    return result;
}
