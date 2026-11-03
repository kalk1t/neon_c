#include "rr_posters.h"
#include "rr_canonical_city.h"
#include "rr_render_policy.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "generated_poster_reference.h"
typedef struct PosterSampleFixture {double uv[2],canvas[2];int texels[4];double weights[2];} PosterSampleFixture;
#include "generated_poster_sampling.h"
static int failures;
#define CHECK(x) do{if(!(x)){if(failures<10)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static uint32_t hash(const unsigned char *p,size_t bytes){uint32_t h=2166136261u;for(size_t i=0;i<bytes;i++)h=(h^p[i])*16777619u;return h;}
static int compare(const void *a,const void *b){unsigned int x=*(const unsigned int *)a,y=*(const unsigned int *)b;return x<y?-1:x>y?1:0;}
static void winding(float p[3][8])
{
    double ax=p[1][0]-p[0][0],ay=p[1][1]-p[0][1],az=p[1][2]-p[0][2],bx=p[2][0]-p[0][0],by=p[2][1]-p[0][1],bz=p[2][2]-p[0][2];
    CHECK((ay*bz-az*by)*p[0][3]+(az*bx-ax*bz)*p[0][4]+(ax*by-ay*bx)*p[0][5]>1);
}
int main(void)
{
    CHECK(sizeof(float)==4);const uint32_t endian=1;CHECK(*(const unsigned char *)&endian==1);CHECK(rr_poster_count()==97);
    bool used[413]={false};unsigned int sizes[413];for(int i=0;i<413;i++)sizes[i]=rr_canonical_city_building(i)->vertices;
    unsigned int quadrants[4]={0};double worst_plane=0;
    for(int i=0;i<rr_poster_count();i++){
        const RRPoster *p=rr_poster_get(i);CHECK(p!=NULL);if(!p)continue;CHECK(p->canonical_index>=0&&p->canonical_index<413);const RRCanonicalBuilding *b=rr_canonical_city_building(p->canonical_index);CHECK(b!=NULL);if(!b)continue;
        CHECK(b->mesh_id==p->mesh_id&&!used[p->canonical_index]);used[p->canonical_index]=true;sizes[p->canonical_index]+=12;
        CHECK(hash((const unsigned char *)p->rows,sizeof(p->rows))==poster_expected_hashes[i]);
        const double *e=poster_expected_colliders[i];CHECK(fabs(b->center.x-e[0])<1e-9&&fabs(b->center.y-e[1])<1e-9&&fabs(b->center.z-e[2])<1e-9);CHECK(fabs(b->half.x-e[3])<1e-9&&fabs(b->half.y-e[4])<1e-9&&fabs(b->half.z-e[5])<1e-9);CHECK(fabs(b->cosine-e[6])<1e-9&&fabs(b->sine-e[7])<1e-9);
        float vertices[12][8];memset(vertices,0x5a,sizeof(vertices));float untouched[12][8];memcpy(untouched,vertices,sizeof(vertices));CHECK(!rr_poster_expand(i,true,vertices,11));CHECK(memcmp(untouched,vertices,sizeof(vertices))==0);CHECK(rr_poster_expand(i,true,vertices,12));CHECK(memcmp(vertices,p->rows,sizeof(p->rows))==0);
        double u0=1,v0=1,u1=0,v1=0;
        for(int j=0;j<6;j++){
            const float *v=p->rows[j];double normal=v[3]*v[3]+v[4]*v[4]+v[5]*v[5];CHECK(fabs(normal-1)<1e-6);
            CHECK(v[0]>=b->mesh_bounds.minimum.x&&v[0]<=b->mesh_bounds.maximum.x&&v[1]>=b->mesh_bounds.minimum.y&&v[1]<=b->mesh_bounds.maximum.y&&v[2]>=b->mesh_bounds.minimum.z&&v[2]<=b->mesh_bounds.maximum.z);
            double plane=(v[0]-p->rows[0][0])*v[3]+(v[1]-p->rows[0][1])*v[4]+(v[2]-p->rows[0][2])*v[5];if(fabs(plane)>worst_plane)worst_plane=fabs(plane);CHECK(fabs(plane)<.0005);
            u0=fmin(u0,v[6]);u1=fmax(u1,v[6]);v0=fmin(v0,v[7]);v1=fmax(v1,v[7]);
        }
        CHECK(u1-u0==.5&&v1-v0==.5&&(u0==0||u0==.5)&&(v0==0||v0==.5));quadrants[(int)(u0*2)+(int)(v0*2)*2]++;
        for(int t=0;t<4;t++)winding(vertices+t*3);
        for(int t=0;t<2;t++)for(int j=0;j<3;j++){const float *front=p->rows[t*3+2-j],*back=vertices[6+t*3+j];for(int k=0;k<8;k++)CHECK(back[k]==(k>=3&&k<6?-front[k]:front[k]));}
    }
    qsort(sizes,413,sizeof(*sizes),compare);unsigned int top=0;for(int i=413-RR_BUILDING_BUDGET;i<413;i++)top+=sizes[i];CHECK(top==112668&&top<=141312&&sizes[412]==2220);
    for(int q=0;q<4;q++){CHECK(quadrants[q]>0);for(int k=0;k<7;k++){double points[3][2];CHECK(rr_poster_glyph(q,k,points));for(int i=0;i<3;i++)for(int j=0;j<2;j++)CHECK(fabs(points[i][j]-poster_expected_glyphs[q][k][i][j])<1e-12);}}
    RRPosterMaterial m=rr_poster_material(),e=poster_expected_material;CHECK(m.width==e.width&&m.height==e.height&&m.anisotropy==e.anisotropy&&m.flip_y==e.flip_y&&m.srgb==e.srgb&&m.double_sided==e.double_sided&&m.unlit==e.unlit&&m.transparent==e.transparent&&m.depth_test==e.depth_test&&m.depth_write==e.depth_write&&m.mipmaps==e.mipmaps&&m.clamp_edge==e.clamp_edge&&m.premultiply_alpha==e.premultiply_alpha&&m.fog==e.fog&&m.tone_mapped==e.tone_mapped&&m.linear_color==e.linear_color&&m.opacity==e.opacity);
    CHECK(rr_poster_rgba_mip_bytes(m.width,m.height)==2796204);CHECK(rr_poster_rgba_mip_bytes(0,1024)==0&&rr_poster_rgba_mip_bytes(8192,1024)==0);
    CHECK(!rr_poster_get(-1)&&!rr_poster_get(97)&&!rr_poster_expand(-1,false,NULL,0)&&!rr_poster_glyph(4,0,NULL)&&!rr_poster_glyph(0,7,NULL));

    for(size_t i=0;i<sizeof(poster_samples)/sizeof(*poster_samples);i++){
        const PosterSampleFixture *f=&poster_samples[i];RRPosterSample a;CHECK(rr_poster_sample(f->uv[0],f->uv[1],&a));CHECK(fabs(a.canvas_u-f->canvas[0])<1e-12&&fabs(a.canvas_v-f->canvas[1])<1e-12);CHECK(a.x0==f->texels[0]&&a.x1==f->texels[1]&&a.y0==f->texels[2]&&a.y1==f->texels[3]);CHECK(fabs(a.tx-f->weights[0])<1e-12&&fabs(a.ty-f->weights[1])<1e-12);
    }
    for(int q=0;q<4;q++){RRPosterSample a;CHECK(rr_poster_sample((q%2)*.5+.25,(q/2)*.5+.25,&a));CHECK((a.x0/256)+(a.y0/512)*2==(q^2));}
    CHECK(!rr_poster_sample(NAN,.5,NULL));RRPosterSample invalid;CHECK(!rr_poster_sample(.5,INFINITY,&invalid));
    printf("POSTER_SAMPLING {\"samples\":121,\"canvas_panels_for_uv_quadrants\":[2,3,0,1],\"pixels_verified\":false,\"failures\":%d}\n",failures);
    printf("POSTER_SUMMARY {\"posters\":97,\"source_vertices\":582,\"double_sided_vertices\":1164,\"byte_exact_hashes\":97,\"source_colliders_unchanged\":97,\"within_existing_mesh_bounds\":97,\"glyph_paths\":28,\"merged_top64_vertices\":%u,\"vertex_bound\":141312,\"atlas_mip_bytes\":%zu,\"quadrants\":[%u,%u,%u,%u],\"worst_plane_error\":%.9g,\"failures\":%d}\n",top,rr_poster_rgba_mip_bytes(m.width,m.height),quadrants[0],quadrants[1],quadrants[2],quadrants[3],worst_plane,failures);
    return failures?1:0;
}
