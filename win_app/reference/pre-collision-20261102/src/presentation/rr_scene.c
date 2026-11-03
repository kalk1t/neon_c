#include "rr_scene.h"
#include "rr_shaders.h"
#include "raymath.h"
#include "rlgl.h"
#include "roadster_meshes.h"
#include "rr_render_policy.h"
#include <string.h>
#include <stdint.h>

static const Color cyan={65,225,255,255},magenta={255,60,180,255};
static Vector3 as_vector(RRVec3 v){Vector3 p={(float)v.x,(float)v.y,(float)v.z};return p;}

static RRVec3 core_vector(Vector3 v){return rr_v3(v.x,v.y,v.z);}
typedef struct RRView {
    RRVec3 eye,forward,right,up;
    double tangent,aspect;
} RRView;
static RRView view_from_camera(Camera3D camera)
{
    Vector3 forward=Vector3Normalize(Vector3Subtract(camera.target,camera.position));
    Vector3 right=Vector3Normalize(Vector3CrossProduct(forward,camera.up));
    Vector3 up=Vector3CrossProduct(right,forward);
    RRView view={core_vector(camera.position),core_vector(forward),core_vector(right),core_vector(up),
        tan(camera.fovy*DEG2RAD*.5),(double)GetScreenWidth()/GetScreenHeight()};
    return view;
}
static bool visible_sphere(const RRView *view,Vector3 center,float radius,float far_plane)
{
    return rr_render_sphere_visible(core_vector(center),radius,view->eye,view->forward,view->right,view->up,
        view->tangent,view->aspect,far_plane);
}

/* Expand a cube into a static, vertex-coloured building mesh once on first sight. */
static void append_box(Mesh *mesh,const Mesh *cube,Vector3 position,Vector3 size,Color color)
{
    for(int i=0;i<cube->triangleCount*3;i++){
        int source=cube->indices?cube->indices[i]:i,destination=mesh->vertexCount++;
        mesh->vertices[destination*3]=position.x+cube->vertices[source*3]*size.x;
        mesh->vertices[destination*3+1]=position.y+cube->vertices[source*3+1]*size.y;
        mesh->vertices[destination*3+2]=position.z+cube->vertices[source*3+2]*size.z;
        memcpy(mesh->normals+destination*3,cube->normals+source*3,3*sizeof(float));
        memcpy(mesh->texcoords+destination*2,cube->texcoords+source*2,2*sizeof(float));
        mesh->colors[destination*4]=color.r;mesh->colors[destination*4+1]=color.g;
        mesh->colors[destination*4+2]=color.b;mesh->colors[destination*4+3]=255;
    }
    mesh->triangleCount=mesh->vertexCount/3;
}

static bool cache_building(RRScene *s,RRBuilding *b)
{
    Mesh mesh={0};
    unsigned int vertices=(unsigned int)(s->cube_template.triangleCount*3*8);
    mesh.vertices=MemAlloc(vertices*12u);mesh.normals=MemAlloc(vertices*12u);
    mesh.texcoords=MemAlloc(vertices*8u);mesh.colors=MemAlloc(vertices*4u);
    if(!mesh.vertices || !mesh.normals || !mesh.texcoords || !mesh.colors){
        MemFree(mesh.vertices);MemFree(mesh.normals);MemFree(mesh.texcoords);MemFree(mesh.colors);
        TraceLog(LOG_ERROR,"CITY: cache allocation failed");return false;
    }
    Color color=b->pattern==0?cyan:b->pattern==1?magenta:(Color){112,95,194,255};
    append_box(&mesh,&s->cube_template,b->pos,b->size,b->color);
    float roof=b->size.y;
    append_box(&mesh,&s->cube_template,(Vector3){b->pos.x,roof+1,b->pos.z},(Vector3){b->size.x+1,1.4f,b->size.z+1},color);
    for(int band=1;band<=3;band++){
        float y=roof*(float)band/4;
        append_box(&mesh,&s->cube_template,(Vector3){b->pos.x,y,b->pos.z-b->size.z*.5f-.2f},(Vector3){b->size.x*.72f,.85f,.3f},color);
        append_box(&mesh,&s->cube_template,(Vector3){b->pos.x+b->size.x*.5f+.2f,y,b->pos.z},(Vector3){.3f,.85f,b->size.z*.72f},color);
    }
    UploadMesh(&mesh,false);b->mesh=mesh;
    return mesh.vaoId!=0;
}

