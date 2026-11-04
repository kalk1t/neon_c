#include "rr_results_advance.h"
#include "results_advance_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"advance line %d: %s\n",__LINE__,#x);failures++;}}while(0)
int main(void){
 RRResultsAdvance a;
 for(size_t i=0;i<COUNT(advance_cases);i++)CHECK(rr_results_advance(advance_cases[i].text,&a)==RR_ADVANCE_READY&&a.range_width==advance_cases[i].width);
 CHECK(rr_results_advance("WWWWWWWWWWWWWWWW",&a)==RR_ADVANCE_READY&&a.ticks==18109440&&a.range_width==282.96875&&a.range_width+26==308.96875);
 CHECK(rr_results_advance("",&a)==RR_ADVANCE_READY&&!a.codepoints&&!a.ticks&&!a.range_width);
 RRResultsAdvance guard,saved;memset(&guard,0x5a,sizeof(guard));saved=guard;
 const char *complex[]={"AW"," A","A ","A  A"};for(size_t i=0;i<COUNT(complex);i++)CHECK(rr_results_advance(complex[i],&guard)==RR_ADVANCE_NEEDS_SHAPING&&!memcmp(&guard,&saved,sizeof(guard)));
 const char *invalid[]={"^","\n","\xc0\xaf","\xed\xa0\x80","\xf0\x9f\x98\x80","\xe2\x89"};for(size_t i=0;i<COUNT(invalid);i++)CHECK(rr_results_advance(invalid[i],&guard)==RR_ADVANCE_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 char long_text[513];memset(long_text,'W',512);long_text[512]=0;CHECK(rr_results_advance(long_text,&guard)==RR_ADVANCE_INVALID);long_text[129]=0;CHECK(rr_results_advance(long_text,&guard)==RR_ADVANCE_INVALID);long_text[128]=0;CHECK(rr_results_advance(long_text,&a)==RR_ADVANCE_READY&&a.codepoints==128);
 CHECK(rr_results_advance(NULL,&guard)==RR_ADVANCE_INVALID&&rr_results_advance("A",NULL)==RR_ADVANCE_INVALID);
 printf("RESULTS_ADVANCE_SUMMARY observed_single_line_ranges=%zu exact_widths=%zu plan_bytes=%zu failures=%u no_character_shaping_or_float_fit_claim=true\n",COUNT(advance_cases),COUNT(advance_cases),sizeof(a),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
