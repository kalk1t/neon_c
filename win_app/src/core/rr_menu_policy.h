#ifndef RR_MENU_POLICY_H
#define RR_MENU_POLICY_H
#include <stdbool.h>
typedef struct RRMenuPolicy { bool armed; } RRMenuPolicy;
enum { RR_MENU_NO_ACTION, RR_MENU_START_TRIAL, RR_MENU_START_TIME_ATTACK };
/* Require a focused, released frame before accepting a start action. Consume
 * the arm once; focus loss and returning to menu require another release.
 * The adapter provides only menu activators, never global keyboard state.
 */
static inline int rr_menu_request(RRMenuPolicy *p,bool focused,bool held,
                                  bool trial_pressed,bool time_pressed)
{
    if(!focused){p->armed=false;return RR_MENU_NO_ACTION;}
    if(!p->armed){if(!held)p->armed=true;return RR_MENU_NO_ACTION;}
    if(trial_pressed || time_pressed){
        p->armed=false;return trial_pressed?RR_MENU_START_TRIAL:RR_MENU_START_TIME_ATTACK;
    }
    return RR_MENU_NO_ACTION;
}
#endif
