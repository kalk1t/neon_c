#ifndef RR_MESH_ASSET_H
#define RR_MESH_ASSET_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#define RR_MESH_MAX_VERTICES 100000u
/* Check file length before LoadFileData can allocate its raw payload. */
static inline bool rr_mesh_asset_size_valid(size_t bytes,unsigned int expected)
{
    if(bytes<8+3*36 || bytes>8+(size_t)RR_MESH_MAX_VERTICES*36 || (bytes-8)%(3*36))return false;
    return !expected || (expected<=RR_MESH_MAX_VERTICES && expected%3==0 && bytes==8+(size_t)expected*36);
}
/* RRC1: little-endian count then float32 position/normal/UV and RGBA8 rows.
 * Validate complete finite payload before allocating CPU/GPU resources. */
static inline bool rr_mesh_asset_validate(const unsigned char *data,size_t bytes,unsigned int *vertices)
{
    if(vertices)*vertices=0;
    if(!data||!rr_mesh_asset_size_valid(bytes,0)||memcmp(data,"RRC1",4)!=0)return false;
    uint32_t count=(uint32_t)data[4]|((uint32_t)data[5]<<8)|((uint32_t)data[6]<<16)|((uint32_t)data[7]<<24);
    if(count==0||count>RR_MESH_MAX_VERTICES||count%3||bytes!=8+(size_t)count*36)return false;
    for(uint32_t i=0;i<count;i++)for(int k=0;k<8;k++){float value;memcpy(&value,data+8+(size_t)i*36+(size_t)k*4,4);if(!isfinite(value))return false;}
    if(vertices)*vertices=count;return true;
}
/* Canonical assets carry world-space positions: reject a valid but wrong mesh
 * before CPU mesh allocation or upload. Bounds are exact source Float32 extrema. */
static inline bool rr_mesh_asset_validate_fit(const unsigned char *data,size_t bytes,unsigned int expected,const double minimum[3],const double maximum[3])
{
    unsigned int count=0;
    if(!minimum||!maximum||!rr_mesh_asset_size_valid(bytes,expected)||!rr_mesh_asset_validate(data,bytes,&count)||count!=expected)return false;
    double low[3]={INFINITY,INFINITY,INFINITY},high[3]={-INFINITY,-INFINITY,-INFINITY};
    for(int k=0;k<3;k++)if(!isfinite(minimum[k])||!isfinite(maximum[k])||minimum[k]>maximum[k])return false;
    for(unsigned int i=0;i<count;i++)for(int k=0;k<3;k++){
        float value;memcpy(&value,data+8+(size_t)i*36+(size_t)k*4,4);
        if(value<low[k])low[k]=value;
        if(value>high[k])high[k]=value;
    }
    for(int k=0;k<3;k++)if(low[k]!=minimum[k]||high[k]!=maximum[k])return false;
    return true;
}
#endif
