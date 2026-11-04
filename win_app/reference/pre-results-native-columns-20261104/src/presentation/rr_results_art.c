#include "rr_results_art.h"
#include "rr_results_fallback_glyphs.h"
#include <stdio.h>
void rr_results_art_unload(RRResultsArt *art){if(!art)return;for(int i=0;i<RR_RESULTS_ART_COUNT;i++){if(art->glyphs[i].id)UnloadTexture(art->glyphs[i]);art->glyphs[i]=(Texture2D){0};}art->ready=false;}
bool rr_results_art_load(RRResultsArt *art)
{
 if(!art||art->ready)return false;for(int i=0;i<RR_RESULTS_ART_COUNT;i++)if(art->glyphs[i].id)return false;
 for(int i=0;i<RR_RESULTS_ART_COUNT;i++){
  const RRResultsFallbackGlyph *g=rr_results_fallback_glyphs+i;char path[256];snprintf(path,sizeof(path),"assets/ui-results-fallback/%s",g->png);art->glyphs[i]=LoadTexture(path);
  if(!art->glyphs[i].id||art->glyphs[i].width!=g->width_pixels||art->glyphs[i].height!=g->height_pixels){rr_results_art_unload(art);return false;}SetTextureFilter(art->glyphs[i],TEXTURE_FILTER_BILINEAR);
 }
 art->ready=true;return true;
}
typedef struct NativeSink {Font font;RRResultsArt *art;} NativeSink;
static void paint(void *context,const RRResultsPaint *p)
{NativeSink *s=context;if(p->fallback<0)DrawTextCodepoint(s->font,(int)p->codepoint,p->origin,p->font_size,p->color);else DrawTexturePro(s->art->glyphs[p->fallback],p->source,p->destination,(Vector2){0},0,p->color);}
static bool compatible(Font a,Font b){return a.texture.id&&a.texture.id==b.texture.id&&a.baseSize==b.baseSize&&a.glyphs==b.glyphs&&a.recs==b.recs&&a.glyphCount==b.glyphCount;}
bool rr_ui_results_row_draw(RRUI *ui,RRResultsArt *art,const RRResultsRow *row)
{
 if(!ui||!ui->loaded||!row||!compatible(ui->bold,row->name.font)||!compatible(ui->bold,row->value.font))return false;
 if(row->name.fallback||row->value.fallback){if(!art||!art->ready)return false;for(int i=0;i<RR_RESULTS_ART_COUNT;i++)if(!art->glyphs[i].id||art->glyphs[i].width!=rr_results_fallback_glyphs[i].width_pixels||art->glyphs[i].height!=rr_results_fallback_glyphs[i].height_pixels)return false;}
 NativeSink sink={ui->bold,art};return rr_results_row_emit(row,paint,&sink);
}
