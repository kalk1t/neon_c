#include "rr_weapon_current.h"
static const RRWeaponRules rules={21,.21,750,1.5,.08,2000,2100,45,7,9};
const RRWeaponRules *rr_weapon_current_rules(void){return &rules;}
void rr_weapon_current_arsenal(RRArsenal *s,double dt,RRArsenalInput input,RRArsenalRandom random,void *ctx,RRArsenalEvents *events){
 rr_weapon_current_arsenal_dispatch(s,dt,input,random,ctx,NULL,NULL,events);
}
void rr_weapon_current_arsenal_dispatch(RRArsenal *s,double dt,RRArsenalInput input,RRArsenalRandom random,void *ctx,RRArsenalNotice notice,void *notice_ctx,RRArsenalEvents *events){rr_arsenal_step_dispatch_pulse(s,dt,input,random,ctx,notice,notice_ctx,events,(RRArsenalPulse){rules.period,rules.heat});}
unsigned rr_weapon_current_lock(RRLocalLock *s,RRVec3 player,RRVec3 aim,const RRLocalTarget *targets,int count,double dt){return rr_lock_update_ranges(s,player,aim,targets,count,dt,rules.acquire,rules.keep);}
RRFireOptions rr_weapon_current_bolt_options(void){return (RRFireOptions){.speed=rules.speed,.life=rules.life,.turn=2.5,.damage=rules.damage,.length=rules.length,.width=rules.width,.radius=3.6,.has_speed=true,.has_life=true,.has_turn=true,.has_damage=true,.kind=RR_SHOT_BOLT};}
RRVec3 rr_weapon_current_muzzle(int model,int side,const double m[16]){
 static const RRVec3 at[]={{1.3,.2-.6,-2.3-.85},{.7,-.74,-2.9},{.25,-1.22,-2.85},{.7,-1,-2.9}};
 if(model<0||model>3)model=0;RRVec3 p=at[model];p.x*=side;
 /* Column-major Three model.matrixWorld; includes the host's body bob/tilt.
  * Supplying that exact matrix avoids guessing render/load state in the core. */
 double w=1/(m[3]*p.x+m[7]*p.y+m[11]*p.z+m[15]);return rr_v3((m[0]*p.x+m[4]*p.y+m[8]*p.z+m[12])*w,(m[1]*p.x+m[5]*p.y+m[9]*p.z+m[13])*w,(m[2]*p.x+m[6]*p.y+m[10]*p.z+m[14])*w);
}
static double linear(double c){return c<.04045?c*.0773993808:pow(c*.9478672986+.0521327014,2.4);}
RRVec3 rr_weapon_current_color(unsigned rgb){return rr_v3((linear(((rgb>>16)&255)/255.)*.65+.35)*3,(linear(((rgb>>8)&255)/255.)*.65+.35)*3,(linear((rgb&255)/255.)*.65+.35)*3);}
RRBoltStreak rr_weapon_current_streak(RRVec3 previous,RRVec3 head,RRVec3 velocity,RRVec3 camera,double flown){
 double far=1+fmin(2,rr_length(rr_sub(head,camera))/120);flown+=rr_length(rr_sub(head,previous));double length=fmin(rules.length*far,flown+1);
 RRBoltStreak p={rr_sub(head,rr_scale(rr_normalize(velocity),length/2)),flown,length,rules.width*far};return p;
}
