#ifndef RR_RESULTS_TEXT_H
#define RR_RESULTS_TEXT_H
#include "rr_ui.h"
#include "rr_results_current.h"
enum {RR_RESULTS_TEXT_TOKENS=128,RR_RESULTS_TEXT_BYTES=512,RR_RESULTS_ART_COUNT=6};
typedef struct RRResultsToken {uint32_t codepoint;float pen,advance;int fallback;} RRResultsToken;
/* Prepared plans borrow the verified Orbitron700 font's CPU arrays; retain its
 * lifetime through synchronous emission. Text/codepoints/advances are copied.
 * Only15CSSpx, zero tracking and explicit device scales1/2/3 qualify. */
typedef struct RRResultsText {Font font;RRResultsToken tokens[RR_RESULTS_TEXT_TOKENS];size_t count;float width;int device_scale;bool fallback;} RRResultsText;
typedef enum RRResultsRowKind {RR_RESULTS_STANDINGS,RR_RESULTS_COINS} RRResultsRowKind;
typedef enum RRResultsRowStatus {RR_RESULTS_ROW_INVALID,RR_RESULTS_ROW_READY,RR_RESULTS_ROW_NEEDS_WRAP} RRResultsRowStatus;
typedef struct RRResultsRow {RRResultsText name,value;float left,value_x,baseline;Color name_color,value_color,shadow_color;bool shadow;} RRResultsRow;
typedef struct RRResultsPaint {uint32_t codepoint;int fallback;Vector2 origin;Rectangle source,destination;float font_size;Color color;bool shadow;} RRResultsPaint;
typedef void (*RRResultsPaintSink)(void *context,const RRResultsPaint *paint);
/* Strict bounded UTF-8, primary cmap coverage, source crop advances/bearings.
 * Failure leaves output unchanged. No '?' substitution, clipping or heap. */
bool rr_results_text_build(Font font,const char *text,int device_scale,RRResultsText *out);
/* Row CONTENT only, not list ordinal/headers/card scrolling. left/right delimit
 * text advance space. Caller binds/cache-freezes identity names at publication.
 * Return NEEDS_WRAP without changing output when single-line fit is unsupported.
 * Uses snapshot display order, not live ranking or account/progression data. */
RRResultsRowStatus rr_results_row_build(Font font,const RRResultsCurrent *snapshot,
 RRResultsRowKind kind,size_t row,const char *const *names,size_t names_count,
 size_t player,int device_scale,float left,float right,float baseline,RRResultsRow *out);
/* Only successful unmodified plans qualify. All commands preflight before the
 * first synchronous sink call. A sink may record or paint; no callback failure
 * protocol/partial retry is invented. Existing four-offset native shadow is an
 * approximation, not CSS8px blur or native pixel acceptance. */
bool rr_results_row_emit(const RRResultsRow *row,RRResultsPaintSink sink,void *context);
#endif
