#include "rr_input_settings.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
typedef struct Cursor {const char *p,*end;} Cursor;
static void ws(Cursor *c){while(c->p<c->end&&(*c->p==' '||*c->p=='\t'||*c->p=='\r'||*c->p=='\n'))c->p++;}
static bool take(Cursor *c,char v){ws(c);if(c->p==c->end||*c->p!=v)return false;c->p++;return true;}
static bool word(Cursor *c,const char *s){ws(c);size_t n=strlen(s);if((size_t)(c->end-c->p)<n||memcmp(c->p,s,n))return false;c->p+=n;return true;}
static bool digit(char v){return v>='0'&&v<='9';}
static bool number(Cursor *c,double *result){
 ws(c);if(c->p==c->end)return false;bool negative=false;if(*c->p=='-'){negative=true;c->p++;}if(c->p==c->end||!digit(*c->p))return false;
 double v=0,scale=1;int digits=0;
 if(*c->p=='0'){c->p++;if(c->p<c->end&&digit(*c->p))return false;}
 else while(c->p<c->end&&digit(*c->p)){if(++digits>32)return false;v=v*10+(*c->p++-'0');}
 if(c->p<c->end&&*c->p=='.'){c->p++;if(c->p==c->end||!digit(*c->p))return false;while(c->p<c->end&&digit(*c->p)){if(++digits>64)return false;scale*=.1;v+=(*c->p++-'0')*scale;}}
 int exponent=0;bool down=false;if(c->p<c->end&&(*c->p=='e'||*c->p=='E')){c->p++;if(c->p<c->end&&(*c->p=='+'||*c->p=='-')){down=*c->p=='-';c->p++;}if(c->p==c->end||!digit(*c->p))return false;while(c->p<c->end&&digit(*c->p)){exponent=exponent*10+(*c->p++-'0');if(exponent>308)return false;}v*=pow(10,down?-exponent:exponent);}
 *result=negative?-v:v;return isfinite(*result);
}
bool rr_input_settings_valid(RRMouseSettings v){return isfinite(v.sensitivity)&&v.sensitivity>=.3&&v.sensitivity<=2.5;}
RRSettingsParse rr_input_settings_decode(const char *data,size_t size,RRMouseSettings *value){
 if(!data||!value||size>RR_INPUT_SETTINGS_BYTES)return RR_SETTINGS_INVALID;Cursor c={data,data+size};if(size>=3&&!memcmp(data,"\xef\xbb\xbf",3))c.p+=3;
 if(!take(&c,'{'))return RR_SETTINGS_INVALID;unsigned seen=0;double version=0;RRMouseSettings v=rr_input_settings_defaults();
 for(int item=0;item<3;item++){
  if(item&&!take(&c,','))return RR_SETTINGS_INVALID;if(!take(&c,'"'))return RR_SETTINGS_INVALID;const char *key=c.p;while(c.p<c.end&&*c.p!='"'){if(*c.p=='\\'||(unsigned char)*c.p<32)return RR_SETTINGS_INVALID;c.p++;}size_t n=(size_t)(c.p-key);if(!take(&c,'"')||!take(&c,':'))return RR_SETTINGS_INVALID;
  unsigned field=n==7&&!memcmp(key,"version",7)?1:n==11&&!memcmp(key,"sensitivity",11)?2:n==6&&!memcmp(key,"invert",6)?4:0;if(!field||(seen&field))return RR_SETTINGS_INVALID;seen|=field;
  if(field==1){if(!number(&c,&version))return RR_SETTINGS_INVALID;}
  if(field==2){if(!number(&c,&v.sensitivity))return RR_SETTINGS_INVALID;}
  if(field==4){if(word(&c,"true"))v.invert=true;else if(word(&c,"false"))v.invert=false;else return RR_SETTINGS_INVALID;}
 }
 if(seen!=7||!take(&c,'}'))return RR_SETTINGS_INVALID;ws(&c);if(c.p!=c.end||!rr_input_settings_valid(v))return RR_SETTINGS_INVALID;if(version!=1)return RR_SETTINGS_UNSUPPORTED;*value=v;return RR_SETTINGS_VALID;
}
size_t rr_input_settings_encode(char *out,size_t capacity,RRMouseSettings v){
 if(!out||!rr_input_settings_valid(v))return 0;uint64_t total=(uint64_t)floor(v.sensitivity*1e15+.5),whole=total/UINT64_C(1000000000000000),fraction=total%UINT64_C(1000000000000000);char decimal[48];
 int n=snprintf(decimal,sizeof(decimal),"%u.%015llu",(unsigned)whole,(unsigned long long)fraction);if(n<0||(size_t)n>=sizeof(decimal))return 0;while(n>0&&decimal[n-1]=='0')decimal[--n]=0;if(n>0&&decimal[n-1]=='.')decimal[--n]=0;
 n=snprintf(out,capacity,"{\"version\":1,\"sensitivity\":%s,\"invert\":%s}\n",decimal,v.invert?"true":"false");return n>=0&&(size_t)n<capacity?(size_t)n:0;
}
