#include "rr_pack_current.h"
#include <string.h>
static RRLocalRacer *racer(RRLocalPack *p,int index){return index?&p->rivals[index-1]:p->player;}
bool rr_pack_grid(int slot,RRVec3 *out)
{if(!out||slot<0||slot>=12)return false;*out=rr_v3(slot%2?8:-8,115.5,20+(slot/2)*13+(slot%2)*5);return true;}
void rr_pack_dispose(RRLocalPack *p){if(p){for(int i=0;i<RR_AI_RIVALS;i++)rr_rival_dispose(&p->rivals[i]);p->player=NULL;p->track=NULL;}}
static int compare_rank(RRLocalPack *p,int ai,int bi)
{
    const RRLocalRacer *a=racer(p,ai),*b=racer(p,bi);
    if(a->race.finished||b->race.finished){if(a->race.finished!=b->race.finished)return a->race.finished?-1:1;return a->race.finish_time<b->race.finish_time?-1:a->race.finish_time>b->race.finish_time?1:0;}
    double ap=rr_ai_progress(a->ai.sample,a->race.lap,a->race.next_checkpoint,p->track->sample_count),bp=rr_ai_progress(b->ai.sample,b->race.lap,b->race.next_checkpoint,p->track->sample_count);return ap>bp?-1:ap<bp?1:0;
}
static void rank(RRLocalPack *p,bool current)
{
    if(!p||!p->player||!p->track)return;
    for(int i=0;i<RR_LOCAL_MAX_CARS;i++){int j=i;p->ranking[i]=i;while(j>0&&compare_rank(p,p->ranking[j],p->ranking[j-1])<0){int temp=p->ranking[j];p->ranking[j]=p->ranking[j-1];p->ranking[j-1]=temp;j--;}}
    for(int i=0;i<RR_LOCAL_MAX_CARS;i++)if(p->ranking[i]==0)p->position=i+1;
    double pp=rr_ai_progress(p->player->ai.sample,p->player->race.lap,p->player->race.next_checkpoint,p->track->sample_count);
    for(int i=0;i<RR_AI_RIVALS;i++){RRLocalRacer *r=&p->rivals[i];double rp=rr_ai_progress(r->ai.sample,r->race.lap,r->race.next_checkpoint,p->track->sample_count);r->car.max_multiplier=current?rr_ai_current_speed_factor(pp,rp,p->track->sample_count,p->track->road->id):rr_ai_speed_factor(pp,rp,p->track->sample_count,p->track->road->ai_speed);}
}
static bool start(RRLocalPack *p,const RRTrack *track,RRLocalRacer *player,RRPackRandom random,void *context,RRPackCurrent *current)
{
    if(!p||!track||!track->road||!player||!player->is_player||player->race.mode!=RR_RACE_TWO_LAPS||!random)return false;p->track=track;p->player=player;
    if(current&&!rr_ai_current_prepare(&current->corners,track))return false;
    int slot=0;for(int i=0;i<RR_AI_RIVALS;i++){if(slot==7)slot++;RRVec3 pos;double values[2];if(!rr_pack_grid(slot++,&pos)||!random(context,i+1,values)||!(current?rr_rival_lifecycle_reset(&p->rivals[i],&current->resources[i],track,pos,0,RR_RACE_TWO_LAPS,values[0],values[1]):rr_rival_reset(&p->rivals[i],track,pos,0,RR_RACE_TWO_LAPS,values[0],values[1])))return false;}
    if(current){for(int i=0;i<RR_LOCAL_MAX_CARS;i++){RRCarPose *pose=&current->poses[i];if(!(pose->ready?rr_car_pose_sync(pose,&racer(p,i)->car):rr_car_pose_init(pose,&racer(p,i)->car)))return false;}}
    if(!rr_coins_init(&p->coins,track))return false;rr_coins_visible(&p->coins,true);rank(p,current!=NULL);p->previous_player_finished=player->race.finished;p->previous_player_sample=player->ai.sample;p->coin_pending=false;return true;
}
typedef struct RandomContext {RRPackRandom random;void *context;int index;} RandomContext;
static bool rival_random(void *context,double values[2]){RandomContext *c=context;return c->random&&c->random(c->context,c->index,values);}
static bool dispatch(RRLocalPack *p,RRPackBranch branch,double dt,double time,double race_time,const struct RRWorld *world,RRPackRandom random,void *context,RRPackRivalNotice notice,void *notice_context,RRPackEvents *events,RRPackCurrent *current)
{
    if(!p||!p->track||!p->player||!events||p->coin_pending||!isfinite(dt)||dt<=0||dt>.05||branch<RR_PACK_IDLE||branch>RR_PACK_PAUSED)return false;*events=(RRPackEvents){0};if(branch==RR_PACK_PAUSED)return true;p->coin_pending=true;p->pending_branch=branch;p->pending_dt=dt;
    if(branch==RR_PACK_COUNTDOWN)for(int i=0;i<RR_AI_RIVALS;i++){if(current){double before=p->rivals[i].car.speed;rr_car_step_current_world_report(&p->rivals[i].car,(RRInput){0},dt,world,NULL);if(!rr_car_pose_after_step(&current->poses[i+1],&p->rivals[i].car,before)){events->error=true;return false;}}else rr_car_step_world(&p->rivals[i].car,(RRInput){0},dt,world);}
    if(branch==RR_PACK_PLAYING){
        if(p->player->race.finished&&!p->previous_player_finished){
            /* Source completeLap ranks before raceRules refreshes player.sample. */
            int sample=p->player->ai.sample;p->player->ai.sample=p->previous_player_sample;rank(p,current!=NULL);p->player->ai.sample=sample;p->finish_position=p->position;events->ranked=true;
        }
        for(int i=0;i<RR_AI_RIVALS;i++){
            RRLocalRacer *r=&p->rivals[i];RRInput input={0};RRAIPeer peers[RR_AI_RIVALS];int count=0,coin=-1;
            RRVec3 target=r->race.next_checkpoint<p->track->gate_count?p->track->gates[r->race.next_checkpoint].pos:rr_v3(0,126,0);
            if(race_time>r->react){for(int k=0;k<RR_LOCAL_MAX_CARS;k++)if(k!=i+1){const RRLocalRacer *other=racer(p,k);peers[count++]=(RRAIPeer){other->car.pos,other->wreck<=0};}coin=rr_coins_ahead(&p->coins,&r->car,90);}
            RandomContext rng={random,context,i+1};
            if(current){bool alive=r->wreck<=0;double before=r->car.speed;if(!rr_rival_lifecycle_drive(r,p->track,&current->corners,world,peers,(size_t)count,target,coin>=0?&p->coins.list[coin].pos:NULL,time,dt,race_time,34,rival_random,&rng,&input,&events->rival[i])){events->error=true;return false;}if(alive?!rr_car_pose_after_step(&current->poses[i+1],&r->car,before):(events->rival[i]&RR_RIVAL_RESPAWN)&&!rr_car_pose_sync(&current->poses[i+1],&r->car)){events->error=true;return false;}}
            else{if(race_time>r->react&&!rr_ai_controls(&r->ai,&r->car,p->track,peers,(size_t)count,target,time,dt,coin>=0?&p->coins.list[coin].pos:NULL,&input)){events->error=true;return false;}events->rival[i]=rr_rival_step_random(r,p->track,world,input,dt,race_time,34,rival_random,&rng);}
            if(events->rival[i]&RR_RIVAL_ERROR){events->error=true;return false;}if(notice)notice(notice_context,r,i+1,events->rival[i]);
        }
        RRLocalRacer contact_view[RR_LOCAL_MAX_CARS];for(int i=0;i<RR_LOCAL_MAX_CARS;i++)contact_view[i]=*racer(p,i);
        if(!(current?rr_local_car_contacts_current:rr_local_car_contacts)(contact_view,RR_LOCAL_MAX_CARS,p->shake,&events->contacts))return false;p->shake=events->contacts.shake;for(int i=0;i<RR_LOCAL_MAX_CARS;i++)racer(p,i)->car=contact_view[i].car;
        p->rank_timer-=dt;if(p->rank_timer<=0){rank(p,current!=NULL);events->ranked=true;p->rank_timer=.2;}
    }
    return true;
}
bool rr_pack_coins_dispatch(RRLocalPack *p,RRCoinNotice notice,void *context,RRCoinEvents *events)
{
    if(!p||!p->player||!p->track||!events||!p->coin_pending)return false;
    RRLocalRacer view[RR_LOCAL_MAX_CARS];for(int i=0;i<RR_LOCAL_MAX_CARS;i++)view[i]=*racer(p,i);
    if(!rr_coins_update_dispatch(&p->coins,p->pending_dt,view,RR_LOCAL_MAX_CARS,p->pending_branch==RR_PACK_PLAYING,notice,context,events))return false;
    for(int i=0;i<RR_LOCAL_MAX_CARS;i++)racer(p,i)->coins=view[i].coins;p->previous_player_finished=p->player->race.finished;p->previous_player_sample=p->player->ai.sample;p->coin_pending=false;return true;
}

