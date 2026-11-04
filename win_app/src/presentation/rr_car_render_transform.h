#ifndef RR_CAR_RENDER_TRANSFORM_H
#define RR_CAR_RENDER_TRANSFORM_H
#include "rr_car_pose.h"
#include "raymath.h"
#include <float.h>
/* Column-major source model group -> named raylib Matrix fields. Its C struct
 * is not a contiguous column-major array: never memcpy/transpose it. The child
 * fit is applied in model space AFTER local lift; Roadster=-.35, code body=0.
 * GLB node transforms are already baked into native vertices; pods separately
 * rotate about baked pivots, then apply this parent. No GPU or allocation. */
static inline bool rr_car_pose_raylib(const RRCarPose *pose,double fit_y,Matrix *out){
 double m[16];if(!out||!isfinite(fit_y)||!rr_car_pose_matrix(pose,m))return false;
 for(int i=0;i<4;i++)m[12+i]+=m[4+i]*fit_y;
 for(int i=0;i<16;i++)if(!isfinite(m[i])||fabs(m[i])>FLT_MAX)return false;
 Matrix result={0};
 result.m0=(float)m[0];result.m1=(float)m[1];result.m2=(float)m[2];result.m3=(float)m[3];
 result.m4=(float)m[4];result.m5=(float)m[5];result.m6=(float)m[6];result.m7=(float)m[7];
 result.m8=(float)m[8];result.m9=(float)m[9];result.m10=(float)m[10];result.m11=(float)m[11];
 result.m12=(float)m[12];result.m13=(float)m[13];result.m14=(float)m[14];result.m15=(float)m[15];
 *out=result;return true;
}
#endif
