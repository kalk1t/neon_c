#include "rr_input_mix.h"
#include <stdio.h>
typedef struct InputCase {unsigned keys;RRInputPad pad;RRInput expected;} InputCase;
#include "input_state_fixture.h"
int main(void){
    int failures=0;
    for(size_t i=0;i<sizeof(input_cases)/sizeof(input_cases[0]);i++){
        const InputCase *c=&input_cases[i];RRInput got=rr_input_mix(c->keys,c->pad);
        if(fabs(got.throttle-c->expected.throttle)>1e-12||fabs(got.brake-c->expected.brake)>1e-12||fabs(got.steer-c->expected.steer)>1e-12||fabs(got.climb-c->expected.climb)>1e-12||got.boost!=c->expected.boost){
            fprintf(stderr,"Input.state fixture %zu failed (keys=%u)\n",i,c->keys);failures++;
        }
    }
    printf("Browser Input.state: %zu source-generated keyboard/controller cases, %d failures\n",sizeof(input_cases)/sizeof(input_cases[0]),failures);
    return failures?1:0;
}
