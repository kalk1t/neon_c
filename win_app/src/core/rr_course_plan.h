#ifndef RR_COURSE_PLAN_H
#define RR_COURSE_PLAN_H
#include "rr_physics.h"
#include "rr_track.h"
#define RR_COURSE_MAX_DOTS 1600
#define RR_COURSE_MAX_ITEMS 1024
#define RR_COURSE_MAX_COLLIDERS 1024
typedef enum RRCourseKind {RR_COURSE_BEACON,RR_COURSE_BARRIER,RR_COURSE_SIGN_POS,RR_COURSE_SIGN_NEG,RR_COURSE_CHEVRON} RRCourseKind;
typedef struct RRCourseItem {RRCourseKind kind;RRVec3 pos,right,up,back;double yaw;} RRCourseItem;
typedef struct RRCourse {
    RRVec3 dots[RR_COURSE_MAX_DOTS];RRCourseItem items[RR_COURSE_MAX_ITEMS];RRShape colliders[RR_COURSE_MAX_COLLIDERS];
    int dot_count,item_count,collider_count,beacons,barriers,signs,chevrons;
} RRCourse;
bool rr_course_plan(RRCourse *course,const RRTrack *track);
#endif
