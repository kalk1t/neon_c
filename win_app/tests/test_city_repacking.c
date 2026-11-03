#include "rr_city_repacking.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct RepackExpected {int parts;unsigned int vertices;uint32_t hash;unsigned int materials[5];uint32_t polys[5];} RepackExpected;
typedef struct RepackFixture {double source[6][4],near[6][4];uint32_t lo,hi;RepackExpected full,local;} RepackFixture;
#include "generated_city_repacking_reference.h"
static RRRepackPlan source_plan,near_plan,repeat_plan;
static int failures,read_calls;
#define CHECK(x) do{if(!(x)){if(failures<12)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static bool read_source(void *context,size_t offset,size_t bytes,unsigned char *target)
{
    FILE *file=context;read_calls++;return offset+bytes<=RR_REPACK_SOURCE_BYTES&&fseek(file,(long)offset,SEEK_SET)==0&&fread(target,1,bytes,file)==bytes;
}
static bool fail_read(void *context,size_t offset,size_t bytes,unsigned char *target)
{(void)context;(void)offset;(void)bytes;(void)target;return false;}
static void expected(const RRRepackPlan *p,const RepackExpected *e)
{
    CHECK(p->count==e->parts&&p->vertices==e->vertices&&p->member_hash==e->hash);CHECK(memcmp(p->material_vertices,e->materials,sizeof(e->materials))==0);
}
static void verify_bytes(const RRRepackPlan *p,const RepackExpected *e,FILE *file,unsigned char *buffer,size_t capacity)
{
    size_t offsets[5];CHECK(rr_repack_execute_diagnostic(p,read_source,file,buffer,capacity,offsets));int draws=0;
    for(int m=0;m<5;m++){if(p->material_vertices[m])draws++;CHECK(rr_repack_poly(buffer+offsets[m],(size_t)p->material_vertices[m]*32)==e->polys[m]);}
    CHECK(draws<=5);for(size_t i=capacity;i<capacity+16;i++)CHECK(buffer[i]==0x5a);
}
int main(int argc,char **argv)
{
    if(argc!=2){puts("Source geometry fixture path required");return 2;}
    FILE *file=NULL;
#if defined(_MSC_VER)
    if(fopen_s(&file,argv[1],"rb"))file=NULL;
#else
    file=fopen(argv[1],"rb");
#endif
    if(!file){puts("Cannot open test-only source geometry fixture");return 2;}
    CHECK(fseek(file,0,SEEK_END)==0&&ftell(file)==(long)RR_REPACK_SOURCE_BYTES);CHECK(rr_repack_part_count()==RR_REPACK_PARTS);
    unsigned int first=0;for(int i=0;i<rr_repack_part_count();i++){const RRRepackPart *p=rr_repack_part(i);CHECK(p->first==first&&p->vertices%3==0&&p->material>=0&&p->material<5&&p->source_batch>=0&&p->source_batch<45);first+=p->vertices;}CHECK(first==1695222);
    const size_t capacity=(size_t)RR_REPACK_VERTEX_BOUND*32;unsigned char *buffer=malloc(capacity+16);if(!buffer){fclose(file);return 2;}memset(buffer+capacity,0x5a,16);
    int source_fit=0,distance_cases=0,full_blocked=0;unsigned int max_near=0,max_full=0;
    for(size_t i=0;i<sizeof(repack_fixtures)/sizeof(repack_fixtures[0]);i++){
        const RepackFixture *f=&repack_fixtures[i];CHECK(rr_repack_plan(f->source,f->near,f->lo,f->hi,false,&source_plan));CHECK(rr_repack_plan(f->source,f->near,f->lo,f->hi,true,&near_plan));CHECK(rr_repack_plan(f->source,f->near,f->lo,f->hi,true,&repeat_plan));CHECK(memcmp(&near_plan,&repeat_plan,sizeof(near_plan))==0);
        expected(&source_plan,&f->full);expected(&near_plan,&f->local);
        CHECK(source_plan.source_frustum_rejected+source_plan.source_requested==RR_REPACK_PARTS);CHECK(near_plan.source_requested==source_plan.count&&near_plan.distance_omitted==source_plan.count-near_plan.count);
        CHECK(source_plan.source_coverage&&near_plan.source_coverage==(near_plan.distance_omitted==0));CHECK(near_plan.fits);
        if(near_plan.distance_omitted)distance_cases++;if(near_plan.vertices>max_near)max_near=near_plan.vertices;if(source_plan.vertices>max_full)max_full=source_plan.vertices;
        verify_bytes(&near_plan,&f->local,file,buffer,capacity);
        if(source_plan.fits){source_fit++;verify_bytes(&source_plan,&f->full,file,buffer,capacity);}else{size_t offsets[5];int previous=read_calls;CHECK(!rr_repack_execute_diagnostic(&source_plan,read_source,file,buffer,capacity,offsets)&&read_calls==previous);full_blocked++;}
    }
    CHECK(source_fit==18&&full_blocked==246&&distance_cases==262&&max_near==76434&&max_full==1674882);
    const RepackFixture *a=&repack_fixtures[136],*b=&repack_fixtures[172];
    CHECK(rr_repack_plan(a->source,a->near,a->lo,a->hi,true,&source_plan));CHECK(rr_repack_plan(b->source,b->near,b->lo,b->hi,true,&near_plan));
    int left=0,right=0;unsigned int shared_union=0;
    while(left<source_plan.count||right<near_plan.count){
        int id;if(right==near_plan.count||(left<source_plan.count&&source_plan.ids[left]<near_plan.ids[right]))id=source_plan.ids[left++];
        else if(left==source_plan.count||near_plan.ids[right]<source_plan.ids[left])id=near_plan.ids[right++];
        else{id=source_plan.ids[left++];right++;}
        shared_union+=rr_repack_part(id)->vertices;
    }
    CHECK(shared_union==147432&&shared_union>RR_REPACK_VERTEX_BOUND);
    const RepackFixture *f=&repack_fixtures[0];CHECK(rr_repack_plan(f->source,f->near,f->lo,f->hi,true,&near_plan));
    size_t offsets[5];CHECK(!rr_repack_execute_diagnostic(&near_plan,read_source,file,buffer,0,offsets));CHECK(!rr_repack_execute_diagnostic(&near_plan,fail_read,NULL,buffer,capacity,offsets));
    printf("HEADLESS repack only:38352 exact source parts,54247104B source fixture;264 source/near view pairs; exact per-material position/normal/UV byte checksums;near max76434 vertices/5 draws;262 distance-omission cases explicit;source max1674882 vertices,18 fit/246 budget-rejected before reads;ideal two-view union147432 exceeds141312;output bound141312 vertices/4521984B plus guards;metadata=%zuB; %d failures.\n",rr_repack_metadata_bytes(),failures);
    free(buffer);fclose(file);return failures?1:0;
}
