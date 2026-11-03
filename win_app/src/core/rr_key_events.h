#ifndef RR_KEY_EVENTS_H
#define RR_KEY_EVENTS_H
#include <stdbool.h>
#include <string.h>
enum { RR_KEY_EVENT_CAPACITY=512 };
typedef struct RRKeyEvents { bool pressed[RR_KEY_EVENT_CAPACITY]; } RRKeyEvents;
static inline void rr_key_events_clear(RRKeyEvents *events){memset(events,0,sizeof(*events));}
static inline void rr_key_events_push(RRKeyEvents *events,int key)
{if(key>0 && key<RR_KEY_EVENT_CAPACITY)events->pressed[key]=true;}
static inline bool rr_key_events_hit(const RRKeyEvents *events,int key)
{return key>0 && key<RR_KEY_EVENT_CAPACITY && events->pressed[key];}
#endif
