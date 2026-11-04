#ifndef RR_INPUT_PREFERENCES_H
#define RR_INPUT_PREFERENCES_H
#include "rr_input_settings.h"
#define RR_SETTINGS_PATH_BYTES 4096
typedef enum RRPreferenceStatus {RR_PREF_OK,RR_PREF_MISSING,RR_PREF_INVALID,RR_PREF_UNSUPPORTED,RR_PREF_UNAVAILABLE,RR_PREF_READ_ONLY,RR_PREF_BUSY,RR_PREF_CONFLICT,RR_PREF_IO_ERROR} RRPreferenceStatus;
typedef struct RRInputPreferences {
 char path[RR_SETTINGS_PATH_BYTES],baseline[RR_INPUT_SETTINGS_BYTES];size_t baseline_size;
 RRMouseSettings saved,pending;double changed_at,retry_at;bool existed,writable,create_parent,dirty;
 RRPreferenceStatus status;
} RRInputPreferences;
/* Explicit path/key entry point lets CPU tests use only isolated fixtures.
 * default is called only by the real app: LocalAppData/native, namespaced key/Web. */
RRPreferenceStatus rr_input_preferences_open(RRInputPreferences *store,const char *path,RRMouseSettings *value);
RRPreferenceStatus rr_input_preferences_default(RRInputPreferences *store,RRMouseSettings *value);
RRPreferenceStatus rr_input_preferences_save(RRInputPreferences *store,RRMouseSettings value);
/* Coalesce changes for0.25s; force on menu exit/blur/shutdown. Never writes at startup. */
RRPreferenceStatus rr_input_preferences_tick(RRInputPreferences *store,RRMouseSettings value,double now,bool force);
#endif
