#include "rr_input_preferences.h"
#include "rr_offline_best_codec.h"
#include "rr_road_picker.h"
#include "offline_best_reference.h"
#include <stdio.h>
#include <string.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
static int checks,failures;
#define CHECK(x) do{checks++;if(!(x)){if(failures<8)fprintf(stderr,"offline best line%d\n",__LINE__);failures++;}}while(0)
static void put(const char *path,const char *raw){
#if defined(__EMSCRIPTEN__)
 EM_ASM({localStorage.setItem(UTF8ToString($0),UTF8ToString($1));},path,raw);
#else
 FILE *f=fopen(path,"wb");CHECK(f!=NULL);if(f){CHECK(fwrite(raw,1,strlen(raw),f)==strlen(raw));CHECK(fclose(f)==0);}
#endif
}
static void absent(const char *path){
#if defined(__EMSCRIPTEN__)
 EM_ASM({localStorage.removeItem(UTF8ToString($0));},path);
#else
 CHECK(DeleteFileA(path)||GetLastError()==ERROR_FILE_NOT_FOUND);
#endif
}
static bool matches(const char *path,const char *raw){
#if defined(__EMSCRIPTEN__)
 return EM_ASM_INT({return localStorage.getItem(UTF8ToString($0))===UTF8ToString($1);},path,raw)!=0;
#else
 char data[1024];FILE *f=fopen(path,"rb");if(!f)return false;size_t n=fread(data,1,sizeof(data)-1,f);data[n]=0;fclose(f);return !strcmp(data,raw);
#endif
}
int main(int argc,char **argv){if(argc!=2)return 1;
#if defined(__EMSCRIPTEN__)
 EM_ASM({globalThis.rrBestMap=new Map();globalThis.rrBestFail=false;globalThis.localStorage=({getItem(k){return rrBestMap.has(k)?rrBestMap.get(k):null;},setItem(k,v){if(rrBestFail)throw Error('fixture quota');rrBestMap.set(k,String(v));},removeItem(k){rrBestMap.delete(k);}});});
#endif
 char raw[26];double decoded;
 for(int i=1;i<=10001;i++){double v=i*.0123456789012345;CHECK(rr_offline_best_encode(raw,sizeof(raw),v)==25);decoded=-1;CHECK(rr_offline_best_decode(raw,25,&decoded)&&!memcmp(&v,&decoded,8));}
 const double edges[]={DBL_MIN,DBL_MAX,nextafter(0.0,1.0),nextafter(1.0,0.0)};
 for(size_t i=0;i<sizeof(edges)/sizeof(edges[0]);i++){CHECK(rr_offline_best_encode(raw,sizeof(raw),edges[i])==25);CHECK(rr_offline_best_decode(raw,25,&decoded)&&!memcmp(&edges[i],&decoded,8));}
 CHECK(!rr_offline_best_encode(raw,sizeof(raw),0));CHECK(!rr_offline_best_encode(raw,sizeof(raw),-1));CHECK(!rr_offline_best_encode(raw,sizeof(raw),NAN));CHECK(!rr_offline_best_encode(raw,sizeof(raw),INFINITY));CHECK(!rr_offline_best_encode(raw,25,90));
 rr_offline_best_encode(raw,sizeof(raw),90);for(size_t n=0;n<25;n++){decoded=-1;CHECK(!rr_offline_best_decode(raw,n,&decoded)&&decoded==-1);}
 // Actual source cases drive the existing picker then persist/reload with bit-exact doubles.
 static RROfflineBestPreferences stores[RR_ROAD_COUNT],loaded;RRRoadPicker picker={0};static char paths[RR_ROAD_COUNT][2048];
 for(int i=0;i<RR_ROAD_COUNT;i++){snprintf(paths[i],sizeof(paths[i]),"%s/offline-best-r%d.txt",argv[1],i+1);absent(paths[i]);CHECK(rr_offline_best_open(&stores[i],paths[i],&decoded)==RR_PREF_MISSING&&decoded==0);CHECK(rr_offline_best_tick(&stores[i],0,0,true)==RR_PREF_MISSING&&!matches(paths[i],""));}
 for(size_t i=0;i<sizeof(best_cases)/sizeof(best_cases[0]);i++){
  const struct BestCase *c=&best_cases[i];picker.current_road=c->id;CHECK(picker.rows[c->id-1].best_lap==c->before);rr_road_picker_lap(&picker,c->lap);CHECK(picker.rows[c->id-1].best_lap==c->after);
  double old=stores[c->id-1].saved;CHECK(rr_offline_best_tick(&stores[c->id-1],c->after,1+i,true)==RR_PREF_OK);CHECK((stores[c->id-1].saved!=old)==(c->wrote!=0));
  CHECK(rr_offline_best_open(&loaded,paths[c->id-1],&decoded)==RR_PREF_OK&&decoded==c->after);
 }
 char path[2048];snprintf(path,sizeof(path),"%s/offline-best-guards.txt",argv[1]);absent(path);
 static RROfflineBestPreferences s,t;CHECK(rr_offline_best_open(&s,path,&decoded)==RR_PREF_MISSING);
 CHECK(rr_offline_best_tick(&s,100,1,false)==RR_PREF_MISSING);CHECK(rr_offline_best_tick(&s,100,1.249,false)==RR_PREF_MISSING);
 CHECK(rr_offline_best_tick(&s,100,1.25,false)==RR_PREF_OK);CHECK(rr_offline_best_open(&t,path,&decoded)==RR_PREF_OK&&decoded==100);
 CHECK(rr_offline_best_tick(&t,90,2,true)==RR_PREF_OK);CHECK(rr_offline_best_tick(&s,80,2,true)==RR_PREF_CONFLICT);CHECK(rr_offline_best_open(&t,path,&decoded)==RR_PREF_OK&&decoded==90);
 const char *invalid[]={"","90","RRBEST2:4056800000000000\n","RRBEST1:7ff0000000000000\n","RRBEST1:0000000000000000\n","RRBEST1:c056800000000000\n","RRBEST1:405680000000000x\n"};
 for(size_t i=0;i<sizeof(invalid)/sizeof(invalid[0]);i++){put(path,invalid[i]);CHECK(rr_offline_best_open(&s,path,&decoded)==RR_PREF_INVALID&&decoded==0);CHECK(rr_offline_best_tick(&s,85,3,true)==RR_PREF_INVALID&&matches(path,invalid[i]));}
 char large[514];memset(large,'x',sizeof(large)-1);large[sizeof(large)-1]=0;put(path,large);CHECK(rr_offline_best_open(&s,path,&decoded)==RR_PREF_INVALID);CHECK(rr_offline_best_tick(&s,85,3,true)==RR_PREF_INVALID&&matches(path,large));
 absent(path);CHECK(rr_offline_best_open(&s,path,&decoded)==RR_PREF_MISSING);CHECK(rr_offline_best_tick(&s,NAN,1,true)==RR_PREF_INVALID);CHECK(rr_offline_best_tick(&s,-1,1,true)==RR_PREF_INVALID);CHECK(rr_offline_best_tick(&s,90,INFINITY,true)==RR_PREF_INVALID);
#if defined(__EMSCRIPTEN__)
 EM_ASM({globalThis.rrBestFail=true;});CHECK(rr_offline_best_tick(&s,90,4,true)==RR_PREF_IO_ERROR);EM_ASM({globalThis.rrBestFail=false;});
 CHECK(rr_offline_best_tick(&s,90,4.1,true)==RR_PREF_IO_ERROR);CHECK(rr_offline_best_tick(&s,90,6,true)==RR_PREF_OK);
 rr_offline_best_encode(raw,sizeof(raw),77.25);put("neon.best","canonical untouched");put("neon.best.r2","canonical2 untouched");put("neon.win.offline.best.r2",raw);
 CHECK(rr_offline_best_default(&t,2,&decoded)==RR_PREF_OK&&decoded==77.25);CHECK(rr_offline_best_tick(&t,75,7,true)==RR_PREF_OK);CHECK(matches("neon.best","canonical untouched")&&matches("neon.best.r2","canonical2 untouched"));
#else
 char lock[2100];snprintf(lock,sizeof(lock),"%s.lock",path);put(lock,"fixture lock");CHECK(rr_offline_best_tick(&s,90,4,true)==RR_PREF_BUSY);absent(lock);
 CHECK(rr_offline_best_tick(&s,90,4.1,true)==RR_PREF_BUSY);CHECK(rr_offline_best_tick(&s,90,6,true)==RR_PREF_OK);
#endif
 CHECK(rr_offline_best_tick(&s,95,8,true)==RR_PREF_OK&&s.saved==90);CHECK(rr_offline_best_tick(&s,0,9,true)==RR_PREF_OK&&s.saved==90);
 CHECK(rr_offline_best_default(&t,0,&decoded)==RR_PREF_UNAVAILABLE&&decoded==0);CHECK(rr_offline_best_default(&t,10,&decoded)==RR_PREF_UNAVAILABLE&&decoded==0);
 printf("Offline bests:63 source strict-improvement cases across9 roads,10005 exact binary64 roundtrips, malformed/oversized/conflict/debounce/force/retry/monotonic preservation; checks=%d failures=%d; isolated fixtures only, no real user/browser/account/unlock writes\n",checks,failures);return failures?1:0;
}
