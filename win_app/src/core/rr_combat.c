#include "rr_combat.h"
#include <string.h>
static bool alive(const RRLocalRacer *r) {return r->wreck<=0;}
unsigned rr_combat_damage(RRLocalRacer *r,double amount,RRShotKind kind,bool active) {
    if(!alive(r)||r->race.finished||!active)return 0;
    if(r->car.roll_time>0)return RR_DAMAGE_DODGED;
    r->hp-=amount;
    const double keep=kind==RR_SHOT_BOLT?.985:kind==RR_SHOT_MISSILE?.5:kind==RR_SHOT_EMP?.65:.6;
    const double stun=kind==RR_SHOT_BOLT?.12:kind==RR_SHOT_MISSILE?1:kind==RR_SHOT_EMP?1.6:.7;
    r->car.speed*=keep;r->car.stun=fmax(r->car.stun,stun);
    if(r->hp<=0) {
        r->wreck=2.4;r->hp=0;r->visible=false;r->car.speed=0;r->car.vel=rr_v3(0,0,0);
        return RR_DAMAGE_APPLIED|RR_DAMAGE_WRECK;
    }
    return RR_DAMAGE_APPLIED;
}
unsigned rr_combat_shot_damage(const RRShot *s,RRLocalRacer *r,bool active) {
    return rr_combat_damage(r,s->has_damage?s->damage:r->is_player?30:51,s->kind,active);
}
size_t rr_combat_emp(RRVec3 origin,RRLocalRacer **racers,size_t count,bool active,
                     RRDamageNotice notice,void *ctx) {
    size_t n=0;
    for(size_t i=0;i<count;++i) {
        RRLocalRacer *r=racers[i];if(r->is_player||!alive(r)||r->race.finished)continue;
        RRVec3 delta=rr_sub(r->car.pos,origin);double d=rr_length(delta);if(d>95)continue;
        ++n;r->fire_cd+=3;
        r->car.vel=rr_add(r->car.vel,rr_scale(rr_normalize(delta),30*(1-d/95)+8));
        unsigned result=rr_combat_damage(r,25*(1-.5*d/95),RR_SHOT_EMP,active);
        if(notice)notice(ctx,r,result);
    }
    return n;
}
static void erase(RRShot *shots,size_t *count,size_t i) {
    memmove(shots+i,shots+i+1,(*count-i-1)*sizeof(*shots));--*count;
}
void rr_shots_step(RRShot *shots,size_t *count,RRLocalRacer **racers,size_t nr,
                   double dt,double floor_height,RRShotWorld world,void *world_ctx,
                   RRShotNotice notice,void *ctx) {
    for(size_t i=*count;i-->0;) {
        RRShot *p=shots+i;RRLocalRacer *tg=p->target;
        if(p->accel!=0)p->speed=fmin(p->max_speed,p->speed+p->accel*dt);
        if(tg&&alive(tg)) {
            RRVec3 rel=rr_sub(tg->car.pos,p->pos),dir=rr_normalize(p->vel);
            if(p->always||rr_dot(rel,dir)>0) {
                dir=rr_normalize(rr_mix(dir,rr_normalize(rel),1-exp(-p->turn*dt)));
                p->vel=rr_scale(dir,p->speed);
            }
        } else if(p->accel!=0)p->vel=rr_scale(rr_normalize(p->vel),p->speed);
        RRVec3 prev=p->pos;p->pos=rr_add(p->pos,rr_scale(p->vel,dt));p->life-=dt;
        RRLocalRacer *victim=NULL;RRVec3 seg=rr_sub(p->pos,prev);double len2=rr_dot(seg,seg);if(len2==0)len2=1;
        for(size_t j=0;j<nr;++j) {
            RRLocalRacer *r=racers[j];if(r==p->owner||!alive(r)||r->race.finished)continue;
            double t=rr_clamp(rr_dot(rr_sub(r->car.pos,prev),seg)/len2,0,1);
            RRVec3 rel=rr_sub(rr_add(prev,rr_scale(seg,t)),r->car.pos);
            if(rr_dot(rel,rel)<p->radius*p->radius){victim=r;break;}
        }
        RRVec3 hit=rr_v3(0,0,0);bool hit_world=false;
        if(!victim) {
            hit_world=world&&world(world_ctx,prev,p->pos,&hit);
            if(!hit_world&&p->pos.y<floor_height){hit_world=true;hit=p->pos;}
        }
        p->visual.flown+=rr_length(rr_sub(p->pos,prev));
        if(p->trail) {
            p->trail_time-=dt;
            if(p->trail_time<=0){p->trail_time=.022;if(notice)notice(ctx,RR_SHOT_TRAIL,p,NULL,p->pos);}
        }
        if(victim||hit_world||p->life<=0) {
            if(notice&&victim)notice(ctx,RR_SHOT_RACER_IMPACT,p,victim,victim->car.pos);
            else if(notice&&hit_world)notice(ctx,RR_SHOT_WORLD_IMPACT,p,NULL,hit);
            erase(shots,count,i);
        }
    }
}
size_t rr_shots_purge(RRShot *shots,size_t *count,RRVec3 origin,double radius,
                      RRLocalRacer *owner,RRShotNotice notice,void *ctx) {
    size_t n=0;
    for(size_t i=*count;i-->0;) {
        RRShot *p=shots+i;if(p->owner==owner||rr_length(rr_sub(p->pos,origin))>radius)continue;
        if(notice)notice(ctx,RR_SHOT_PURGED,p,NULL,p->pos);
        erase(shots,count,i);++n;
    }
    return n;
}
void rr_shots_clear(size_t *count) {*count=0;}
