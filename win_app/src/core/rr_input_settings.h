#ifndef RR_INPUT_SETTINGS_H
#define RR_INPUT_SETTINGS_H
#include "rr_mouse_aim.h"
#include <stddef.h>
#define RR_INPUT_SETTINGS_BYTES 512
typedef enum RRSettingsParse {RR_SETTINGS_VALID,RR_SETTINGS_INVALID,RR_SETTINGS_UNSUPPORTED} RRSettingsParse;
static inline RRMouseSettings rr_input_settings_defaults(void){return (RRMouseSettings){1,false};}
bool rr_input_settings_valid(RRMouseSettings value);
RRSettingsParse rr_input_settings_decode(const char *data,size_t size,RRMouseSettings *value);
/* Locale-independent ASCII JSON; input UI values retain better than1e-14 precision. */
size_t rr_input_settings_encode(char *out,size_t capacity,RRMouseSettings value);
#endif