bool rr_scene_load(RRScene *s,const RRCorridor *corridor,bool fallback,bool glow)
{
    memset(s,0,sizeof(*s));
    s->world_shader=LoadShaderFromMemory(rr_world_vertex,rr_world_fragment);
    s->glow=glow;
    if(glow)s->post_shader=LoadShaderFromMemory(NULL,rr_post_fragment);
    if(!IsShaderValid(s->world_shader) || s->world_shader.id==rlGetShaderIdDefault())return false;
    if(glow && (!IsShaderValid(s->post_shader) || s->post_shader.id==rlGetShaderIdDefault()))return false;
    s->view_position_location=GetShaderLocation(s->world_shader,"viewPos");
    if(glow)s->post_resolution_location=GetShaderLocation(s->post_shader,"resolution");
    RRRenderSize size=rr_render_size(GetScreenWidth(),GetScreenHeight());
    if(!size.width || !size.height)return false;
    s->target=LoadRenderTexture(size.width,size.height);
    if(!IsRenderTextureValid(s->target))return false;
    SetTextureFilter(s->target.texture,TEXTURE_FILTER_BILINEAR);
    s->cube_template=GenMeshCube(1,1,1);
    if(!s->cube_template.vertices || !s->cube_template.normals || !s->cube_template.texcoords)return false;
    s->city_material=LoadMaterialDefault();
    if(!s->city_material.maps)return false;
    s->city_material.shader=s->world_shader;
    if(!fallback && FileExists("assets/models/roadster.glb")){
        s->car=LoadModel("assets/models/roadster.glb");
        s->car_loaded=IsModelValid(s->car) && s->car.meshCount==(int)(sizeof(rr_roadster_hi_meshes)/sizeof(rr_roadster_hi_meshes[0]));
        if(s->car_loaded){
            for(int i=0;i<s->car.materialCount;i++)s->car.materials[i].shader=s->world_shader;
            BoundingBox bounds=GetModelBoundingBox(s->car);
            TraceLog(LOG_INFO,"ROADSTER: %i meshes, bounds %.2f x %.2f x %.2f",s->car.meshCount,bounds.max.x-bounds.min.x,bounds.max.y-bounds.min.y,bounds.max.z-bounds.min.z);
        }
    }
    RRCityPlan plan;rr_city_plan(&plan,corridor);
    for(int i=0;i<plan.count;i++){
        const RRPlannedBuilding *source=&plan.buildings[i];
        RRBuilding *b=&s->buildings[s->building_count++];
        b->pos=(Vector3){source->x,source->top*.5f,source->z};
        b->size=(Vector3){source->width,source->top,source->depth};
        b->pattern=source->pattern;b->color=(Color){source->red,source->green,source->blue,255};
    }
    TraceLog(LOG_INFO,"CITY: shared all-route corridor points=%d coarse=%d gates=%d; buildings=%d lowered=%d rejected=%d",corridor->point_count,corridor->coarse_count,corridor->gate_count,plan.count,plan.lowered,plan.rejected);
    return true;
}

