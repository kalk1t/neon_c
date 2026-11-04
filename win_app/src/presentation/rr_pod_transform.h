#ifndef RR_POD_TRANSFORM_H
#define RR_POD_TRANSFORM_H
#include "raymath.h"
/* Three default Euler XYZ uses Rx*Ry*Rz on column vectors. Raylib Multiply
 * lists application order, so apply Z then X, about the baked GLB pivot. */
static inline Matrix rr_pod_transform(Vector3 p,double pitch,double roll){
 return MatrixMultiply(MatrixMultiply(MatrixMultiply(MatrixTranslate(-p.x,-p.y,-p.z),MatrixRotateZ((float)roll)),MatrixRotateX((float)pitch)),MatrixTranslate(p.x,p.y,p.z));
}
#endif
