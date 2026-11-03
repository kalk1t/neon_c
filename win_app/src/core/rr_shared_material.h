/* Actual browser sculpt saturation/value mask; input colors are linear. */
#ifndef RR_SHARED_MATERIAL_H
#define RR_SHARED_MATERIAL_H
#include "rr_math.h"
#define RR_CYAN_TREE_GLOW 2.4
#define RR_CYAN_TREE_HEIGHT 13.0
static inline double rr_source_smoothstep(double a,double b,double x){double k=rr_clamp((x-a)/(b-a),0,1);return k*k*(3-2*k);}
static inline double rr_sculpt_neon(double r,double g,double b){double hi=fmax(r,fmax(g,b)),lo=fmin(r,fmin(g,b)),sat=hi>0?(hi-lo)/hi:0;return rr_source_smoothstep(0.45,0.8,sat)*rr_source_smoothstep(0.12,0.4,hi);}
#endif
