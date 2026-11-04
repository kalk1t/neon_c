#ifndef RR_AUDIO_DISPATCH_H
#define RR_AUDIO_DISPATCH_H
#include "rr_audio_cues.h"
#include <stdint.h>
enum {RR_AUDIO_MAX_VOICES=18};
typedef struct RRAudioVoiceSlot {uint64_t id;bool active;} RRAudioVoiceSlot;
typedef struct RRAudioVoiceBank {RRAudioVoiceSlot slots[RR_AUDIO_MAX_VOICES];unsigned active;uint64_t next_id;} RRAudioVoiceBank;
typedef double (*RRAudioOffsetRandom)(void *context);
/* Tone topology: oscillator -> gain -> optional panner -> warmed fx.
 * Rumble: shared noise buffer -> sweep filter -> gain -> optional panner -> fx.
 * Gain targets are zero/start, request.volume/attack_end, gain_end/decay_end;
 * frequency ramp ends at decay_end when rumble or request.slide is nonzero.
 * These are scheduling targets, not rendered automation curves or samples. */
typedef struct RRAudioVoiceCommand {
 RRAudioCueRequest request;uint64_t id;size_t request_index;
 double start,attack_end,decay_end,stop,pan,filter_q,noise_offset,gain_start,gain_end;
 bool panner,ready;
} RRAudioVoiceCommand;
typedef struct RRAudioDispatch {
 RRAudioVoiceCommand commands[RR_CUE_REQUESTS];size_t count,skipped;unsigned offset_draws;bool complete;
} RRAudioDispatch;
/* Construction only: old backend callbacks must be quiesced before reinit.
 * Tokens belong to this single bank lifetime, not across device/bank recreation. */
void rr_audio_voice_bank_init(RRAudioVoiceBank *bank);
/* Admitted delayed voices count immediately. Only explicit backend ended/cancel
 * delivery retires a token; never infer callback delivery from clock deadlines.
 * Duplicate/stale token delivery is harmless, including after slot reuse. */
bool rr_audio_voice_end(RRAudioVoiceBank *bank,uint64_t id);
/* Warmed tone/rumble graph COMMANDS only, not PCM/DSP/playback. Context readiness
 * belongs to caller. Master mute/volume do not bypass source cue admission.
 * Caller already consumed recipe blaster RNG; this calls random ONLY for admitted
 * rumble offsets, after reserving/incrementing its voice as source does.
 * Single-owner bank; random must not reenter. Guarded/skipped requests do not
 * inspect unused parameters. Preflight invalid admitted inputs preserve bank/out.
 * Invalid random provider is a separate C failure policy: previous/current token
 * reservations and consumed draw count remain in out, complete=false, current
 * command ready=false. Retire those tokens explicitly; never replay whole plan.
 * No audio context/device, graph node/sample allocation, thread, main activation,
 * account or network. out must not alias bank or plan. */
bool rr_audio_dispatch(RRAudioVoiceBank *bank,bool context_ready,double audio_time,const RRAudioCuePlan *plan,RRAudioOffsetRandom random,void *context,RRAudioDispatch *out);
#endif
