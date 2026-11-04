#include "rr_input_preferences.h"
#include "rr_offline_best_codec.h"
#include <string.h>
#include <stdio.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>

EM_JS(int,rr_web_preferences_read,(const char *path,char *out,int capacity),{
 try{const value=localStorage.getItem(UTF8ToString(path));if(value===null)return 0;const n=lengthBytesUTF8(value);if(n>capacity)return -2;stringToUTF8(value,out,capacity+1);return n+1;}catch(e){return -1;}
})
EM_JS(double,rr_web_preferences_legacy_sens,(void),{try{return Number(localStorage.getItem('neon.sens'))||1;}catch(e){return 1;}})
EM_JS(int,rr_web_preferences_legacy_invert,(void),{try{return localStorage.getItem('neon.invert')==='1';}catch(e){return 0;}})
EM_JS(int,rr_web_preferences_write,(const char *path,const char *data,int size),{try{localStorage.setItem(UTF8ToString(path),UTF8ToString(data,size));return 1;}catch(e){return 0;}})

#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wchar.h>
#else
#error Input preferences require the Windows or Emscripten platform adapter
#endif

/* Read/replace use one versioned JSON blob.0 missing,1 present,-1 unavailable,-2 oversize. */
static int read_blob(const char *path,char *out,size_t *size){
#if defined(__EMSCRIPTEN__)
 int n=rr_web_preferences_read(path,out,RR_INPUT_SETTINGS_BYTES);
 if(n<=0){*size=0;return n;}*size=(size_t)(n-1);return 1;
#else
 wchar_t w[RR_SETTINGS_PATH_BYTES];if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,path,-1,w,RR_SETTINGS_PATH_BYTES))return -1;
 HANDLE file=CreateFileW(w,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
 if(file==INVALID_HANDLE_VALUE){DWORD error=GetLastError();*size=0;return error==ERROR_FILE_NOT_FOUND||error==ERROR_PATH_NOT_FOUND?0:-1;}
 LARGE_INTEGER length;DWORD got=0;int result=-1;if(GetFileSizeEx(file,&length)){if(length.QuadPart>RR_INPUT_SETTINGS_BYTES)result=-2;else if(length.QuadPart>=0&&ReadFile(file,out,(DWORD)length.QuadPart,&got,NULL)&&got==(DWORD)length.QuadPart){*size=got;result=1;}}CloseHandle(file);return result;
#endif
}
static bool same(double a,double b){return fabs(a-b)<=1e-14;}
static bool equal(RRMouseSettings a,RRMouseSettings b){return same(a.sensitivity,b.sensitivity)&&a.invert==b.invert;}
RRPreferenceStatus rr_input_preferences_open(RRInputPreferences *s,const char *path,RRMouseSettings *value){
 if(!s||!value)return RR_PREF_UNAVAILABLE;memset(s,0,sizeof(*s));s->saved=s->pending=*value=rr_input_settings_defaults();s->status=RR_PREF_UNAVAILABLE;
 if(!path||strlen(path)>=sizeof(s->path))return s->status;memcpy(s->path,path,strlen(path)+1);char raw[RR_INPUT_SETTINGS_BYTES+1];size_t size=0;int read=read_blob(path,raw,&size);
 if(read<0){s->status=read==-2?RR_PREF_INVALID:RR_PREF_UNAVAILABLE;return s->status;}
 if(!read){s->writable=true;s->status=RR_PREF_MISSING;return s->status;}
 s->existed=true;s->baseline_size=size;memcpy(s->baseline,raw,size);RRSettingsParse parsed=rr_input_settings_decode(raw,size,value);
 if(parsed!=RR_SETTINGS_VALID){s->status=parsed==RR_SETTINGS_UNSUPPORTED?RR_PREF_UNSUPPORTED:RR_PREF_INVALID;return s->status;}
 s->saved=s->pending=*value;s->writable=true;s->status=RR_PREF_OK;return s->status;
}
RRPreferenceStatus rr_input_preferences_default(RRInputPreferences *s,RRMouseSettings *value){
#if defined(__EMSCRIPTEN__)
 RRPreferenceStatus status=rr_input_preferences_open(s,"neon.input.v1",value);
 if(status==RR_PREF_MISSING){double sensitivity=rr_web_preferences_legacy_sens();int invert=rr_web_preferences_legacy_invert();RRMouseSettings legacy={sensitivity,invert!=0};if(rr_input_settings_valid(legacy))s->saved=s->pending=*value=legacy;}
 return status;
#else
 wchar_t base[RR_SETTINGS_PATH_BYTES];DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",base,RR_SETTINGS_PATH_BYTES);char path[RR_SETTINGS_PATH_BYTES];
 if(!n||n>=RR_SETTINGS_PATH_BYTES||n+64>=RR_SETTINGS_PATH_BYTES){memset(s,0,sizeof(*s));*value=s->saved=s->pending=rr_input_settings_defaults();return s->status=RR_PREF_UNAVAILABLE;}
 const wchar_t suffix[]=L"\\RoadstersRace\\input-settings-v1.json";memcpy(base+n,suffix,sizeof(suffix));if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,base,-1,path,RR_SETTINGS_PATH_BYTES,NULL,NULL)){memset(s,0,sizeof(*s));*value=s->saved=s->pending=rr_input_settings_defaults();return s->status=RR_PREF_UNAVAILABLE;}
 RRPreferenceStatus status=rr_input_preferences_open(s,path,value);s->create_parent=true;return status;
