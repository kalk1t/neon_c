#include "rr_arsenal.h"
#include <math.h>
#include <string.h>
static void emit(RRArsenalEvents *e,unsigned kind,RRArsenalNotice notice,void *ctx) { e->order[e->count++]=kind;if(notice)notice(ctx,kind,e); }
static double draw(RRArsenalEvents *e, RRArsenalRandom rng, void *context) {
    ++e->random_draws; return rng(context);
}
void rr_arsenal_restart(RRArsenal *s) {
    s->heat=0; s->overheated=false; s->bolt_cd=0; s->missiles=3;
    s->emp_cd=0; s->spin_cd=0;
}
void rr_arsenal_init(RRArsenal *s) { s->side=1; rr_arsenal_restart(s); }
void rr_arsenal_step_dispatch_pulse(RRArsenal *s,double dt,RRArsenalInput i,
    RRArsenalRandom rng,void *context,RRArsenalNotice notice,void *notice_ctx,RRArsenalEvents *e,RRArsenalPulse pulse) {
    memset(e,0,sizeof(*e));
    s->bolt_cd-=dt;
    s->heat=fmax(0,s->heat-.45*dt*(s->overheated?1.4:i.fire?.35:1));
    if(s->overheated&&s->heat<=.25) s->overheated=false;
    if(i.fire&&!s->overheated&&s->bolt_cd<=0) {
        s->bolt_cd=pulse.period; s->side=-s->side; e->bolt_side=s->side;
        e->spread_x=(draw(e,rng,context)-.5)*.008;
        e->spread_y=(draw(e,rng,context)-.5)*.008;
        emit(e,RR_ARSENAL_BOLT,notice,notice_ctx);
        s->heat+=pulse.heat;
        if(s->heat>=1) { s->heat=1; s->overheated=true; emit(e,RR_ARSENAL_OVERHEAT,notice,notice_ctx); }
    }
    s->missiles=fmin(3,s->missiles+dt/4.5);
    if(i.missile_hit) {
        if(s->missiles>=1) {
            s->missiles-=1; e->missile_homing=i.lock_charge>=1&&i.has_lock;
            emit(e,RR_ARSENAL_MISSILE,notice,notice_ctx);
        } else emit(e,RR_ARSENAL_DENY,notice,notice_ctx);
    }
    s->spin_cd=fmax(0,s->spin_cd-dt);
    if(i.spin_hit) {
        if(s->spin_cd>0) emit(e,RR_ARSENAL_DENY,notice,notice_ctx);
        else {
            s->spin_cd=6;
            e->spin_side=i.steer>0?1:i.steer<0?-1:draw(e,rng,context)<.5?-1:1;
            emit(e,RR_ARSENAL_SPIN,notice,notice_ctx);
        }
    }
    s->emp_cd=fmax(0,s->emp_cd-dt);
    if(i.emp_hit) {
        if(s->emp_cd>0) { emit(e,RR_ARSENAL_DENY,notice,notice_ctx); return; }
        s->emp_cd=11; emit(e,RR_ARSENAL_EMP,notice,notice_ctx);
    }
}

void rr_arsenal_step_dispatch(RRArsenal *s,double dt,RRArsenalInput input,RRArsenalRandom random,void *ctx,RRArsenalNotice notice,void *notice_ctx,RRArsenalEvents *events){
    rr_arsenal_step_dispatch_pulse(s,dt,input,random,ctx,notice,notice_ctx,events,(RRArsenalPulse){.09,.06});
}
void rr_arsenal_step(RRArsenal *s,double dt,RRArsenalInput i,RRArsenalRandom rng,void *context,RRArsenalEvents *e) {
    rr_arsenal_step_dispatch(s,dt,i,rng,context,NULL,NULL,e);
}
