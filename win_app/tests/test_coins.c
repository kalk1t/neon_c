#include "rr_coins.h"
#include <stdio.h>
#include <string.h>
typedef struct CoinPlacement {RRVec3 pos;int sample;} CoinPlacement;
typedef struct CoinLayout {int road,count;CoinPlacement list[RR_COIN_MAX];} CoinLayout;
typedef struct CoinScalar {bool active;double timer,pop;} CoinScalar;
typedef struct CoinState {int count;bool hidden;CoinScalar list[RR_COIN_MAX];} CoinState;
typedef struct CoinRacer {RRVec3 pos,fwd;int coins;bool alive,finished;} CoinRacer;
typedef struct CoinAhead {int road,inactive;CoinRacer racer;double max;int best;} CoinAhead;
typedef struct CoinUpdate {int road;CoinState before;int count;CoinRacer racers[3];double dt;bool active;CoinState after;CoinRacer after_racers[3];RRCoinEvents events;} CoinUpdate;
typedef struct CoinReset {int road;CoinState before,after;} CoinReset;
typedef struct CoinTrace {int road;CoinState before;} CoinTrace;
typedef struct CoinFrame {int trace;double dt;bool active;CoinRacer racers[2];CoinState after;CoinRacer after_racers[2];RRCoinEvents events;} CoinFrame;
typedef struct CoinTie {double max;int best;} CoinTie;
#include "generated_coin_reference.h"
static int failures;static double worst;static RRTrack track;static RRCoins field;static RRLocalRacer racers[3];
#define CHECK(x) do{if(!(x)){if(failures<12)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-8);}
static void init(int road){CHECK(rr_track_init_road(&track,road));CHECK(rr_coins_init(&field,&track));}
static void apply(const CoinState *s){CHECK(field.count==s->count);field.hidden=s->hidden;for(int i=0;i<field.count;i++){field.list[i].active=s->list[i].active;field.list[i].timer=s->list[i].timer;field.list[i].pop=s->list[i].pop;}}
static void compare(const CoinState *s){CHECK(field.count==s->count&&field.hidden==s->hidden);for(int i=0;i<field.count;i++){CHECK(field.list[i].active==s->list[i].active);near(field.list[i].timer,s->list[i].timer);near(field.list[i].pop,s->list[i].pop);}}
static void peers(const CoinRacer *data,int count){for(int i=0;i<count;i++){racers[i]=(RRLocalRacer){0};racers[i].car.pos=data[i].pos;racers[i].car.fwd=data[i].fwd;racers[i].coins=data[i].coins;racers[i].wreck=data[i].alive?0:1;racers[i].race.finished=data[i].finished;}}
static void collected(const RRCoinEvents *a,const RRCoinEvents *e,const CoinRacer *after,int count){CHECK(a->count==e->count);for(int i=0;i<a->count&&i<e->count;i++)CHECK(a->list[i].coin==e->list[i].coin&&a->list[i].racer==e->list[i].racer);for(int i=0;i<count;i++)CHECK(racers[i].coins==after[i].coins);}
int main(void)
{
    int placements=0,max_coins=0;size_t collections=0;
    for(size_t i=0;i<sizeof(layouts)/sizeof(*layouts);i++){
        const CoinLayout *l=&layouts[i];init(l->road);CHECK(field.count==l->count);if(l->count>max_coins)max_coins=l->count;
        for(int k=0;k<field.count;k++){near(field.list[k].pos.x,l->list[k].pos.x);near(field.list[k].pos.y,l->list[k].pos.y);near(field.list[k].pos.z,l->list[k].pos.z);CHECK(field.list[k].sample==l->list[k].sample&&field.list[k].active&&field.list[k].timer==0&&field.list[k].pop==0);placements++;}
    }
    int road=0;
    for(size_t i=0;i<sizeof(ahead_cases)/sizeof(*ahead_cases);i++){const CoinAhead *f=&ahead_cases[i];if(road!=f->road){road=f->road;init(road);}rr_coins_reset(&field);if(f->inactive>=0)field.list[f->inactive].active=false;RRCar car={0};car.pos=f->racer.pos;car.fwd=f->racer.fwd;CHECK(rr_coins_ahead(&field,&car,f->max)==f->best);}
    for(size_t i=0;i<sizeof(update_cases)/sizeof(*update_cases);i++){const CoinUpdate *f=&update_cases[i];init(f->road);apply(&f->before);peers(f->racers,f->count);RRCoinEvents events;CHECK(rr_coins_update(&field,f->dt,racers,(size_t)f->count,f->active,&events));compare(&f->after);collected(&events,&f->events,f->after_racers,f->count);collections+=(size_t)events.count;}
    for(size_t i=0;i<sizeof(reset_cases)/sizeof(*reset_cases);i++){const CoinReset *f=&reset_cases[i];init(f->road);apply(&f->before);rr_coins_reset(&field);compare(&f->after);}
    int trace=-1;
    for(size_t i=0;i<sizeof(frames)/sizeof(*frames);i++){const CoinFrame *f=&frames[i];if(trace!=f->trace){trace=f->trace;init(traces[trace].road);apply(&traces[trace].before);}peers(f->racers,2);RRCoinEvents events;CHECK(rr_coins_update(&field,f->dt,racers,2,f->active,&events));compare(&f->after);collected(&events,&f->events,f->after_racers,2);collections+=(size_t)events.count;}
    /* Source equal-distance ties select the later list entry. */
    field=(RRCoins){0};field.count=2;field.list[0]=(RRLocalCoin){{-10,0,-30},0,true,0,0};field.list[1]=(RRLocalCoin){{10,0,-30},1,true,0,0};RRCar car={0};car.fwd=rr_v3(0,0,-1);
    for(size_t i=0;i<sizeof(tie_cases)/sizeof(*tie_cases);i++)CHECK(rr_coins_ahead(&field,&car,tie_cases[i].max)==tie_cases[i].best);
    /* Reactivation never also collects in that update, including dt0. */
    field.count=1;field.list[0]=(RRLocalCoin){{0,0,0},0,false,0,.8};racers[0]=(RRLocalRacer){0};RRCoinEvents events;CHECK(rr_coins_update(&field,0,racers,1,true,&events));CHECK(events.count==0&&field.list[0].active&&field.list[0].pop==0);CHECK(rr_coins_update(&field,0,racers,1,true,&events));CHECK(events.count==1&&!field.list[0].active&&racers[0].coins==1);
    field.list[0].active=true;racers[0].car.pos=rr_v3(6.5,0,0);CHECK(rr_coins_update(&field,0,racers,1,true,&events));CHECK(events.count==0);rr_coins_visible(&field,false);CHECK(field.hidden);rr_coins_reset(&field);CHECK(field.hidden&&racers[0].coins==1);rr_coins_visible(&field,true);CHECK(!field.hidden);
    CHECK(!rr_coins_init(NULL,&track)&&!rr_coins_update(NULL,0,NULL,0,false,&events));CHECK(!rr_coins_update(&field,0,NULL,1,false,&events));CHECK(rr_coins_ahead(NULL,&car,90)==-1);
    printf("COINS_SUMMARY {\"roads\":9,\"placements\":%d,\"max_selected_road_coins\":%d,\"pool_bound\":%d,\"ahead_cases\":324,\"update_cases\":108,\"replay_frames\":1440,\"reset_cases\":9,\"collection_events\":%zu,\"max_error\":%.12g,\"pool_bytes\":%zu,\"event_bytes\":%zu,\"runtime_activation\":false,\"wallet_writes\":false,\"failures\":%d}\n",placements,max_coins,RR_COIN_MAX,collections,worst,sizeof(field),sizeof(events),failures);return failures?1:0;
}