#endif
}
static bool baseline_equal(RRInputPreferences *s){char raw[RR_INPUT_SETTINGS_BYTES+1];size_t n=0;int result=read_blob(s->path,raw,&n);return s->existed?result==1&&n==s->baseline_size&&!memcmp(raw,s->baseline,n):result==0;}
static RRPreferenceStatus replace_blob(RRInputPreferences *s,const char *data,size_t size){
#if defined(__EMSCRIPTEN__)
 if(!baseline_equal(s))return RR_PREF_CONFLICT;
 int ok=rr_web_preferences_write(s->path,data,(int)size);return ok?RR_PREF_OK:RR_PREF_IO_ERROR;
#else
 wchar_t dest[RR_SETTINGS_PATH_BYTES],lockpath[RR_SETTINGS_PATH_BYTES],temp[RR_SETTINGS_PATH_BYTES];int len=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s->path,-1,dest,RR_SETTINGS_PATH_BYTES);if(!len||len+32>=RR_SETTINGS_PATH_BYTES)return RR_PREF_UNAVAILABLE;
 if(s->create_parent){wchar_t parent[RR_SETTINGS_PATH_BYTES];memcpy(parent,dest,(size_t)len*sizeof(wchar_t));int end=len-2;while(end>0&&parent[end]!='\\'&&parent[end]!='/')end--;if(end<=0)return RR_PREF_UNAVAILABLE;parent[end]=0;if(!CreateDirectoryW(parent,NULL)&&GetLastError()!=ERROR_ALREADY_EXISTS)return RR_PREF_IO_ERROR;DWORD attributes=GetFileAttributesW(parent);if(attributes==INVALID_FILE_ATTRIBUTES||!(attributes&FILE_ATTRIBUTE_DIRECTORY))return RR_PREF_IO_ERROR;}
 memcpy(lockpath,dest,(size_t)len*sizeof(wchar_t));memcpy(lockpath+len-1,L".lock",6*sizeof(wchar_t));HANDLE lock=CreateFileW(lockpath,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE,NULL);if(lock==INVALID_HANDLE_VALUE){DWORD error=GetLastError();return error==ERROR_FILE_EXISTS||error==ERROR_ALREADY_EXISTS||error==ERROR_SHARING_VIOLATION?RR_PREF_BUSY:RR_PREF_IO_ERROR;}
 RRPreferenceStatus result=RR_PREF_IO_ERROR;HANDLE file=INVALID_HANDLE_VALUE;bool own_temp=false;DWORD wrote=0;
 if(!baseline_equal(s)){result=RR_PREF_CONFLICT;goto done;}
 for(int i=0;i<16;i++){int printed=swprintf(temp,RR_SETTINGS_PATH_BYTES,L"%ls.tmp.%d",dest,i);if(printed<0)goto done;file=CreateFileW(temp,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_WRITE_THROUGH,NULL);if(file!=INVALID_HANDLE_VALUE){own_temp=true;break;}if(GetLastError()!=ERROR_FILE_EXISTS&&GetLastError()!=ERROR_ALREADY_EXISTS)goto done;}
 if(!own_temp){result=RR_PREF_BUSY;goto done;}
 if(!WriteFile(file,data,(DWORD)size,&wrote,NULL)||wrote!=size||!FlushFileBuffers(file))goto done;
 if(!CloseHandle(file)){file=INVALID_HANDLE_VALUE;goto done;}file=INVALID_HANDLE_VALUE;
 if(!baseline_equal(s)){result=RR_PREF_CONFLICT;goto done;}
 if(MoveFileExW(temp,dest,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)){own_temp=false;result=RR_PREF_OK;}
