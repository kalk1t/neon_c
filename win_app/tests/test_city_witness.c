#include "rr_city_witness.h"
#include "rr_city_triangle.h"
#include "rr_city_occlusion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct TriangleFixture {RRVec3 a,b,c;bool intersects,front;} TriangleFixture;
typedef struct RepackExpected {int parts;unsigned int vertices;uint32_t hash;unsigned int materials[5];uint32_t polys[5];} RepackExpected;
typedef struct RepackFixture {double source[6][4],near[6][4];uint32_t lo,hi;RepackExpected full,local;} RepackFixture;
#include "generated_city_triangle_reference.h"
#include "generated_city_repacking_reference.h"
#include "generated_city_occlusion_cameras.h"
#include "generated_coarse_witness_masks.h"
typedef struct UniqueSlot {uint32_t hash,index_plus_one;} UniqueSlot;
static int failures;static unsigned char *geometry;static RRWitnessScene scene;
#define CHECK(x) do{if(!(x)){if(failures<10)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static RRVec3 point(unsigned int i){float p[3];memcpy(p,geometry+(size_t)i*32,12);return rr_v3(p[0],p[1],p[2]);}
static void unique(UniqueSlot *slots,unsigned int index,unsigned int *count)
{
    const unsigned char *p=geometry+(size_t)index*32;uint32_t hash=2166136261u;for(int i=0;i<32;i++)hash=(hash^p[i])*16777619u;uint32_t slot=hash&((1u<<20)-1u);
    while(slots[slot].index_plus_one){if(slots[slot].hash==hash&&memcmp(geometry+(size_t)(slots[slot].index_plus_one-1)*32,p,32)==0)return;slot=(slot+1)&((1u<<20)-1u);}slots[slot]=(UniqueSlot){hash,index+1};(*count)++;
}
static bool blocker_proof(int view,const RRVec3 *polygon,int count,unsigned int blocker)
{
    RROcclusionPoint candidate[16],occluder[3];for(int i=0;i<count;i++)if(!rr_occlusion_project(polygon[i],occlusion_mvp[view],&candidate[i]))return false;
    for(unsigned int i=0;i<3;i++)if(!rr_occlusion_project(point(blocker+i),occlusion_mvp[view],&occluder[i]))return false;
    return rr_witness_polygon_occluded(candidate,count,occluder);
}
int main(int argc,char **argv)
{
    for(size_t i=0;i<sizeof(triangle_fixtures)/sizeof(triangle_fixtures[0]);i++){
        const TriangleFixture *f=&triangle_fixtures[i];RRVec3 o=rr_v3(0,0,10),target=rr_scale(rr_add(rr_add(f->a,f->b),f->c),1.0/3),d=rr_normalize(rr_sub(target,o));double t;bool ambiguous;
        CHECK(rr_witness_triangle(o,d,f->a,f->b,f->c,false,&t,&ambiguous)==f->front);CHECK(!ambiguous);
        CHECK(rr_witness_triangle(o,d,f->a,f->b,f->c,true,&t,&ambiguous));CHECK(rr_length(rr_sub(rr_add(o,rr_scale(d,t)),target))<1e-8);
    }
    RROcclusionPoint covering[3]={{-4,-4,10},{4,-4,10},{0,4,10}},behind[3]={{-.2,-.2,20},{.2,-.2,20},{0,.2,20}};
    CHECK(rr_witness_polygon_occluded(behind,3,covering));behind[1].depth=8;CHECK(!rr_witness_polygon_occluded(behind,3,covering));behind[1].depth=20;behind[1].x=8;CHECK(!rr_witness_polygon_occluded(behind,3,covering));
    if(argc!=2)return 2;FILE *file=NULL;
#if defined(_MSC_VER)
    if(fopen_s(&file,argv[1],"rb"))file=NULL;
#else
    file=fopen(argv[1],"rb");
#endif
    if(!file)return 2;geometry=malloc(RR_REPACK_SOURCE_BYTES);if(!geometry){fclose(file);return 2;}CHECK(fread(geometry,1,RR_REPACK_SOURCE_BYTES,file)==RR_REPACK_SOURCE_BYTES&&fgetc(file)==EOF);fclose(file);CHECK(rr_witness_scene_init(&scene,geometry));if(!scene.nodes)return 2;
    RRWitnessView validation={occlusion_mvp[255],repack_fixtures[255].lo,repack_fixtures[255].hi};
    for(unsigned int i=0;i<64;i++){
        unsigned int first=(i*8821u%565074u)*3;RRVec3 target=rr_scale(rr_add(rr_add(point(first),point(first+1)),point(first+2)),1.0/3),o=triangle_camera_eyes[255],delta=rr_sub(target,o);double length=rr_length(delta);RRVec3 d=rr_scale(delta,1/length);
        RRWitnessHit a=rr_witness_cast(&scene,&validation,o,d,length+.01,first,false),b=rr_witness_cast(&scene,&validation,o,d,length+.01,first,true);
        CHECK(a.hit==b.hit);if(a.hit)CHECK(fabs(a.distance-b.distance)<1e-6);
    }
    const int views[2]={255,0};
    for(int v=0;v<2;v++){
        int view=views[v];const RepackFixture *f=&repack_fixtures[view];RRWitnessView visibility={occlusion_mvp[view],f->lo,f->hi};RRVec3 eye=triangle_camera_eyes[view];unsigned int candidates=0,witnessed=0,sampled_blocked=0,uncertain=0,rays=0,unique_count=0,examples=0,proved=0,newly_proved=0;uint32_t hash=2166136261u;
        UniqueSlot *slots=calloc((size_t)1u<<20,sizeof(*slots));CHECK(slots!=NULL);if(!slots)break;
        for(int id=0;id<rr_repack_part_count();id++){
            const RRRepackPart *part=rr_repack_part(id);bool parent=part->source_batch<32?((f->lo>>(unsigned int)part->source_batch)&1u)!=0:((f->hi>>(unsigned int)(part->source_batch-32))&1u)!=0;if(!parent)continue;
            double expanded[6][4];memcpy(expanded,f->source,sizeof(expanded));for(int p=0;p<6;p++)expanded[p][3]+=.02;RRBatchVolume bounds={part->minimum,part->maximum,{0,0,0},0,part->vertices,part->material};if(!rr_batch_box_visible(&bounds,expanded))continue;
            for(unsigned int i=0;i<part->vertices;i+=3){unsigned int first=part->first+i;RRVec3 a=point(first),b=point(first+1),c=point(first+2),polygon[16];if(part->material!=4&&!rr_triangle_front(a,b,c,eye))continue;if(rr_occlusion_clip(a,b,c,f->source,.02,polygon)<=0)continue;candidates++;
                int count=rr_occlusion_clip(a,b,c,f->source,0,polygon);if(count<3){uncertain++;continue;}RRVec3 center=rr_v3(0,0,0);for(int p=0;p<count;p++)center=rr_add(center,polygon[p]);center=rr_scale(center,1.0/count);
                RRVec3 samples[5]={center,rr_mix(center,polygon[0],.3),rr_mix(center,polygon[count/3],.3),rr_mix(center,polygon[2*count/3],.3),rr_add(rr_scale(center,.7),rr_scale(rr_add(polygon[0],polygon[count/2]),.15))};
                bool visible=false,all_blocked=true;unsigned int blockers[5]={0};
                for(int sample=0;sample<5;sample++){
                    bool strict_inside=true;for(int plane=0;plane<6;plane++){const double *q=f->source[plane];if(q[0]*samples[sample].x+q[1]*samples[sample].y+q[2]*samples[sample].z+q[3]<=1e-7){strict_inside=false;break;}}
                    RRVec3 delta=rr_sub(samples[sample],eye);double length=rr_length(delta),epsilon=.0001+1e-7*length;if(!strict_inside||length<.31){all_blocked=false;continue;}RRVec3 d=rr_scale(delta,1/length);double own_distance;bool own_uncertain;
                    if(!rr_witness_triangle(eye,d,a,b,c,part->material==4,&own_distance,&own_uncertain)||own_uncertain||fabs(own_distance-length)>epsilon){all_blocked=false;continue;}rays++;
                    RRWitnessHit hit=rr_witness_cast(&scene,&visibility,eye,d,length+epsilon,first,false);
                    if(hit.hit&&hit.distance<length-epsilon){blockers[sample]=hit.triangle;continue;}
                    all_blocked=false;if(!hit.hit&&!hit.ambiguous){visible=true;break;}
                }
                bool coarse_hidden=(coarse_witness_masks[v][(first/3)/8]&(1u<<((first/3)%8)))!=0;
                if(visible){CHECK(!coarse_hidden);witnessed++;hash=(hash^first)*16777619u;for(unsigned int j=0;j<3;j++)unique(slots,first+j,&unique_count);}
                else if(all_blocked){sampled_blocked++;if(examples<32){bool proof=false;unsigned int chosen=0;for(int j=0;j<5;j++)if(blocker_proof(view,polygon,count,blockers[j])){proof=true;chosen=blockers[j];break;}if(proof){proved++;if(!coarse_hidden)newly_proved++;}printf("WITNESS_BLOCKERS {\"view\":%d,\"triangle_first_vertex\":%u,\"blockers\":[%u,%u,%u,%u,%u],\"whole_polygon_single_blocker_proof\":%s,\"coarse_already_occluded\":%s,\"new_proof_beyond_coarse\":%s,\"proof_blocker\":%u}\n",view,first,blockers[0],blockers[1],blockers[2],blockers[3],blockers[4],proof?"true":"false",coarse_hidden?"true":"false",proof&&!coarse_hidden?"true":"false",chosen);examples++;}}
                else uncertain++;
            }
        }
        CHECK(witnessed+sampled_blocked+uncertain==candidates);CHECK(witnessed>0);
        printf("WITNESS_SUMMARY {\"view\":%d,\"candidate_triangles\":%u,\"city_only_witnessed_triangles\":%u,\"all_five_samples_blocked_not_proven_hidden\":%u,\"unresolved_triangles\":%u,\"rays\":%u,\"witness_set_hash\":%u,\"exact_attribute_tuples\":%u,\"attribute_bytes32\":%zu,\"ordinary_triangle_index_bytes32\":%zu,\"bounded_proof_examples\":%u,\"whole_polygon_proved_examples\":%u,\"new_whole_polygon_proofs_beyond_coarse\":%u,\"bvh_nodes\":%d,\"bvh_capacity_bytes\":%zu,\"node_bytes_used\":%zu,\"failures\":%d}\n",view,candidates,witnessed,sampled_blocked,uncertain,rays,hash,unique_count,(size_t)unique_count*32,(size_t)witnessed*3*4,examples,proved,newly_proved,scene.count,(size_t)scene.capacity*sizeof(*scene.nodes),(size_t)scene.count*sizeof(*scene.nodes),failures);free(slots);
    }
    rr_witness_scene_free(&scene);free(geometry);return failures?1:0;
}
