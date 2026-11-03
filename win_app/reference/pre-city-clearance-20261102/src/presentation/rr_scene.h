#ifndef RR_SCENE_H
#define RR_SCENE_H

#include "raylib.h"
#include "rr_track.h"
#include "rr_flight.h"

typedef struct RRBuilding { Vector3 pos, size; Color color; int pattern; Mesh mesh; } RRBuilding;
typedef struct RRScene {
    Model car;
    Shader world_shader, post_shader;
    RenderTexture2D target;
    Texture2D logo;
    Mesh cube_template;
    Material city_material;
    RRBuilding buildings[700];
    int building_count;
    int view_position_location;
    int post_resolution_location;
    bool car_loaded;
    bool glow;
} RRScene;

bool rr_scene_load(RRScene *scene,const RRTrack *track,bool fallback,bool glow);
void rr_scene_unload(RRScene *scene);
bool rr_scene_begin(RRScene *scene,Camera3D camera);
void rr_scene_world(RRScene *scene,const RRTrack *track,Camera3D camera,int next_gate,double time);
void rr_scene_car(RRScene *scene,const RRCar *car);
void rr_scene_finish(RRScene *scene);

#endif
