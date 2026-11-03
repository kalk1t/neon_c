#include "rr_city_batching.h"
#include <stddef.h>
static bool valid(const RRBatchVolume *v,const double p[6][4])
{
    if(!v||!p||!isfinite(v->radius)||v->radius<0||!isfinite(v->center.x)||!isfinite(v->center.y)||!isfinite(v->center.z))return false;
    if(!isfinite(v->minimum.x)||!isfinite(v->minimum.y)||!isfinite(v->minimum.z)||!isfinite(v->maximum.x)||!isfinite(v->maximum.y)||!isfinite(v->maximum.z)||v->minimum.x>v->maximum.x||v->minimum.y>v->maximum.y||v->minimum.z>v->maximum.z)return false;
    for(int i=0;i<6;i++)for(int j=0;j<4;j++)if(!isfinite(p[i][j]))return false;
    return true;
}
bool rr_batch_sphere_visible(const RRBatchVolume *v,const double p[6][4])
{
    if(!valid(v,p))return false;
    for(int i=0;i<6;i++)if(p[i][0]*v->center.x+p[i][1]*v->center.y+p[i][2]*v->center.z+p[i][3]<-v->radius)return false;
    return true;
}
bool rr_batch_box_visible(const RRBatchVolume *v,const double p[6][4])
{
    if(!valid(v,p))return false;
    for(int i=0;i<6;i++){
        double x=p[i][0]>0?v->maximum.x:v->minimum.x,y=p[i][1]>0?v->maximum.y:v->minimum.y,z=p[i][2]>0?v->maximum.z:v->minimum.z;
        if(p[i][0]*x+p[i][1]*y+p[i][2]*z+p[i][3]<0)return false;
    }
    return true;
}
