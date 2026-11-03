#include "rr_scene.h"
#include "rr_shaders.h"
#include "raymath.h"
#include "rlgl.h"
#include "roadster_meshes.h"
#include "rr_render_policy.h"
#include "rr_atmosphere.h"
#include "rr_shared_material.h"
#include "cyan_tree_asset.h"
#include "control_tower_asset.h"
#include "rr_control_tower_data.h"
#include "rr_mesh_asset.h"
#include "rr_canonical_city.h"
#include "rr_city_crown_data.h"
#include "city_crown_asset.h"
#include "rr_facade.h"
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

static bool load_course_mesh(Mesh *mesh,const char *path);
static bool cache_building(RRScene *s,RRBuilding *b)
{
    if(b->canonical_index>=0){
        Mesh mesh={0};
        if(!load_course_mesh(&mesh,TextFormat("assets/city-core/b%04d.rrmesh",b->canonical_index))){b->mesh_failed=true;TraceLog(LOG_ERROR,"CITY_CORE: missing/invalid mesh=%d",b->canonical_index);return false;}
        UploadMesh(&mesh,false);b->mesh=mesh;return mesh.vaoId!=0;
    }
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

/* CPU decode only; VRAM uploads share the existing two-per-frame budget. */
static bool load_course_mesh(Mesh *mesh,const char *path)
{
    int bytes=0;unsigned char *data=LoadFileData(path,&bytes);unsigned int count=0;
    if(!data)return false;
    if(bytes<0 || !rr_mesh_asset_validate(data,(size_t)bytes,&count)){UnloadFileData(data);return false;}
    *mesh=(Mesh){0};mesh->vertexCount=(int)count;mesh->triangleCount=(int)count/3;
    mesh->vertices=MemAlloc(count*12u);mesh->normals=MemAlloc(count*12u);mesh->texcoords=MemAlloc(count*8u);mesh->colors=MemAlloc(count*4u);
    if(!mesh->vertices||!mesh->normals||!mesh->texcoords||!mesh->colors){UnloadFileData(data);UnloadMesh(*mesh);*mesh=(Mesh){0};return false;}
    for(unsigned int i=0;i<count;i++){
        const unsigned char *row=data+8+i*36u;
        memcpy(mesh->vertices+i*3,row,12);memcpy(mesh->normals+i*3,row+12,12);memcpy(mesh->texcoords+i*2,row+24,8);memcpy(mesh->colors+i*4,row+32,4);
    }
    UnloadFileData(data);return true;
}
static Matrix course_transform(const RRCourseItem *item)
{
    Matrix m={0};m.m0=(float)item->right.x;m.m1=(float)item->right.y;m.m2=(float)item->right.z;
    m.m4=(float)item->up.x;m.m5=(float)item->up.y;m.m6=(float)item->up.z;
    m.m8=(float)item->back.x;m.m9=(float)item->back.y;m.m10=(float)item->back.z;
    m.m12=(float)item->pos.x;m.m13=(float)item->pos.y;m.m14=(float)item->pos.z;m.m15=1;return m;
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
    s->view_forward_location=GetShaderLocation(s->world_shader,"viewForward");
    s->sculpt_glow_location=GetShaderLocation(s->world_shader,"sculptGlow");
    const float fog_color[]={(float)rr_fog_color_srgb[0],(float)rr_fog_color_srgb[1],(float)rr_fog_color_srgb[2]};
    float fog_density=(float)RR_FOG_DENSITY;
    SetShaderValue(s->world_shader,GetShaderLocation(s->world_shader,"fogColor"),fog_color,SHADER_UNIFORM_VEC3);
    SetShaderValue(s->world_shader,GetShaderLocation(s->world_shader,"fogDensity"),&fog_density,SHADER_UNIFORM_FLOAT);
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
    s->facade_location=GetShaderLocation(s->world_shader,"cityFacade");
    unsigned char *window_pixels=MemAlloc(RR_WINDOW_BYTES);
    if(!window_pixels || !rr_window_pixels(window_pixels,RR_WINDOW_BYTES)){MemFree(window_pixels);return false;}
    Image window_image={window_pixels,RR_WINDOW_WIDTH,RR_WINDOW_HEIGHT,1,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    s->window_texture=LoadTextureFromImage(window_image);UnloadImage(window_image);
    if(!IsTextureValid(s->window_texture))return false;
    GenTextureMipmaps(&s->window_texture);SetTextureFilter(s->window_texture,TEXTURE_FILTER_TRILINEAR);SetTextureWrap(s->window_texture,TEXTURE_WRAP_REPEAT);
    s->facade_material=LoadMaterialDefault();if(!s->facade_material.maps)return false;
    s->facade_material.shader=s->world_shader;s->facade_material.maps[MATERIAL_MAP_DIFFUSE].texture=s->window_texture;
    TraceLog(LOG_INFO,"FACADE: source seed99 windows=%dx%d mipmaps=%d repeat/trilinear intensity=.95; canonical body only, UV flipY; PBR/anisotropy deferred",RR_WINDOW_WIDTH,RR_WINDOW_HEIGHT,s->window_texture.mipmaps);

    const char *course_paths[]={"beacon","barrier","sign-pos","sign-neg","chevron","ring-station","platform","platform-column","tree-magenta","tree-cyan","cyan-sculpt"};
    for(int i=0;i<RR_FALLBACK_MESH_COUNT;i++)if(!load_course_mesh(&s->course_meshes[i],TextFormat("assets/course/%s.rrmesh",course_paths[i]))){TraceLog(LOG_ERROR,"COURSE: invalid/missing mesh %s",course_paths[i]);return false;}
    TraceLog(LOG_INFO,"COURSE: ten existing browser fallback prefab meshes decoded; deferred uploads");
    if(load_course_mesh(&s->course_meshes[10],"assets/course/cyan-sculpt.rrmesh") && s->course_meshes[10].vertexCount==RR_CYAN_TREE_MESH_VERTICES){
        s->tree_texture=LoadTexture("assets/models/cyan-tree-base.png");
        if(IsTextureValid(s->tree_texture)){
            s->tree_material=LoadMaterialDefault();
            if(s->tree_material.maps){s->tree_material.shader=s->world_shader;s->tree_material.maps[MATERIAL_MAP_DIFFUSE].texture=s->tree_texture;s->tree_loaded=true;}
        }
    }
    TraceLog(LOG_INFO,"SCENERY: existing cyan tree loaded=%d vertices=%d fit=%.9f texture=%dx%d; fallback retained",s->tree_loaded,s->course_meshes[10].vertexCount,(double)RR_CYAN_TREE_FIT,s->tree_texture.width,s->tree_texture.height);
    if(rr_control_tower_placement(rr_control_fallback_bounds,rr_control_model_size,&s->control_placement) &&
       load_course_mesh(&s->course_meshes[11],"assets/course/control-sculpt.rrmesh") && s->course_meshes[11].vertexCount==RR_CONTROL_TOWER_MESH_VERTICES){
        s->control_texture=LoadTexture("assets/models/control-tower-base.png");
        if(IsTextureValid(s->control_texture)){
            GenTextureMipmaps(&s->control_texture);SetTextureFilter(s->control_texture,TEXTURE_FILTER_TRILINEAR);SetTextureWrap(s->control_texture,TEXTURE_WRAP_REPEAT);
            TraceLog(LOG_INFO,"HERO: source sampler linear/trilinear/repeat, mipmaps=%d",s->control_texture.mipmaps);
            s->control_material=LoadMaterialDefault();
            if(s->control_material.maps){s->control_material.shader=s->world_shader;s->control_material.maps[MATERIAL_MAP_DIFFUSE].texture=s->control_texture;s->control_loaded=true;}
        }
    }
    if(s->control_loaded)s->control_world_bounds=rr_hero_world_bounds(rr_control_model_bounds,s->control_placement);
    TraceLog(LOG_INFO,"HERO: control loaded=%d vertices=%d texture=%dx%d pos=%.6f,%.6f,%.6f scale=%.9f yaw=%.9f glow=%.1f",s->control_loaded,s->course_meshes[11].vertexCount,s->control_texture.width,s->control_texture.height,s->control_placement.pos.x,s->control_placement.pos.y,s->control_placement.pos.z,s->control_placement.scale,s->control_placement.yaw,(double)RR_CONTROL_TOWER_GLOW);
    if(load_course_mesh(&s->course_meshes[12],"assets/course/city-crown.rrmesh") && s->course_meshes[12].vertexCount==RR_CITY_CROWN_MESH_VERTICES){
        s->crown_texture=LoadTexture("assets/models/city-crown-base.png");
        if(IsTextureValid(s->crown_texture)){
            GenTextureMipmaps(&s->crown_texture);SetTextureFilter(s->crown_texture,TEXTURE_FILTER_TRILINEAR);SetTextureWrap(s->crown_texture,TEXTURE_WRAP_REPEAT);
            s->crown_material=LoadMaterialDefault();if(s->crown_material.maps){s->crown_material.shader=s->world_shader;s->crown_material.maps[MATERIAL_MAP_DIFFUSE].texture=s->crown_texture;s->crown_loaded=true;}
        }
    }
    TraceLog(LOG_INFO,"CITY_CROWN: loaded=%d vertices=%d texture=%dx%d mipmaps=%d anchors=%d draw-cap=4 glow=%.1f",s->crown_loaded,s->course_meshes[12].vertexCount,s->crown_texture.width,s->crown_texture.height,s->crown_texture.mipmaps,rr_canonical_hero_count(),(double)RR_CITY_CROWN_GLOW);
    if(!fallback && FileExists("assets/models/roadster.glb")){
        s->car=LoadModel("assets/models/roadster.glb");
        s->car_loaded=IsModelValid(s->car) && s->car.meshCount==(int)(sizeof(rr_roadster_hi_meshes)/sizeof(rr_roadster_hi_meshes[0]));
        if(s->car_loaded){
            for(int i=0;i<s->car.materialCount;i++)s->car.materials[i].shader=s->world_shader;
            BoundingBox bounds=GetModelBoundingBox(s->car);
            TraceLog(LOG_INFO,"ROADSTER: %i meshes, bounds %.2f x %.2f x %.2f",s->car.meshCount,bounds.max.x-bounds.min.x,bounds.max.y-bounds.min.y,bounds.max.z-bounds.min.z);
        }
    }
    RRCityPlan plan;if(!rr_canonical_city_plan(&plan,corridor))return false;
    for(int i=0;i<plan.count;i++){
        const RRPlannedBuilding *source=&plan.buildings[i];
        RRBuilding *b=&s->buildings[s->building_count++];
        b->pos=(Vector3){source->x,source->top*.5f,source->z};
        b->size=(Vector3){source->width,source->top,source->depth};
        b->canonical_index=source->canonical_index;
        if(b->canonical_index>=0){const RRCanonicalBuilding *canonical=rr_canonical_city_building(b->canonical_index);b->pos=as_vector(rr_scale(rr_add(canonical->mesh_bounds.minimum,canonical->mesh_bounds.maximum),.5));b->size=as_vector(rr_sub(canonical->mesh_bounds.maximum,canonical->mesh_bounds.minimum));}
        b->pattern=source->pattern;b->color=(Color){source->red,source->green,source->blue,255};
    }
    TraceLog(LOG_INFO,"CITY_CORE: prototype kept=%d removed=%d canonical=%d",plan.prototype_kept,plan.prototype_removed,plan.canonical_count);
    TraceLog(LOG_INFO,"CITY: shared all-route corridor points=%d coarse=%d gates=%d; buildings=%d lowered=%d rejected=%d",corridor->point_count,corridor->coarse_count,corridor->gate_count,plan.count,plan.lowered,plan.rejected);
    return true;
}

void rr_scene_unload(RRScene *s)
{
    for(int i=0;i<RR_COURSE_MESH_COUNT;i++)if(s->course_meshes[i].vertexCount)UnloadMesh(s->course_meshes[i]);
    for(int i=0;i<s->building_count;i++)if(s->buildings[i].mesh.vertexCount)UnloadMesh(s->buildings[i].mesh);
    if(s->cube_template.vertexCount)UnloadMesh(s->cube_template);
    if(s->facade_material.maps)MemFree(s->facade_material.maps);
    if(s->window_texture.id)UnloadTexture(s->window_texture);
    if(s->city_material.maps)MemFree(s->city_material.maps);
    if(s->tree_material.maps)MemFree(s->tree_material.maps);
    if(s->crown_material.maps)MemFree(s->crown_material.maps);
    if(s->crown_texture.id)UnloadTexture(s->crown_texture);
    if(s->control_material.maps)MemFree(s->control_material.maps);
    if(s->control_texture.id)UnloadTexture(s->control_texture);
    if(s->tree_texture.id)UnloadTexture(s->tree_texture);
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
    float sculpt_glow=0;SetShaderValue(s->world_shader,s->sculpt_glow_location,&sculpt_glow,SHADER_UNIFORM_FLOAT);
    Vector3 fog_forward=Vector3Normalize(Vector3Subtract(camera.target,camera.position));
    SetShaderValue(s->world_shader,s->view_forward_location,&fog_forward,SHADER_UNIFORM_VEC3);
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

static void draw_gate_fallback(const RRGate *g,bool active)
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
    DrawCylinderEx((Vector3){base.x,base.y-12,base.z},(Vector3){base.x,base.y+2.6f,base.z},13.7f,13.7f,24,(Color){20,24,43,255});
    DrawCylinderEx((Vector3){base.x,base.y-480,base.z},(Vector3){base.x,base.y-12,base.z},5.2f,5.2f,12,(Color){21,25,42,255});
}

void rr_scene_world(RRScene *s,const RRTrack *track,const RRCourse *course,const RRSharedScenery *shared,Camera3D camera,int next_gate,double time)
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
    /* Residency, as well as visibility, stays within the existing 64-building cap. */
    for(int i=0;i<s->building_count;i++)if(s->buildings[i].mesh.vertexCount){
        bool keep=false;for(int j=0;j<count;j++)if(visible[j]==i){keep=true;break;}
        if(!keep){UnloadMesh(s->buildings[i].mesh);s->buildings[i].mesh=(Mesh){0};}
    }
    /* Bound uploads to two newly visible buildings per frame, nearest first. */
    int uploads=0;
    for(int i=0;i<RR_COURSE_MESH_COUNT && uploads<2;i++)if(s->course_meshes[i].vertexCount && !s->course_meshes[i].vaoId){UploadMesh(&s->course_meshes[i],false);uploads++;TraceLog(LOG_INFO,"COURSE: prefab mesh=%d vertices=%d uploaded=%d",i,s->course_meshes[i].vertexCount,s->course_meshes[i].vaoId!=0);}
    for(int candidate=0;candidate<count;candidate++){
        int i=visible[candidate];
        RRBuilding *b=&s->buildings[i];
        if(!b->mesh.vertexCount && !b->mesh_failed && uploads<2){
            uploads++;
            if(!cache_building(s,b))continue;
        }
        if(b->mesh.vaoId){
            float facade=b->canonical_index>=0?.95f:0;
            SetShaderValue(s->world_shader,s->facade_location,&facade,SHADER_UNIFORM_FLOAT);
            DrawMesh(b->mesh,b->canonical_index>=0?s->facade_material:s->city_material,MatrixIdentity());
            facade=0;SetShaderValue(s->world_shader,s->facade_location,&facade,SHADER_UNIFORM_FLOAT);
        }
        else{
            /* Keep a cheap silhouette while detailed geometry waits for its upload budget. */
            s->city_material.maps[MATERIAL_MAP_DIFFUSE].color=b->color;
            Matrix transform=MatrixMultiply(MatrixScale(b->size.x,b->size.y,b->size.z),MatrixTranslate(b->pos.x,b->pos.y,b->pos.z));
            DrawMesh(s->cube_template,s->city_material,transform);
            s->city_material.maps[MATERIAL_MAP_DIFFUSE].color=WHITE;
        }
    }
    s->city_resident=s->canonical_resident=0;
    for(int i=0;i<s->building_count;i++)if(s->buildings[i].mesh.vertexCount){s->city_resident++;if(s->buildings[i].canonical_index>=0)s->canonical_resident++;}
    if(s->crown_loaded && s->course_meshes[12].vaoId){
        int selected[4],selected_count=0;double distance[4];
        for(int i=0;i<rr_canonical_hero_count();i++){
            RRHeroPlacement p;if(!rr_canonical_city_hero_placement(i,rr_city_crown_model_size,&p))continue;
            RRHeroBounds bounds=rr_hero_world_bounds(rr_city_crown_bounds,p);RRVec3 center=rr_scale(rr_add(bounds.minimum,bounds.maximum),.5);float radius=(float)(rr_length(rr_sub(bounds.maximum,bounds.minimum))*.5);
            if(!visible_sphere(&view,as_vector(center),radius,1000))continue;
            double d=rr_length(rr_sub(center,core_vector(camera.position)));int slot=selected_count;if(slot==4){if(d>=distance[3])continue;slot--;}else selected_count++;
            while(slot>0 && d<distance[slot-1]){selected[slot]=selected[slot-1];distance[slot]=distance[slot-1];slot--;}
            selected[slot]=i;distance[slot]=d;
        }
        for(int i=0;i<selected_count;i++){
            RRHeroPlacement p;rr_canonical_city_hero_placement(selected[i],rr_city_crown_model_size,&p);RRCourseItem transform={0};transform.pos=p.pos;
            transform.right=rr_v3(cos(p.yaw)*p.scale,0,-sin(p.yaw)*p.scale);transform.up=rr_v3(0,p.scale,0);transform.back=rr_v3(sin(p.yaw)*p.scale,0,cos(p.yaw)*p.scale);
            rlDrawRenderBatchActive();float glow=(float)RR_CITY_CROWN_GLOW;SetShaderValue(s->world_shader,s->sculpt_glow_location,&glow,SHADER_UNIFORM_FLOAT);
            rlDisableBackfaceCulling();DrawMesh(s->course_meshes[12],s->crown_material,course_transform(&transform));rlEnableBackfaceCulling();glow=0;SetShaderValue(s->world_shader,s->sculpt_glow_location,&glow,SHADER_UNIFORM_FLOAT);
        }
    }
    BeginShaderMode(s->world_shader);
    DrawPlane((Vector3){0,20,0},(Vector2){12000,12000},(Color){36,24,61,255});
    DrawCube((Vector3){0,107,20},100,8,160,(Color){22,25,43,255});
    DrawCube((Vector3){-50,111,20},.8f,1,160,cyan);
    DrawCube((Vector3){50,111,20},.8f,1,160,cyan);
    DrawCube((Vector3){-26,131,0},6.8f,40,7.2f,(Color){36,40,60,255});
    DrawCube((Vector3){26,131,0},6.8f,40,7.2f,(Color){36,40,60,255});
    DrawCube((Vector3){0,146,0},60,8.8f,6.8f,(Color){35,37,58,255});
    DrawCube((Vector3){0,143.6f,-3.6f},48,.6f,.5f,magenta);
    DrawCube((Vector3){0,102,20},72,6,124,(Color){18,22,39,255});
    for(int side=-1;side<=1;side+=2)DrawCube((Vector3){side*22.0f,-89,20},12,380,12,(Color){21,25,42,255});
    if(s->control_loaded && s->course_meshes[11].vaoId){
        RRVec3 center=rr_scale(rr_add(s->control_world_bounds.minimum,s->control_world_bounds.maximum),.5);
        float radius=(float)(rr_length(rr_sub(s->control_world_bounds.maximum,s->control_world_bounds.minimum))*.5);
        if(visible_sphere(&view,as_vector(center),radius,1000)){
            const RRHeroPlacement *p=&s->control_placement;RRCourseItem transform={0};transform.pos=p->pos;
            transform.right=rr_v3(cos(p->yaw)*p->scale,0,-sin(p->yaw)*p->scale);transform.up=rr_v3(0,p->scale,0);transform.back=rr_v3(sin(p->yaw)*p->scale,0,cos(p->yaw)*p->scale);
            rlDrawRenderBatchActive();float glow=(float)RR_CONTROL_TOWER_GLOW;SetShaderValue(s->world_shader,s->sculpt_glow_location,&glow,SHADER_UNIFORM_FLOAT);
            rlDisableBackfaceCulling();DrawMesh(s->course_meshes[11],s->control_material,course_transform(&transform));rlEnableBackfaceCulling();
            glow=0;SetShaderValue(s->world_shader,s->sculpt_glow_location,&glow,SHADER_UNIFORM_FLOAT);
        }
    }else{
        DrawCylinderEx((Vector3){-80,-400,10},(Vector3){-80,155,10},7,7,12,(Color){21,25,42,255});
        DrawCylinderEx((Vector3){-80,152,10},(Vector3){-80,179,10},16.8f,16.8f,16,(Color){27,31,51,255});
    }
    for(int i=0;i<track->gate_count;i++){
        const RRGate *gate=&track->gates[i];
        if(!visible_sphere(&view,as_vector(gate->pos),170,900))continue;
        if(s->course_meshes[5].vaoId){
            RRCourseItem station={0};station.pos=rr_add(gate->pos,rr_v3(0,-22,0));
            station.right=rr_v3(cos(gate->yaw),0,-sin(gate->yaw));station.up=rr_v3(0,1,0);station.back=rr_v3(sin(gate->yaw),0,cos(gate->yaw));
            DrawMesh(s->course_meshes[5],s->city_material,course_transform(&station));
            if(i==next_gate){Vector3 center=as_vector(gate->pos);DrawCircle3D(center,13.7f,(Vector3){0,1,0},(float)(gate->yaw*RAD2DEG),(Color){130,255,180,255});}
        }else draw_gate_fallback(gate,i==next_gate);
    }
    int furniture[16],furniture_count=0;float furniture_distance[16];
    for(int i=0;i<course->item_count+shared->prop_count;i++){
        bool is_shared=i>=course->item_count;
        const RRCourseItem *item=is_shared?NULL:&course->items[i];
        Vector3 p=as_vector(is_shared?shared->props[i-course->item_count].pos:item->pos);
        if(!visible_sphere(&view,p,18,item && item->kind==RR_COURSE_CHEVRON?500.0f:750.0f))continue;
        float distance=Vector3LengthSqr(Vector3Subtract(p,camera.position));int slot=furniture_count;
        if(slot==16){if(distance>=furniture_distance[15])continue;slot--;}else furniture_count++;
        while(slot>0&&distance<furniture_distance[slot-1]){furniture[slot]=furniture[slot-1];furniture_distance[slot]=furniture_distance[slot-1];slot--;}
        furniture[slot]=i;furniture_distance[slot]=distance;
    }
    for(int i=0;i<furniture_count;i++){
        int index=furniture[i],mesh_index;RRCourseItem transform;
        if(index>=course->item_count){
            const RRSharedProp *prop=&shared->props[index-course->item_count];transform=(RRCourseItem){0};transform.pos=prop->pos;
            transform.right=rr_v3(cos(prop->yaw)*prop->scale,0,-sin(prop->yaw)*prop->scale);transform.up=rr_v3(0,prop->scale,0);transform.back=rr_v3(sin(prop->yaw)*prop->scale,0,cos(prop->yaw)*prop->scale);mesh_index=6+prop->kind;
        }else{transform=course->items[index];mesh_index=(int)transform.kind;}
        bool sculpted=index>=course->item_count && shared->props[index-course->item_count].kind>=RR_SHARED_TREE_M && s->tree_loaded && s->course_meshes[10].vaoId;
        if(sculpted){
            transform.right=rr_scale(transform.right,RR_CYAN_TREE_FIT);transform.up=rr_scale(transform.up,RR_CYAN_TREE_FIT);transform.back=rr_scale(transform.back,RR_CYAN_TREE_FIT);
            rlDrawRenderBatchActive();float glow=(float)RR_CYAN_TREE_GLOW;SetShaderValue(s->world_shader,s->sculpt_glow_location,&glow,SHADER_UNIFORM_FLOAT);
            rlDisableBackfaceCulling();DrawMesh(s->course_meshes[10],s->tree_material,course_transform(&transform));rlEnableBackfaceCulling();
            glow=0;SetShaderValue(s->world_shader,s->sculpt_glow_location,&glow,SHADER_UNIFORM_FLOAT);
        }else if(s->course_meshes[mesh_index].vaoId)DrawMesh(s->course_meshes[mesh_index],s->city_material,course_transform(&transform));
    }
    int dot_budget=256;
    for(int i=0;i<course->dot_count && dot_budget>0;i++){
        Vector3 p=as_vector(course->dots[i]);
        if(visible_sphere(&view,p,.5f,650)){DrawSphereEx(p,.3f,3,4,(Color){43,255,122,255});dot_budget--;}
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
