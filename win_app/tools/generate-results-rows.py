from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');d=json.loads((p/'reference/ui-parity-20261103/results-rows-capture/reference.json').read_text(encoding='utf-8'))
def q(s):return '"'+''.join(chr(b) if 32<=b<127 and b not in [34,92] else '\\"' if b==34 else '\\\\' if b==92 else '\\%03o'%b for b in s.encode('utf-8'))+'"'
def num(v):return format(v,'.17g')
lines=['typedef struct ExpectedRow {const char *name,*value;bool me;} ExpectedRow;','static const struct RowBoard {size_t count,time_count;const char *names[11];RRResultsTime times[11];RRResultsCoin coins[11];ExpectedRow time_rows[11],coin_rows[11];} row_boards[]={']
for b in d['boards']:
 times=','.join('{%d,%s,%s}'%(v['racer'],num(v['seconds']),'true' if v['estimated'] else 'false') for v in b['model']['times']) or '{0,0,false}'
 coins=','.join('{%d,%d}'%(v['racer'],v['coins']) for v in b['model']['coins'])
 def exp(rows):return ','.join('{%s,%s,%s}'%(q(v['name']),q(v['value']),'true' if v['me'] else 'false') for v in rows) or '{NULL,NULL,false}'
 lines.append('{%d,%d,{%s},{%s},{%s},{%s},{%s}},'%(len(b['names']),len(b['model']['times']),','.join(q(n) for n in b['names']),times,coins,exp(b['expected']['times']),exp(b['expected']['coins'])))
