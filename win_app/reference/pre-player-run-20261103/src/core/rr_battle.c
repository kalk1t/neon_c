#include "rr_battle.h"
RRVec3 rr_combat_muzzle(const RRCar *c) {
    double k=1-c->roll_time/.55;
    double spin=c->roll_time>0?-c->roll_direction*RR_PI*2*k*k*(3-2*k):0;
    double z=c->roll+spin+sin(c->t*26)*c->stun*.5;
    RRVec3 v=rr_v3(sin(z)*.4,-cos(z)*.4,-3.15);
    v=rr_v3(v.x,cos(c->pitch)*v.y-sin(c->pitch)*v.z,sin(c->pitch)*v.y+cos(c->pitch)*v.z);
    return rr_add(c->pos,rr_v3(cos(c->yaw)*v.x+sin(c->yaw)*v.z,v.y,-sin(c->yaw)*v.x+cos(c->yaw)*v.z));
}
bool rr_battle_rival_fire(RRFireLocal *l,RRLocalRacer *r,double dt,double race_time,bool active,double road_fire) {
    if(race_time<6||r->race.finished||!(r->wreck<=0)||!active)return false;
    r->fire_cd-=dt;if(r->fire_cd>0)return false;
    RRLocalRacer *best=NULL;double bd=INFINITY;
    for(size_t i=0;i<l->racer_count;i++){
        RRLocalRacer *o=l->racers[i];if(o==r||!(o->wreck<=0)||o->race.finished)continue;
        RRVec3 d=rr_sub(o->car.pos,r->car.pos);double distance=rr_length(d);
        if(distance<15||distance>230||rr_dot(d,r->car.fwd)/distance<cos(.3))continue;
        if(distance<bd){bd=distance;best=o;}
    }
    if(!best){r->fire_cd=.4;return false;}
    if(l->fx.three_warm_uuids)for(unsigned i=0;i<4;i++)(void)l->random(l->random_ctx);
    RRFireOptions options={0};options.length=22;options.width=7.5;
    if(l->count>=l->capacity)l->error=true;
    else l->shots[l->count++]=rr_fire_init(r,rr_combat_muzzle(&r->car),r->car.fwd,fmax(0,r->car.speed),best,options).shot;
    r->fire_cd=(4+l->random(l->random_ctx)*4)*road_fire;return true;
}
void rr_battle_bind(RRLocalBattle *b,RRLocalPack *p){b->pack=p;b->roster[0]=p->player;for(int i=0;i<RR_AI_RIVALS;i++)b->roster[i+1]=p->rivals+i;b->fire.player=p->player;b->fire.racers=b->roster;b->fire.racer_count=RR_LOCAL_MAX_CARS;b->fire.shake=&p->shake;}
void rr_battle_restart(RRLocalBattle *b){rr_arsenal_restart(&b->arsenal);rr_lock_init(&b->lock);b->fire.locked=NULL;rr_fire_clear(&b->fire);}
typedef struct {RRLocalBattle *battle;double dt,race_time;bool active;} StepContext;
static bool random_pair(void *p,int index,double values[2]){RRLocalBattle *b=p;(void)index;values[0]=b->fire.random(b->fire.random_ctx);values[1]=b->fire.random(b->fire.random_ctx);return true;}
static void rival_notice(void *p,RRLocalRacer *r,int index,unsigned result){StepContext *s=p;RRFireLocal *l=&s->battle->fire;(void)index;
    if(result&RR_RIVAL_SMOKE)if(!rr_fx_request(&l->fx,RR_FX_SMOKE,l->random,l->random_ctx))l->error=true;
    (void)rr_battle_rival_fire(l,r,s->dt,s->race_time,s->active,s->battle->pack->track->road->fire_interval);
}
static void coin_notice(void *p,const RRCoinCollection *event){RRLocalBattle *b=p;(void)event;if(!rr_fx_request(&b->fire.fx,RR_FX_IMPACT,b->fire.random,b->fire.random_ctx))b->fire.error=true;}
bool rr_battle_after_player(RRLocalBattle *b,RRPackBranch branch,bool active,double dt,double time,double race_time,
    const struct RRWorld *world,RRShotWorld segment,void *segment_context,RRArsenalInput input,RRPackEvents *events) {
    StepContext context={b,dt,race_time,active};
    if(!rr_pack_rivals_dispatch(b->pack,branch,dt,time,race_time,world,random_pair,b,rival_notice,&context,events))return false;
    if(branch==RR_PACK_PAUSED)return true;
    if(branch==RR_PACK_PLAYING&&active){
        if(b->pack->player->wreck<=0){RRLocalTarget targets[RR_AI_RIVALS];for(int i=0;i<RR_AI_RIVALS;i++){RRLocalRacer *r=b->roster[i+1];targets[i]=(RRLocalTarget){r->car.pos,r->car.vel,true,r->wreck<=0,r->race.finished};}
            (void)rr_lock_update(&b->lock,b->fire.player->car.pos,b->fire.aim,targets,RR_AI_RIVALS,dt);b->fire.locked=b->lock.target<0?NULL:b->roster[b->lock.target+1];input.has_lock=b->fire.locked!=NULL;input.lock_charge=b->lock.charge;
            RRArsenalEvents arsenal;rr_fire_player_step(&b->fire,&b->arsenal,dt,input,&arsenal);
        }else {b->fire.locked=NULL;b->lock.target=-1;}
    }
    rr_fire_projectiles_tick(&b->fire,dt,24,segment,segment_context);
    if(!b->fire.locked)b->lock.target=-1;
    if(!rr_pack_coins_dispatch(b->pack,coin_notice,b,&events->coins))return false;
    return !b->fire.error&&!b->fire.fx.error;
}
