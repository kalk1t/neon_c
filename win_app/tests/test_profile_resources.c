#include "rr_weapon_current.h"
#include "rr_battle_current.h"
#include "rr_audio_submit.h"
#include <stdio.h>
#include <string.h>
#include "profile_resources_reference.h"
static unsigned checks,failures;static double worst;static size_t at;
#define CHECK(x) do{checks++;if(!(x)){if(failures<12)fprintf(stderr,"profile resource frame%zu line%d: %s\n",at,__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-11);}
static void state(const RRArsenal *s,const double v[7]){double a[]={s->heat,s->bolt_cd,s->missiles,s->spin_cd,s->emp_cd,s->overheated?1:0,s->side};for(int k=0;k<7;k++)near(a[k],v[k]);}
static void load(RRArsenal *s,const double v[7]){s->heat=v[0];s->bolt_cd=v[1];s->missiles=v[2];s->spin_cd=v[3];s->emp_cd=v[4];s->overheated=v[5]!=0;s->side=(int)v[6];}
typedef struct {uint32_t rng;unsigned draws,notice_at,bolts;const ResourceFrame *frame;RRArsenal *arsenal;RRAudioVoiceBank bank;} Harness;
static double random_unit(void *ctx){Harness *h=ctx;h->rng=h->rng*UINT32_C(1664525)+UINT32_C(1013904223);h->draws++;return h->rng/4294967296.0;}
static void notice(void *ctx,unsigned kind,const RRArsenalEvents *events){Harness *h=ctx;(void)events;CHECK(h->notice_at<h->frame->count);if(h->notice_at>=h->frame->count)return;const ResourceNotice *o=&h->frame->notices[h->notice_at++];CHECK(kind==o->kind&&h->rng==o->rng&&h->draws==o->draws);state(h->arsenal,o->state);h->bolts+=kind==RR_ARSENAL_BOLT?1:0;RRAudioCueSubmission out;CHECK(rr_audio_submit_arsenal(&h->bank,false,kind,random_unit,h,NULL,NULL,&out)&&out.complete&&out.voices.count==0&&out.clock_reads==0&&h->bank.active==0);}
static double fixed_random(void *ctx){(void)ctx;return .5;}
static bool binding(void *ctx,int index,const RRLocalRacer *r,RRFireBinding *b){(void)ctx;(void)index;(void)r;memset(b,0,sizeof(*b));b->model_world[0]=b->model_world[5]=b->model_world[10]=b->model_world[15]=1;return true;}
static RRTrack track;static RRLocalPack pack;static RRLocalBattle battle;static RRBattleCurrent current;static RRLocalRacer player;
int main(void){
 RRArsenal s={0};Harness h={0};h.arsenal=&s;rr_audio_voice_bank_init(&h.bank);unsigned callbacks=0,total_draws=0,restarts=0;
 for(at=0;at<sizeof(resource_frames)/sizeof(*resource_frames);at++){
 const ResourceFrame *f=&resource_frames[at];if(f->load){load(&s,f->before);h.rng=f->rng_before;h.draws=f->draws_before;}if(f->restart){rr_weapon_current_arsenal_restart_profile(&s,(RRCarProfile)f->profile);restarts++;}state(&s,f->before);CHECK(h.rng==f->rng_before&&h.draws==f->draws_before);h.frame=f;h.notice_at=h.bolts=0;RRArsenalEvents e;
 rr_weapon_current_arsenal_profile_dispatch(&s,f->dt,f->input,(RRCarProfile)f->profile,random_unit,&h,notice,&h,&e);state(&s,f->after);CHECK(h.rng==f->rng_after&&h.draws==f->draws_after&&h.notice_at==f->count&&e.count==f->count);CHECK(e.random_draws+h.bolts==f->draws_after-f->draws_before);for(unsigned j=0;j<e.count&&j<f->count;j++)CHECK(e.order[j]==f->notices[j].kind);CHECK(e.bolt_side==f->bolt&&e.spin_side==f->spin&&e.missile_homing==f->homing);near(e.spread_x,f->spread[0]);near(e.spread_y,f->spread[1]);callbacks+=h.notice_at;total_draws+=h.draws-f->draws_before;
 }
 CHECK(rr_track_init_road(&track,1));RRShot shots[64];RRFxLife fx[128];battle.fire.shots=shots;battle.fire.capacity=64;battle.fire.fx=(RRFxLedger){fx,0,128,false,false};battle.fire.random=fixed_random;battle.arsenal.side=-1;
 for(int profile=0;profile<3;profile++){
 memset(&player,0,sizeof(player));player.is_player=true;rr_car_reset(&player.car,rr_v3(0,400,0),0,0);player.car.profile=(RRCarProfile)profile;player.hp=rr_car_stats((RRCarProfile)profile)->hp;rr_race_reset(&player.race,RR_RACE_TWO_LAPS,player.car.pos,0);CHECK(rr_battle_current_start(&current,&battle,&pack,&track,&player));current.binding=binding;near(battle.arsenal.missiles,rr_car_stats((RRCarProfile)profile)->missiles);CHECK(battle.arsenal.side==-1);
 // Real after-player tail must retain Titan's fourth charge and Viper's4s spin.
 battle.arsenal.missiles=rr_car_stats((RRCarProfile)profile)->missiles-.1;RRPackEvents events;CHECK(rr_battle_current_after_player(&current,RR_PACK_PLAYING,true,.01,1,0,NULL,NULL,NULL,(RRArsenalInput){.spin_hit=true,.steer=1},&events));near(battle.arsenal.spin_cd,rr_car_stats((RRCarProfile)profile)->spin_cd);near(battle.arsenal.missiles,rr_car_stats((RRCarProfile)profile)->missiles-.1+.01/4.5);CHECK(!pack.coin_pending&&!current.error);near(player.car.speed,101.25);rr_pack_dispose(&pack);
 }
 rr_arsenal_init(&s);s.side=-1;s.missiles=4;s.spin_cd=0;RRArsenalEvents e;rr_weapon_current_arsenal_dispatch(&s,0,(RRArsenalInput){.spin_hit=true,.steer=1},NULL,NULL,NULL,NULL,&e);near(s.missiles,3);near(s.spin_cd,6);rr_weapon_current_arsenal_restart_profile(&s,(RRCarProfile)-1);near(s.missiles,3);CHECK(s.side==-1);
 printf("PROFILE_RESOURCES_SUMMARY frames=%zu restarts=%u callbacks=%u random_draws=%u checks=%u max_error=%.17g failures=%u; source Sound context absent, current full-pack restart/fire routed, EMP effects/production inactive,no graphics/accounts\n",sizeof(resource_frames)/sizeof(*resource_frames),restarts,callbacks,total_draws,checks,worst,failures);return failures?1:0;
}
