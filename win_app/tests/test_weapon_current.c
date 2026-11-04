#include "rr_weapon_current.h"
#include <stdio.h>
typedef struct WeaponFrame{int reset;double dt;RRArsenalInput input;double random[3],expected[7];unsigned draws,count,order[6];int side,spin,homing;}WeaponFrame;
typedef struct LockFrame{int reset;double dt;RRLocalTarget targets[3];int target;double charge;unsigned events;}LockFrame;
typedef struct MuzzleFrame{int model,side;double world[16];RRVec3 expected;}MuzzleFrame;
typedef struct ColorFrame{unsigned rgb;RRVec3 expected;}ColorFrame;
typedef struct StreakFrame{RRVec3 previous,head,velocity,camera;double flown;RRVec3 center;double next_flown,length,width;}StreakFrame;
#include "weapon_current_fixture.h"
static int failures;static double worst;static unsigned cursor;static const double *random_values;
static double random_draw(void *unused){(void)unused;if(cursor>=3){failures++;return .5;}return random_values[cursor++];}
static void near_value(double x,double y){double e=fabs(x-y);if(e>worst)worst=e;if(!isfinite(x)||e>1e-10){if(failures<8)fprintf(stderr,"weapon current got%.17g expected%.17g\n",x,y);failures++;}}
static void near_vector(RRVec3 x,RRVec3 y){near_value(x.x,y.x);near_value(x.y,y.y);near_value(x.z,y.z);}
int main(void){RRArsenal s;rr_arsenal_init(&s);int overheat_count=0,bolts=0;
 for(size_t i=0;i<sizeof(weapon_frames)/sizeof(weapon_frames[0]);i++){
  const WeaponFrame *c=&weapon_frames[i];if(c->reset){rr_arsenal_init(&s);bolts=0;}cursor=0;random_values=c->random;RRArsenalEvents e;rr_weapon_current_arsenal(&s,c->dt,c->input,random_draw,NULL,&e);
  double got[]={s.heat,s.bolt_cd,s.missiles,s.spin_cd,s.emp_cd,s.overheated,s.side};for(int j=0;j<7;j++)near_value(got[j],c->expected[j]);if(e.count!=c->count||cursor!=c->draws||e.bolt_side!=c->side||e.spin_side!=c->spin||e.missile_homing!=(c->homing!=0))failures++;for(unsigned j=0;j<e.count;j++){if(e.order[j]!=c->order[j])failures++;if(e.order[j]==RR_ARSENAL_BOLT)bolts++;if(i<600&&e.order[j]==RR_ARSENAL_OVERHEAT&&!overheat_count)overheat_count=bolts;}
 }
 if(overheat_count!=22)failures++;
 RRLocalLock lock;rr_lock_init(&lock);for(size_t i=0;i<sizeof(lock_frames)/sizeof(lock_frames[0]);i++){const LockFrame *c=&lock_frames[i];if(c->reset)rr_lock_init(&lock);unsigned e=rr_weapon_current_lock(&lock,rr_v3(0,100,0),rr_v3(0,0,-1),c->targets,3,c->dt);if(e!=c->events||lock.target!=c->target)failures++;near_value(lock.charge,c->charge);}
 for(size_t i=0;i<sizeof(muzzle_frames)/sizeof(muzzle_frames[0]);i++){const MuzzleFrame *c=&muzzle_frames[i];near_vector(rr_weapon_current_muzzle(c->model,c->side,c->world),c->expected);}
 for(size_t i=0;i<sizeof(color_frames)/sizeof(color_frames[0]);i++)near_vector(rr_weapon_current_color(color_frames[i].rgb),color_frames[i].expected);
 for(size_t i=0;i<sizeof(streak_frames)/sizeof(streak_frames[0]);i++){const StreakFrame *c=&streak_frames[i];RRBoltStreak b=rr_weapon_current_streak(c->previous,c->head,c->velocity,c->camera,c->flown);near_vector(b.center,c->center);near_value(b.flown,c->next_flown);near_value(b.length,c->length);near_value(b.width,c->width);}
 const RRWeaponRules *r=rr_weapon_current_rules();const double actual[]={r->damage,r->period,r->speed,r->life,r->heat,r->acquire,r->keep,r->length,r->width,r->source_protocol,RR_STRAFE_LEAN};for(int i=0;i<11;i++)near_value(actual[i],current_rule_values[i]);
 RRFireOptions o=rr_weapon_current_bolt_options();RRLocalRacer victim={0},owner={0};victim.hp=100;RRFireInit shot=rr_fire_init(&owner,rr_v3(0,0,0),rr_v3(0,0,-1),95,NULL,o);near_value(shot.shot.speed,845);near_value(shot.shot.life,1.5);near_value(shot.length,45);near_value(shot.width,7);for(int i=0;i<5;i++){unsigned hit=rr_combat_shot_damage(&shot.shot,&victim,true);if(i<4){near_value(victim.hp,100-(i+1)*21);if(hit&RR_DAMAGE_WRECK)failures++;}else{if(!(hit&RR_DAMAGE_WRECK))failures++;near_value(victim.hp,0);}}
 printf("v0.2.14 preparation:2400 actual arsenal frames,720 lock frames,576 muzzle poses,75 HDR colors,240 streak states;22 bolts to overheat/5 to wreck; max numeric error %.12g, %d failures; production inactive,no GPU/network\n",worst,failures);return failures?1:0;
}