void rr_scene_unload(RRScene *s)
{
    for(int i=0;i<s->building_count;i++)if(s->buildings[i].mesh.vertexCount)UnloadMesh(s->buildings[i].mesh);
    if(s->cube_template.vertexCount)UnloadMesh(s->cube_template);
    if(s->city_material.maps)MemFree(s->city_material.maps);
    // raylib UnloadModel leaves textures/shaders to the caller. Deduplicate maps.
    if(s->car.meshCount>0){
        Texture2D textures[256];int count=0;
        for(int m=0;m<s->car.materialCount;m++)for(int i=0;i<=MATERIAL_MAP_BRDF;i++){
            Texture2D t=s->car.materials[m].maps[i].texture;
            if(t.id==0 || t.id==rlGetTextureIdDefault())continue;
            bool found=false;for(int j=0;j<count;j++)if(textures[j].id==t.id)found=true;
            if(!found && count<256)textures[count++]=t;
        }
        UnloadModel(s->car);
        for(int i=0;i<count;i++)UnloadTexture(textures[i]);
    }
    if(s->logo.id)UnloadTexture(s->logo);
    if(s->target.id)UnloadRenderTexture(s->target);
    if(s->world_shader.id && s->world_shader.id!=rlGetShaderIdDefault())UnloadShader(s->world_shader);
    if(s->post_shader.id && s->post_shader.id!=rlGetShaderIdDefault())UnloadShader(s->post_shader);
    memset(s,0,sizeof(*s));
}

bool rr_scene_begin(RRScene *s,Camera3D camera)
{
    RRRenderSize size=rr_render_size(GetScreenWidth(),GetScreenHeight());
    int w=size.width,h=size.height;
    if(!w || !h)return false;
    if(w!=s->target.texture.width || h!=s->target.texture.height){
        TraceLog(LOG_INFO,"RENDER: resizing bounded target to %dx%d",w,h);
        UnloadRenderTexture(s->target);s->target=(RenderTexture2D){0};
        s->target=LoadRenderTexture(w,h);
        if(!IsRenderTextureValid(s->target)){TraceLog(LOG_ERROR,"RENDER: target allocation failed");return false;}
        SetTextureFilter(s->target.texture,TEXTURE_FILTER_BILINEAR);
    }
    if(!IsRenderTextureValid(s->target))return false;
    SetShaderValue(s->world_shader,s->view_position_location,&camera.position,SHADER_UNIFORM_VEC3);
    BeginTextureMode(s->target);
    ClearBackground((Color){6,8,24,255});
    DrawRectangleGradientV(0,0,w,h,(Color){6,8,24,255},(Color){55,34,88,255});
    // Distant moon stays fixed in world space while the camera turns.
    Vector3 moon=Vector3Add(camera.position,(Vector3){-2600,1700,-5500});
    Vector2 at=GetWorldToScreenEx(moon,camera,w,h);
    Vector3 direction=Vector3Normalize(Vector3Subtract(camera.target,camera.position));
    if(Vector3DotProduct(direction,Vector3Subtract(moon,camera.position))>0){
        DrawCircleGradient(at,72,Fade((Color){135,134,255,255},.2f),BLANK);
        DrawCircleV(at,32,(Color){170,174,235,255});
        DrawCircleV((Vector2){at.x-9,at.y-6},29,(Color){119,123,194,255});
    }
    BeginMode3D(camera);
    return true;
}

static void draw_gate(const RRGate *g,bool active)
{
    Color trim=active?(Color){130,255,180,255}:cyan;
    Vector3 center=as_vector(g->pos);
    double cx=cos(g->yaw),sz=-sin(g->yaw);
    for(int i=0;i<16;i++){
        double a=i*2*RR_PI/16,b=(i+1)*2*RR_PI/16;
        Vector3 p={(float)(g->pos.x+cx*cos(a)*15.5),(float)(g->pos.y+sin(a)*15.5),(float)(g->pos.z+sz*cos(a)*15.5)};
        Vector3 q={(float)(g->pos.x+cx*cos(b)*15.5),(float)(g->pos.y+sin(b)*15.5),(float)(g->pos.z+sz*cos(b)*15.5)};
        DrawCylinderEx(p,q,2.3f,2.3f,6,(Color){22,27,46,255});
        p.x+=(float)(-sin(g->yaw)*2.15);p.z+=(float)(-cos(g->yaw)*2.15);
        q.x+=(float)(-sin(g->yaw)*2.15);q.z+=(float)(-cos(g->yaw)*2.15);
        DrawCylinderEx(p,q,.38f,.38f,5,trim);
    }
    Vector3 base={center.x,center.y-22,center.z};
    DrawCylinder(base,13.7f,13.7f,6,24,(Color){20,24,43,255});
    DrawCylinder((Vector3){base.x,base.y-140,base.z},3.5f,5,140,12,(Color){21,25,42,255});
}

