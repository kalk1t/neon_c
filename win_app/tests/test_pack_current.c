#include "rr_pack_current.h"
#include <stdio.h>
#include <string.h>
typedef struct ResourceFixture {double heat,bolt_cd;int side;bool overheated;double missiles,seeker_cd,spin_cd,emp_cd;bool locked;double lock_time;} ResourceFixture;

typedef struct PackState {RRVec3 pos,vel,fwd;double yaw,pitch,roll,speed,boost,max_multiplier,stun;bool boosting;int sample;double dodge;int lap,next,lap_count;double lap_time,finish_time,hp,wreck,lost,stuck;int coins;bool finished,visible;uint32_t passed;double fire_cd,react,smoke;ResourceFixture resources;} PackState;
typedef struct PackRandomPair {int index;double values[2];} PackRandomPair;
typedef struct PackRandomFixture {int count;PackRandomPair pairs[10];} PackRandomFixture;
typedef struct PackMeta {int ranking[11],position,finish_position;double rank_timer,shake;} PackMeta;
typedef struct CoinScalar {bool active;double timer,pop;} CoinScalar;
typedef struct PackTrace {int road;RRAIDriver drivers[10];PackState player;PackRandomFixture random;PackState after[10];PackMeta meta;} PackTrace;
typedef struct Notice {int index;RRVec3 pos;double speed,hp,wreck,smoke;int draws;bool smoked;} Notice;

