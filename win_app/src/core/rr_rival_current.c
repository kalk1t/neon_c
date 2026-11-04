#include "rr_rival_current.h"
#include "rr_weapon_current.h"
void rr_rival_current_reset(RRRivalCurrent *s,double initial){s->heat=0;s->overheated=false;s->bolt_cd=initial;}
bool rr_rival_current_step(RRFireLocal *l,RRLocalRacer *r,RRRivalCurrent *s,double dt,double time,bool active,double road_fire,const RRFireBinding *binding,RRRivalCurrentCue cue,void *cue_context,bool *fired){
 if(fired)*fired=false;
 if(!l)return false;
 if(!r||!s||!fired||!isfinite(dt)||dt<0||!isfinite(time)||!isfinite(road_fire)||road_fire<0||l->error||l->fx.error){l->error=true;return false;}
 if(time<6||r->race.finished||!(r->wreck<=0)||!active)return true;
 if(!l->random||!binding||binding->model<0||binding->model>3){l->error=true;return false;}
 for(int i=0;i<16;i++)if(!isfinite(binding->model_world[i])){l->error=true;return false;}
 RRVec3 muzzle=rr_weapon_current_muzzle(binding->model,s->side? -s->side:-1,binding->model_world);
 if(!isfinite(muzzle.x)||!isfinite(muzzle.y)||!isfinite(muzzle.z)){l->error=true;return false;}
 r->fire_cd=fmax(0,r->fire_cd-dt);s->bolt_cd-=dt;
 RRLocalRacer *best=NULL;double distance=INFINITY;
 const RRWeaponRules *w=rr_weapon_current_rules();
 for(size_t i=0;i<l->racer_count;i++){
  RRLocalRacer *o=l->racers[i];if(o==r||!(o->wreck<=0)||o->race.finished)continue;
  RRVec3 delta=rr_sub(o->car.pos,r->car.pos);double d=rr_length(delta);
  if(d<15||d>w->speed*w->life||rr_dot(delta,r->car.fwd)/d<cos(.3))continue;
  if(d<distance){distance=d;best=o;}
 }
 bool firing=best&&r->fire_cd<=0;
 s->heat=fmax(0,s->heat-.45*dt*(s->overheated?1.4:firing?.35:1));
 if(s->overheated&&s->heat<=.25)s->overheated=false;
 if(!firing||s->overheated||s->bolt_cd>0)return true;
 s->bolt_cd=w->period*3;s->side=s->side?-s->side:-1;
 double speed=fmax(0,r->car.speed),lead=rr_length(rr_sub(best->car.pos,muzzle))/(speed+w->speed);
 RRVec3 dir=rr_normalize(rr_sub(rr_add(best->car.pos,rr_scale(best->car.vel,lead*.9)),muzzle));
 double err=.03*road_fire;
 dir.x+=(l->random(l->random_ctx)-.5)*2*err;
 dir.y+=(l->random(l->random_ctx)-.5)*2*err;
 dir.z+=(l->random(l->random_ctx)-.5)*2*err;
 if(l->fx.three_warm_uuids)for(unsigned i=0;i<4;i++)(void)l->random(l->random_ctx);
 if(l->count>=l->capacity){l->error=true;return false;}
 RRFireOptions options=rr_weapon_current_bolt_options();options.turn=0;
 RRShot shot=rr_fire_init(r,muzzle,dir,speed,NULL,options).shot;
 shot.visual.color=rr_weapon_current_color(binding->accent);shot.visual.painted=true;l->shots[l->count++]=shot;
 s->heat+=w->heat;if(s->heat>=1){s->heat=1;s->overheated=true;}
 *fired=true;if(cue)cue(cue_context,r);return true;
}
