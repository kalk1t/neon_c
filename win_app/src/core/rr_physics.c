#include "rr_physics.h"
RRShape rr_box(RRVec3 c,RRVec3 h,double yaw)
{
    double extent=hypot(h.x,h.z);RRShape s={0};s.kind=RR_SHAPE_BOX;s.center=c;s.half=h;s.cosine=cos(yaw);s.sine=sin(yaw);
    s.bounds[0]=c.x-extent;s.bounds[1]=c.z-extent;s.bounds[2]=c.x+extent;s.bounds[3]=c.z+extent;return s;
}
RRShape rr_cylinder(double x,double z,double radius,double y0,double y1)
{
    RRShape s={0};s.kind=RR_SHAPE_CYLINDER;s.center=rr_v3(x,0,z);s.radius=radius;s.y0=y0;s.y1=y1;
    s.bounds[0]=x-radius;s.bounds[1]=z-radius;s.bounds[2]=x+radius;s.bounds[3]=z+radius;return s;
}
RRShape rr_torus(RRVec3 c,double yaw,double radius,double tube)
{
    RRShape s={0};s.kind=RR_SHAPE_TORUS;s.center=c;s.radius=radius;s.tube=tube;s.cosine=cos(yaw);s.sine=sin(yaw);
    double e=radius+tube;s.bounds[0]=c.x-e;s.bounds[1]=c.z-e;s.bounds[2]=c.x+e;s.bounds[3]=c.z+e;return s;
}
bool rr_world_add(RRWorld *w,RRShape shape){if(w->count>=RR_PHYSICS_MAX_SHAPES)return false;w->shapes[w->count++]=shape;return true;}
static double sign_or_one(double v){return v<0?-1:1;}
RRContact rr_shape_resolve(const RRShape *s,RRVec3 p,double radius)
{
    RRContact out={0};double dx=p.x-s->center.x,dz=p.z-s->center.z;
    if(s->kind==RR_SHAPE_CYLINDER){
        double dr=hypot(dx,dz),cy=rr_clamp(p.y,s->y0,s->y1);
        if(dr>s->radius){
            RRVec3 e=rr_v3(dx/dr*(dr-s->radius),p.y-cy,dz/dr*(dr-s->radius));double d=rr_length(e);
            if(d>=radius)return out;out.normal=rr_scale(e,1/d);out.depth=radius-d;
        }else{
            if(p.y<s->y0-radius || p.y>s->y1+radius)return out;
            double side=s->radius-dr,top=s->y1-p.y,bot=p.y-s->y0;
            if(top<side && top<bot){out.normal=rr_v3(0,1,0);out.depth=top+radius;}
            else if(bot<side){out.normal=rr_v3(0,-1,0);out.depth=bot+radius;}
            else{out.normal=rr_v3(dx/(dr?dr:1),0,dz/(dr?dr:1));out.depth=side+radius;}
        }
    }else{
        double lx=s->cosine*dx-s->sine*dz,lz=s->sine*dx+s->cosine*dz,ly=p.y-s->center.y;
        RRVec3 n;double d;
        if(s->kind==RR_SHAPE_TORUS){
            double radial=hypot(lx,ly);
            /* At the exact hoop center JS reports a false 0/0 contact.
             * Leave that one hole point clear, retaining finite JS contacts. */
            if(radial==0 && lz==0)return out;
            if(radial==0)radial=1e-6;
            n=rr_v3(lx-lx/radial*s->radius,ly-ly/radial*s->radius,lz);d=rr_length(n);
            if(d>=s->tube+radius)return out;
            /* Exact tube center has no defined normal in JS (0/0). Keep the
             * native simulation finite with an axial push only at that point. */
            n=d>0?rr_scale(n,1/d):rr_v3(0,0,1);out.depth=s->tube+radius-d;
        }else{
            n=rr_v3(lx-rr_clamp(lx,-s->half.x,s->half.x),ly-rr_clamp(ly,-s->half.y,s->half.y),lz-rr_clamp(lz,-s->half.z,s->half.z));d=rr_length(n);
            if(d>1e-6){if(d>=radius)return out;n=rr_scale(n,1/d);out.depth=radius-d;}
            else{
                double px=s->half.x-fabs(lx),py=s->half.y-fabs(ly),pz=s->half.z-fabs(lz);n=rr_v3(0,0,0);
                if(px<py && px<pz){n.x=sign_or_one(lx);out.depth=px+radius;}
                else if(py<pz){n.y=sign_or_one(ly);out.depth=py+radius;}
                else{n.z=sign_or_one(lz);out.depth=pz+radius;}
            }
        }
        out.normal=rr_v3(s->cosine*n.x+s->sine*n.z,n.y,-s->sine*n.x+s->cosine*n.z);
    }
    out.hit=true;return out;
}
RRContact rr_world_collide(const RRWorld *w,RRVec3 *p,double radius)
{
    RRContact strongest={0};if(!w)return strongest;
    double cell_x=floor(p->x/120),cell_z=floor(p->z/120);
    for(int i=0;i<w->count;i++){
        const RRShape *s=&w->shapes[i];
        if(s->off || cell_x<floor(s->bounds[0]/120) || cell_x>floor(s->bounds[2]/120) || cell_z<floor(s->bounds[1]/120) || cell_z>floor(s->bounds[3]/120))continue;
        RRContact contact=rr_shape_resolve(s,*p,radius);
        if(contact.hit){*p=rr_add(*p,rr_scale(contact.normal,contact.depth));if(!strongest.hit||contact.depth>strongest.depth)strongest=contact;}
    }
    return strongest;
}
bool rr_world_segment(const RRWorld *w,RRVec3 a,RRVec3 b,double radius,RRVec3 *hit)
{
    int steps=(int)fmax(1,ceil(rr_length(rr_sub(a,b))/3));
    for(int i=1;i<=steps;i++){RRVec3 p=rr_mix(a,b,(double)i/steps),q=p;if(rr_world_collide(w,&q,radius).hit){*hit=p;return true;}}
    return false;
}
