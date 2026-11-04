#include "rr_results_advance.h"
#include "results_characters_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,total;
static RRResultsCharacters actual,guard,saved;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"characters line %d: %s\n",__LINE__,#x);failures++;}}while(0)
int main(void){
 for(size_t i=0;i<COUNT(character_cases);i++){
  const CharacterCase *c=character_cases+i;CHECK(rr_results_characters(c->text,&actual)==RR_ADVANCE_READY&&actual.count==c->count&&actual.range_width==c->width);
  for(size_t j=0;j<actual.count&&j<c->count;j++){const RRResultsCharacterRange *r=actual.ranges+j;CHECK(r->codepoint==c->chars[j].cp&&r->left==c->chars[j].left&&r->right==c->chars[j].right);total++;}
 }
 CHECK(rr_results_characters("\342\211\210",&actual)==RR_ADVANCE_READY&&actual.advance==9.90234375&&actual.range_width==9.90625);
 RRResultsAdvance width;CHECK(rr_results_advance("\342\211\210",&width)==RR_ADVANCE_READY&&width.ticks==633750&&width.advance==9.90234375&&width.range_width==9.90625);
 memset(&guard,0x5a,sizeof(guard));saved=guard;
 const char *needs[]={"AWX"," A","A ","A  A"};for(size_t i=0;i<COUNT(needs);i++)CHECK(rr_results_characters(needs[i],&guard)==RR_ADVANCE_NEEDS_SHAPING&&!memcmp(&guard,&saved,sizeof(guard)));
 const char *bad[]={"^","A\nB","\xc0\xaf","\xed\xa0\x80","\xe2\x89","\xf0\x9f\x98\x80"};for(size_t i=0;i<COUNT(bad);i++)CHECK(rr_results_characters(bad[i],&guard)==RR_ADVANCE_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_characters(NULL,&guard)==RR_ADVANCE_INVALID&&rr_results_characters("A",NULL)==RR_ADVANCE_INVALID);
 CHECK(rr_results_characters("",&actual)==RR_ADVANCE_READY&&!actual.count&&!actual.advance&&!actual.range_width);
 char long_text[130];memset(long_text,'W',129);long_text[129]=0;CHECK(rr_results_characters(long_text,&guard)==RR_ADVANCE_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));long_text[128]=0;CHECK(rr_results_characters(long_text,&actual)==RR_ADVANCE_READY&&actual.count==128);
 printf("RESULTS_CHARACTERS_SUMMARY supported_nodes=%zu character_ranges=%u output_bytes=%zu failures=%u approx_raw_metric_verified=true no_raster_float_or_main_claim=true\n",COUNT(character_cases),total,sizeof(actual),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
