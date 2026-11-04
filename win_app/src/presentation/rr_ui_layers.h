#ifndef RR_UI_LAYERS_H
#define RR_UI_LAYERS_H
#include "rr_session.h"
#include <stdint.h>
typedef enum RRUIPaintLayer {RR_UI_LAYER_HUD,RR_UI_LAYER_NITRO,RR_UI_LAYER_MENU,RR_UI_LAYER_PAUSE,RR_UI_LAYER_RESULTS,RR_UI_LAYER_ERROR} RRUIPaintLayer;
typedef struct RRUIPaintPlan {int count;RRUIPaintLayer layers[3];} RRUIPaintPlan;
/* #hud is hidden by pause/toMenu, retained through showResults. NITRO is a
 * separate z6 element; menu/pause/results overlays are z10. No new resources. */
static inline RRUIPaintPlan rr_ui_paint_plan(RRSessionPhase phase)
{
    RRUIPaintPlan plan={0};
    if(phase==RR_SESSION_ACTIVE||phase==RR_SESSION_COUNTDOWN||phase==RR_SESSION_FINISHED||phase==RR_SESSION_RESULTS)plan.layers[plan.count++]=RR_UI_LAYER_HUD;
    plan.layers[plan.count++]=RR_UI_LAYER_NITRO;
    if(phase==RR_SESSION_MENU)plan.layers[plan.count++]=RR_UI_LAYER_MENU;
    else if(phase==RR_SESSION_PAUSED)plan.layers[plan.count++]=RR_UI_LAYER_PAUSE;
    else if(phase==RR_SESSION_RESULTS)plan.layers[plan.count++]=RR_UI_LAYER_RESULTS;
    else if(phase==RR_SESSION_ERROR)plan.layers[plan.count++]=RR_UI_LAYER_ERROR;
    return plan;
}
/* Source rgba(3,4,20,.55), nearest representable8-bit alpha140. */
static inline uint32_t rr_ui_results_backdrop(void){return UINT32_C(0x0304148c);}
#endif
