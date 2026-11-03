#include "rr_city_occlusion.h"
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
#include "generated_city_occlusion_cameras.h"
static RROcclusion depth;static unsigned char *geometry;static int failures;
static unsigned char witness_masks[2][70635];static int record_mask=-1;
#define CHECK(x) do{if(!(x)){if(failures<10)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static RRVec3 point(unsigned int index){float p[3];memcpy(p,geometry+(size_t)index*32,12);return rr_v3(p[0],p[1],p[2]);}
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
int main(int argc,char **argv)
{
    const double box[6][4]={{1,0,0,1},{-1,0,0,1},{0,1,0,1},{0,-1,0,1},{0,0,1,1},{0,0,-1,1}};RRVec3 polygon[16];
    for(size_t i=0;i<sizeof(triangle_fixtures)/sizeof(triangle_fixtures[0]);i++){const TriangleFixture *f=&triangle_fixtures[i];CHECK((rr_occlusion_clip(f->a,f->b,f->c,box,0,polygon)>0)==f->intersects);}
    RROcclusionPoint covering[3]={{-4,-4,10},{4,-4,10},{0,4,10}},behind[4]={{-1,-1,20},{1,-1,20},{1,1,20},{-1,1,20}};double maximum;
    CHECK(rr_occlusion_tile_depth(covering,-1,-1,1,1,&maximum)&&maximum>10&&maximum<12);CHECK(!rr_occlusion_tile_depth(covering,3,3,4,4,&maximum));
    rr_occlusion_clear(&depth);CHECK(!rr_occlusion_hidden(&depth,behind,4));rr_occlusion_raster(&depth,covering);CHECK(depth.covered==RR_OCCLUSION_W*RR_OCCLUSION_H&&rr_occlusion_hidden(&depth,behind,4));
    for(int i=0;i<4;i++)behind[i].depth=8;CHECK(!rr_occlusion_hidden(&depth,behind,4));for(int i=0;i<4;i++)behind[i].depth=20;depth.depth[0]=INFINITY;CHECK(!rr_occlusion_hidden(&depth,behind,4));
    RROcclusionPoint quad[4]={{-2,-2,10},{2,-2,10},{2,2,10},{-2,2,10}};rr_occlusion_clear(&depth);rr_occlusion_raster_quad(&depth,quad);CHECK(depth.covered==RR_OCCLUSION_W*RR_OCCLUSION_H&&rr_occlusion_hidden(&depth,behind,4));
    quad[2].x=-1;quad[2].y=-1;rr_occlusion_clear(&depth);rr_occlusion_raster_quad(&depth,quad);CHECK(depth.covered==0);
    if(argc<2||argc>3)return 2;FILE *file=NULL;
#if defined(_MSC_VER)
    if(fopen_s(&file,argv[1],"rb"))file=NULL;
#else
    file=fopen(argv[1],"rb");
#endif
    if(!file)return 2;geometry=malloc(RR_REPACK_SOURCE_BYTES);if(!geometry){fclose(file);return 2;}CHECK(fread(geometry,1,RR_REPACK_SOURCE_BYTES,file)==RR_REPACK_SOURCE_BYTES&&fgetc(file)==EOF);fclose(file);
    unsigned int max_retained=0,max_occluded=0;int worst=0,fit=0,cases=0;
    for(int view=0;view<264;view++){
        if(argc==3&&view!=0&&view!=255)continue;record_mask=argc==3?(view==0?0:1):-1;cases++;
        rr_occlusion_clear(&depth);unsigned int count,hidden;uint32_t hash;triangles(view,true,&count,&hidden,&hash);triangles(view,false,&count,&hidden,&hash);
        CHECK(count+hidden<=1695222);if(count>max_retained){max_retained=count;worst=view;}if(hidden>max_occluded)max_occluded=hidden;if(count<=RR_REPACK_VERTEX_BOUND)fit++;
        printf("OCCLUSION_VIEW {\"view\":%d,\"retained_vertices\":%u,\"occluded_vertices\":%u,\"covered_tiles\":%u,\"triangle_set_hash\":%u}\n",view,count,hidden,depth.covered,hash);
    }
    if(argc==3){FILE *output=NULL;
#if defined(_MSC_VER)
        if(fopen_s(&output,argv[2],"wb"))output=NULL;
#else
        output=fopen(argv[2],"wb");
#endif
        CHECK(output!=NULL);if(output){const unsigned int ids[2]={0,255};CHECK(fwrite("CWM1",1,4,output)==4);for(int i=0;i<2;i++){CHECK(fwrite(&ids[i],4,1,output)==1);CHECK(fwrite(witness_masks[i],1,70635,output)==70635);}fclose(output);}
    }
    printf("OCCLUSION_SUMMARY {\"cases\":%d,\"width\":128,\"height\":96,\"screen_guard\":0.01,\"depth_guard_ulp_factor\":32,\"source_far\":22000,\"worst_view\":%d,\"max_retained_vertices\":%u,\"within_bound\":%d,\"max_occluded_vertices\":%u,\"depth_buffer_bytes\":%zu,\"failures\":%d}\n",cases,worst,max_retained,fit,max_occluded,sizeof(depth),failures);
    free(geometry);return failures?1:0;
}
