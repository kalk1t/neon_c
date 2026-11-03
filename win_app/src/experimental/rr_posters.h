#ifndef RR_POSTERS_H
#define RR_POSTERS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* Headless preparation only: not linked to rr_core or the application. */
typedef struct RRPoster {int canonical_index,mesh_id,catalog_id;float rows[6][8];} RRPoster;
typedef struct RRPosterMaterial {int width,height,anisotropy;bool flip_y,srgb,double_sided,unlit,transparent,depth_test,depth_write,mipmaps,clamp_edge,premultiply_alpha,fog,tone_mapped;double linear_color,opacity;} RRPosterMaterial;
int rr_poster_count(void);
const RRPoster *rr_poster_get(int index);
bool rr_poster_expand(int index,bool double_sided,float (*vertices)[8],size_t capacity);
bool rr_poster_glyph(int quadrant,int glyph,double points[3][2]);
RRPosterMaterial rr_poster_material(void);
size_t rr_poster_rgba_mip_bytes(int width,int height);
typedef struct RRPosterSample {double canvas_u,canvas_v;int x0,x1,y0,y1;double tx,ty;} RRPosterSample;
/* Base-level source address model only; no pixels, mip LOD or GPU appearance. */
bool rr_poster_sample(double u,double v,RRPosterSample *out);
#endif
