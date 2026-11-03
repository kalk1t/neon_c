#ifndef RR_MESH_ASSET_H
#define RR_MESH_ASSET_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#define RR_MESH_MAX_VERTICES 100000u
/* RRC1: little-endian count then float32 position/normal/UV and RGBA8 rows.
 * Validate complete finite payload before allocating CPU/GPU resources. */
static inline bool rr_mesh_asset_validate(const unsigned char *data,size_t bytes,unsigned int *vertices)
{
    if(vertices)*vertices=0;
    if(!data||bytes<8||memcmp(data,"RRC1",4)!=0)return false;
    uint32_t count=(uint32_t)data[4]|((uint32_t)data[5]<<8)|((uint32_t)data[6]<<16)|((uint32_t)data[7]<<24);
    if(count==0||count>RR_MESH_MAX_VERTICES||count%3||bytes!=8+(size_t)count*36)return false;
    for(uint32_t i=0;i<count;i++)for(int k=0;k<8;k++){float value;memcpy(&value,data+8+(size_t)i*36+(size_t)k*4,4);if(!isfinite(value))return false;}
    if(vertices)*vertices=count;return true;
}
#endif