done:
 if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);if(own_temp)DeleteFileW(temp);CloseHandle(lock);return result;
#endif
}
RRPreferenceStatus rr_input_preferences_save(RRInputPreferences *s,RRMouseSettings value){
 if(!s||!s->writable)return RR_PREF_READ_ONLY;char data[RR_INPUT_SETTINGS_BYTES];size_t n=rr_input_settings_encode(data,sizeof(data),value);if(!n)return s->status=RR_PREF_INVALID;
 RRPreferenceStatus status=replace_blob(s,data,n);s->status=status;if(status==RR_PREF_CONFLICT)s->writable=false;
 if(status==RR_PREF_OK){memcpy(s->baseline,data,n);s->baseline_size=n;s->existed=true;s->saved=s->pending=value;s->dirty=false;}return status;
}
RRPreferenceStatus rr_input_preferences_tick(RRInputPreferences *s,RRMouseSettings value,double now,bool force){
 if(!s||!rr_input_settings_valid(value)||!isfinite(now))return RR_PREF_INVALID;
 if(!equal(value,s->pending)){s->pending=value;s->changed_at=now;s->retry_at=0;s->dirty=!equal(value,s->saved);}
 if(!s->dirty)return s->status;if(!s->writable)return s->status;
 if(now<s->retry_at)return s->status;
 if(force||now-s->changed_at>=.25){RRPreferenceStatus status=rr_input_preferences_save(s,value);if(status!=RR_PREF_OK)s->retry_at=now+2;return status;}return s->status;
}


RRPreferenceStatus rr_units_preferences_open(RRUnitsPreferences *s,const char *path,RRSpeedUnit *value){
 if(!s||!value)return RR_PREF_UNAVAILABLE;memset(s,0,sizeof(*s));*value=s->saved=s->pending=RR_SPEED_KMH;
 RRInputPreferences *b=&s->blob;b->status=RR_PREF_UNAVAILABLE;
 if(!path||strlen(path)>=sizeof(b->path))return b->status;memcpy(b->path,path,strlen(path)+1);
 char raw[RR_INPUT_SETTINGS_BYTES+1];size_t size=0;int read=read_blob(path,raw,&size);
 if(read<0)return b->status=read==-2?RR_PREF_INVALID:RR_PREF_UNAVAILABLE;
 if(!read){b->writable=true;return b->status=RR_PREF_MISSING;}
 b->existed=true;b->baseline_size=size;memcpy(b->baseline,raw,size);
 if(size!=3||(memcmp(raw,"kmh",3)&&memcmp(raw,"mph",3)))return b->status=RR_PREF_INVALID;
 *value=s->saved=s->pending=memcmp(raw,"mph",3)?RR_SPEED_KMH:RR_SPEED_MPH;b->writable=true;return b->status=RR_PREF_OK;
}
RRPreferenceStatus rr_units_preferences_default(RRUnitsPreferences *s,RRSpeedUnit *value){
#if defined(__EMSCRIPTEN__)
 return rr_units_preferences_open(s,"neon.units",value);
#else
 wchar_t base[RR_SETTINGS_PATH_BYTES];char path[RR_SETTINGS_PATH_BYTES];DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",base,RR_SETTINGS_PATH_BYTES);
 if(!n||n>=RR_SETTINGS_PATH_BYTES||n+64>=RR_SETTINGS_PATH_BYTES)return rr_units_preferences_open(s,NULL,value);
 const wchar_t suffix[]=L"\\RoadstersRace\\speed-units-v1.txt";memcpy(base+n,suffix,sizeof(suffix));
 if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,base,-1,path,RR_SETTINGS_PATH_BYTES,NULL,NULL))return rr_units_preferences_open(s,NULL,value);
 RRPreferenceStatus status=rr_units_preferences_open(s,path,value);s->blob.create_parent=true;return status;
