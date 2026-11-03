#include <stdio.h>
#include "rr_menu_policy.h"
#include "rr_key_events.h"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);failures++;}}while(0)
int main(void)
{
    RRMenuPolicy p={0};
    CHECK(rr_menu_request(&p,true,true,true,false)==RR_MENU_NO_ACTION);
    CHECK(rr_menu_request(&p,true,true,false,true)==RR_MENU_NO_ACTION);
    CHECK(rr_menu_request(&p,true,false,false,false)==RR_MENU_NO_ACTION);
    RRKeyEvents events;rr_key_events_clear(&events);
    rr_key_events_push(&events,257); // queued Enter survives press+release between rendered frames
    CHECK(rr_key_events_hit(&events,257));
    rr_key_events_push(&events,257);CHECK(rr_key_events_hit(&events,257));
    rr_key_events_push(&events,-1);rr_key_events_push(&events,512);
    CHECK(!rr_key_events_hit(&events,-1) && !rr_key_events_hit(&events,512));
    rr_key_events_clear(&events);CHECK(!rr_key_events_hit(&events,257));
    CHECK(rr_menu_request(&p,true,true,false,true)==RR_MENU_START_TIME_ATTACK);
    CHECK(rr_menu_request(&p,true,true,false,true)==RR_MENU_NO_ACTION);
    CHECK(rr_menu_request(&p,true,false,false,false)==RR_MENU_NO_ACTION);
    CHECK(rr_menu_request(&p,false,false,true,false)==RR_MENU_NO_ACTION);
    CHECK(rr_menu_request(&p,true,true,true,false)==RR_MENU_NO_ACTION);
    CHECK(rr_menu_request(&p,true,false,false,false)==RR_MENU_NO_ACTION);
    CHECK(rr_menu_request(&p,true,true,true,false)==RR_MENU_START_TRIAL);
    CHECK(rr_menu_request(&p,true,false,false,false)==RR_MENU_NO_ACTION);
    CHECK(rr_menu_request(&p,true,true,true,true)==RR_MENU_START_TRIAL);
    p=(RRMenuPolicy){0}; // adapter resets the gate when returning from a session
    CHECK(rr_menu_request(&p,true,false,true,false)==RR_MENU_NO_ACTION);
    CHECK(rr_menu_request(&p,true,false,false,false)==RR_MENU_NO_ACTION);
    puts(failures?"Menu activation policy failed.":"Menu release, focus, consumption and simultaneous-input policy passed.");
    return failures?1:0;
}
