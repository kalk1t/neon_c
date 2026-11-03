#ifndef RR_MATH_H
#define RR_MATH_H

#include <math.h>

#define RR_PI 3.14159265358979323846
typedef struct RRVec3 { double x, y, z; } RRVec3;
static inline double rr_clamp(double v, double lo, double hi) { return fmax(lo, fmin(hi, v)); }
static inline double rr_lerp(double a, double b, double t) { return a + (b - a)*t; }
static inline RRVec3 rr_v3(double x, double y, double z) { RRVec3 v = { x, y, z }; return v; }
static inline RRVec3 rr_add(RRVec3 a, RRVec3 b) { return rr_v3(a.x+b.x, a.y+b.y, a.z+b.z); }
static inline RRVec3 rr_sub(RRVec3 a, RRVec3 b) { return rr_v3(a.x-b.x, a.y-b.y, a.z-b.z); }
static inline RRVec3 rr_scale(RRVec3 v, double s) { return rr_v3(v.x*s, v.y*s, v.z*s); }
static inline double rr_dot(RRVec3 a, RRVec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline double rr_length(RRVec3 v) { return sqrt(rr_dot(v,v)); }
static inline RRVec3 rr_normalize(RRVec3 v) { double n=rr_length(v); return n>0 ? rr_scale(v,1/n) : rr_v3(0,0,0); }
static inline RRVec3 rr_mix(RRVec3 a, RRVec3 b, double t) { return rr_v3(rr_lerp(a.x,b.x,t),rr_lerp(a.y,b.y,t),rr_lerp(a.z,b.z,t)); }

#endif
