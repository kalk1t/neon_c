#include "rr_input_preferences.h"
#include <stdio.h>
#include <string.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
typedef struct CallbackCase{int index;double value;int invert;}CallbackCase;
#include "input_preferences_callback_fixture.h"
static int checks,failures;static double worst;
#define CHECK(x) do{checks++;if(!(x)){if(failures<8)fprintf(stderr,"input preferences line%d\n",__LINE__);failures++;}}while(0)
static void check_near(double x,double y){double e=fabs(x-y);if(e>worst)worst=e;CHECK(isfinite(x)&&e<1e-14);}
static void put(const char *path,const char *bytes){
#if defined(__EMSCRIPTEN__)
 EM_ASM({localStorage.setItem(UTF8ToString($0),UTF8ToString($1));},path,bytes);
#else
 FILE *f=fopen(path,"wb");CHECK(f!=NULL);if(f){CHECK(fwrite(bytes,1,strlen(bytes),f)==strlen(bytes));CHECK(fclose(f)==0);}
#endif
}
static void absent(const char *path){
#if defined(__EMSCRIPTEN__)
 EM_ASM({localStorage.removeItem(UTF8ToString($0));},path);
#else
 wchar_t w[4096];if(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,path,-1,w,4096))DeleteFileW(w);
#endif
}
static bool matches(const char *path,const char *bytes){
#if defined(__EMSCRIPTEN__)
 return EM_ASM_INT({return localStorage.getItem(UTF8ToString($0))===UTF8ToString($1);},path,bytes)!=0;
#else
 char data[1024];FILE *f=fopen(path,"rb");if(!f)return false;size_t n=fread(data,1,sizeof(data)-1,f);data[n]=0;fclose(f);return !strcmp(data,bytes);
#endif
}
static void make_path(char *out,size_t n,const char *root,const char *name){int got=snprintf(out,n,"%s/%s",root,name);CHECK(got>0&&(size_t)got<n);}
int main(int argc,char **argv){if(argc!=2)return 1;
#if defined(__EMSCRIPTEN__)
 // This test process uses only this fresh in-memory map, never browser/user storage.
 EM_ASM({globalThis.rrPrefMap=new Map();globalThis.rrPrefFail=false;globalThis.localStorage=({getItem(k){return rrPrefMap.has(k)?rrPrefMap.get(k):null;},setItem(k,v){if(rrPrefFail)throw Error('fixture quota');rrPrefMap.set(k,String(v));},removeItem(k){rrPrefMap.delete(k);}});});
#endif
 char data[RR_INPUT_SETTINGS_BYTES],path[2048];RRMouseSettings value={0};
 for(size_t i=0;i<sizeof(callback_cases)/sizeof(callback_cases[0]);i++){
  const CallbackCase *c=&callback_cases[i];RRMouseSettings input={c->value,c->invert!=0};size_t n=rr_input_settings_encode(data,sizeof(data),input);CHECK(n>0&&n<128);CHECK(rr_input_settings_decode(data,n,&value)==RR_SETTINGS_VALID);check_near(value.sensitivity,c->value);CHECK(value.invert==input.invert);
  for(size_t cut=0;cut<n-1;cut++){RRMouseSettings sentinel={1.7,true};CHECK(rr_input_settings_decode(data,cut,&sentinel)==RR_SETTINGS_INVALID);CHECK(sentinel.sensitivity==1.7&&sentinel.invert);}
 }
 const char *invalid[]={"{}","[]","null","{\"version\":1,\"sensitivity\":0.2,\"invert\":true}","{\"version\":1,\"sensitivity\":2.6,\"invert\":true}","{\"version\":1,\"sensitivity\":NaN,\"invert\":true}","{\"version\":1,\"sensitivity\":1e999,\"invert\":true}","{\"version\":1,\"sensitivity\":01,\"invert\":true}","{\"version\":1,\"sensitivity\":1,\"invert\":1}","{\"version\":1,\"sensitivity\":1,\"sensitivity\":1}","{\"version\":1,\"sensitivity\":1,\"invert\":true,\"coins\":99}","{\"version\":1,\"sensitivity\":1,\"invert\":true}x"};
 for(size_t i=0;i<sizeof(invalid)/sizeof(invalid[0]);i++)CHECK(rr_input_settings_decode(invalid[i],strlen(invalid[i]),&value)==RR_SETTINGS_INVALID);
 const char *future="{\"version\":2,\"sensitivity\":1,\"invert\":true}";CHECK(rr_input_settings_decode(future,strlen(future),&value)==RR_SETTINGS_UNSUPPORTED);
 const char *reordered="\xef\xbb\xbf { \"invert\":false,\"sensitivity\":1.25e0,\"version\":1 }\r\n";CHECK(rr_input_settings_decode(reordered,strlen(reordered),&value)==RR_SETTINGS_VALID);check_near(value.sensitivity,1.25);CHECK(!value.invert);
 CHECK(rr_input_settings_encode(data,1,(RRMouseSettings){1,false})==0);CHECK(rr_input_settings_encode(data,sizeof(data),(RRMouseSettings){NAN,false})==0);
 // Missing store: startup/ticks do not create anything, debounce and forced close save edits.
 make_path(path,sizeof(path),argv[1],"valid.json");absent(path);RRInputPreferences s,t;CHECK(rr_input_preferences_open(&s,path,&value)==RR_PREF_MISSING);CHECK(value.sensitivity==1&&!value.invert);
 CHECK(rr_input_preferences_tick(&s,value,0,false)==RR_PREF_MISSING);CHECK(!s.existed&&!s.dirty);
 value=(RRMouseSettings){1.25,true};rr_input_preferences_tick(&s,value,1,false);CHECK(s.dirty&&!s.existed);rr_input_preferences_tick(&s,value,1.24,false);CHECK(!s.existed);CHECK(rr_input_preferences_tick(&s,value,1.25,false)==RR_PREF_OK);CHECK(!s.dirty);
 CHECK(rr_input_preferences_open(&t,path,&value)==RR_PREF_OK);check_near(value.sensitivity,1.25);CHECK(value.invert);
 CHECK(rr_input_preferences_tick(&s,(RRMouseSettings){.3,false},2,true)==RR_PREF_OK);CHECK(rr_input_preferences_open(&t,path,&value)==RR_PREF_OK);check_near(value.sensitivity,.3);CHECK(!value.invert);
 // Competing/newer client wins: exact baseline comparison refuses a stale overwrite.
 CHECK(rr_input_preferences_save(&s,(RRMouseSettings){2,false})==RR_PREF_OK);CHECK(rr_input_preferences_save(&t,(RRMouseSettings){2.5,true})==RR_PREF_CONFLICT);CHECK(!t.writable);CHECK(rr_input_preferences_open(&t,path,&value)==RR_PREF_OK);check_near(value.sensitivity,2);CHECK(!value.invert);
 // Unsupported/malformed/oversized data is preserved, never rewritten as defaults.
 for(int kind=0;kind<3;kind++){
  const char *text=kind==0?future:kind==1?invalid[10]:"broken";put(path,text);RRPreferenceStatus status=rr_input_preferences_open(&s,path,&value);CHECK(status==(kind==0?RR_PREF_UNSUPPORTED:RR_PREF_INVALID));CHECK(!s.writable);CHECK(rr_input_preferences_save(&s,(RRMouseSettings){1.5,false})==RR_PREF_READ_ONLY);CHECK(matches(path,text));
 }
 char huge[RR_INPUT_SETTINGS_BYTES+8];memset(huge,'x',sizeof(huge)-1);huge[sizeof(huge)-1]=0;put(path,huge);CHECK(rr_input_preferences_open(&s,path,&value)==RR_PREF_INVALID);CHECK(!s.writable&&matches(path,huge));
#if defined(__EMSCRIPTEN__)
 // Existing browser preferences are read as fallback, but never mutated or migrated until a user edit.
 EM_ASM({rrPrefMap.delete('neon.input.v1');rrPrefMap.set('neon.sens','1.55');rrPrefMap.set('neon.invert','1');});
 CHECK(rr_input_preferences_default(&s,&value)==RR_PREF_MISSING);check_near(value.sensitivity,1.55);CHECK(value.invert);CHECK(rr_input_preferences_tick(&s,value,0,false)==RR_PREF_MISSING);CHECK(EM_ASM_INT({return rrPrefMap.has('neon.input.v1');})==0);
 CHECK(rr_input_preferences_save(&s,(RRMouseSettings){2.1,false})==RR_PREF_OK);CHECK(EM_ASM_INT({return rrPrefMap.get('neon.sens')==='1.55'&&rrPrefMap.get('neon.invert')==='1';})!=0);
 EM_ASM({rrPrefFail=true;});CHECK(rr_input_preferences_save(&s,(RRMouseSettings){.8,true})==RR_PREF_IO_ERROR);EM_ASM({rrPrefFail=false;});CHECK(rr_input_preferences_default(&t,&value)==RR_PREF_OK);check_near(value.sensitivity,2.1);CHECK(!value.invert);
 EM_ASM({rrPrefFail=true;});CHECK(rr_input_preferences_tick(&s,(RRMouseSettings){.8,true},10,true)==RR_PREF_IO_ERROR);EM_ASM({rrPrefFail=false;});CHECK(rr_input_preferences_tick(&s,(RRMouseSettings){.8,true},10.1,true)==RR_PREF_IO_ERROR);CHECK(rr_input_preferences_default(&t,&value)==RR_PREF_OK);check_near(value.sensitivity,2.1);CHECK(rr_input_preferences_tick(&s,(RRMouseSettings){.8,true},12,false)==RR_PREF_OK);
#else
 // Existing temp/lock files are not adopted or removed. Actual Windows rename failure preserves the target.
 absent(path);CHECK(rr_input_preferences_open(&s,path,&value)==RR_PREF_MISSING);CHECK(rr_input_preferences_save(&s,(RRMouseSettings){1,false})==RR_PREF_OK);
 char auxiliary[2100];snprintf(auxiliary,sizeof(auxiliary),"%s.tmp.0",path);put(auxiliary,"preexisting fixture");CHECK(rr_input_preferences_save(&s,(RRMouseSettings){1.5,true})==RR_PREF_OK);CHECK(matches(auxiliary,"preexisting fixture"));
 snprintf(auxiliary,sizeof(auxiliary),"%s.lock",path);put(auxiliary,"preexisting lock fixture");CHECK(rr_input_preferences_save(&s,(RRMouseSettings){1.6,true})==RR_PREF_BUSY);CHECK(matches(auxiliary,"preexisting lock fixture"));absent(auxiliary);
 wchar_t w[4096];CHECK(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,path,-1,w,4096)>0);HANDLE held=CreateFileW(w,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(held!=INVALID_HANDLE_VALUE);
 CHECK(rr_input_preferences_save(&s,(RRMouseSettings){1.6,false})==RR_PREF_IO_ERROR);if(held!=INVALID_HANDLE_VALUE)CloseHandle(held);CHECK(rr_input_preferences_open(&t,path,&value)==RR_PREF_OK);check_near(value.sensitivity,1.5);CHECK(value.invert);CHECK(rr_input_preferences_save(&s,(RRMouseSettings){1.6,false})==RR_PREF_OK);
 snprintf(auxiliary,sizeof(auxiliary),"%s.lock",path);put(auxiliary,"retry lock");CHECK(rr_input_preferences_tick(&s,(RRMouseSettings){1.7,true},10,true)==RR_PREF_BUSY);absent(auxiliary);CHECK(rr_input_preferences_tick(&s,(RRMouseSettings){1.7,true},10.1,true)==RR_PREF_BUSY);CHECK(rr_input_preferences_open(&t,path,&value)==RR_PREF_OK);check_near(value.sensitivity,1.6);CHECK(rr_input_preferences_tick(&s,(RRMouseSettings){1.7,true},12,false)==RR_PREF_OK);
 make_path(path,sizeof(path),argv[1],"utf8-\xc3\xa9.json");absent(path);CHECK(rr_input_preferences_open(&s,path,&value)==RR_PREF_MISSING);CHECK(rr_input_preferences_save(&s,(RRMouseSettings){.75,true})==RR_PREF_OK);CHECK(rr_input_preferences_open(&t,path,&value)==RR_PREF_OK);check_near(value.sensitivity,.75);CHECK(value.invert);
#endif
 // Units use the same guarded replace primitive, a separate path and raw browser-compatible values.
 RRUnitsPreferences us,ut;RRSpeedUnit unit;
 make_path(path,sizeof(path),argv[1],"units-v1.txt");absent(path);
 CHECK(rr_units_preferences_open(&us,path,&unit)==RR_PREF_MISSING&&unit==RR_SPEED_KMH);
 CHECK(rr_units_preferences_tick(&us,unit,0,true)==RR_PREF_MISSING);CHECK(!matches(path,"kmh"));
 CHECK(rr_units_preferences_tick(&us,RR_SPEED_MPH,1,false)==RR_PREF_MISSING);
 CHECK(rr_units_preferences_tick(&us,RR_SPEED_MPH,1.249,false)==RR_PREF_MISSING);
 CHECK(rr_units_preferences_tick(&us,RR_SPEED_MPH,1.25,false)==RR_PREF_OK&&matches(path,"mph"));
 CHECK(rr_units_preferences_open(&ut,path,&unit)==RR_PREF_OK&&unit==RR_SPEED_MPH);
 CHECK(rr_units_preferences_tick(&ut,RR_SPEED_KMH,2,true)==RR_PREF_OK&&matches(path,"kmh"));
 CHECK(rr_units_preferences_save(&us,RR_SPEED_MPH)==RR_PREF_CONFLICT&&matches(path,"kmh"));
 const char *bad_units[]={"","MPH","mph\n","{\"version\":2}","mphx"};
 for(size_t i=0;i<sizeof(bad_units)/sizeof(bad_units[0]);i++){
  put(path,bad_units[i]);CHECK(rr_units_preferences_open(&us,path,&unit)==RR_PREF_INVALID&&unit==RR_SPEED_KMH);
  CHECK(rr_units_preferences_save(&us,RR_SPEED_MPH)==RR_PREF_READ_ONLY&&matches(path,bad_units[i]));
 }
 char large_units[RR_INPUT_SETTINGS_BYTES+2];memset(large_units,'x',sizeof(large_units)-1);large_units[sizeof(large_units)-1]=0;put(path,large_units);CHECK(rr_units_preferences_open(&us,path,&unit)==RR_PREF_INVALID);CHECK(rr_units_preferences_save(&us,RR_SPEED_MPH)==RR_PREF_READ_ONLY&&matches(path,large_units));
 absent(path);CHECK(rr_units_preferences_open(&us,path,&unit)==RR_PREF_MISSING);
 CHECK(rr_units_preferences_tick(&us,RR_SPEED_MPH,NAN,true)==RR_PREF_INVALID);
 CHECK(rr_units_preferences_save(&us,(RRSpeedUnit)9)==RR_PREF_INVALID);CHECK(!matches(path,"mph"));
 CHECK(rr_units_preferences_tick(&us,RR_SPEED_MPH,3,false)==RR_PREF_INVALID);
 CHECK(rr_units_preferences_tick(&us,RR_SPEED_KMH,3.1,true)==RR_PREF_INVALID&&!us.blob.dirty);
#if defined(__EMSCRIPTEN__)
 put("neon.units","mph");CHECK(rr_units_preferences_default(&ut,&unit)==RR_PREF_OK&&unit==RR_SPEED_MPH);
 EM_ASM({globalThis.rrPrefFail=true;});CHECK(rr_units_preferences_tick(&ut,RR_SPEED_KMH,4,true)==RR_PREF_IO_ERROR);
 EM_ASM({globalThis.rrPrefFail=false;});CHECK(rr_units_preferences_tick(&ut,RR_SPEED_KMH,4.1,true)==RR_PREF_IO_ERROR);
 CHECK(rr_units_preferences_tick(&ut,RR_SPEED_KMH,6,true)==RR_PREF_OK&&matches("neon.units","kmh"));
#else
 CHECK(rr_units_preferences_open(&us,path,&unit)==RR_PREF_MISSING);
 char units_lock[2100];snprintf(units_lock,sizeof(units_lock),"%s.lock",path);put(units_lock,"fixture lock");
 CHECK(rr_units_preferences_tick(&us,RR_SPEED_MPH,4,true)==RR_PREF_BUSY);absent(units_lock);
 CHECK(rr_units_preferences_tick(&us,RR_SPEED_MPH,4.1,true)==RR_PREF_BUSY);
 CHECK(rr_units_preferences_tick(&us,RR_SPEED_MPH,6,true)==RR_PREF_OK&&matches(path,"mph"));
#endif
 printf("Units preferences: isolated roundtrip,debounce/forced save/no startup write,malformed preservation,conflict,cancel and failed-write retry checked\n");
 printf("Input preferences:45 browser callback values,strict schema/truncation,debounce/forced save/conflict/preservation/platform failure; %d checks,max numeric error %.12g, %d failures; isolated native files or inert Web storage only,no GPU/user-store access\n",checks,worst,failures);return failures?1:0;
}
