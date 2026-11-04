#ifndef RR_AUDIO_CURRENT_H
#define RR_AUDIO_CURRENT_H
#include "rr_session.h"
#include "rr_rival.h"
typedef enum RRAudioParameter {RR_AUDIO_OSC0,RR_AUDIO_OSC1,RR_AUDIO_OSC2,RR_AUDIO_ENGINE_LOWPASS,RR_AUDIO_ENGINE_GAIN,RR_AUDIO_WIND_LOWPASS,RR_AUDIO_WIND_GAIN,RR_AUDIO_MASTER_GAIN} RRAudioParameter;
typedef struct RRAudioTarget {RRAudioParameter parameter;double target,time,time_constant;} RRAudioTarget;
enum {RR_AUDIO_TARGETS=7};
typedef struct RRAudioPlan {RRAudioTarget targets[RR_AUDIO_TARGETS];size_t count;} RRAudioPlan;
/* Commands are WebAudio-style setTargetAtTime targets, NOT samples or a mixer.
 * Caller owns node/device readiness and monotonic audio-context seconds. Empty
 * plans model missing context before method input reads, as in source. No device,
 * callback, heap, RNG, graph allocation or audible output. Invalid ready inputs
 * or derived overflow leave output unchanged. No implicit gain/speed clamping. */
bool rr_audio_engine_plan(bool nodes_ready,double speed_fraction,bool boosting,bool active,double audio_time,RRAudioPlan *out);
bool rr_audio_master_plan(bool nodes_ready,double volume,bool muted,double audio_time,RRAudioPlan *out);
/* playing is CAPTURED at the source main-loop playing declaration, not derived
 * again from current_phase after the countdown branch. Current phase is used
 * only for countdown. Thus GO can be inactive for its first frame. Forward speed
 * only, max(0,speed)/current CAR.MAX; sideways HUD speed does not rev the engine.
 * Source alive is wreck<=0, not HP or visibility. No production main activation. */
bool rr_audio_engine_frame(bool nodes_ready,bool playing,RRSessionPhase current_phase,const RRLocalRacer *player,double audio_time,RRAudioPlan *out);
#endif
