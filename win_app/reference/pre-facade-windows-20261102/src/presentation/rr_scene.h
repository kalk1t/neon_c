#ifndef RR_SCENE_H
#define RR_SCENE_H

#include "raylib.h"
#include "rr_track.h"
#include "rr_flight.h"
#include "rr_city_plan.h"
#include "rr_course_plan.h"
#include "rr_shared_scenery.h"
#include "rr_hero_asset.h"
#define RR_COURSE_MESH_COUNT 13
#define RR_FALLBACK_MESH_COUNT 10

typedef struct RRBuilding { Vector3 pos, size; Color color; int pattern,canonical_index; bool mesh_failed; Mesh mesh; } RRBuilding;
typedef struct RRScene {
    Model car;
    Shader world_shader, post_shader;
    RenderTexture2D target;
    Texture2D logo;
    Mesh cube_template;
    Mesh course_meshes[RR_COURSE_MESH_COUNT];
    Material city_material,tree_material,control_material,crown_material;
    Texture2D tree_texture,control_texture,crown_texture;
    RRHeroPlacement control_placement;
    RRHeroBounds control_world_bounds;
    bool tree_loaded,control_loaded,crown_loaded;
    RRBuilding buildings[RR_CITY_MAX_BUILDINGS];
    int building_count,city_resident,canonical_resident;
    int view_position_location,view_forward_location,sculpt_glow_location;
    int post_resolution_location;
    bool car_loaded;
    bool glow;
} RRScene;

bool rr_scene_load(RRScene *scene,const RRCorridor *corridor,bool fallback,bool glow);
void rr_scene_unload(RRScene *scene);
bool rr_scene_begin(RRScene *scene,Camera3D camera);
void rr_scene_world(RRScene *scene,const RRTrack *track,const RRCourse *course,const RRSharedScenery *shared,Camera3D camera,int next_gate,double time);
void rr_scene_car(RRScene *scene,const RRCar *car);
void rr_scene_finish(RRScene *scene);

#endif
