#ifndef RR_CSS_FONT_METRICS_H
#define RR_CSS_FONT_METRICS_H
#include <math.h>
/* Verified unmodified faces: em1000; Orbitron hhea1011/-243, Rajdhani930/-346.
 * raylib rasterizes ascent-descent pixels, CSS font-size specifies em pixels. */
static inline double rr_css_font_ratio(int weight){return weight==0?1.276:1.254;}
static inline double rr_css_font_ascent(int weight){return weight==0?.930:1.011;}
static inline double rr_css_font_size(double css_size,int weight){return css_size*rr_css_font_ratio(weight);}
/* Nominal hhea line-box baseline; exact Chromium hinting/raster remains open. */
static inline double rr_css_line_baseline(double top,double height,double css_size,int weight)
{return top+(height-rr_css_font_size(css_size,weight))*.5+css_size*rr_css_font_ascent(weight);}
static inline double rr_css_raylib_origin(double baseline,double css_size,int weight,int native_base)
{
    if(native_base<=0)return baseline-css_size;
    double raster_ascent=floor(native_base*rr_css_font_ascent(weight)/rr_css_font_ratio(weight));
    return baseline-raster_ascent*rr_css_font_size(css_size,weight)/native_base;
}
#endif
