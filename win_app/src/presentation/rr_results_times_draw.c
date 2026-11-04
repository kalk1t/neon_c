#include "rr_results_times_draw.h"
#include "rr_css_font_metrics.h"
bool rr_ui_results_times_draw(RRUI *ui,const RRResultsTimes *text,float center,float y)
{
 if(!ui||!ui->loaded||!text||text->count>2||!ui->bold.glyphs||!ui->bold.recs||ui->bold.baseSize<=0)return false;
 if(text->count){int dash=GetGlyphIndex(ui->bold,8212);if(dash<0||dash>=ui->bold.glyphCount||ui->bold.glyphs[dash].value!=8212)return false;}
 float widths[2]={0};for(size_t i=0;i<text->count;i++)widths[i]=MeasureTextEx(ui->bold,text->laps[i],(float)rr_css_font_size(16,700),0).x;
 RRResultsTimesLayout layout;if(!rr_results_times_layout(text,center,y,widths,&layout))return false;
 rr_ui_text_center_css_glow(ui,text->total,center,layout.total_baseline,32,0,900,(Color){255,255,255,255},true);
 for(size_t i=0;i<text->count;i++)rr_ui_text_css(ui,text->laps[i],layout.left[i],layout.lap_baseline,16,0,700,
  i==text->best?(Color){255,43,214,255}:(Color){207,239,255,255},false);
 return true;
}
