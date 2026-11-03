#include "rr_coins.h"
#include <stdlib.h>
static int compare_int(const void *a,const void *b){int x=*(const int *)a,y=*(const int *)b;return x<y?-1:x>y?1:0;}
bool rr_coins_init(RRCoins *coins,const RRTrack *track)
{
    if(!coins||!track||track->sample_count<=0||track->gate_count<0||track->gate_count>RR_TRACK_MAX_GATES)return false;
    int ends[RR_TRACK_MAX_GATES+1];for(int i=0;i<track->gate_count;i++){ends[i]=track->gate_samples[i];if(ends[i]<0||ends[i]>=track->sample_count)return false;}ends[track->gate_count]=track->sample_count;qsort(ends,(size_t)track->gate_count+1,sizeof(*ends),compare_int);
    *coins=(RRCoins){0};int previous=0,n=track->sample_count;
    for(int gap=0;gap<=track->gate_count;gap++){
        int end=ends[gap],span=(end-previous+n)%n;double lane=(coins->count%3-1)*7;
        for(int k=0;k<RR_COINS_PER_GAP;k++){
            int index=(int)floor(previous+span*(.3+.4*((double)k/(RR_COINS_PER_GAP-1)))+.5)%n;RRVec3 t=track->tangents[index],right=rr_normalize(rr_v3(-t.z,0,t.x));
            RRVec3 pos=rr_add(track->samples[index],rr_scale(right,lane+(k%2?7:-7)));pos.y+=1+cos(k*.7)*1.5;
            coins->list[coins->count++]=(RRLocalCoin){pos,index,true,0,0};
        }previous=end;
    }return true;
}
void rr_coins_reset(RRCoins *coins)
{
    if(!coins)return;for(int i=0;i<coins->count;i++){coins->list[i].active=true;coins->list[i].timer=coins->list[i].pop=0;}
}
void rr_coins_visible(RRCoins *coins,bool visible){if(coins)coins->hidden=!visible;}
int rr_coins_ahead(const RRCoins *coins,const RRCar *car,double max_distance)
{
    if(!coins||!car||!isfinite(max_distance)||max_distance<0)return -1;int best=-1;double bd=max_distance*max_distance;
    for(int i=0;i<coins->count;i++){
        const RRLocalCoin *c=&coins->list[i];if(!c->active)continue;RRVec3 d=rr_sub(c->pos,car->pos);double along=rr_dot(d,car->fwd),distance=rr_dot(d,d);
        if(along<15||distance>bd||along*along<distance*.85)continue;bd=distance;best=i;
    }return best;
}
bool rr_coins_update(RRCoins *coins,double dt,RRLocalRacer *racers,size_t count,bool active,RRCoinEvents *events)
{
    if(!coins||!events||(count&&!racers)||count>RR_LOCAL_MAX_CARS||!isfinite(dt)||dt<0)return false;events->count=0;
    for(int i=0;i<coins->count;i++){
        RRLocalCoin *c=&coins->list[i];
        if(!c->active){c->timer-=dt;if(c->timer<=0){c->active=true;c->pop=0;}}
        else if(active)for(size_t j=0;j<count;j++){
            RRLocalRacer *r=&racers[j];if(!(r->wreck<=0)||r->race.finished)continue;RRVec3 d=rr_sub(r->car.pos,c->pos);
            if(rr_dot(d,d)<6.5*6.5){c->active=false;c->timer=6;r->coins++;events->list[events->count++]=(RRCoinCollection){i,(int)j};break;}
        }
        c->pop=fmin(1,c->pop+dt*3);
    }return true;
}
