#ifndef RR_PHYSICS_H
#define RR_PHYSICS_H
#include "rr_math.h"
#include <stdbool.h>
#define RR_PHYSICS_MAX_SHAPES 1024
typedef enum RRShapeKind {RR_SHAPE_BOX,RR_SHAPE_CYLINDER,RR_SHAPE_TORUS} RRShapeKind;
typedef struct RRShape {RRShapeKind kind;RRVec3 center,half;double cosine,sine,radius,tube,y0,y1,bounds[4];bool off;} RRShape;
typedef struct RRContact {RRVec3 normal;double depth;bool hit;} RRContact;
typedef struct RRWorld {RRShape shapes[RR_PHYSICS_MAX_SHAPES];int count;} RRWorld;
RRShape rr_box(RRVec3 center,RRVec3 half,double yaw);
RRShape rr_cylinder(double x,double z,double radius,double y0,double y1);
RRShape rr_torus(RRVec3 center,double yaw,double radius,double tube);
bool rr_world_add(RRWorld *world,RRShape shape);
RRContact rr_shape_resolve(const RRShape *shape,RRVec3 p,double radius);
/* Original cell is fixed before pushes, retaining browser insertion order. */
RRContact rr_world_collide(const RRWorld *world,RRVec3 *p,double radius);
bool rr_world_segment(const RRWorld *world,RRVec3 a,RRVec3 b,double radius,RRVec3 *hit);
#endif
