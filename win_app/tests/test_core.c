#include <stdio.h>
#include <math.h>
#include "rr_flight.h"
#include "rr_track.h"

typedef struct FlightFixture { int scenario; double dt; RRInput input; int spin; double expected[16]; } FlightFixture;
typedef struct CurveFixture { double u; RRVec3 point, at, tangent; } CurveFixture;
#include "generated_reference.h"

static int failures=0;
static double worst_error=0;
static void near_value(double got,double want,double tolerance,const char *label,size_t row)
{
    double error=fabs(got-want);
    if(error>worst_error) worst_error=error;
    if(!isfinite(got) || error>tolerance){
        if(failures<8) fprintf(stderr,"%s[%zu]: got %.17g, expected %.17g (error %.3g)\n",label,row,got,want,error);
        failures++;
    }
}
static void near_vector(RRVec3 got,RRVec3 want,const char *label,size_t row)
{
    near_value(got.x,want.x,1e-8,label,row);near_value(got.y,want.y,1e-8,label,row);near_value(got.z,want.z,1e-8,label,row);
}
int main(void)
{
    RRCar car={0}; int previous=-1;
    size_t count=sizeof(flight_fixtures)/sizeof(flight_fixtures[0]);
    for(size_t i=0;i<count;i++){
        const FlightFixture *f=&flight_fixtures[i];
        if(f->scenario!=previous){rr_car_reset(&car,rr_v3(0,118,0),0,0);previous=f->scenario;}
        if(f->spin)rr_car_dodge(&car,f->spin);
        rr_car_step(&car,f->input,f->dt);
        const double actual[]={car.pos.x,car.pos.y,car.pos.z,car.vel.x,car.vel.y,car.vel.z,car.yaw,car.pitch,car.roll,car.speed,car.boost,car.roll_time,car.surge,car.thrust,car.nitro,car.stun};
        for(int j=0;j<16;j++)near_value(actual[j],f->expected[j],1e-8,"flight",i);
    }
    static RRTrack track;
    if(!rr_track_init(&track)){fputs("Track initialization failed\n",stderr);return 1;}
    near_value(track.length,reference_length,1e-8,"track length",0);
    for(size_t i=0;i<sizeof(curve_fixtures)/sizeof(curve_fixtures[0]);i++){
        const CurveFixture *f=&curve_fixtures[i];
        near_vector(rr_track_point(f->u),f->point,"curve",i);
        near_vector(rr_track_point_at(&track,f->u),f->at,"arc length mapping",i);
        near_vector(rr_track_tangent(f->u),f->tangent,"tangent",i);
    }
    for(size_t i=0;i<RR_TRACK_GATES;i++){
        near_vector(track.gates[i].pos,reference_gates[i].pos,"gate position",i);
        near_vector(track.gates[i].forward,reference_gates[i].forward,"gate forward",i);
    }
    near_value(rr_frame_dt(3),0.05,0,"frame cap",0);
    near_value(rr_frame_dt(-1),0,0,"invalid frame",0);
    // Gate crossings require forward plane passage through the aperture.
    RRGate gate={ {0,118,0},{0,0,-1},0 };
    if(!rr_gate_crossed(&gate,rr_v3(0,118,40),rr_v3(0,118,-40),13.2))failures++;
    if(rr_gate_crossed(&gate,rr_v3(0,118,-40),rr_v3(0,118,40),13.2))failures++;
    if(rr_gate_crossed(&gate,rr_v3(30,118,40),rr_v3(30,118,-40),13.2))failures++;
    if(rr_gate_crossed(&gate,rr_v3(0,118,40),rr_v3(0,118,20),13.2))failures++;
    printf("%zu browser flight steps; 81 spline/arc/tangent samples; 8 gates. Worst absolute error %.3g.\n",count,worst_error);
    return failures?1:0;
}
