#include "rr_hero_asset.h"
#include "rr_control_tower_data.h"
#include "rr_mesh_asset.h"
#include <stdio.h>
typedef struct HeroPlacementFixture {RRVec3 minimum,maximum,model_size,pos;double scale,yaw;RRVec3 world_min,world_max;} HeroPlacementFixture;
#include "generated_hero_reference.h"
static int failures;static double worst;
#define CHECK(x) do{if(!(x)){printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&e<1e-8);}
int main(void)
{
    for(size_t i=0;i<sizeof(hero_placement_fixtures)/sizeof(hero_placement_fixtures[0]);i++){
        const HeroPlacementFixture *f=&hero_placement_fixtures[i];RRHeroPlacement p;
        CHECK(rr_control_tower_placement((RRHeroBounds){f->minimum,f->maximum},f->model_size,&p));near(p.pos.x,f->pos.x);near(p.pos.y,f->pos.y);near(p.pos.z,f->pos.z);near(p.scale,f->scale);near(p.yaw,f->yaw);
        RRHeroBounds bounds=rr_hero_world_bounds(rr_control_model_bounds,p);near(bounds.minimum.x,f->world_min.x);near(bounds.minimum.y,f->world_min.y);near(bounds.minimum.z,f->world_min.z);near(bounds.maximum.x,f->world_max.x);near(bounds.maximum.y,f->world_max.y);near(bounds.maximum.z,f->world_max.z);
    }
    RRHeroPlacement placement;CHECK(rr_control_tower_placement(rr_control_fallback_bounds,rr_control_model_size,&placement));CHECK(RR_CONTROL_TOWER_GLOW==2.6);
    CHECK(!rr_control_tower_placement(rr_control_fallback_bounds,rr_v3(1,0,1),&placement));CHECK(!rr_control_tower_placement(rr_control_fallback_bounds,rr_v3(1,NAN,1),&placement));
    CHECK(!rr_control_tower_placement((RRHeroBounds){rr_v3(0,1,0),rr_v3(0,0,0)},rr_control_model_size,&placement));
    unsigned char data[8+3*36]={0};memcpy(data,"RRC1",4);data[4]=3;unsigned int vertices=0;
    CHECK(rr_mesh_asset_validate(data,sizeof(data),&vertices)&&vertices==3);
    CHECK(!rr_mesh_asset_validate(NULL,sizeof(data),&vertices));CHECK(vertices==0);
    CHECK(!rr_mesh_asset_validate(data,7,&vertices));CHECK(!rr_mesh_asset_validate(data,sizeof(data)-1,&vertices));
    data[0]='X';CHECK(!rr_mesh_asset_validate(data,sizeof(data),&vertices));data[0]='R';
    data[4]=0;CHECK(!rr_mesh_asset_validate(data,sizeof(data),&vertices));data[4]=4;CHECK(!rr_mesh_asset_validate(data,sizeof(data),&vertices));
    memset(data+4,255,4);CHECK(!rr_mesh_asset_validate(data,sizeof(data),&vertices));memset(data+4,0,4);data[4]=3;
    float invalid=NAN;memcpy(data+8+6*4,&invalid,4);CHECK(!rr_mesh_asset_validate(data,sizeof(data),&vertices));
    invalid=0;memcpy(data+8+6*4,&invalid,4);CHECK(rr_mesh_asset_validate(data,sizeof(data),&vertices));
    printf("Control hero: %zu actual browser replacement transforms/world bounds; invalid-size and mesh resource/finite-payload guards; source glow=2.6; worst %.3g; %d failures.\n",sizeof(hero_placement_fixtures)/sizeof(hero_placement_fixtures[0]),worst,failures);return failures?1:0;
}
