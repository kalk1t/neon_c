#include "rr_ai.h"
#include <limits.h>
#include "rr_ai_liveries.h"
const RRAILivery *rr_ai_livery(int index){return index>=0&&index<RR_AI_RIVALS?&rr_ai_liveries[index]:NULL;}
bool rr_ai_controls(RRAIDriver *r,const RRCar *car,const RRTrack *track,
    const RRAIPeer *peers,size_t count,RRVec3 target,double time,double dt,const RRVec3 *coin,RRInput *out)
{
    if(!r||!car||!track||!out||(count&&!peers)||count>RR_AI_RIVALS||track->sample_count<=0||r->sample<0||r->sample>=track->sample_count||!isfinite(car->speed))return false;
    double look=14+floor(fmax(0,car->speed)/14+.5);if(look>INT_MAX-r->sample)return false;
    int index=(r->sample+(int)look)%track->sample_count;RRVec3 tangent=track->tangents[index],right=rr_normalize(rr_v3(-tangent.z,0,tangent.x));
    for(size_t i=0;i<count;i++){
        if(!peers[i].alive)continue;RRVec3 relative=rr_sub(peers[i].pos,car->pos);double ahead=rr_dot(relative,car->fwd);
        if(ahead>0&&ahead<32){double lateral=rr_dot(relative,right);if(fabs(lateral)<5.5)r->dodge=lateral>0?-7:7;}
    }
    r->dodge*=exp(-1.2*dt);double gate_distance=rr_length(rr_sub(car->pos,target)),lane_k=rr_clamp((gate_distance-60)/160,0,1);
    double offset=(r->lane+sin(time*.25+r->seed)*3+r->dodge)*lane_k;RRVec3 aim=rr_add(track->samples[index],rr_scale(right,offset));aim.y+=r->lane_y*lane_k;if(coin)aim=rr_mix(aim,*coin,.85);
    RRVec3 relative=rr_sub(aim,car->pos);double dy=atan2(-relative.x,-relative.z)-car->yaw;dy=atan2(sin(dy),cos(dy));double pitch=atan2(relative.y,hypot(relative.x,relative.z));
    *out=(RRInput){1,fabs(dy)>.55?1:0,rr_clamp(-dy*2.6,-1,1),rr_clamp((pitch/.62)*1.4,-1,1),fabs(dy)<.07&&car->boost>.35&&sin(time*.7+r->seed)>.2-r->skill*.6};return true;
}
double rr_ai_progress(int sample,int lap,int next_checkpoint,int track_samples)
{
    if(track_samples<=0)return 0;double s=sample;if(next_checkpoint==0&&s>track_samples/2.0)s-=track_samples;return (double)(lap-1)*track_samples+s;
}
double rr_ai_speed_factor(double player_progress,double rival_progress,int track_samples,double road_factor)
{
    if(track_samples<=0)return road_factor;double gap=(player_progress-rival_progress)/(track_samples*.15);return road_factor*(1+rr_clamp(gap*.05,-.05,.07));
}
