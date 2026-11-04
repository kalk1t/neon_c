#include "rr_spin_indicator.h"
#include "rr_spin_control.h"
#include <stdio.h>
typedef struct SpinCase {double cooldown,fill;int cool;double opacity;} SpinCase;
#include "spin_indicator_fixture.h"
static double area(RRSpinPoint a,RRSpinPoint b,RRSpinPoint c){return ((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x))*.5;}
static void paint(double rgba[4],uint32_t color){double a=(color&255)/255.0;for(int i=0;i<3;i++){double value=(color>>(24-8*i))&255;rgba[i]=value/255*a+rgba[i]*(1-a);}rgba[3]=a+rgba[3]*(1-a);}
int main(void){int failures=0,max_triangles=0;double worst=0;
for(size_t i=0;i<sizeof(spin_cases)/sizeof(spin_cases[0]);i++){
 int before=failures;const SpinCase *c=&spin_cases[i];RRSpinIndicator style=rr_spin_indicator(c->cooldown);double e=fabs(style.fill-c->fill);if(e>worst)worst=e;if(e>1e-12||style.cool!=(c->cool!=0)||style.opacity!=c->opacity)failures++;
 if(rr_spin_indicator_color(style,0)!=UINT32_C(0xff2bd640)||rr_spin_indicator_color(style,1)!=UINT32_C(0x19e6ffff))failures++;
 unsigned child_alpha=(unsigned)floor(c->opacity*255+.5);if(rr_spin_indicator_color(style,2)!=(UINT32_C(0x0a042000)|child_alpha)||rr_spin_indicator_color(style,3)!=(UINT32_C(0xc4fbff00)|child_alpha))failures++;
 int total=0;for(int layer=0;layer<3;layer++){
   int n=rr_spin_indicator_sectors(style,layer);total+=n;double sum=0,span=layer==1?c->fill:1,r=layer==2?31:37;
   for(int k=0;k<n;k++){RRSpinPoint t[3];if(!rr_spin_indicator_triangle(style,layer,k,t)){failures++;continue;}double a=area(t[0],t[1],t[2]);if(a>1e-10||fabs(hypot(t[1].x,t[1].y)-r)>1e-10||fabs(hypot(t[2].x,t[2].y)-r)>1e-10)failures++;sum-=a;
     /* Progress begins at twelve o'clock and terminates clockwise at fill. */
     if(k==0&&(fabs(t[2].x)>1e-10||fabs(t[2].y+r)>1e-10))failures++;
     if(k==n-1&&(fabs(t[1].x-r*cos(-RR_PI*.5+2*RR_PI*span))>1e-10||fabs(t[1].y-r*sin(-RR_PI*.5+2*RR_PI*span))>1e-10))failures++;
   }
   double exact=RR_PI*r*r*span,bound=exact*(1-sin(2*RR_PI/64)/(2*RR_PI/64));if(sum>exact+1e-9||exact-sum>bound+1e-9)failures++;
 }
 if(total>max_triangles)max_triangles=total;if(total>192)failures++;
 /* Sample the union of emitted triangles, then source-over their actual byte
  * colors. Compare independent ideal CSS disk/conic/child coverage and alpha.
  * Points avoid curve silhouettes, the label and ambiguous conic boundaries. */
 const double angles[]={.05,.25,.5,.75,.95},radii[]={15,34};
 for(int r=0;r<2;r++)for(int a=0;a<5;a++){
   double turn=angles[a];if(fabs(turn-c->fill)<1e-10)continue;
   RRSpinPoint point={radii[r]*cos(-RR_PI*.5+2*RR_PI*turn),radii[r]*sin(-RR_PI*.5+2*RR_PI*turn)};double got[4]={0};
   for(int layer=0;layer<3;layer++)for(int j=0;j<rr_spin_indicator_sectors(style,layer);j++){
     RRSpinPoint t[3];rr_spin_indicator_triangle(style,layer,j,t);
     if(area(t[0],t[1],point)<=1e-9&&area(t[1],t[2],point)<=1e-9&&area(t[2],t[0],point)<=1e-9){paint(got,rr_spin_indicator_color(style,layer));break;}
   }
   bool progress=turn<c->fill;double alpha=progress?1:.25,base[3]={progress?25:255,progress?230:43,progress?255:214},want[4]={base[0]/255*alpha,base[1]/255*alpha,base[2]/255*alpha,alpha};
   if(radii[r]<31){const double top[3]={10,4,32};for(int k=0;k<3;k++)want[k]=top[k]/255*c->opacity+want[k]*(1-c->opacity);want[3]=c->opacity+want[3]*(1-c->opacity);}
   for(int k=0;k<4;k++)if(fabs(got[k]-want[k])>2.0/255)failures++;
 }
 /* CSS child opacity is applied after background+glyph grouping. Verify the
  * positive additive delta for ideal glyph masks over opaque viewport colors.
  * Blur/shadow profile is explicitly outside this exact group-math check. */
 const double covers[]={0,.25,.75,1},backdrops[][3]={{0,0,0},{.2,.4,.6},{1,1,1}};
 uint32_t delta=rr_spin_indicator_text_delta(style,false);
 for(int d=0;d<3;d++)for(int sector=0;sector<2;sector++)for(int g=0;g<4;g++){
   double color[3]={sector?255:25,sector?43:230,sector?214:255},base_alpha=sector?.25:1,background[3]={10,4,32},label[3]={196,251,255};
   for(int k=0;k<3;k++){
     double under=color[k]/255*base_alpha+backdrops[d][k]*(1-base_alpha);
     double opacity=(rr_spin_indicator_color(style,2)&255)/255.0;
     double existing=background[k]/255*opacity+under*(1-opacity);
     double got=existing+((delta>>(24-8*k))&255)/255.0*(delta&255)/255.0*covers[g];
     double want=c->opacity*(label[k]/255*covers[g]+background[k]/255*(1-covers[g]))+(1-c->opacity)*under;
     if(fabs(got-want)>1.0/255)failures++;
   }
 }
 if(failures>before)fprintf(stderr,"Spin row%zu cooldown%.17g fill%.17g added%d failures\n",i,c->cooldown,c->fill,failures-before);
}
RRSpinIndicator ready=rr_spin_indicator(0);RRSpinPoint invalid[3];if(rr_spin_indicator_triangle(ready,1,64,invalid)||rr_spin_indicator_triangle(ready,3,0,invalid)||rr_spin_indicator_sectors(rr_spin_indicator(6),1)!=0)failures++;
/* Existing gameplay cooldown drives style, with no extra timer or RNG call. */
double cooldown=6;for(int frame=0;frame<130;frame++){rr_spin_control_step(&cooldown,true,.05,false,1,NULL,NULL);RRSpinIndicator s=rr_spin_indicator(cooldown);if(s.cool!=(cooldown>0)||s.fill<0||s.fill>1)failures++;}
printf("Spin indicator: %zu actual HUD states, five reused Chrome styles, clockwise disk geometry/compositing bounds; max%d triangles, worst fill%.17g, %d failures\n",sizeof(spin_cases)/sizeof(spin_cases[0]),max_triangles,worst,failures);return failures?1:0;}
