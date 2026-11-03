#include "rr_corridor.h"
#include <string.h>
static void reset(RRCorridor *c)
{
    memset(c,0,sizeof(*c));c->minimum=rr_v3(HUGE_VAL,HUGE_VAL,HUGE_VAL);c->maximum=rr_v3(-HUGE_VAL,-HUGE_VAL,-HUGE_VAL);
}
static void bounds(RRCorridor *c,RRVec3 p)
{
    c->minimum=rr_v3(fmin(c->minimum.x,p.x),fmin(c->minimum.y,p.y),fmin(c->minimum.z,p.z));
    c->maximum=rr_v3(fmax(c->maximum.x,p.x),fmax(c->maximum.y,p.y),fmax(c->maximum.z,p.z));
}
static bool index_grid(RRCorridor *c)
{
    if(c->coarse_count<=0)return false;
    double min_x=HUGE_VAL,min_z=HUGE_VAL,max_x=-HUGE_VAL,max_z=-HUGE_VAL;
    for(int i=0;i<c->coarse_count;i++){
        RRVec3 p=c->coarse[i];min_x=fmin(min_x,p.x);min_z=fmin(min_z,p.z);max_x=fmax(max_x,p.x);max_z=fmax(max_z,p.z);
    }
    c->cell_min_x=(int)floor(min_x/RR_CORRIDOR_CELL);c->cell_min_z=(int)floor(min_z/RR_CORRIDOR_CELL);
    c->cell_width=(int)floor(max_x/RR_CORRIDOR_CELL)-c->cell_min_x+1;
    c->cell_height=(int)floor(max_z/RR_CORRIDOR_CELL)-c->cell_min_z+1;
    if(c->cell_width>RR_CORRIDOR_GRID_SIDE||c->cell_height>RR_CORRIDOR_GRID_SIDE)return false;
    for(int i=0;i<RR_CORRIDOR_GRID_CELLS;i++)c->heads[i]=-1;
    for(int i=0;i<c->coarse_count;i++){
        int x=(int)floor(c->coarse[i].x/RR_CORRIDOR_CELL)-c->cell_min_x;
        int z=(int)floor(c->coarse[i].z/RR_CORRIDOR_CELL)-c->cell_min_z;
        int cell=x*c->cell_height+z;
        if(c->heads[cell]<0)c->occupied_cells++;
        c->next[i]=c->heads[cell];c->heads[cell]=i;
    }
    return true;
}
bool rr_corridor_init_all(RRCorridor *c,RRTrack *scratch)
{
    reset(c);
    for(int id=1;id<=RR_ROAD_COUNT;id++){
        if(!rr_track_init_road(scratch,id))return false;
        if(c->point_count+scratch->sample_count>RR_CORRIDOR_MAX_POINTS||c->gate_count+scratch->gate_count>RR_CORRIDOR_MAX_GATES)return false;
        for(int i=0;i<scratch->sample_count;i++){
            RRVec3 p=scratch->samples[i];c->points[c->point_count++]=p;bounds(c,p);
            if(i%5==0){if(c->coarse_count>=RR_CORRIDOR_MAX_COARSE)return false;c->coarse[c->coarse_count++]=p;}
        }
        for(int i=0;i<scratch->gate_count;i++)c->gates[c->gate_count++]=scratch->gates[i];
    }
    return index_grid(c)&&rr_track_init(scratch);
}
bool rr_corridor_init_points(RRCorridor *c,const RRVec3 *points,int count,const RRGate *gates,int gate_count)
{
    if(count<1||count>RR_CORRIDOR_MAX_COARSE||gate_count<0||gate_count>RR_CORRIDOR_MAX_GATES)return false;
    reset(c);c->point_count=count;c->coarse_count=count;c->gate_count=gate_count;
    for(int i=0;i<count;i++){c->points[i]=points[i];c->coarse[i]=points[i];bounds(c,points[i]);}
    for(int i=0;i<gate_count;i++)c->gates[i]=gates[i];
    return index_grid(c);
}
RRNearestXZ rr_corridor_nearest_xz(const RRCorridor *c,double x,double z)
{
    double distance=HUGE_VAL,height=0;
    for(int i=0;i<c->coarse_count;i++){
        RRVec3 p=c->coarse[i];double dx=p.x-x,dz=p.z-z,d=dx*dx+dz*dz;
        if(d<distance){distance=d;height=p.y;}
    }
    return (RRNearestXZ){sqrt(distance),height};
}
bool rr_corridor_clear(const RRCorridor *c,double x,double z,double radius,double top,double margin)
{
    if(!isfinite(x)||!isfinite(z)||!isfinite(radius)||!isfinite(top)||!isfinite(margin)||radius<0||margin<0)return false;
    double cx=floor(x/RR_CORRIDOR_CELL),cz=floor(z/RR_CORRIDOR_CELL);
    double reach=ceil((radius+margin)/RR_CORRIDOR_CELL);
    if(!isfinite(reach))return false;
    double low_x=fmax(0,cx-reach-c->cell_min_x),high_x=fmin(c->cell_width-1,cx+reach-c->cell_min_x);
    double low_z=fmax(0,cz-reach-c->cell_min_z),high_z=fmin(c->cell_height-1,cz+reach-c->cell_min_z);
    if(low_x>high_x||low_z>high_z)return true;
    int lo_x=(int)low_x,hi_x=(int)high_x,lo_z=(int)low_z,hi_z=(int)high_z;
    for(int i=lo_x;i<=hi_x;i++)for(int j=lo_z;j<=hi_z;j++)
        for(int p=c->heads[i*c->cell_height+j];p>=0;p=c->next[p]){
            RRVec3 point=c->coarse[p];
            if(hypot(point.x-x,point.z-z)-radius<margin && top>point.y-margin)return false;
        }
    return true;
}
