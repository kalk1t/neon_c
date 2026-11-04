#include "rr_results_advance.h"
#include <string.h>
#include "rr_results_advance_data.h"
static bool decode(const unsigned char *s,size_t n,size_t *used,uint32_t *out)
{
 uint32_t c=s[0];if(c<128){*out=c;*used=1;return true;}
 unsigned count=c>=194&&c<=223?2:c>=224&&c<=239?3:c>=240&&c<=244?4:0;if(!count||count>n)return false;
 c&=(1u<<(7-count))-1u;for(unsigned i=1;i<count;i++){if((s[i]&192)!=128)return false;c=(c<<6)|(s[i]&63);}
 if((count==2&&c<128)||(count==3&&c<2048)||(count==4&&c<65536)||c>1114111||(c>=55296&&c<=57343))return false;
 *out=c;*used=count;return true;
}
RRResultsAdvanceStatus rr_results_advance(const char *text,RRResultsAdvance *out)
{
 if(!text||!out)return RR_ADVANCE_INVALID;size_t bytes=0;while(bytes<512&&text[bytes])bytes++;if(bytes==512)return RR_ADVANCE_INVALID;
 RRResultsAdvance value={0};uint32_t previous=0;bool complex=false;
 for(size_t offset=0;offset<bytes;){
  uint32_t cp=0,ticks=0;size_t used=0;if(value.codepoints==128||!decode((const unsigned char*)text+offset,bytes-offset,&used,&cp))return RR_ADVANCE_INVALID;offset+=used;
  bool found=false;for(size_t i=0;i<sizeof(rr_results_advances)/sizeof(rr_results_advances[0]);i++)if(rr_results_advances[i].cp==cp){found=true;ticks=rr_results_advances[i].ticks;break;}
  if(!found)return RR_ADVANCE_INVALID;
  if(cp==32&&(!value.codepoints||previous==32||offset==bytes))complex=true;
  for(size_t i=0;i<sizeof(rr_results_nonzero_pairs)/sizeof(rr_results_nonzero_pairs[0]);i++)if(rr_results_nonzero_pairs[i].left==previous&&rr_results_nonzero_pairs[i].right==cp)complex=true;
  value.ticks+=ticks;value.codepoints++;previous=cp;
 }
 if(complex)return RR_ADVANCE_NEEDS_SHAPING;
 value.advance=(double)value.ticks/64000;value.range_width=(double)((value.ticks+999)/1000)/64;*out=value;return RR_ADVANCE_READY;
}
