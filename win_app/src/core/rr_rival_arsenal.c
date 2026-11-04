#include "rr_rival_arsenal.h"
#include "rr_weapon_current.h"
static void notify(RRRivalArsenalCue cue,void *ctx,unsigned event,RRLocalRacer *r,RRLocalRacer *target){if(cue)cue(ctx,event,r,target);}
static bool spawn(RRFireLocal *l,RRLocalRacer *r,RRVec3 from,RRVec3 direction,double speed,RRLocalRacer *target,RRFireOptions options,const RRFireBinding *binding){
 if(l->fx.three_warm_uuids)for(unsigned i=0;i<4;i++)(void)l->random(l->random_ctx);
 if(l->count>=l->capacity){l->error=true;return false;}
 RRShot shot=rr_fire_init(r,from,direction,speed,target,options).shot;
 if(options.kind==RR_SHOT_BOLT){shot.visual.color=rr_weapon_current_color(binding->accent);shot.visual.painted=true;}
 l->shots[l->count++]=shot;return true;
}
static void dodge(RRFireLocal *l,RRLocalRacer *r,RRRivalLifeResources *s){
 if(s->spin_cd>0)return;
 for(size_t i=0;i<l->count;i++){
  RRShot *p=l->shots+i;if(p->kind!=RR_SHOT_MISSILE||p->target!=r||p->dodge_tried||rr_length(rr_sub(p->pos,r->car.pos))>120)continue;
  p->dodge_tried=true;if(l->random(l->random_ctx)>=.5)continue;
  s->spin_cd=6;rr_car_dodge_current(&r->car,l->random(l->random_ctx)<.5?-1:1);return;
 }
}
static void emp(RRFireLocal *l,RRLocalRacer *r,RRRivalLifeResources *s,RRRivalArsenalCue cue,void *ctx){
 if(s->emp_cd>0)return;RRLocalRacer *near[RR_LOCAL_MAX_CARS];size_t count=0;
 for(size_t i=0;i<l->racer_count;i++){RRLocalRacer *o=l->racers[i];if(o!=r&&o->wreck<=0&&!o->race.finished&&rr_length(rr_sub(o->car.pos,r->car.pos))<=95)near[count++]=o;}
 if(count<2)return;s->emp_cd=11;rr_fire_effect(l,RR_FX_WAVE);(void)rr_fire_purge(l,r->car.pos,95,r);notify(cue,ctx,RR_RIVAL_CUE_EMP,r,NULL);
 for(size_t i=0;i<count;i++){
  RRLocalRacer *o=near[i];RRVec3 delta=rr_sub(o->car.pos,r->car.pos);double d=rr_length(delta);if(!o->is_player)o->fire_cd+=3;
  o->car.vel=rr_add(o->car.vel,rr_scale(rr_normalize(delta),30*(1-d/95)+8));(void)rr_fire_damage(l,o,25*(1-.5*d/95),RR_SHOT_EMP,true);
  if(o==l->player)notify(cue,ctx,RR_RIVAL_NOTICE_EMP,r,o);
 }
}
bool rr_rival_arsenal_step(RRFireLocal *l,RRLocalRacer *r,RRRivalLifeResources *s,double dt,double time,bool active,double road_fire,const RRFireBinding *binding,RRRivalArsenalCue cue,void *ctx){
 if(!l)return false;
 if(!r||!s||r->is_player||!isfinite(dt)||dt<0||!isfinite(time)||!isfinite(road_fire)||road_fire<0||l->racer_count>RR_LOCAL_MAX_CARS||l->error||l->fx.error){l->error=true;return false;}
 if(time<6||r->race.finished||!(r->wreck<=0)||!active)return true;
 if(!l->random||!binding||binding->model<0||binding->model>3){l->error=true;return false;}
 for(int i=0;i<16;i++)if(!isfinite(binding->model_world[i])){l->error=true;return false;}
 for(int side=-1;side<=1;side++){RRVec3 p=rr_weapon_current_muzzle(binding->model,side,binding->model_world);if(!isfinite(p.x)||!isfinite(p.y)||!isfinite(p.z)){l->error=true;return false;}}
 r->fire_cd=fmax(0,r->fire_cd-dt);s->bolt.bolt_cd-=dt;s->missiles=fmin(3,s->missiles+dt/4.5);s->seeker_cd=fmax(0,s->seeker_cd-dt);s->spin_cd=fmax(0,s->spin_cd-dt);s->emp_cd=fmax(0,s->emp_cd-dt);
 dodge(l,r,s);RRLocalRacer *best=NULL;double distance=INFINITY;
 for(size_t i=0;i<l->racer_count;i++){RRLocalRacer *o=l->racers[i];if(o==r||!(o->wreck<=0)||o->race.finished)continue;RRVec3 delta=rr_sub(o->car.pos,r->car.pos);double d=rr_length(delta);if(d<15||d>2000||rr_dot(delta,r->car.fwd)/d<cos(35*RR_PI/180))continue;if(d<distance){distance=d;best=o;}}
 if(best!=s->locked){s->locked=best;s->lock_time=0;}s->lock_time=best?fmin(1,s->lock_time+dt/.5):0;
 bool armed=r->fire_cd<=0;if(armed)emp(l,r,s,cue,ctx);
 if(armed&&best&&s->lock_time>=1&&distance<=600&&s->missiles>=1&&s->seeker_cd<=0){
  double speed=fmax(0,r->car.speed);s->missiles-=1;s->seeker_cd=1.5;
  RRFireOptions options={.speed=60,.accel=420,.max_speed=speed+330,.turn=7,.life=3.6,.damage=48,.length=12,.width=6,.radius=5.5,.has_speed=true,.has_turn=true,.has_life=true,.has_damage=true,.always=true,.trail=true,.kind=RR_SHOT_MISSILE};
  if(!spawn(l,r,rr_weapon_current_muzzle(binding->model,0,binding->model_world),rr_add(r->car.fwd,rr_v3(0,.08,0)),speed,best,options,binding))return false;
  notify(cue,ctx,RR_RIVAL_CUE_SEEKER,r,best);if(best==l->player)notify(cue,ctx,RR_RIVAL_NOTICE_SEEKER,r,best);
 }
 bool firing=armed&&best&&distance<=750*1.5;s->bolt.heat=fmax(0,s->bolt.heat-.45*dt*(s->bolt.overheated?1.4:firing?.35:1));if(s->bolt.overheated&&s->bolt.heat<=.25)s->bolt.overheated=false;
 if(!firing||s->bolt.overheated||s->bolt.bolt_cd>0)return !l->error&&!l->fx.error;
 s->bolt.bolt_cd=.21*3;s->bolt.side=-(s->bolt.side?s->bolt.side:1);RRVec3 from=rr_weapon_current_muzzle(binding->model,s->bolt.side,binding->model_world);double speed=fmax(0,r->car.speed),lead=rr_length(rr_sub(best->car.pos,from))/(speed+750);
 RRVec3 dir=rr_normalize(rr_sub(rr_add(best->car.pos,rr_scale(best->car.vel,lead*.9)),from));double error=.03*road_fire;
 dir.x+=(l->random(l->random_ctx)-.5)*2*error;dir.y+=(l->random(l->random_ctx)-.5)*2*error;dir.z+=(l->random(l->random_ctx)-.5)*2*error;
 RRFireOptions bolt=rr_weapon_current_bolt_options();bolt.turn=0;if(!spawn(l,r,from,dir,speed,NULL,bolt,binding))return false;
 s->bolt.heat+=.08;if(s->bolt.heat>=1){s->bolt.heat=1;s->bolt.overheated=true;}notify(cue,ctx,RR_RIVAL_CUE_BOLT,r,NULL);return !l->error&&!l->fx.error;
}