bool rr_pack_tick_after_player(RRLocalPack *p,RRPackBranch branch,double dt,double time,double race_time,const struct RRWorld *world,RRPackRandom random,void *context,RRPackEvents *events)
{
    if(!rr_pack_rivals_after_player(p,branch,dt,time,race_time,world,random,context,events))return false;
    return !p->coin_pending||rr_pack_coins_after_weapons(p,&events->coins);
}

bool rr_pack_rivals_after_player(RRLocalPack *p,RRPackBranch branch,double dt,double time,double race_time,const struct RRWorld *world,RRPackRandom random,void *context,RRPackEvents *events) {
    return rr_pack_rivals_dispatch(p,branch,dt,time,race_time,world,random,context,NULL,NULL,events);
}
bool rr_pack_coins_after_weapons(RRLocalPack *p,RRCoinEvents *events) {return rr_pack_coins_dispatch(p,NULL,NULL,events);}

void rr_pack_rank(RRLocalPack *p){rank(p,false);}
bool rr_pack_start(RRLocalPack *p,const RRTrack *t,RRLocalRacer *player,RRPackRandom random,void *ctx){return start(p,t,player,random,ctx,NULL);}
bool rr_pack_rivals_dispatch(RRLocalPack *p,RRPackBranch branch,double dt,double time,double race_time,const struct RRWorld *w,RRPackRandom random,void *ctx,RRPackRivalNotice notice,void *notice_ctx,RRPackEvents *events){return dispatch(p,branch,dt,time,race_time,w,random,ctx,notice,notice_ctx,events,NULL);}
static bool current_valid(const RRPackCurrent *c){return c&&c->pack&&c->pack->track==c->corners.track&&!isnan(rr_ai_current_corner_limit(&c->corners,0,0));}
bool rr_pack_current_start(RRPackCurrent *c,RRLocalPack *p,const RRTrack *t,RRLocalRacer *player,RRPackRandom random,void *ctx){if(!c)return false;c->pack=NULL;if(!start(p,t,player,random,ctx,c))return false;c->pack=p;return true;}
bool rr_pack_current_rank(RRPackCurrent *c){if(!current_valid(c))return false;rank(c->pack,true);return true;}
bool rr_pack_current_rivals_dispatch(RRPackCurrent *c,RRPackBranch branch,double dt,double time,double race_time,const struct RRWorld *w,RRPackRandom random,void *ctx,RRPackRivalNotice notice,void *notice_ctx,RRPackEvents *events){if(!current_valid(c))return false;return dispatch(c->pack,branch,dt,time,race_time,w,random,ctx,notice,notice_ctx,events,c);}
