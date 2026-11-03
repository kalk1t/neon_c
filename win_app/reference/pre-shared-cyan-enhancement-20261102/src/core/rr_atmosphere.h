/* Source main.js FogExp2 configuration; native lighting remains separate. */
#ifndef RR_ATMOSPHERE_H
#define RR_ATMOSPHERE_H
#include "rr_math.h"
#define RR_FOG_DENSITY 0.00062
static const double rr_fog_color_linear[3]={0.012286488353353374,0.009134058699157796,0.061246054224174035};
static const double rr_fog_color_srgb[3]={0.11373043869144592,0.09412231523416364,0.2745159391178933};
static inline double rr_fog_amount(double view_depth){return 1-exp(-RR_FOG_DENSITY*RR_FOG_DENSITY*view_depth*view_depth);}
#endif
