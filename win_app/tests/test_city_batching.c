#include "rr_city_batching.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef struct BatchExpected {int count;unsigned int vertices;uint32_t hash;} BatchExpected;
typedef struct BatchViewFixture {double planes[6][4];BatchExpected expected[10];} BatchViewFixture;
#include "generated_city_batches.h"
#include "generated_city_batch_reference.h"
static int failures;
#define CHECK(x) do{if(!(x)){if(failures<10)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
int main(void)
{
    CHECK(sizeof(batch_schemes)/sizeof(batch_schemes[0])==5&&batch_schemes[0].count==45);unsigned int source_materials[5]={0};size_t volume_bytes=0;
    for(int scheme=0;scheme<5;scheme++){
        unsigned int sum=0,materials[5]={0};volume_bytes+=(size_t)batch_schemes[scheme].count*sizeof(RRBatchVolume);
        for(int i=0;i<batch_schemes[scheme].count;i++){
            const RRBatchVolume *v=&batch_schemes[scheme].items[i];CHECK(v->vertices>0&&v->vertices%3==0);sum+=v->vertices;
            CHECK(isfinite(v->radius)&&v->radius>0&&v->minimum.x<=v->maximum.x&&v->minimum.y<=v->maximum.y&&v->minimum.z<=v->maximum.z);
            if(scheme<4){CHECK(v->material>=0&&v->material<5);if(v->material>=0&&v->material<5)materials[v->material]+=v->vertices;}
        }
        CHECK(sum==1695222);if(!scheme)memcpy(source_materials,materials,sizeof(materials));else if(scheme<4)CHECK(memcmp(source_materials,materials,sizeof(materials))==0);
    }
    unsigned int max_native128_box=0,max_source_sphere=0;int max_draws128=0;
    for(size_t view=0;view<sizeof(batch_view_fixtures)/sizeof(batch_view_fixtures[0]);view++){
        const BatchViewFixture *f=&batch_view_fixtures[view];
        for(int scheme=0;scheme<5;scheme++)for(int box=0;box<2;box++){
            int count=0;unsigned int vertices=0;uint32_t hash=2166136261u;
            for(int i=0;i<batch_schemes[scheme].count;i++){
                const RRBatchVolume *v=&batch_schemes[scheme].items[i];bool visible=box?rr_batch_box_visible(v,f->planes):rr_batch_sphere_visible(v,f->planes);
                if(visible){count++;vertices+=v->vertices;hash=(hash^(uint32_t)i)*16777619u;}
            }
            const BatchExpected *expected=&f->expected[scheme*2+box];CHECK(count==expected->count&&vertices==expected->vertices&&hash==expected->hash);
            if(view<264&&scheme==1&&box){if(vertices>max_native128_box)max_native128_box=vertices;if(count>max_draws128)max_draws128=count;}
            if(scheme==0&&!box&&vertices>max_source_sphere)max_source_sphere=vertices;
        }
    }
    CHECK(max_native128_box==92676&&max_draws128==265&&max_source_sphere==1695222);
    RRBatchVolume invalid=batch_schemes[0].items[0];invalid.radius=NAN;CHECK(!rr_batch_sphere_visible(&invalid,batch_view_fixtures[0].planes));
    invalid=batch_schemes[0].items[0];invalid.minimum.x=invalid.maximum.x+1;CHECK(!rr_batch_box_visible(&invalid,batch_view_fixtures[0].planes));
    CHECK(!rr_batch_box_visible(NULL,batch_view_fixtures[0].planes));
    printf("HEADLESS batching:528 route/roam frusta x5 exact-topology schemes x2 source Three bounds;5280 visible counts/vertex sums/set hashes;1695222 vertices/material totals conserved per scheme; original45 source batches; near128/AABB max92676 vertices,265 draws; full source sphere max1695222; metadata=%zuB; %d failures.\n",volume_bytes,failures);
    return failures?1:0;
}