void rr_scene_world(RRScene *s,const RRTrack *track,Camera3D camera,int next_gate,double time)
{
    RRView view=view_from_camera(camera);
    int visible[RR_BUILDING_BUDGET],count=0;
    float distances[RR_BUILDING_BUDGET];
    /* Retain the nearest buildings intersecting the camera frustum. */
    for(int i=0;i<s->building_count;i++){
        RRBuilding *b=&s->buildings[i];
        Vector3 offset=Vector3Subtract(b->pos,camera.position);
        float distance=Vector3LengthSqr(offset),radius=Vector3Length(b->size)*.5f+32;
        if(distance>1000*1000 || !visible_sphere(&view,b->pos,radius,1000))continue;
        int slot=count;
        if(slot==RR_BUILDING_BUDGET){if(distance>=distances[slot-1])continue;slot--;}
        else count++;
        while(slot>0 && distance<distances[slot-1]){visible[slot]=visible[slot-1];distances[slot]=distances[slot-1];slot--;}
        visible[slot]=i;distances[slot]=distance;
    }
    /* Bound uploads to two newly visible buildings per frame, nearest first. */
    int uploads=0;
    for(int candidate=0;candidate<count;candidate++){
        int i=visible[candidate];
        RRBuilding *b=&s->buildings[i];
        if(!b->mesh.vertexCount && uploads<2){
            uploads++;
            if(!cache_building(s,b))continue;
        }
        if(b->mesh.vaoId)DrawMesh(b->mesh,s->city_material,MatrixIdentity());
        else{
            /* Keep a cheap silhouette while detailed geometry waits for its upload budget. */
            s->city_material.maps[MATERIAL_MAP_DIFFUSE].color=b->color;
            Matrix transform=MatrixMultiply(MatrixScale(b->size.x,b->size.y,b->size.z),MatrixTranslate(b->pos.x,b->pos.y,b->pos.z));
            DrawMesh(s->cube_template,s->city_material,transform);
            s->city_material.maps[MATERIAL_MAP_DIFFUSE].color=WHITE;
        }
    }
    BeginShaderMode(s->world_shader);
    DrawPlane((Vector3){0,20,0},(Vector2){12000,12000},(Color){36,24,61,255});
    DrawCube((Vector3){0,108,40},62,8,150,(Color){22,25,43,255});
    DrawCube((Vector3){-30,112,40},.8f,1,150,cyan);
    DrawCube((Vector3){30,112,40},.8f,1,150,cyan);
    DrawCube((Vector3){-23,122,0},3,32,4,(Color){36,40,60,255});
    DrawCube((Vector3){23,122,0},3,32,4,(Color){36,40,60,255});
    DrawCube((Vector3){0,139,0},49,4,5,(Color){35,37,58,255});
    DrawCube((Vector3){0,136.8f,-2.6f},44,.6f,.5f,magenta);
    for(int i=0;i<track->gate_count;i++){
        if(visible_sphere(&view,as_vector(track->gates[i].pos),170,900))draw_gate(&track->gates[i],i==next_gate);
    }
    for(int i=0;i<track->sample_count;i+=8){
        Vector3 p=as_vector(track->samples[i]);p.y-=2.5f;
        if(visible_sphere(&view,p,.5f,650))DrawSphereEx(p,.34f,3,4,(Color){85,255,182,255});
    }
    // Distant flying traffic: decoration only, deliberately clear of the course.
    for(int i=0;i<4;i++){
        float x=(float)(sin(time*.035+i*2.2)*1900),z=(float)(cos(time*.026+i*1.8)*1900);
        float y=310.0f+i*9;
        DrawCube((Vector3){x,y,z},3,.7f,7,(Color){40,42,65,255});
        DrawCube((Vector3){x,y,z+4},2,.4f,2,cyan);
    }
    EndShaderMode();
}

