#include "rr_mesh_asset.h"
#include "rr_canonical_city.h"
#include <stdio.h>
#include <stdlib.h>
#include "generated_mesh_loader_fixture.h"
static int failures;
#define CHECK(x) do{if(!(x)){printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static bool fit(const unsigned char *data,size_t bytes,const RRCanonicalBuilding *b)
{
    double low[3]={b->mesh_bounds.minimum.x,b->mesh_bounds.minimum.y,b->mesh_bounds.minimum.z};
    double high[3]={b->mesh_bounds.maximum.x,b->mesh_bounds.maximum.y,b->mesh_bounds.maximum.z};
    return rr_mesh_asset_validate_fit(data,bytes,b->vertices,low,high);
}
int main(int argc,char **argv)
{
    const RRCanonicalBuilding *b=NULL;
    for(int i=0;i<rr_canonical_city_count();i++)if(rr_canonical_city_building(i)->mesh_id==RR_MESH_FIXTURE_ID)b=rr_canonical_city_building(i);
    CHECK(b!=NULL);if(!b)return 2;
    CHECK(fit(canonical_mesh_fixture,sizeof(canonical_mesh_fixture),b));
    unsigned char shifted[sizeof(canonical_mesh_fixture)];memcpy(shifted,canonical_mesh_fixture,sizeof(shifted));
    for(size_t i=8;i<sizeof(shifted);i+=36){float x;memcpy(&x,shifted+i,4);x+=1;memcpy(shifted+i,&x,4);}
    CHECK(rr_mesh_asset_validate(shifted,sizeof(shifted),NULL));CHECK(!fit(shifted,sizeof(shifted),b));
    RRCanonicalBuilding wrong=*b;wrong.vertices+=3;CHECK(!fit(canonical_mesh_fixture,sizeof(canonical_mesh_fixture),&wrong));
    wrong=*b;wrong.mesh_bounds.minimum.x=NAN;CHECK(!fit(canonical_mesh_fixture,sizeof(canonical_mesh_fixture),&wrong));
    wrong=*b;wrong.mesh_bounds.minimum.x=wrong.mesh_bounds.maximum.x+1;CHECK(!fit(canonical_mesh_fixture,sizeof(canonical_mesh_fixture),&wrong));
    CHECK(!rr_mesh_asset_size_valid(0,0));CHECK(!rr_mesh_asset_size_valid(8+100002u*36u,0));CHECK(!rr_mesh_asset_size_valid(sizeof(shifted)+1,0));
    CHECK(rr_mesh_asset_size_valid(8+99999u*36u,99999));CHECK(!rr_mesh_asset_size_valid(sizeof(shifted),b->vertices+3));
    CHECK(!rr_mesh_asset_validate_fit(canonical_mesh_fixture,sizeof(canonical_mesh_fixture),b->vertices,NULL,NULL));
    int checked=0;
    if(argc==2)for(int i=0;i<rr_canonical_city_count();i++){
        const RRCanonicalBuilding *source=rr_canonical_city_building(i);char path[1024];
        int n=snprintf(path,sizeof(path),"%s/b%04d.rrmesh",argv[1],source->mesh_id);CHECK(n>0&&(size_t)n<sizeof(path));
        FILE *file=NULL;
#if defined(_MSC_VER)
        if(fopen_s(&file,path,"rb"))file=NULL;
#else
        file=fopen(path,"rb");
#endif
        CHECK(file!=NULL);if(!file)continue;
        CHECK(fseek(file,0,SEEK_END)==0);long length=ftell(file);bool valid=length>=0&&rr_mesh_asset_size_valid((size_t)length,source->vertices);CHECK(valid);
        if(valid){unsigned char *data=malloc((size_t)length);CHECK(data!=NULL);if(data){rewind(file);CHECK(fread(data,1,(size_t)length,file)==(size_t)length);CHECK(fgetc(file)==EOF);CHECK(fit(data,(size_t)length,source));free(data);checked++;}}
        fclose(file);
    }
    printf("Mesh loader: source fixture, finite-but-displaced/wrong-count rejection, invalid bounds and raw file-size budget; %d installed assets checked; %d failures.\n",checked,failures);
    return failures?1:0;
}
