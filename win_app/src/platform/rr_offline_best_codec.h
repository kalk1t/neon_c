#ifndef RR_OFFLINE_BEST_CODEC_H
#define RR_OFFLINE_BEST_CODEC_H
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
/* Exact binary64 bits in ASCII, locale/endian independent on IEEE binary64 hosts.
 * Private offline record format, deliberately not a browser/account import. */
_Static_assert(sizeof(double)==8&&FLT_RADIX==2&&DBL_MANT_DIG==53&&DBL_MAX_EXP==1024,"Offline records require binary64 doubles");
static inline size_t rr_offline_best_encode(char *out,size_t capacity,double seconds){
 if(!out||capacity<26||!isfinite(seconds)||seconds<=0)return 0;
 uint64_t bits;memcpy(&bits,&seconds,8);memcpy(out,"RRBEST1:",8);const char hex[]="0123456789abcdef";
 for(int i=0;i<16;i++)out[8+i]=hex[(bits>>(60-i*4))&15];out[24]='\n';out[25]=0;return 25;
}
static inline bool rr_offline_best_decode(const char *raw,size_t size,double *seconds){
 if(!raw||!seconds||size!=25||memcmp(raw,"RRBEST1:",8)||raw[24]!='\n')return false;
 uint64_t bits=0;for(int i=0;i<16;i++){unsigned char c=(unsigned char)raw[8+i];int n=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:-1;if(n<0)return false;bits=(bits<<4)|(unsigned)n;}
 double value;memcpy(&value,&bits,8);if(!isfinite(value)||value<=0)return false;*seconds=value;return true;
}
#endif
