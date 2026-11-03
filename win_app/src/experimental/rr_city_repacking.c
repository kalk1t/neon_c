#include "rr_city_repacking.h"
#include <string.h>
#include "generated_city_repacking.h"
int rr_repack_part_count(void){return (int)(sizeof(repack_parts)/sizeof(repack_parts[0]));}
const RRRepackPart *rr_repack_part(int i){return i>=0&&i<rr_repack_part_count()?&repack_parts[i]:NULL;}
size_t rr_repack_metadata_bytes(void){return sizeof(repack_parts);}
bool rr_repack_plan(const double source[6][4],const double near[6][4],uint32_t lo,uint32_t hi,bool near_only,RRRepackPlan *plan)
{
    if(!source||!near||!plan)return false;
    memset(plan,0,sizeof(*plan));plan->near_only=near_only;plan->member_hash=2166136261u;
    for(int i=0;i<6;i++)for(int j=0;j<4;j++)if(!isfinite(source[i][j])||!isfinite(near[i][j]))return false;
    for(int i=0;i<rr_repack_part_count();i++){
        const RRRepackPart *p=&repack_parts[i];bool parent=p->source_batch<32?((lo>>(unsigned int)p->source_batch)&1u)!=0:((hi>>(unsigned int)(p->source_batch-32))&1u)!=0;
        RRBatchVolume box={p->minimum,p->maximum,{0,0,0},0,p->vertices,p->material};
        if(!parent||!rr_batch_box_visible(&box,source)){plan->source_frustum_rejected++;continue;}
        plan->source_requested++;plan->source_vertices+=p->vertices;bool inside_near=rr_batch_box_visible(&box,near);
        if(!inside_near)plan->distance_omitted++;
        if(near_only&&!inside_near)continue;
        plan->ids[plan->count++]=i;plan->vertices+=p->vertices;plan->material_vertices[p->material]+=p->vertices;plan->member_hash=(plan->member_hash^(uint32_t)i)*16777619u;
    }
    plan->fits=plan->vertices<=RR_REPACK_VERTEX_BOUND;
    plan->source_coverage=!near_only||!plan->distance_omitted;
    return true;
}
bool rr_repack_execute_diagnostic(const RRRepackPlan *plan,RRRepackRead reader,void *context,unsigned char *target,size_t capacity,size_t offsets[5])
{
    if(!plan||!reader||!target||!offsets||!plan->fits||plan->count<0||plan->count>RR_REPACK_PARTS||plan->vertices>RR_REPACK_VERTEX_BOUND||capacity<(size_t)plan->vertices*32)return false;
    unsigned int counts[5]={0};
    for(int i=0;i<plan->count;i++){
        const RRRepackPart *p=rr_repack_part(plan->ids[i]);if(!p||(i&&plan->ids[i]<=plan->ids[i-1])||p->material<0||p->material>=5)return false;counts[p->material]+=p->vertices;
    }
    unsigned int total=0;
    for(int m=0;m<5;m++){if(counts[m]!=plan->material_vertices[m])return false;total+=counts[m];}
    if(total!=plan->vertices)return false;
    size_t at=0,cursors[5]={0};for(int m=0;m<5;m++){offsets[m]=at;at+=(size_t)counts[m]*32;}
    for(int i=0;i<plan->count;i++){
        const RRRepackPart *p=&repack_parts[plan->ids[i]];size_t bytes=(size_t)p->vertices*32,offset=(size_t)p->first*32;
        if(offset+bytes>RR_REPACK_SOURCE_BYTES||!reader(context,offset,bytes,target+offsets[p->material]+cursors[p->material]))return false;
        cursors[p->material]+=bytes;
    }
    return true;
}
uint32_t rr_repack_poly(const unsigned char *data,size_t bytes)
{
    uint32_t hash=0;for(size_t i=0;i<bytes;i++)hash=hash*16777619u+data[i];return hash;
}
