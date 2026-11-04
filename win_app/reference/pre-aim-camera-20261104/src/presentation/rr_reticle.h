#ifndef RR_RETICLE_H
#define RR_RETICLE_H
#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>
#include <math.h>
typedef struct RRHUDPoint {bool valid,shown;Vector2 position;float ndc_z;} RRHUDPoint;
/* Same CPU projection path as raylib GetWorldToScreenEx, retaining clip z for
 * HUD.screen's strict z>1 rejection. Presentation follows actual host planes. */
static inline RRHUDPoint rr_reticle_project(Vector3 point,Camera3D camera,int width,int height,double near_plane,double far_plane)
{
    RRHUDPoint out={0};if(width<=0||height<=0||near_plane<=0||far_plane<=near_plane||camera.fovy<=0||camera.fovy>=180)return out;
    Quaternion world={point.x,point.y,point.z,1};
    world=QuaternionTransform(world,MatrixLookAt(camera.position,camera.target,camera.up));
    world=QuaternionTransform(world,MatrixPerspective(camera.fovy*DEG2RAD,(double)width/height,near_plane,far_plane));
    if(!isfinite(world.w)||world.w==0)return out;
    out.ndc_z=world.z/world.w;
    out.position=(Vector2){(world.x/world.w+1)*.5f*width,(1-world.y/world.w)*.5f*height};
    out.valid=isfinite(out.position.x)&&isfinite(out.position.y)&&isfinite(out.ndc_z);out.shown=out.valid&&out.ndc_z<=1;return out;
}
static inline bool rr_reticle_aim_visible(bool projected,bool captured,bool alive)
{return projected&&captured&&alive;}
/* CSS border-box44px/2px ring; pseudo-elements live inside the40px padding
 * box, so their bounds are +/-30, with30%/70% horizontal mask stops +/-12. */
typedef struct RRReticleShape {double cross_outer,cross_inner,tick_end,tick_gap,aim_outer,aim_inner,dot;} RRReticleShape;
static inline RRReticleShape rr_reticle_shape(double zoom)
{RRReticleShape s={22*zoom,20*zoom,30*zoom,12*zoom,13*zoom,11*zoom,2*zoom};return s;}
#endif
