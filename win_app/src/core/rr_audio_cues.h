#ifndef RR_AUDIO_CUES_H
#define RR_AUDIO_CUES_H
#include <stdbool.h>
#include <stddef.h>
typedef enum RRAudioCue {RR_CUE_COUNTDOWN,RR_CUE_CHECKPOINT,RR_CUE_LAP,RR_CUE_COIN,RR_CUE_BOOST_ON,RR_CUE_BOOST_OFF,RR_CUE_BLASTER,RR_CUE_OVERHEAT,RR_CUE_MISSILE,RR_CUE_EMP,RR_CUE_DENY,RR_CUE_LOCK_TICK,RR_CUE_LOCKED,RR_CUE_ZAP,RR_CUE_HIT_MARK,RR_CUE_HIT,RR_CUE_DAMAGE,RR_CUE_EXPLOSION,RR_CUE_COUNT} RRAudioCue;
typedef enum RRAudioRequestKind {RR_CUE_TONE,RR_CUE_RUMBLE} RRAudioRequestKind;
typedef enum RRAudioWave {RR_CUE_SINE,RR_CUE_TRIANGLE} RRAudioWave;
typedef enum RRAudioFilter {RR_CUE_LOWPASS,RR_CUE_BANDPASS} RRAudioFilter;
typedef struct RRAudioCueRequest {RRAudioRequestKind kind;RRAudioWave wave;RRAudioFilter filter;double frequency,duration,volume,when,slide,pan,from,to;} RRAudioCueRequest;
enum {RR_CUE_REQUESTS=4};
typedef struct RRAudioCuePlan {RRAudioCueRequest requests[RR_CUE_REQUESTS];size_t count;unsigned recipe_random_draws;} RRAudioCuePlan;
/* Pure ordered SOURCE CALL recipes, BEFORE tone/rumble context/gain/voice guards.
 * amount is explicit vol for coin/zap/damage/explosion, impact k for hit; unused
 * by other cues. flag is GO for countdown / big for hitMark. pan is raw source
 * input, not yet clamped by out(). blaster_unit is one ALREADY consumed [0,1)
 * source RNG value, read only by blaster. This function never advances RNG.
 * recipe_random_draws counts that pre-request blaster draw only; admitted rumble
 * offsets consume separate RNG in the future dispatcher. Preserve request order.
 * These are NOT approved voices, samples or audible output. Context/vol<=.002/
 * MAX_VOICES18, source envelopes, offsets, panner and onended lifecycle remain
 * external. No backend/network/account/main activation. Failure preserves out. */
bool rr_audio_cue_recipe(RRAudioCue cue,double amount,double pan,bool flag,double blaster_unit,RRAudioCuePlan *out);
#endif