lines.append('};')
m=json.loads((p/'reference/ui-parity-20261103/results-rows-capture/asset-provenance/glyph-metrics.json').read_text(encoding='utf-8'));m=[g for g in m if g['fontSizeCSS']==15]
lines+=['static const struct ExpectedGlyph {uint32_t cp;int scale,w,h;float x,y,dw,dh,advance;} expected_glyphs[]={']
for g in m:lines.append('{%du,%d,%d,%d,%sf,%sf,%sf,%sf,%sf},'%(g['codepoint'],g['deviceScale'],g['widthPixels'],g['heightPixels'],num(g['offsetFromBaselineCSS']['x'])+'.0' if float(g['offsetFromBaselineCSS']['x']).is_integer() else num(g['offsetFromBaselineCSS']['x']),num(g['offsetFromBaselineCSS']['y'])+'.0' if float(g['offsetFromBaselineCSS']['y']).is_integer() else num(g['offsetFromBaselineCSS']['y']),num(g['drawWidthCSS'])+'.0' if float(g['drawWidthCSS']).is_integer() else num(g['drawWidthCSS']),num(g['drawHeightCSS'])+'.0' if float(g['drawHeightCSS']).is_integer() else num(g['drawHeightCSS']),num(g['advanceCSS'])+'.0' if float(g['advanceCSS']).is_integer() else num(g['advanceCSS'])))
lines.append('};');(p/'tests/results_rows_reference.h').write_text('\n'.join(lines)+'\n',encoding='utf-8')
(p/'tests/test_results_rows.c').write_text('''#include "rr_results_text.h"
#include "rr_results_fallback_glyphs.h"
#include "rr_ui_codepoints.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "results_rows_reference.h"
static unsigned failures,paints,rows,total_paints,fallback_paints,primary_paints;
static RRResultsPaint captured[1280];static RRResultsRow prepared,guard,saved;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\\n",__LINE__,#x);failures++;}}while(0)
static bool near(double a,double b){return fabs(a-b)<.001;}
static void record(void *context,const RRResultsPaint *p){CHECK(context==captured);CHECK(paints<1280);if(paints<1280)captured[paints++]=*p;total_paints++;if(p->fallback<0)primary_paints++;else fallback_paints++;}
static const struct ExpectedGlyph *expected(uint32_t cp,int scale){for(size_t i=0;i<6;i++)if(expected_glyphs[i].cp==cp&&expected_glyphs[i].scale==scale)return expected_glyphs+i;return NULL;}
static int utf8(char out[5],uint32_t cp){int n=0;if(cp<128)out[n++]=(char)cp;else if(cp<2048){out[n++]=(char)(192|(cp>>6));out[n++]=(char)(128|(cp&63));}else{out[n++]=(char)(224|(cp>>12));out[n++]=(char)(128|((cp>>6)&63));out[n++]=(char)(128|(cp&63));}out[n]=0;return n;}
static float width(Font font,const RRResultsText *text){double sum=0;for(size_t i=0;i<text->count;i++){const RRResultsToken *t=text->tokens+i;CHECK(near(t->pen,sum));const struct ExpectedGlyph *g=expected(t->codepoint,text->device_scale);float advance;
 if(g){advance=g->advance;CHECK(t->fallback>=0&&rr_results_fallback_glyphs[t->fallback].codepoint==t->codepoint);}
 else{char s[5];utf8(s,t->codepoint);advance=MeasureTextEx(font,s,18.81f,0).x;CHECK(advance>0&&t->fallback==-1);}CHECK(near(t->advance,advance));sum+=advance;}CHECK(near(sum,text->width));return (float)sum;}
static bool color(Color a,Color b){return a.r==b.r&&a.g==b.g&&a.b==b.b&&a.a==b.a;}
static void verify(Font font,const ExpectedRow *source,RRResultsRowKind kind,int scale){
 float nw=width(font,&prepared.name),vw=width(font,&prepared.value);CHECK(near(prepared.value_x,1100.75-vw)&&near(prepared.left,100.25)&&nw+vw<1000.5);
 Color name=source->me?(Color){255,255,255,255}:kind==RR_RESULTS_COINS?(Color){243,224,176,255}:(Color){207,239,255,255};
 Color value=kind==RR_RESULTS_COINS?(Color){255,210,90,255}:source->me?(Color){255,43,214,255}:(Color){153,187,221,255};Color shadow=kind==RR_RESULTS_COINS?(Color){255,160,18,31}:(Color){255,43,214,31};
 paints=0;CHECK(rr_results_row_emit(&prepared,record,captured));CHECK(paints==(prepared.name.count+prepared.value.count)*(source->me?5:1));
 size_t index=0;for(int part=0;part<2;part++){
  const RRResultsText *text=part?&prepared.value:&prepared.name;float x=part?prepared.value_x:100.25f;char decoded[512];size_t bytes=0;
  for(int pass=source->me?0:4;pass<5;pass++){
   for(size_t j=0;j<text->count;j++){
   const RRResultsToken *t=text->tokens+j;CHECK(index<paints);if(index>=paints)continue;const RRResultsPaint *p=captured+index++;
   float dx=pass==0?-2.f:pass==1?2.f:0.f,dy=pass==2?-2.f:pass==3?2.f:0.f;
   CHECK(p->codepoint==t->codepoint&&p->shadow==(pass<4)&&near(p->font_size,18.81));CHECK(color(p->color,pass<4?shadow:part?value:name));
   const struct ExpectedGlyph *g=expected(t->codepoint,scale);
   if(g){CHECK(p->fallback>=0&&near(p->source.width,g->w)&&near(p->source.height,g->h));CHECK(near(p->destination.x,x+t->pen+g->x+dx)&&near(p->destination.y,200.5+g->y+dy));CHECK(near(p->destination.width,g->dw)&&near(p->destination.height,g->dh));}
   else CHECK(p->fallback==-1&&near(p->origin.x,x+t->pen+dx)&&near(p->origin.y,200.5-51*18.81/64+dy));
   if(pass==4){char s[5];int n=utf8(s,p->codepoint);CHECK(bytes+(size_t)n<sizeof(decoded));if(bytes+(size_t)n<sizeof(decoded)){memcpy(decoded+bytes,s,(size_t)n);bytes+=(size_t)n;}}
   }
  }
  decoded[bytes]=0;CHECK(!strcmp(decoded,part?source->value:source->name));
 }CHECK(index==paints);rows++;
}
int main(int argc,char **argv){CHECK(argc==3);if(argc!=3)return 1;SetTraceLogLevel(LOG_WARNING);size_t rgba=0;unsigned ink_pixels=0;
 for(int i=0;i<6;i++){
  const RRResultsFallbackGlyph *g=rr_results_fallback_glyphs+i;const struct ExpectedGlyph *e=expected(g->codepoint,g->device_scale);CHECK(e&&g->font_size_css==15&&g->source_tracking==0);if(!e)continue;
  CHECK(g->width_pixels==e->w&&g->height_pixels==e->h&&near(g->offset_x,e->x)&&near(g->offset_y,e->y)&&near(g->draw_width,e->dw)&&near(g->draw_height,e->dh)&&near(g->advance,e->advance));
  char path[1024];snprintf(path,sizeof(path),"%s/%s",argv[2],g->png);Image image=LoadImage(path);CHECK(image.data&&image.width==e->w&&image.height==e->h);if(!image.data)continue;Color *pixels=LoadImageColors(image);CHECK(pixels);if(!pixels){UnloadImage(image);continue;}
  unsigned ink=0,clear=0;for(int j=0;j<image.width*image.height;j++){if(pixels[j].a){CHECK(pixels[j].r==255&&pixels[j].g==255&&pixels[j].b==255);ink++;}else clear++;}CHECK(ink&&clear);ink_pixels+=ink;rgba+=(size_t)image.width*image.height*4;UnloadImageColors(pixels);UnloadImage(image);
 }CHECK(rgba==24072&&rgba+27060<=65536);
 int filebytes=0,count=0,cps[RR_UI_MAX_CODEPOINTS];unsigned char *data=LoadFileData(argv[1],&filebytes);CHECK(data);if(!data)return 1;
 int requested=rr_ui_codepoints(cps,700);GlyphInfo *glyphs=LoadFontData(data,filebytes,64,cps,requested,FONT_DEFAULT,&count);CHECK(glyphs&&count==96);if(!glyphs){UnloadFileData(data);return 1;}
 Rectangle *rects=NULL;Image atlas=GenImageFontAtlas(glyphs,&rects,count,64,4,0);CHECK(atlas.data&&rects);if(!rects){UnloadFontData(glyphs,count);UnloadFileData(data);return 1;}
 /* CPU measurement marker only, no GPU texture creation/bind/upload/destroy. */
 Font font={0};font.texture.id=1;font.baseSize=64;font.glyphCount=count;font.glyphs=glyphs;font.recs=rects;
 CHECK(glyphs[GetGlyphIndex(font,8776)].value!=8776&&glyphs[GetGlyphIndex(font,9819)].value!=9819);
 for(size_t i=0;i<sizeof(row_boards)/sizeof(row_boards[0]);i++){
  const struct RowBoard *b=row_boards+i;RRResultsCurrent snapshot={0};snapshot.time_count=b->time_count;snapshot.coin_count=b->count;memcpy(snapshot.times,b->times,sizeof(snapshot.times));memcpy(snapshot.coins,b->coins,sizeof(snapshot.coins));
  for(int scale=1;scale<=3;scale++)for(int kind=0;kind<2;kind++){size_t n=kind==RR_RESULTS_COINS?b->count:b->time_count;for(size_t j=0;j<n;j++){
   CHECK(rr_results_row_build(font,&snapshot,(RRResultsRowKind)kind,j,b->names,b->count,0,scale,100.25f,1100.75f,200.5f,&prepared)==RR_RESULTS_ROW_READY);verify(font,kind==RR_RESULTS_COINS?b->coin_rows+j:b->time_rows+j,(RRResultsRowKind)kind,scale);
  }}
 }
 static RRResultsText text,text_saved;memset(&text,0x5a,sizeof(text));text_saved=text;
 const char *bad[]={"^","\\xc0\\xaf","\\xed\\xa0\\x80","\\xe2\\x89","\\xf0\\x9f\\x98\\x80","A\\nB"};for(size_t i=0;i<6;i++){CHECK(!rr_results_text_build(font,bad[i],1,&text));CHECK(!memcmp(&text,&text_saved,sizeof(text)));}
 char long_text[513];memset(long_text,'A',sizeof(long_text));long_text[512]=0;CHECK(!rr_results_text_build(font,long_text,1,&text));long_text[129]=0;CHECK(!rr_results_text_build(font,long_text,1,&text));long_text[128]=0;CHECK(rr_results_text_build(font,long_text,1,&text)&&text.count==128);
 CHECK(!rr_results_text_build(font,"A",0,&text)&&!rr_results_text_build(font,"A",4,&text));CHECK(!rr_results_text_build((Font){0},"A",1,&text));
 RRResultsCurrent snapshot={0};snapshot.coin_count=1;snapshot.coins[0]=(RRResultsCoin){0,7};char name[]="A";const char *names[]={name};
 CHECK(rr_results_row_build(font,&snapshot,RR_RESULTS_COINS,0,names,1,0,3,100.25f,1100.75f,200.5f,&prepared)==RR_RESULTS_ROW_READY);name[0]='Z';snapshot.coins[0].coins=99;CHECK(prepared.name.tokens[prepared.name.count-1].codepoint=='A'&&prepared.value.tokens[0].codepoint=='7');
 memset(&guard,0x5a,sizeof(guard));saved=guard;CHECK(rr_results_row_build(font,&snapshot,RR_RESULTS_COINS,0,names,1,0,3,100,101,200,&guard)==RR_RESULTS_ROW_NEEDS_WRAP);CHECK(!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_row_build(font,&snapshot,RR_RESULTS_STANDINGS,0,names,1,0,3,0,1000,200,&guard)==RR_RESULTS_ROW_INVALID);CHECK(rr_results_row_build(font,&snapshot,RR_RESULTS_COINS,0,names,1,0,3,0,1000,NAN,&guard)==RR_RESULTS_ROW_INVALID);CHECK(!memcmp(&guard,&saved,sizeof(guard)));
 paints=0;prepared.value_x=NAN;CHECK(!rr_results_row_emit(&prepared,record,captured)&&!paints);CHECK(!rr_results_row_emit(NULL,record,captured)&&!paints);CHECK(!rr_results_row_emit(&prepared,NULL,captured)&&!paints);
 printf("RESULTS_ROWS_SUMMARY boards=%zu source_rows=%u paints=%u primary=%u fallback=%u rgba_new=%zu rgba_with_existing=%zu ink_pixels=%u plan_bytes=%zu row_bytes=%zu failures=%u recorded_not_rendered=true\\n",sizeof(row_boards)/sizeof(row_boards[0]),rows,total_paints,primary_paints,fallback_paints,rgba,rgba+27060,ink_pixels,sizeof(RRResultsText),sizeof(RRResultsRow),failures);
 UnloadImage(atlas);MemFree(rects);UnloadFontData(glyphs,count);UnloadFileData(data);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
''',encoding='utf-8')
if Path(__file__).resolve().parent.name=='roadsters-ui-audit':
 shutil.copy2(Path(__file__).resolve().with_name('capture-results-rows.mjs'),p/'tools/capture-results-rows.mjs');shutil.copy2(__file__,p/'tools/generate-results-rows.py')
print('Generated',len(d['boards']),'source boards and PNG/measurement/emission CPU checks.')
