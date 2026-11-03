#include "rr_city_witness.h"
#include "rr_city_coverage.h"
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

typedef struct UniqueSlot {uint32_t hash,index_plus_one;} UniqueSlot;
static int failures;static unsigned char *geometry;static RRWitnessScene scene;
#define CHECK(x) do{if(!(x)){if(failures<10)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static RRVec3 point(unsigned int i){float p[3];memcpy(p,geometry+(size_t)i*32,12);return rr_v3(p[0],p[1],p[2]);}
static void unique(UniqueSlot *slots,unsigned int index,unsigned int *count)
{
    const unsigned char *p=geometry+(size_t)index*32;uint32_t hash=2166136261u;for(int i=0;i<32;i++)hash=(hash^p[i])*16777619u;uint32_t slot=hash&((1u<<20)-1u);
    while(slots[slot].index_plus_one){if(slots[slot].hash==hash&&memcmp(geometry+(size_t)(slots[slot].index_plus_one-1)*32,p,32)==0)return;slot=(slot+1)&((1u<<20)-1u);}slots[slot]=(UniqueSlot){hash,index+1};(*count)++;
}
static RROcclusion depth;static unsigned char witness_masks[2][70635];static int record_mask=0;
static bool parent_visible(const RRRepackPart *p,const RepackFixture *f){return p->source_batch<32?((f->lo>>(unsigned int)p->source_batch)&1u)!=0:((f->hi>>(unsigned int)(p->source_batch-32))&1u)!=0;}
static double edge2(RROcclusionPoint a,RROcclusionPoint b,RROcclusionPoint c){return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);}
static void pair_occluder(int view,const RRRepackPart *part,unsigned int first)
{
    RRVec3 world[6];RROcclusionPoint projected[6],unique[4],hull[8];int count=0;
    for(int i=0;i<6;i++){
        world[i]=point(first+(unsigned int)i);
        for(int j=0;j<6;j++){const double *p=repack_fixtures[view].source[j];if(p[0]*world[i].x+p[1]*world[i].y+p[2]*world[i].z+p[3]<0)return;}
        if(!rr_occlusion_project(world[i],occlusion_mvp[view],&projected[i]))return;
        bool found=false;for(int j=0;j<i;j++)if(world[i].x==world[j].x&&world[i].y==world[j].y&&world[i].z==world[j].z){found=true;break;}
        if(!found){if(count==4)return;unique[count++]=projected[i];}
    }
    if(count!=4||(part->material!=4&&(!rr_triangle_front(world[0],world[1],world[2],triangle_camera_eyes[view])||!rr_triangle_front(world[3],world[4],world[5],triangle_camera_eyes[view]))))return;
    for(int i=1;i<4;i++){RROcclusionPoint p=unique[i];int j=i;while(j>0&&(p.x<unique[j-1].x||(p.x==unique[j-1].x&&p.y<unique[j-1].y))){unique[j]=unique[j-1];j--;}unique[j]=p;}
    int n=0;for(int i=0;i<4;i++){while(n>=2&&edge2(hull[n-2],hull[n-1],unique[i])<=0)n--;hull[n++]=unique[i];}
    int lower=n;for(int i=2;i>=0;i--){while(n>lower&&edge2(hull[n-2],hull[n-1],unique[i])<=0)n--;hull[n++]=unique[i];}n--;
    if(n!=4)return;
    double hull_area=0;for(int i=0;i<4;i++)hull_area+=hull[i].x*hull[(i+1)%4].y-hull[(i+1)%4].x*hull[i].y;
    double triangle_area=fabs(edge2(projected[0],projected[1],projected[2]))+fabs(edge2(projected[3],projected[4],projected[5]));
    if(fabs(triangle_area-fabs(hull_area))>1e-12*(fabs(hull_area)+1))return;
    rr_occlusion_raster_quad(&depth,hull);
}
static void triangles(int view,bool build,unsigned int *retained,unsigned int *occluded,uint32_t *hash)
{
    const RepackFixture *f=&repack_fixtures[view];double planes[6][4];memcpy(planes,f->source,sizeof(planes));for(int i=0;i<6;i++)if(!build)planes[i][3]+=.02;
    *retained=*occluded=0;*hash=2166136261u;
    for(int id=0;id<rr_repack_part_count();id++){
        const RRRepackPart *p=rr_repack_part(id);if(!parent_visible(p,f))continue;RRBatchVolume box={p->minimum,p->maximum,{0,0,0},0,p->vertices,p->material};if(!rr_batch_box_visible(&box,planes))continue;
        for(unsigned int i=0;i<p->vertices;i+=3){unsigned int first=p->first+i;if(build&&i%6==0&&i+6<=p->vertices)pair_occluder(view,p,first);RRVec3 a=point(first),b=point(first+1),c=point(first+2),polygon[16];if(p->material!=4&&!rr_triangle_front(a,b,c,triangle_camera_eyes[view]))continue;
            int count=rr_occlusion_clip(a,b,c,f->source,build?0:.02,polygon);CHECK(count>=0);if(count<=0)continue;RROcclusionPoint projected[16];bool stable=true;
            for(int j=0;j<count;j++)if(!rr_occlusion_project(polygon[j],occlusion_mvp[view],&projected[j])){stable=false;break;}
            if(build){if(stable)for(int j=1;j+1<count;j++){RROcclusionPoint triangle[3]={projected[0],projected[j],projected[j+1]};rr_occlusion_raster(&depth,triangle);}}
            else if(stable&&rr_occlusion_hidden(&depth,projected,count)){*occluded+=3;if(record_mask>=0){unsigned int triangle=first/3;witness_masks[record_mask][triangle/8]|=(unsigned char)(1u<<(triangle%8));}}
            else{*retained+=3;*hash=(*hash^first)*16777619u;}
        }
    }
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
    RROcclusionPoint target[4]={{-1,-1,20},{1,-1,20},{1,1,20},{-1,1,20}};
    RROcclusionPoint union_test[2][3]={{{-4,-4,10},{2,-4,10},{-1,6,10}},{{-2,-4,10},{4,-4,10},{1,6,10}}};unsigned int peak;
    CHECK(!rr_witness_polygon_occluded(target,4,union_test[0]));CHECK(!rr_witness_polygon_occluded(target,4,union_test[1]));CHECK(rr_coverage_union(target,4,union_test,2,&peak));
    RROcclusionPoint swap[3];memcpy(swap,union_test[0],sizeof(swap));memcpy(union_test[0],union_test[1],sizeof(swap));memcpy(union_test[1],swap,sizeof(swap));CHECK(rr_coverage_union(target,4,union_test,2,&peak));
    for(int j=0;j<3;j++)union_test[0][j].x+=10;CHECK(!rr_coverage_union(target,4,union_test,2,&peak));
    for(int j=0;j<3;j++){union_test[0][j].x-=10;union_test[0][j].depth=30;}CHECK(!rr_coverage_union(target,4,union_test,2,&peak));
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
    const int views[4]={255,249,251,0};
    for(int v=0;v<4;v++){
        int view=views[v];unsigned int old_retained,old_hidden;uint32_t old_hash;memset(witness_masks,0,sizeof(witness_masks));rr_occlusion_clear(&depth);triangles(view,true,&old_retained,&old_hidden,&old_hash);triangles(view,false,&old_retained,&old_hidden,&old_hash);
const RepackFixture *f=&repack_fixtures[view];RRWitnessView visibility={occlusion_mvp[view],f->lo,f->hi};RRVec3 eye=triangle_camera_eyes[view];unsigned int candidates=0,witnessed=0,sampled_blocked=0,uncertain=0,rays=0,unique_count=0;uint32_t hash=2166136261u;
        unsigned int singles=0,unions=0,union_only=0,retained_vertices=0,retained_tuples=0,max_pieces=0;UniqueSlot *retained_slots=calloc((size_t)1u<<20,sizeof(*retained_slots));CHECK(retained_slots!=NULL);if(!retained_slots)break;
        UniqueSlot *slots=calloc((size_t)1u<<20,sizeof(*slots));CHECK(slots!=NULL);if(!slots)break;
        for(int id=0;id<rr_repack_part_count();id++){
            const RRRepackPart *part=rr_repack_part(id);bool parent=part->source_batch<32?((f->lo>>(unsigned int)part->source_batch)&1u)!=0:((f->hi>>(unsigned int)(part->source_batch-32))&1u)!=0;if(!parent)continue;
            double expanded[6][4];memcpy(expanded,f->source,sizeof(expanded));for(int p=0;p<6;p++)expanded[p][3]+=.02;RRBatchVolume bounds={part->minimum,part->maximum,{0,0,0},0,part->vertices,part->material};if(!rr_batch_box_visible(&bounds,expanded))continue;
            for(unsigned int i=0;i<part->vertices;i+=3){unsigned int first=part->first+i;RRVec3 a=point(first),b=point(first+1),c=point(first+2),polygon[16];if(part->material!=4&&!rr_triangle_front(a,b,c,eye))continue;if(rr_occlusion_clip(a,b,c,f->source,.02,polygon)<=0)continue;candidates++;
                RRVec3 padded[16];int padded_count=rr_occlusion_clip(a,b,c,f->source,.02,padded);int count=rr_occlusion_clip(a,b,c,f->source,0,polygon);if(count<3){uncertain++;retained_vertices+=3;for(unsigned int j=0;j<3;j++)unique(retained_slots,first+j,&retained_tuples);continue;}RRVec3 center=rr_v3(0,0,0);for(int p=0;p<count;p++)center=rr_add(center,polygon[p]);center=rr_scale(center,1.0/count);
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
                bool coarse_hidden=(witness_masks[0][(first/3)/8]&(1u<<((first/3)%8)))!=0;
                bool single=false,combined=false;
                if(visible){CHECK(!coarse_hidden);witnessed++;hash=(hash^first)*16777619u;for(unsigned int j=0;j<3;j++)unique(slots,first+j,&unique_count);}
                else if(all_blocked){
                    sampled_blocked++;RROcclusionPoint candidate[16],occluders[5][3];int number=0;bool stable=padded_count>=3;for(int j=0;j<padded_count&&stable;j++)stable=rr_occlusion_project(padded[j],occlusion_mvp[view],&candidate[j]);
                    for(int j=0;j<5;j++){bool duplicate=false;for(int k=0;k<j;k++)if(blockers[j]==blockers[k])duplicate=true;if(duplicate)continue;bool okay=true;for(unsigned int k=0;k<3;k++)if(!rr_occlusion_project(point(blockers[j]+k),occlusion_mvp[view],&occluders[number][k]))okay=false;if(!okay)continue;number++;if(blocker_proof(view,padded,padded_count,blockers[j]))single=true;}
                    unsigned int peak=0;if(stable&&number)combined=rr_coverage_union(candidate,padded_count,occluders,number,&peak);if(peak>max_pieces)max_pieces=peak;
                    if(single){CHECK(combined);singles++;}if(combined){unions++;if(!single)union_only++;}
                }else uncertain++;
                if(!coarse_hidden&&!single&&!combined){retained_vertices+=3;for(unsigned int j=0;j<3;j++)unique(retained_slots,first+j,&retained_tuples);}
            }
        }
        CHECK(witnessed+sampled_blocked+uncertain==candidates);CHECK(witnessed>0);
        CHECK(retained_vertices<=old_retained);CHECK(retained_vertices>=witnessed*3);CHECK(singles<=unions);
        printf("COVERAGE_SUMMARY {\"view\":%d,\"candidate_triangles\":%u,\"coarse_retained_vertices\":%u,\"coarse_hash\":%u,\"city_only_witnessed_triangles\":%u,\"all_five_samples_blocked\":%u,\"unresolved\":%u,\"rays\":%u,\"single_blocker_proofs\":%u,\"combined_blocker_proofs\":%u,\"combined_only_proofs\":%u,\"retained_vertices\":%u,\"retained_exact_tuples\":%u,\"retained_indexed32_bytes\":%zu,\"vertex_bound\":%u,\"vertices_over_bound\":%u,\"maximum_uncovered_pieces\":%u,\"failures\":%d}\n",view,candidates,old_retained,old_hash,witnessed,sampled_blocked,uncertain,rays,singles,unions,union_only,retained_vertices,retained_tuples,(size_t)retained_tuples*32+(size_t)retained_vertices*4,RR_REPACK_VERTEX_BOUND,retained_vertices>RR_REPACK_VERTEX_BOUND?retained_vertices-RR_REPACK_VERTEX_BOUND:0,max_pieces,failures);
        free(slots);free(retained_slots);
    }
    rr_witness_scene_free(&scene);free(geometry);return failures?1:0;
}
