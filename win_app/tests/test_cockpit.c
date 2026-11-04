#include "rr_cockpit_plan.h"
#include "cockpit-reference.h"
#include <string.h>
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);failures++;}}while(0)
int main(void)
{
    for(int id=0;id<3;id++){
        RRCKLayout l=rr_cockpit_layout(id);double values[33];int n=0;
        for(int i=0;i<2;i++){values[n++]=l.control[i].x;values[n++]=l.control[i].y;values[n++]=l.control[i].w;values[n++]=l.control[i].h;}
        for(int i=0;i<4;i++){values[n++]=l.panel[i].x;values[n++]=l.panel[i].y;values[n++]=l.panel[i].w;values[n++]=l.panel[i].h;}
        values[n++]=l.degrees;values[n++]=l.pivot_x;values[n++]=l.pivot_y;for(int i=0;i<4;i++)values[n++]=l.font[i];values[n++]=l.controls;values[n++]=l.race_column;
        CHECK(n==33);for(int i=0;i<n;i++)CHECK(fabs(values[i]-layout_reference[id][i])<1e-12);
    }
    for(size_t i=0;i<sizeof(fit_cases)/sizeof(fit_cases[0]);i++){
        RRCKFit f=rr_cockpit_fit(fit_cases[i].W,fit_cases[i].H);CHECK(fabs(f.w-fit_cases[i].w)<1e-8&&fabs(f.h-fit_cases[i].h)<1e-8&&fabs(f.font-fit_cases[i].font)<1e-8);
        CHECK(fabs(f.y+f.h-fit_cases[i].H+fit_cases[i].bottom)<1e-8);CHECK(f.x<=0&&f.w>=fit_cases[i].W);
        RRCKLayout l=rr_cockpit_layout(fit_cases[i].car);for(int c=0;c<l.controls;c++){RRUIRect r=rr_cockpit_box(f,l.control[c]);CHECK(r.w>0&&r.h>0);}
    }
    for(size_t i=0;i<sizeof(cockpit_cases)/sizeof(cockpit_cases[0]);i++){
        RRCKReadout v;rr_cockpit_readout(&v,cockpit_cases[i].car,cockpit_cases[i].speed,cockpit_cases[i].boost,cockpit_cases[i].time,cockpit_cases[i].heat,cockpit_cases[i].hot!=0,cockpit_cases[i].lap,cockpit_cases[i].laps,cockpit_cases[i].steer);
        CHECK(strcmp(v.speed,cockpit_cases[i].speed_out)==0&&strcmp(v.boost,cockpit_cases[i].boost_out)==0&&strcmp(v.time,cockpit_cases[i].time_out)==0&&strcmp(v.lap,cockpit_cases[i].lap_out)==0&&strcmp(v.heat,cockpit_cases[i].heat_out)==0&&strcmp(v.charge,cockpit_cases[i].charge)==0&&fabs(v.angle-cockpit_cases[i].angle)<1e-9);
    }
    CHECK(rr_cockpit_layout(1).controls==2&&rr_cockpit_layout(1).race_column);CHECK(rr_cockpit_layout(2).pivot_y==.39);
    CHECK(rr_cockpit_visible(true,RR_SESSION_ACTIVE,0,0,true));CHECK(!rr_cockpit_visible(true,RR_SESSION_ACTIVE,.0125001,0,true));CHECK(rr_cockpit_visible(true,RR_SESSION_ACTIVE,.0124999,0,true));
    CHECK(!rr_cockpit_visible(true,RR_SESSION_PAUSED,0,0,true)&&rr_cockpit_visible(true,RR_SESSION_RESULTS,0,0,true)&&!rr_cockpit_visible(true,RR_SESSION_MENU,0,0,true));
    CHECK(!rr_cockpit_visible(true,RR_SESSION_ACTIVE,0,0,false));CHECK(!rr_cockpit_visible(false,RR_SESSION_ACTIVE,0,0,true));
    CHECK(fabs(rr_cockpit_steer(0,1,.05,true)-.4)<1e-12);CHECK(fabs(rr_cockpit_steer(1,1,.05,false)-.6)<1e-12);
    RRCar car={0};car.pos=rr_v3(3,100,4);car.fwd=rr_v3(0,0,-1);RRCKSeat seat=rr_cockpit_seat(&car);CHECK(fabs(seat.position.x-3)<1e-12&&fabs(seat.position.y-101.5)<1e-12&&fabs(seat.position.z-3.4)<1e-12);CHECK(fabs(seat.forward.y+sin(.2))<1e-12&&fabs(rr_length(seat.forward)-1)<1e-12&&seat.extra_fov==8);
    for(size_t i=0;i<sizeof(seat_reference)/sizeof(seat_reference[0]);i++){
        const double *v=seat_reference[i];car.fwd=rr_v3(v[0],v[1],v[2]);seat=rr_cockpit_seat(&car);CHECK(rr_length(rr_sub(seat.position,rr_v3(v[3],v[4],v[5])))<1e-12);CHECK(rr_length(rr_sub(seat.forward,rr_v3(v[6],v[7],v[8])))<1e-12);
    }
    RRCKReadout units;rr_cockpit_readout(&units,0,0,0,0,0,false,1,2,0);CHECK(!strcmp(units.unit,"KM/H"));
    rr_cockpit_speed_unit(&units,hypot(30,40),RR_SPEED_MPH);CHECK(!strcmp(units.speed,"112")&&!strcmp(units.unit,"MPH"));
    rr_cockpit_speed_unit(&units,25,RR_SPEED_KMH);CHECK(!strcmp(units.speed,"90")&&!strcmp(units.unit,"KM/H"));
    printf("Cockpit: 18 source fits, 36 source readouts, 3 car layouts, visibility/easing/seat: %d failures\n",failures);return failures?1:0;
}
