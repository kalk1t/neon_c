#include "rr_city_triangle.h"
#include "rr_city_repacking.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct TriangleFixture {RRVec3 a,b,c;bool intersects,front;} TriangleFixture;
typedef struct RepackExpected {int parts;unsigned int vertices;uint32_t hash;unsigned int materials[5];uint32_t polys[5];} RepackExpected;
typedef struct RepackFixture {double source[6][4],near[6][4];uint32_t lo,hi;RepackExpected full,local;} RepackFixture;
#include "generated_city_triangle_reference.h"
#include "generated_city_repacking_reference.h"
typedef struct TriangleResult {unsigned int candidate,strict,padded,back_rejected,unique,unique_material[5];uint32_t hash;} TriangleResult;
typedef struct VertexSlot {uint32_t hash,index_plus_one,material;} VertexSlot;
static unsigned char *geometry;static int failures;static double margin=.02;
#define CHECK(x) do{if(!(x)){if(failures<10)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static RRVec3 point(unsigned int index){float p[3];memcpy(p,geometry+(size_t)index*32,12);return rr_v3(p[0],p[1],p[2]);}
static bool inside_box(const RRRepackPart *part,const double planes[6][4],double pad)
{
    for(int i=0;i<6;i++){const double *p=planes[i];double x=p[0]>=0?part->minimum.x:part->maximum.x,y=p[1]>=0?part->minimum.y:part->maximum.y,z=p[2]>=0?part->minimum.z:part->maximum.z;if(p[0]*x+p[1]*y+p[2]*z+p[3]<-pad)return false;}return true;
}
static void unique_vertex(VertexSlot *slots,unsigned int vertex,unsigned int material,TriangleResult *result)
{
    const unsigned char *data=geometry+(size_t)vertex*32;uint32_t hash=2166136261u^material;for(int i=0;i<32;i++)hash=(hash^data[i])*16777619u;
    uint32_t slot=hash&((1u<<21)-1u);
    while(slots[slot].index_plus_one){const VertexSlot *s=&slots[slot];if(s->hash==hash&&s->material==material&&memcmp(geometry+(size_t)(s->index_plus_one-1)*32,data,32)==0)return;slot=(slot+1)&((1u<<21)-1u);}
    slots[slot]=(VertexSlot){hash,vertex+1,material};result->unique++;result->unique_material[material]++;
}
static TriangleResult analyze(int view,VertexSlot *slots)
{
    TriangleResult result={0};result.hash=2166136261u;const RepackFixture *f=&repack_fixtures[view];double padded[6][4];memcpy(padded,f->source,sizeof(padded));for(int j=0;j<6;j++)padded[j][3]+=margin;
    for(int id=0;id<rr_repack_part_count();id++){
        const RRRepackPart *p=rr_repack_part(id);bool parent=p->source_batch<32?((f->lo>>(unsigned int)p->source_batch)&1u)!=0:((f->hi>>(unsigned int)(p->source_batch-32))&1u)!=0;if(!parent)continue;
        RRBatchVolume bounds={p->minimum,p->maximum,{0,0,0},0,p->vertices,p->material};bool strict_box=rr_batch_box_visible(&bounds,f->source);
        if(strict_box)result.candidate+=p->vertices;
        if(!rr_batch_box_visible(&bounds,padded))continue;
        bool all_strict=inside_box(p,f->source,0),all_padded=inside_box(p,f->source,margin);
        for(unsigned int i=0;i<p->vertices;i+=3){
            unsigned int first=p->first+i;RRVec3 a=point(first),b=point(first+1),c=point(first+2);
            if(p->material!=4&&!rr_triangle_front(a,b,c,triangle_camera_eyes[view])){result.back_rejected+=3;continue;}
            if(strict_box&&(all_strict||rr_triangle_frustum(a,b,c,f->source,0)))result.strict+=3;
            if(all_padded||rr_triangle_frustum(a,b,c,f->source,margin)){
                result.padded+=3;result.hash=(result.hash^first)*16777619u;
                if(slots)for(unsigned int j=0;j<3;j++)unique_vertex(slots,first+j,(unsigned int)p->material,&result);
            }
        }
    }
    return result;
}
int main(int argc,char **argv)
{
    const double box[6][4]={{1,0,0,1},{-1,0,0,1},{0,1,0,1},{0,-1,0,1},{0,0,1,1},{0,0,-1,1}};
    for(size_t i=0;i<sizeof(triangle_fixtures)/sizeof(triangle_fixtures[0]);i++){
        const TriangleFixture *f=&triangle_fixtures[i];CHECK(rr_triangle_frustum(f->a,f->b,f->c,box,0)==f->intersects);CHECK(rr_triangle_front(f->a,f->b,f->c,rr_v3(0,0,10))==f->front);
    }
    CHECK(rr_triangle_frustum(rr_v3(-2,0,0),rr_v3(0,2,0),rr_v3(2,-2,0),box,0));CHECK(!rr_triangle_frustum(rr_v3(2,2,2),rr_v3(3,2,2),rr_v3(2,3,2),box,0));
    if(argc!=2){puts("Geometry fixture required");return 2;}FILE *file=NULL;
#if defined(_MSC_VER)
    if(fopen_s(&file,argv[1],"rb"))file=NULL;
#else
    file=fopen(argv[1],"rb");
#endif
    if(!file)return 2;geometry=malloc(RR_REPACK_SOURCE_BYTES);if(!geometry){fclose(file);return 2;}CHECK(fread(geometry,1,RR_REPACK_SOURCE_BYTES,file)==RR_REPACK_SOURCE_BYTES);CHECK(fgetc(file)==EOF);fclose(file);
    unsigned int maximum=0;int worst=0,fit=0;
    for(int view=0;view<264;view++){
        for(int p=0;p<4;p++){const double *plane=repack_fixtures[view].source[p];RRVec3 eye=triangle_camera_eyes[view];CHECK(fabs(plane[0]*eye.x+plane[1]*eye.y+plane[2]*eye.z+plane[3])<1e-7);}
        TriangleResult r=analyze(view,NULL);CHECK(r.candidate==repack_fixtures[view].full.vertices&&r.padded>=r.strict&&r.padded<=1695222);
        if(r.padded>maximum){maximum=r.padded;worst=view;}if(r.padded<=RR_REPACK_VERTEX_BOUND)fit++;
        printf("TRIANGLE_VIEW {\"view\":%d,\"candidate\":%u,\"strict_vertices\":%u,\"padded_vertices\":%u,\"back_rejected_vertices\":%u,\"triangle_set_hash\":%u}\n",view,r.candidate,r.strict,r.padded,r.back_rejected,r.hash);
    }
    VertexSlot *slots=calloc((size_t)1u<<21,sizeof(VertexSlot));CHECK(slots!=NULL);TriangleResult indexed={0};if(slots)indexed=analyze(worst,slots);
    CHECK(indexed.padded==maximum&&indexed.unique<=indexed.padded);
    printf("TRIANGLE_SUMMARY {\"cases\":264,\"goldens\":600,\"margin\":%.8g,\"worst_view\":%d,\"max_retained_vertices\":%u,\"within_bound\":%d,\"unique_vertices_worst\":%u,\"index_entries_worst\":%u,\"unique_materials\":[%u,%u,%u,%u,%u],\"attribute_bytes32_worst\":%zu,\"indices_bytes32_worst\":%zu,\"analysis_hash_workspace\":%zu,\"failures\":%d}\n",margin,worst,maximum,fit,indexed.unique,indexed.padded,indexed.unique_material[0],indexed.unique_material[1],indexed.unique_material[2],indexed.unique_material[3],indexed.unique_material[4],(size_t)indexed.unique*32,(size_t)indexed.padded*4,((size_t)1u<<21)*sizeof(VertexSlot),failures);
    free(slots);free(geometry);return failures?1:0;
}
