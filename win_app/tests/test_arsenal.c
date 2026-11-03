#include "rr_arsenal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "generated_arsenal_reference.h"
static unsigned failures; static size_t current; static double max_error;
#define CHECK(x) do {if(!(x)){if(failures<12)fprintf(stderr,"fixture %zu line %d: %s\n",current,__LINE__,#x);++failures;}}while(0)
static void near(double a,double b) {double d=fabs(a-b);if(d>max_error)max_error=d;CHECK(isfinite(a)&&d<1e-12);}
static void state(const RRArsenal *a,const RRArsenal *b) {
    near(a->heat,b->heat);near(a->bolt_cd,b->bolt_cd);near(a->missiles,b->missiles);
    near(a->spin_cd,b->spin_cd);near(a->emp_cd,b->emp_cd);
    CHECK(a->overheated==b->overheated&&a->side==b->side);
}
typedef struct {const double *values; unsigned used;} Random;
static double random_value(void *p) {Random *r=p;CHECK(r->used<3);if(r->used>=3)return 0;return r->values[r->used++];}
int main(void) {
    RRArsenal s;rr_arsenal_init(&s);unsigned draws=0,events=0,overheats=0,denies=0,restarts=0;
    for(current=0;current<sizeof(fixtures)/sizeof(fixtures[0]);++current) {
        const ArsenalFixture *f=&fixtures[current];if(f->load)s=f->before;state(&s,&f->before);
        Random r={f->random,0};RRArsenalEvents e={0};
        if(f->action==1){rr_arsenal_restart(&s);++restarts;}
        else if(f->action==2)rr_arsenal_init(&s);
        else rr_arsenal_step(&s,f->dt,f->input,random_value,&r,&e);
        state(&s,&f->after);CHECK(e.count==f->count&&e.random_draws==f->draws&&r.used==f->draws);
        for(unsigned j=0;j<6;++j){CHECK(e.order[j]==f->order[j]);overheats+=e.order[j]==RR_ARSENAL_OVERHEAT;denies+=e.order[j]==RR_ARSENAL_DENY;}
        near(e.spread_x,f->spread[0]);near(e.spread_y,f->spread[1]);
        CHECK(e.spin_side==f->spin&&e.missile_homing==f->homing);
        CHECK(e.bolt_side==(e.order[0]==RR_ARSENAL_BOLT?s.side:0));draws+=r.used;events+=e.count;
    }
    /* No RNG dependency when all actions are absent or on cooldown. */
    rr_arsenal_init(&s);RRArsenalEvents e;RRArsenalInput quiet={0};
    rr_arsenal_step(&s,.05,quiet,NULL,NULL,&e);CHECK(e.count==0&&e.random_draws==0);
    printf("ARSENAL_SUMMARY cases=%zu draws=%u events=%u overheats=%u denies=%u restarts=%u max_error=%.17g state_bytes=%zu event_bytes=%zu\n",sizeof(fixtures)/sizeof(fixtures[0]),draws,events,overheats,denies,restarts,max_error,sizeof(s),sizeof(e));
    return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