#endif
}
RRPreferenceStatus rr_units_preferences_save(RRUnitsPreferences *s,RRSpeedUnit value){
 if(!s)return RR_PREF_UNAVAILABLE;RRInputPreferences *b=&s->blob;if(!b->writable)return RR_PREF_READ_ONLY;
 if(value!=RR_SPEED_KMH&&value!=RR_SPEED_MPH)return b->status=RR_PREF_INVALID;
 const char *raw=value==RR_SPEED_MPH?"mph":"kmh";RRPreferenceStatus status=replace_blob(b,raw,3);b->status=status;
 if(status==RR_PREF_CONFLICT)b->writable=false;
 if(status==RR_PREF_OK){memcpy(b->baseline,raw,3);b->baseline_size=3;b->existed=true;s->saved=s->pending=value;b->dirty=false;}
 return status;
}
RRPreferenceStatus rr_units_preferences_tick(RRUnitsPreferences *s,RRSpeedUnit value,double now,bool force){
 if(!s||(value!=RR_SPEED_KMH&&value!=RR_SPEED_MPH)||!isfinite(now))return RR_PREF_INVALID;
 RRInputPreferences *b=&s->blob;
 if(value!=s->pending){s->pending=value;b->changed_at=now;b->retry_at=0;b->dirty=value!=s->saved;}
 if(!b->dirty||!b->writable||now<b->retry_at)return b->status;
 if(force||now-b->changed_at>=.25){RRPreferenceStatus status=rr_units_preferences_save(s,value);if(status!=RR_PREF_OK)b->retry_at=now+2;return status;}return b->status;
}


RRPreferenceStatus rr_offline_best_open(RROfflineBestPreferences *s,const char *path,double *seconds){
 if(!s||!seconds)return RR_PREF_UNAVAILABLE;memset(s,0,sizeof(*s));*seconds=0;RRInputPreferences *b=&s->blob;b->status=RR_PREF_UNAVAILABLE;
 if(!path||strlen(path)>=sizeof(b->path))return b->status;memcpy(b->path,path,strlen(path)+1);
 char raw[RR_INPUT_SETTINGS_BYTES+1];size_t size=0;int read=read_blob(path,raw,&size);
 if(read<0)return b->status=read==-2?RR_PREF_INVALID:RR_PREF_UNAVAILABLE;
 if(!read){b->writable=true;return b->status=RR_PREF_MISSING;}
 b->existed=true;b->baseline_size=size;memcpy(b->baseline,raw,size);
 if(!rr_offline_best_decode(raw,size,seconds))return b->status=RR_PREF_INVALID;
 s->saved=s->pending=*seconds;b->writable=true;return b->status=RR_PREF_OK;
}
RRPreferenceStatus rr_offline_best_default(RROfflineBestPreferences *s,int road_id,double *seconds){
 if(road_id<1||road_id>9)return rr_offline_best_open(s,NULL,seconds);
#if defined(__EMSCRIPTEN__)
 char path[64];snprintf(path,sizeof(path),"neon.win.offline.best.r%d",road_id);return rr_offline_best_open(s,path,seconds);
#else
 wchar_t base[RR_SETTINGS_PATH_BYTES],suffix[64];char path[RR_SETTINGS_PATH_BYTES];DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",base,RR_SETTINGS_PATH_BYTES);
 if(!n||n>=RR_SETTINGS_PATH_BYTES||n+64>=RR_SETTINGS_PATH_BYTES)return rr_offline_best_open(s,NULL,seconds);
 int wrote=swprintf(suffix,64,L"\\RoadstersRace\\offline-best-r%d-v1.txt",road_id);if(wrote<0||wrote>=64)return rr_offline_best_open(s,NULL,seconds);memcpy(base+n,suffix,((size_t)wrote+1)*sizeof(wchar_t));
 if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,base,-1,path,RR_SETTINGS_PATH_BYTES,NULL,NULL))return rr_offline_best_open(s,NULL,seconds);
 RRPreferenceStatus status=rr_offline_best_open(s,path,seconds);s->blob.create_parent=true;return status;
#endif
}
RRPreferenceStatus rr_offline_best_tick(RROfflineBestPreferences *s,double seconds,double now,bool force){
 if(!s||!isfinite(seconds)||seconds<0||!isfinite(now))return RR_PREF_INVALID;RRInputPreferences *b=&s->blob;
 if(seconds>0&&(s->pending==0||seconds<s->pending)){s->pending=seconds;b->changed_at=now;b->retry_at=0;b->dirty=s->pending!=s->saved;}
 if(!b->dirty||!b->writable||now<b->retry_at)return b->status;
 if(!force&&now-b->changed_at<.25)return b->status;
 char raw[26];size_t size=rr_offline_best_encode(raw,sizeof(raw),s->pending);if(!size)return b->status=RR_PREF_INVALID;
 RRPreferenceStatus status=replace_blob(b,raw,size);b->status=status;if(status==RR_PREF_CONFLICT)b->writable=false;
 if(status==RR_PREF_OK){memcpy(b->baseline,raw,size);b->baseline_size=size;b->existed=true;b->dirty=false;s->saved=s->pending;}else b->retry_at=now+2;
 return status;
}
