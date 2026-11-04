#include "rr_fire.h"
#include "rr_weapon_current.h"
#include <string.h>
RRFireInit rr_fire_init(RRLocalRacer *owner,RRVec3 pos,RRVec3 dir,double inherit,RRLocalRacer *target,RRFireOptions o) {
    RRFireInit out={0};RRShot *s=&out.shot;s->owner=owner;s->target=target;s->kind=o.kind;
    s->pos=pos;s->speed=inherit+(o.has_speed?o.speed:240);s->vel=rr_scale(rr_normalize(dir),s->speed);
    s->accel=o.accel;s->max_speed=o.max_speed!=0?o.max_speed:s->speed;s->life=o.has_life?o.life:2.4;
    s->turn=o.has_turn?o.turn:owner->is_player?5:2.2;s->damage=o.damage;s->has_damage=o.has_damage;
    s->radius=o.radius!=0?o.radius:3.2;s->always=o.always;s->trail=o.trail;
    out.length=o.length!=0?o.length:18;out.width=o.width!=0?o.width:6;s->visual.length=out.length;s->visual.width=out.width;return out;
}
static void sprite(RRFxLedger *f,double life,RRArsenalRandom random,void *ctx) {
    if(f->three_warm_uuids)for(unsigned i=0;i<8;i++)(void)random(ctx);
    (void)random(ctx);(void)random(ctx);f->items[f->count++]=(RRFxLife){0,life};
}
bool rr_fx_request(RRFxLedger *f,unsigned kind,RRArsenalRandom random,void *ctx) {
    size_t add=kind==RR_FX_EXPLODE?12:kind==RR_FX_TRAIL?2:1;
    if((kind==RR_FX_SMOKE&&f->count>260)||(kind==RR_FX_TRAIL&&f->count>=300))return true;
    if(f->count>f->capacity||add>f->capacity-f->count){f->error=true;return false;}
    if(kind==RR_FX_EXPLODE) {
        sprite(f,.7,random,ctx);sprite(f,.5,random,ctx);sprite(f,.3,random,ctx);
        for(unsigned i=0;i<9;i++){for(unsigned j=0;j<4;j++)(void)random(ctx);double life=1.6+random(ctx);sprite(f,life,random,ctx);}
    } else if(kind==RR_FX_SMOKE) {
        (void)random(ctx);double life=1.1+random(ctx)*.6;sprite(f,life,random,ctx);
    } else if(kind==RR_FX_TRAIL) {sprite(f,.7,random,ctx);sprite(f,.18,random,ctx);}
    else {if(kind==RR_FX_WAVE&&f->three_warm_uuids)for(unsigned i=0;i<16;i++)(void)random(ctx);sprite(f,kind==RR_FX_FLASH?.07:kind==RR_FX_IMPACT?.5:kind==RR_FX_SPARK?.35:.45,random,ctx);}
    return true;
}
void rr_fx_tick(RRFxLedger *f,double dt) {
    for(size_t i=f->count;i-->0;){RRFxLife *p=f->items+i;p->t+=dt;if(p->t/p->life>=1){memmove(p,p+1,(f->count-i-1)*sizeof(*p));--f->count;}}
}
static void effect(RRFireLocal *l,unsigned kind){if(!rr_fx_request(&l->fx,kind,l->random,l->random_ctx))l->error=true;}
static void damaged(RRFireLocal *l,RRLocalRacer *r,unsigned result) {
    if((result&RR_DAMAGE_APPLIED)&&r->is_player&&l->shake)*l->shake=fmin(1.2,*l->shake+.6);
    if(result&RR_DAMAGE_WRECK){effect(l,RR_FX_EXPLODE);if(l->locked==r)l->locked=NULL;if(r->is_player&&l->shake)*l->shake=1.4;}
}
static void emp_notice(void *p,RRLocalRacer *r,unsigned result){damaged(p,r,result);}
typedef struct {RRFireLocal *local;bool race_active;} ProjectileNotice;
static void shot_notice_current(void *p,unsigned event,const RRShot *s,RRLocalRacer *r,RRVec3 point) {
    ProjectileNotice *notice=p;RRFireLocal *l=notice->local;(void)point;
    if(event==RR_SHOT_TRAIL){effect(l,RR_FX_TRAIL);return;}
    effect(l,event==RR_SHOT_PURGED||s->kind==RR_SHOT_BOLT?RR_FX_SPARK:RR_FX_IMPACT);
    if(event==RR_SHOT_RACER_IMPACT)damaged(l,r,rr_combat_shot_damage(s,r,notice->race_active));
}
static void shot_notice(void *p,unsigned event,const RRShot *shot,RRLocalRacer *r,RRVec3 point){ProjectileNotice notice={p,true};shot_notice_current(&notice,event,shot,r,point);}
typedef struct {RRFireLocal *local;double speed;const RRFireBinding *binding;} Dispatch;
static void dispatch(void *p,unsigned kind,const RRArsenalEvents *e) {
    Dispatch *d=p;RRFireLocal *l=d->local;RRFireOptions o={0};
    if(kind==RR_ARSENAL_BOLT||kind==RR_ARSENAL_MISSILE) {
        RRVec3 from=l->muzzle,dir=l->aim;RRLocalRacer *target=l->locked;
        if(kind==RR_ARSENAL_BOLT) {
            from=d->binding?rr_weapon_current_muzzle(d->binding->model,e->bolt_side,d->binding->model_world):rr_add(from,rr_scale(l->side_axis,e->bolt_side*1.3));
            if(target){RRLocalTarget t={0};t.pos=target->car.pos;t.vel=target->car.vel;(void)rr_target_lead(from,&t,d->speed+(d->binding?rr_weapon_current_rules()->speed:430),&dir);}
            dir.x+=e->spread_x;dir.y+=e->spread_y;
            o=d->binding?rr_weapon_current_bolt_options():(RRFireOptions){.speed=430,.life=1.3,.turn=2.5,.damage=6,.length=24,.width=5,.radius=3.6,.has_speed=true,.has_life=true,.has_turn=true,.has_damage=true,.kind=RR_SHOT_BOLT};
        } else {
            if(d->binding)from=rr_weapon_current_muzzle(d->binding->model,0,d->binding->model_world);
            target=e->missile_homing?target:NULL;dir.y+=.08;
            o=(RRFireOptions){.speed=60,.accel=420,.max_speed=d->speed+330,.life=3.6,.turn=target?7:0,.damage=48,.length=12,.width=6,.radius=5.5,.has_speed=true,.has_life=true,.has_turn=true,.has_damage=true,.always=true,.trail=true,.kind=RR_SHOT_MISSILE};
        }
        if(l->fx.three_warm_uuids)for(unsigned i=0;i<4;i++)(void)l->random(l->random_ctx);
        RRVec3 color=d->binding?rr_weapon_current_color(d->binding->accent):rr_v3(0,0,0);
        if(l->count>=l->capacity)l->error=true;else{
            RRShot shot=rr_fire_init(l->player,from,dir,d->speed,target,o).shot;
            if(d->binding&&kind==RR_ARSENAL_BOLT){shot.visual.color=color;shot.visual.painted=true;}
            l->shots[l->count++]=shot;
        }
        if(kind==RR_ARSENAL_BOLT){
            effect(l,RR_FX_FLASH);
            if(d->binding&&d->binding->flash){RRFireFlash flash={from,color};d->binding->flash(d->binding->flash_context,&flash);}
        }
    } else if(kind==RR_ARSENAL_SPIN)rr_car_dodge(&l->player->car,e->spin_side);
    else if(kind==RR_ARSENAL_EMP) {
        effect(l,RR_FX_WAVE);
        (void)rr_shots_purge(l->shots,&l->count,l->player->car.pos,95,l->player,shot_notice,l);
        /* Source sound.emp precedes the roster damage loop. */
        if(l->cue)l->cue(l->cue_ctx,kind);
        if(l->shake)*l->shake+=.5;
        (void)rr_combat_emp(l->player->car.pos,l->racers,l->racer_count,true,emp_notice,l);return;
    }
    if(l->cue)l->cue(l->cue_ctx,kind);
    if(l->shake&&kind==RR_ARSENAL_BOLT)*l->shake=fmin(.5,*l->shake+.03);
    if(l->shake&&kind==RR_ARSENAL_MISSILE)*l->shake+=.25;
}
void rr_fire_player_step(RRFireLocal *l,RRArsenal *s,double dt,RRArsenalInput input,RRArsenalEvents *events) {
    Dispatch d={l,fmax(0,l->player->car.speed),NULL};
    rr_arsenal_step_dispatch(s,dt,input,l->random,l->random_ctx,dispatch,&d,events);
}
bool rr_fire_player_current_step(RRFireLocal *l,RRArsenal *s,double dt,RRArsenalInput input,const RRFireBinding *binding,RRArsenalEvents *events){
    if(!l)return false;
    if(!s||!events||l->error||l->fx.error){l->error=true;return false;}
    if(!l->player||!l->random||!binding||binding->model<0||binding->model>3){l->error=true;return false;}
    for(int i=0;i<16;i++)if(!isfinite(binding->model_world[i])){l->error=true;return false;}
    for(int side=-1;side<=1;side++){RRVec3 p=rr_weapon_current_muzzle(binding->model,side,binding->model_world);if(!isfinite(p.x)||!isfinite(p.y)||!isfinite(p.z)){l->error=true;return false;}}
    Dispatch d={l,fmax(0,l->player->car.speed),binding};
    rr_weapon_current_arsenal_dispatch(s,dt,input,l->random,l->random_ctx,dispatch,&d,events);return !l->error&&!l->fx.error;
}
RRShotStreak rr_fire_shot_streak(const RRShot *shot,RRVec3 camera){
    double far=1+fmin(2,rr_length(rr_sub(shot->pos,camera))/120),length=fmin(shot->visual.length*far,shot->visual.flown+1);
    RRShotStreak out={rr_sub(shot->pos,rr_scale(rr_normalize(shot->vel),length/2)),length,shot->visual.width*far};return out;
}
void rr_fire_projectiles_current_tick(RRFireLocal *l,double dt,double floor_height,RRShotWorld world,void *ctx,bool race_active){
    ProjectileNotice notice={l,race_active};rr_shots_step(l->shots,&l->count,l->racers,l->racer_count,dt,floor_height,world,ctx,shot_notice_current,&notice);rr_fx_tick(&l->fx,dt);
}
void rr_fire_projectiles_tick(RRFireLocal *l,double dt,double floor_height,RRShotWorld world,void *ctx) {
    rr_fire_projectiles_current_tick(l,dt,floor_height,world,ctx,true);
}
void rr_fire_clear(RRFireLocal *l){rr_shots_clear(&l->count);}