void rr_scene_car(RRScene *s,const RRCar *car)
{
    double k=1-car->roll_time/RR_SPIN_ROLL;
    double spin=car->roll_time>0?-car->roll_direction*RR_PI*2*k*k*(3-2*k):0;
    Matrix rotation=MatrixMultiply(MatrixMultiply(MatrixRotateZ((float)(car->roll+spin)),MatrixRotateX((float)car->pitch)),MatrixRotateY((float)car->yaw));
    Matrix transform=MatrixMultiply(MatrixMultiply(MatrixTranslate(0,-.35f,0),rotation),MatrixTranslate((float)car->pos.x,(float)car->pos.y,(float)car->pos.z));
    if(s->car_loaded){
        for(int i=0;i<s->car.meshCount;i++)if(rr_roadster_hi_meshes[i])DrawMesh(s->car.meshes[i],s->car.materials[s->car.meshMaterial[i]],transform);
    }else{
        BeginShaderMode(s->world_shader);
        rlPushMatrix();rlTranslatef((float)car->pos.x,(float)car->pos.y,(float)car->pos.z);
        rlRotatef((float)(car->yaw*RAD2DEG),0,1,0);rlRotatef((float)(car->pitch*RAD2DEG),1,0,0);rlRotatef((float)((car->roll+spin)*RAD2DEG),0,0,1);
        DrawCube((Vector3){0,-.15f,0},2.2f,.7f,5.6f,(Color){40,44,59,255});
        DrawCube((Vector3){0,.45f,.3f},1.65f,.8f,2.2f,(Color){38,101,134,255});
        for(int x=-1;x<=1;x+=2)for(int z=-1;z<=1;z+=2){
            DrawCube((Vector3){x*1.85f,-.15f,z*1.6f},.85f,1,1.6f,(Color){57,61,76,255});
            DrawCube((Vector3){x*1.85f,.4f,z*1.6f},.7f,.08f,1.4f,cyan);
        }
        rlPopMatrix();EndShaderMode();
    }
    BeginShaderMode(s->world_shader);
    Vector3 origin=as_vector(car->pos),right={(float)cos(car->yaw),0,(float)-sin(car->yaw)};
    for(int side=-1;side<=1;side+=2){
        Vector3 p=Vector3Add(origin,Vector3Scale(right,side*1.55f));
        p=Vector3Subtract(p,Vector3Scale(as_vector(car->fwd),2.8f));p.y-=.45f;
        Vector3 end=Vector3Subtract(p,Vector3Scale(as_vector(car->fwd),(float)(.5+car->thrust*2+car->nitro*5)));
        DrawCylinderEx(p,end,.22f,.02f,8,car->boosting?magenta:cyan);
    }
    EndShaderMode();
}

void rr_scene_finish(RRScene *s)
{
    EndMode3D();EndTextureMode();
    BeginDrawing();ClearBackground(BLACK);
    Vector2 size={(float)GetScreenWidth(),(float)GetScreenHeight()};
    Vector2 render_size={(float)s->target.texture.width,(float)s->target.texture.height};
    if(s->glow){
        SetShaderValue(s->post_shader,s->post_resolution_location,&render_size,SHADER_UNIFORM_VEC2);
        BeginShaderMode(s->post_shader);
    }
    DrawTexturePro(s->target.texture,(Rectangle){0,0,(float)s->target.texture.width,(float)-s->target.texture.height},(Rectangle){0,0,size.x,size.y},(Vector2){0,0},0,WHITE);
    if(s->glow)EndShaderMode();
    // Caller draws UI before EndDrawing.
}