typedef struct PackFrame {int trace;bool restart;RRPackBranch branch;double dt,time,race_time;uint32_t mask;PackState player;double input_shake;PackRandomFixture random;PackState after[10],after_player;int coin_count;CoinScalar coins[RR_COIN_MAX];RRCoinEvents events;int notice_count;Notice notices[10];PackMeta meta;} PackFrame;
#include "pack_current_reference.h"
static RRPackCurrent tuned;static const PackFrame *notice_frame;static unsigned notice_count,total_notices,smoke_notices;
static RRTrack track;static RRLocalPack pack;static RRLocalRacer player;static int failures;static double worst;static const PackRandomFixture *random_fixture;static int random_at;static size_t rng_pairs,collections,respawns,contacts,paused,countdown,playing;
#define CHECK(x) do{if(!(x)){if(failures<16)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-5);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
static bool random_draw(void *context,int index,double values[2])
{
    (void)context;CHECK(random_at<random_fixture->count);if(random_at>=random_fixture->count)return false;const PackRandomPair *r=&random_fixture->pairs[random_at++];CHECK(r->index==index);values[0]=r->values[0];values[1]=r->values[1];rng_pairs++;return true;
}
static void use_random(const PackRandomFixture *r){random_fixture=r;random_at=0;}
static void apply_player(const PackState *s)
{
    player.is_player=true;player.car.pos=s->pos;player.car.vel=s->vel;player.car.fwd=s->fwd;player.car.yaw=s->yaw;player.car.pitch=s->pitch;player.car.roll=s->roll;player.car.speed=s->speed;player.car.boost=s->boost;player.car.max_multiplier=s->max_multiplier;player.car.stun=s->stun;player.car.boosting=s->boosting;
    player.ai.sample=s->sample;player.ai.dodge=s->dodge;player.race.mode=RR_RACE_TWO_LAPS;player.race.lap=s->lap;player.race.next_checkpoint=s->next;player.lap_count=(size_t)s->lap_count;player.race.lap_time=s->lap_time;player.race.finish_time=s->finish_time;player.hp=s->hp;player.wreck=s->wreck;player.lost=s->lost;player.stuck=s->stuck;player.coins=s->coins;player.race.finished=s->finished;player.visible=s->visible;player.passed=s->passed;player.fire_cd=s->fire_cd;player.react=s->react;player.smoke_time=s->smoke;
}
static void compare(const RRLocalRacer *r,const PackState *e)
{
    vector(r->car.pos,e->pos);vector(r->car.vel,e->vel);vector(r->car.fwd,e->fwd);near(r->car.yaw,e->yaw);near(r->car.pitch,e->pitch);near(r->car.roll,e->roll);near(r->car.speed,e->speed);near(r->car.boost,e->boost);near(r->car.max_multiplier,e->max_multiplier);near(r->car.stun,e->stun);CHECK(r->car.boosting==e->boosting&&r->ai.sample==e->sample);near(r->ai.dodge,e->dodge);
    CHECK(r->race.lap==e->lap&&r->race.next_checkpoint==e->next&&r->lap_count==(size_t)e->lap_count);near(r->race.lap_time,e->lap_time);near(r->race.finish_time,e->finish_time);near(r->hp,e->hp);near(r->wreck,e->wreck);near(r->lost,e->lost);near(r->stuck,e->stuck);CHECK(r->coins==e->coins&&r->race.finished==e->finished&&r->visible==e->visible&&r->passed==e->passed&&!r->error);near(r->fire_cd,e->fire_cd);near(r->react,e->react);near(r->smoke_time,e->smoke);
}
static void metadata(const PackMeta *m){for(int i=0;i<11;i++)CHECK(pack.ranking[i]==m->ranking[i]);CHECK(pack.position==m->position&&pack.finish_position==m->finish_position);near(pack.rank_timer,m->rank_timer);near(pack.shake,m->shake);}
static void resource_compare(const RRRivalLifeResources *s,const ResourceFixture *e){near(s->bolt.heat,e->heat);near(s->bolt.bolt_cd,e->bolt_cd);CHECK(s->bolt.side==e->side&&s->bolt.overheated==e->overheated);near(s->missiles,e->missiles);near(s->seeker_cd,e->seeker_cd);near(s->spin_cd,e->spin_cd);near(s->emp_cd,e->emp_cd);CHECK((s->locked!=NULL)==e->locked);near(s->lock_time,e->lock_time);}
static void notice(void *context,RRLocalRacer *r,int index,unsigned flags){(void)context;CHECK(notice_count<(unsigned)notice_frame->notice_count);if(notice_count>=(unsigned)notice_frame->notice_count)return;const Notice *e=notice_frame->notices+notice_count++;CHECK(index==e->index);vector(r->car.pos,e->pos);near(r->car.speed,e->speed);near(r->hp,e->hp);near(r->wreck,e->wreck);near(r->smoke_time,e->smoke);CHECK(random_at*2==e->draws&&!!(flags&RR_RIVAL_SMOKE)==e->smoked);CHECK(pack.coin_pending);total_notices++;if(flags&RR_RIVAL_SMOKE)smoke_notices++;}
static void start(const PackState *p,const PackRandomFixture *random,const PackState after[10],const PackMeta *meta)
{
    apply_player(p);use_random(random);CHECK(rr_pack_current_start(&tuned,&pack,&track,&player,random_draw,NULL));CHECK(random_at==random->count);for(int i=0;i<10;i++){compare(&pack.rivals[i],&after[i]);resource_compare(&tuned.resources[i],&after[i].resources);}metadata(meta);
}
static void mutations(uint32_t mask)
{
    if(mask&1){pack.rivals[0].wreck=.03;pack.rivals[0].hp=0;pack.rivals[0].visible=false;}
    if(mask&2){pack.rivals[1].stuck=2.49;pack.rivals[1].car.speed=0;pack.rivals[1].car.vel=rr_v3(0,0,0);}
    if(mask&4){pack.rivals[2].car.pos.y=25;pack.rivals[2].lost=1.09;}
    if(mask&8){pack.rivals[3].race.finished=true;pack.rivals[3].race.lap=3;pack.rivals[3].race.finish_time=.2;}
    if(mask&32){pack.rivals[5].hp=31;pack.rivals[5].smoke_time=.001;}
    if(mask&16){pack.rivals[4].car.pos=rr_add(pack.coins.list[0].pos,rr_v3(2,0,0));pack.rivals[4].car.speed=0;pack.rivals[4].car.vel=rr_v3(0,0,0);pack.rivals[4].ai.sample=rr_track_nearest(&track,pack.rivals[4].car.pos);}
}
int main(void)
{
    int trace=-1;size_t restarts=0;
    for(size_t k=0;k<sizeof(frames)/sizeof(*frames);k++){
        const PackFrame *f=&frames[k];if(trace!=f->trace){rr_pack_dispose(&pack);pack=(RRLocalPack){0};tuned=(RRPackCurrent){0};player=(RRLocalRacer){0};trace=f->trace;const PackTrace *t=&traces[trace];CHECK(rr_track_init_road(&track,t->road));pack.rank_timer=.13;pack.shake=.1;for(int i=0;i<10;i++){rr_car_reset(&pack.rivals[i].car,rr_v3(0,118,0),0,0);pack.rivals[i].ai=t->drivers[i];}start(&t->player,&t->random,t->after,&t->meta);}
        if(f->restart){start(&f->player,&f->random,f->after,&f->meta);restarts++;continue;}
        mutations(f->mask);apply_player(&f->player);pack.shake=f->input_shake;use_random(&f->random);RRPackEvents e;notice_frame=f;notice_count=0;CHECK(rr_pack_current_rivals_dispatch(&tuned,f->branch,f->dt,f->time,f->race_time,NULL,random_draw,NULL,notice,NULL,&e));CHECK(notice_count==(unsigned)f->notice_count);if(f->branch!=RR_PACK_PAUSED){RRPackEvents rejected;CHECK(pack.coin_pending&&!rr_pack_current_rivals_dispatch(&tuned,f->branch,f->dt,f->time,f->race_time,NULL,random_draw,NULL,NULL,NULL,&rejected));CHECK(rr_pack_coins_dispatch(&pack,NULL,NULL,&e.coins));CHECK(!pack.coin_pending&&!rr_pack_coins_dispatch(&pack,NULL,NULL,&rejected.coins));}else CHECK(!pack.coin_pending);CHECK(!e.error&&random_at==f->random.count);
        for(int i=0;i<10;i++){compare(&pack.rivals[i],&f->after[i]);resource_compare(&tuned.resources[i],&f->after[i].resources);if(e.rival[i]&RR_RIVAL_RESPAWN)respawns++;}compare(&player,&f->after_player);metadata(&f->meta);
        CHECK(pack.coins.count==f->coin_count);for(int i=0;i<pack.coins.count;i++){CHECK(pack.coins.list[i].active==f->coins[i].active);near(pack.coins.list[i].timer,f->coins[i].timer);near(pack.coins.list[i].pop,f->coins[i].pop);}
        CHECK(e.coins.count==f->events.count);for(int i=0;i<e.coins.count&&i<f->events.count;i++)CHECK(e.coins.list[i].coin==f->events.list[i].coin&&e.coins.list[i].racer==f->events.list[i].racer);collections+=(size_t)e.coins.count;contacts+=e.contacts.count;if(f->branch==RR_PACK_PAUSED)paused++;if(f->branch==RR_PACK_COUNTDOWN)countdown++;if(f->branch==RR_PACK_PLAYING)playing++;
    }
    rr_pack_dispose(&pack);CHECK(pack.player==NULL&&!rr_pack_current_rank(&tuned));RRVec3 pos;CHECK(!rr_pack_grid(-1,&pos)&&!rr_pack_grid(12,&pos));CHECK(!rr_pack_start(NULL,&track,&player,random_draw,NULL));
    printf("PACK_CURRENT_SUMMARY {\"roads\":9,\"traces\":18,\"ticks\":4320,\"restarts\":%zu,\"countdown_ticks\":%zu,\"playing_ticks\":%zu,\"paused_ticks\":%zu,\"rng_pairs\":%zu,\"respawns\":%zu,\"contact_pairs\":%zu,\"coin_collections\":%zu,\"max_state_error\":%.12g,\"pack_bytes\":%zu,\"event_bytes\":%zu,\"player_phase\":\"caller\",\"combat\":false,\"production_activation\":false,\"failures\":%d}\n",restarts,countdown,playing,paused,rng_pairs,respawns,contacts,collections,worst,sizeof(pack),sizeof(RRPackEvents),failures);printf("CURRENT_PACK_DETAIL notices=%u smoke_notices=%u context_bytes=%zu production_inactive=true\n",total_notices,smoke_notices,sizeof(tuned));return failures?1:0;
}
