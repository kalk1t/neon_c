#ifndef RR_CITY_REPACKING_H
#define RR_CITY_REPACKING_H
#include "rr_city_batching.h"
#include <stddef.h>
#include <stdint.h>
#define RR_REPACK_PARTS 38352
#define RR_REPACK_VERTEX_BOUND 141312u
#define RR_REPACK_SOURCE_BYTES 54247104u
#define RR_REPACK_MATERIALS 5
/* All interfaces are diagnostic/test-only; never game readiness/authorization. */
typedef struct RRRepackPart {RRVec3 minimum,maximum;unsigned int first,vertices;int material,source_batch;} RRRepackPart;
typedef struct RRRepackPlan {int ids[RR_REPACK_PARTS];int count,source_requested,source_frustum_rejected,distance_omitted;unsigned int vertices,source_vertices,material_vertices[5];uint32_t member_hash;bool near_only,fits,source_coverage;} RRRepackPlan;
typedef bool (*RRRepackRead)(void *context,size_t offset,size_t bytes,unsigned char *target);
int rr_repack_part_count(void);
const RRRepackPart *rr_repack_part(int index);
size_t rr_repack_metadata_bytes(void);
bool rr_repack_plan(const double source_planes[6][4],const double near_planes[6][4],uint32_t source_mask_lo,uint32_t source_mask_hi,bool near_only,RRRepackPlan *plan);
bool rr_repack_execute_diagnostic(const RRRepackPlan *plan,RRRepackRead reader,void *context,unsigned char *target,size_t capacity,size_t material_offsets[5]);
uint32_t rr_repack_poly(const unsigned char *data,size_t bytes);
#endif
