#include "rr_pod_transform.h"
#include "rr_pod_pivots.h"
#include <stdio.h>
typedef struct PodCase{int pod;double pitch,roll;Vector3 baked,expected;}PodCase;
#include "pod_fixture.h"
int main(void){int failures=0;double worst=0;
 for(size_t i=0;i<sizeof(pod_cases)/sizeof(pod_cases[0]);i++){
  const PodCase *c=&pod_cases[i];Matrix m=rr_pod_transform(rr_pod_pivots[c->pod],c->pitch,c->roll);Vector3 v=Vector3Transform(c->baked,m);double error=Vector3Distance(v,c->expected);if(error>worst)worst=error;if(error>2e-6)failures++;
  Vector3 pivot=rr_pod_pivots[c->pod];if(Vector3Distance(Vector3Transform(pivot,m),pivot)>1e-6)failures++;
 }
 printf("Roadster pods:400 Three Euler/pivot samples, max float error %.12g, %d failures; no GPU\n",worst,failures);return failures?1:0;
}
