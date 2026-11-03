#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "rr_scene.h"
#include "rr_session.h"
#include "rr_finish.h"
#include "rr_world_plan.h"
#include "rr_canonical_city.h"
#include "rr_menu_policy.h"
#include "rr_recovery.h"
#include "rr_road_picker.h"
#include "rr_key_events.h"
#include "rr_minimap.h"
#include "rr_render_policy.h"
#include "rr_ui.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

typedef struct RRApp {
    RRTrack track; RRCorridor corridor; RRWorld world; RRCityPlan city_plan; RRCourse course; RRSharedScenery shared; RRCar car; RRScene scene; RRSession session; RRRaceMode selected_mode; RRMenuPolicy menu_policy; RRRecovery recovery; RRRoadPicker picker; RRKeyEvents key_events; RRMinimap minimap;
    Camera3D camera; RRUI ui; float road_scroll;
    double aim_yaw,aim_pitch,look_yaw,look_pitch,camera_yaw,camera_pitch,camera_fov;
    double spin_cooldown,message_time,warning_time,picker_notice;
    unsigned warning;
    int camera_mode,frame;
    bool smoke,fallback,failed,scene_ready,load_requested,glow,quit,overloaded;
    int slow_frames,target_fps;
    double diagnostic_time,worst_frame;
} RRApp;
static RRApp app;
#if !defined(PLATFORM_WEB)
static FILE *diagnostic_log;
static void log_message(int level,const char *message,va_list args)
{
    FILE *output=diagnostic_log?diagnostic_log:stderr;
    time_t now=time(NULL);struct tm stamp;
    char date[32]="unknown time";
    if(gmtime_s(&stamp,&now)==0)strftime(date,sizeof(date),"%Y-%m-%dT%H:%M:%SZ",&stamp);
    fprintf(output,"%s [%d] ",date,level);vfprintf(output,message,args);fputc('\n',output);fflush(output);
}
#endif
static const Color ink={5,6,26,255},cyan={25,230,255,255},muted={143,179,201,255};
static const Color magenta={255,43,214,255},white={232,251,255,255};
static Vector3 vector(RRVec3 p){return (Vector3){(float)p.x,(float)p.y,(float)p.z};}

static bool key_hit(int key)
{
    return rr_key_events_hit(&app.key_events,key)||IsKeyPressed(key);
}
static void read_key_events(void)
{
    rr_key_events_clear(&app.key_events);
    for(int key=GetKeyPressed();key;key=GetKeyPressed())rr_key_events_push(&app.key_events,key);
    if(rr_key_events_hit(&app.key_events,KEY_ENTER)||rr_key_events_hit(&app.key_events,KEY_P)||rr_key_events_hit(&app.key_events,KEY_ESCAPE)||rr_key_events_hit(&app.key_events,KEY_R))
        TraceLog(LOG_INFO,"INPUT: queued enter=%d pause=%d escape=%d reset=%d phase=%d",rr_key_events_hit(&app.key_events,KEY_ENTER),rr_key_events_hit(&app.key_events,KEY_P),rr_key_events_hit(&app.key_events,KEY_ESCAPE),rr_key_events_hit(&app.key_events,KEY_R),app.session.phase);
}

static void target_fps(int fps)
{
    if(app.target_fps==fps)return;
    app.target_fps=fps;
#if defined(PLATFORM_WEB)
    /* Yield to the browser scheduler instead of sleeping on its main thread. */
    emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT,1000/fps);
#else
    SetTargetFPS(fps);
#endif
}

static void center_aim(void){app.aim_yaw=app.car.yaw;app.aim_pitch=app.car.pitch;GetMouseDelta();}
static void capture(void){DisableCursor();center_aim();}
static void reset_car(bool beginning)
{
    if(beginning)rr_car_reset(&app.car,rr_v3(0,115.5,42),0,0);
    else rr_session_respawn(&app.session,&app.car);
    if(beginning)rr_recovery_init(&app.recovery,&app.track,app.car.pos);
    else rr_recovery_rebase(&app.recovery,&app.track,app.car.pos);
    rr_minimap_init(&app.minimap,&app.track);
    app.camera_yaw=app.car.yaw;app.camera_pitch=0;center_aim();
}
static void start_flight(RRRaceMode mode)
{
    if(app.failed)return;
    app.selected_mode=mode;app.picker.mode=mode;
    if(!app.scene_ready){app.load_requested=true;return;}
    TraceLog(LOG_INFO,"STATE: start offline session mode=%d",mode);
    if(!rr_road_picker_start(&app.picker,&app.track,&app.session,&app.car)){app.failed=true;return;}
    if(!rr_course_plan(&app.course,&app.track)||!rr_world_plan_shared(&app.world,&app.city_plan,&app.course,&app.shared,mode==RR_RACE_FREE_ROAM)){app.failed=true;rr_session_menu(&app.session);EnableCursor();return;}
    app.session.world=&app.world;
    TraceLog(LOG_INFO,"COURSE: road=%d dots=%d items=%d colliders=%d beacons=%d barriers=%d signs=%d chevrons=%d",app.track.road->id,app.course.dot_count,app.course.item_count,app.course.collider_count,app.course.beacons,app.course.barriers,app.course.signs,app.course.chevrons);
    TraceLog(LOG_INFO,"COLLISION: native city/start/road world ready shapes=%d road=%d",app.world.count,app.track.road->id);
    rr_minimap_init(&app.minimap,&app.track);
    rr_recovery_init(&app.recovery,&app.track,app.car.pos);app.warning_time=0;app.warning=0;
    app.spin_cooldown=0;app.message_time=0;
    app.look_yaw=0;app.look_pitch=0;app.camera_fov=68;
    app.camera_yaw=0;app.camera_pitch=0;app.slow_frames=0;app.overloaded=false;
    /* Existing diagnostic smoke path remains distinct from normal countdown. */
    if(app.smoke){app.session.phase=RR_SESSION_ACTIVE;center_aim();}else capture();
}
static bool pad_pressed(int button){return IsGamepadAvailable(0) && IsGamepadButtonPressed(0,button);}
static bool pad_down(int button){return IsGamepadAvailable(0) && IsGamepadButtonDown(0,button);}
static double axis(int id){double v=GetGamepadAxisMovement(0,id);return fabs(v)<.15?0:v;}

static RRInput controls(double dt)
{
    RRInput in={0};
    in.throttle=IsKeyDown(KEY_W)?1:0;in.brake=IsKeyDown(KEY_S)?1:0;
    in.steer=(IsKeyDown(KEY_D)||IsKeyDown(KEY_RIGHT)?1:0)-(IsKeyDown(KEY_A)||IsKeyDown(KEY_LEFT)?1:0);
    in.climb=(IsKeyDown(KEY_E)||IsKeyDown(KEY_UP)?1:0)-(IsKeyDown(KEY_Q)||IsKeyDown(KEY_DOWN)?1:0);
    in.boost=IsKeyDown(KEY_LEFT_SHIFT)||IsKeyDown(KEY_RIGHT_SHIFT)||pad_down(GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    if(IsGamepadAvailable(0)){
        double sx=axis(GAMEPAD_AXIS_LEFT_X),sy=-axis(GAMEPAD_AXIS_LEFT_Y);
        if(sx)in.steer=sx;if(sy)in.climb=sy;
        in.throttle=fmax(in.throttle,rr_clamp((GetGamepadAxisMovement(0,GAMEPAD_AXIS_RIGHT_TRIGGER)+1)*.5,0,1));
        in.brake=fmax(in.brake,rr_clamp((GetGamepadAxisMovement(0,GAMEPAD_AXIS_LEFT_TRIGGER)+1)*.5,0,1));
    }
    Vector2 delta=GetMouseDelta();
    if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
        app.look_yaw-=delta.x*.0021*1.6;app.look_pitch=rr_clamp(app.look_pitch+delta.y*.0021*1.6,-.45,1.15);
    }else{
        app.aim_yaw-=delta.x*.0021;app.aim_pitch-=delta.y*.0021;
        app.look_yaw=atan2(sin(app.look_yaw),cos(app.look_yaw))*exp(-4*dt);app.look_pitch*=exp(-4*dt);
    }
    bool key_steer=in.steer!=0,key_climb=in.climb!=0;
    if(key_steer)app.aim_yaw=app.car.yaw;
    if(key_climb)app.aim_pitch=app.car.pitch;
    double difference=atan2(sin(app.aim_yaw-app.car.yaw),cos(app.aim_yaw-app.car.yaw));
    difference=rr_clamp(difference,-1.1,1.1);app.aim_yaw=app.car.yaw+difference;
    app.aim_pitch=rr_clamp(app.aim_pitch,-.62,.62);
    if(!key_steer)in.steer=rr_clamp(-difference*2.8,-1,1);
    if(!key_climb)in.climb=rr_clamp(app.aim_pitch/.62*1.25,-1,1);
    return in;
}

static void update_camera(double dt)
{
    Vector3 car=vector(app.car.pos);
    if(app.session.phase==RR_SESSION_MENU){
        app.camera.position=Vector3Add(car,(Vector3){10,3.8f,-12});
        app.camera.target=Vector3Add(car,(Vector3){5,.3f,3.8f});app.camera.fovy=42;
        return;
    }
    double difference=atan2(sin(app.car.yaw-app.camera_yaw),cos(app.car.yaw-app.camera_yaw));
    app.camera_yaw+=difference*(1-exp(-7*dt));
    app.camera_pitch+=(app.car.pitch*.6-app.camera_pitch)*(1-exp(-5*dt));
    double yaw=app.camera_yaw+app.look_yaw,pitch=rr_clamp(app.camera_pitch-app.look_pitch,-1.3,1.3);
    Vector3 forward={(float)(-sin(yaw)*cos(pitch)),(float)sin(pitch),(float)(-cos(yaw)*cos(pitch))};
    double sf=fmax(0,app.car.speed)/RR_CAR_MAX;
    float distance=(float)((app.camera_mode?15:9)*(1+sf*.22));
    app.camera.position=Vector3Subtract(car,Vector3Scale(forward,distance));
    app.camera.position.y+=(app.camera_mode?4.8f:2.7f)-(float)(sf*.6);
    double orbit=fmin(1,(fabs(atan2(sin(app.look_yaw),cos(app.look_yaw)))+fabs(app.look_pitch))*4);
    app.camera.target=Vector3Add(car,Vector3Scale(vector(app.car.fwd),(float)(9*(1-orbit))));app.camera.target.y+=.9f;
    app.camera_fov+=(68+15*fmin(1,sf)+9*app.car.nitro-app.camera_fov)*(1-exp(-4*dt));
    app.camera.fovy=(float)app.camera_fov;
    Vector3 right={(float)cos(yaw),0,(float)-sin(yaw)};
    app.camera.up=Vector3Normalize(Vector3Add((Vector3){0,1,0},Vector3Scale(right,(float)(-app.car.roll*.28*(1-orbit)))));
}

static bool button(Rectangle area,const char *label,bool primary)
{
    bool hover=CheckCollisionPointRec(GetMousePosition(),area);
    Color color=primary?cyan:(Color){37,43,67,255};
    DrawRectangleRec(area,hover?ColorBrightness(color,.14f):color);
    DrawText(label,(int)area.x+22,(int)area.y+17,22,primary?ink:white);
    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void open_roads(RRRaceMode mode)
{
    /* Persistence/profile adapters are not connected. Do not import or grant progress. */
    rr_road_picker_open(&app.picker,mode,1,0);
    app.menu_policy=(RRMenuPolicy){0};app.picker_notice=0;app.road_scroll=0;EnableCursor();
    TraceLog(LOG_INFO,"STATE: road picker mode=%d open=%d current=%d",mode,app.picker.open_count,app.picker.current_road);
}
static Rectangle ui_rect(RRUIRect r){return (Rectangle){r.x,r.y,r.w,r.h};}
static void road_picker_ui(void)
{
    int w=GetScreenWidth(),h=GetScreenHeight();RRUIRoadLayout l=rr_ui_road_layout((float)w,(float)h);
    DrawRectangle(0,0,w,h,(Color){3,4,20,180});rr_ui_panel(l.panel,cyan);
    rr_ui_text(&app.ui,app.picker.mode==RR_RACE_TIME_ATTACK?"TIME ATTACK":"RACE TRIAL",l.list.x,l.panel.y+24,32,3,900,white,true);
    rr_ui_text(&app.ui,"CHOOSE YOUR ROAD",l.list.x,l.panel.y+66,20,2,700,cyan,false);
    rr_ui_text(&app.ui,"OFFLINE TRIALS DO NOT UNLOCK ROADS",l.list.x,l.panel.y+104,13,1,500,muted,false);
    int before=app.picker.focus_road;
    if(key_hit(KEY_DOWN)||key_hit(KEY_TAB))rr_road_picker_move(&app.picker,1);
    if(key_hit(KEY_UP))rr_road_picker_move(&app.picker,-1);
    if(before!=app.picker.focus_road)app.road_scroll=rr_ui_scroll_focus(l,app.road_scroll,app.picker.focus_road);
    if(CheckCollisionPointRec(GetMousePosition(),ui_rect(l.list)))app.road_scroll=fmaxf(0,fminf(l.max_scroll,app.road_scroll-GetMouseWheelMove()*40));
    int clicked=0;BeginScissorMode((int)l.list.x,(int)l.list.y,(int)l.list.w,(int)l.list.h);
    for(int i=0;i<RR_ROAD_COUNT;i++){
        int id=i+1;float y=l.list.y+i*l.stride-app.road_scroll;RRUIRect row={l.list.x,y,l.list.w,l.row_h};
        if(y+l.row_h<l.list.y||y>l.list.y+l.list.h)continue;
        bool locked=id>app.picker.open_count,focus=id==app.picker.focus_road;
        float opacity=locked?.45f:1.f;Color edge=id==app.picker.current_road?magenta:cyan;
        bool hover=CheckCollisionPointRec(GetMousePosition(),ui_rect(l.list))&&rr_ui_hit(row,GetMousePosition().x,GetMousePosition().y,14);
        rr_ui_row(row,edge,opacity,hover||focus);
        rr_ui_text(&app.ui,TextFormat("%d",id),row.x+16,y+10,26,0,900,Fade(locked?muted:cyan,opacity),!locked);
        rr_ui_text_fit(&app.ui,TextFormat("%s%s",locked?"?? ":"",rr_road_by_id(id)->name),row.x+68,y+6,row.w-180,16,2,700,Fade(white,opacity));
        rr_ui_pips(row.x+row.w-95,y+7,id,opacity);
        const RRRoadRow *data=&app.picker.rows[i];
        rr_ui_text_fit(&app.ui,TextFormat("%.2f KM / %d GATES / 2 LAPS",data->length/1000,data->gates),row.x+68,y+31,row.w-242,11,1.5f,500,Fade(muted,opacity));
        const char *hint=locked?TextFormat("WIN ROAD %d IN 1ST",id-1):data->best_lap>0?TextFormat("BEST %.2f S",data->best_lap):"NO LAP YET";
        rr_ui_text(&app.ui,hint,row.x+row.w-164,y+31,10,0,700,Fade(locked?(Color){255,184,242,255}:cyan,opacity),false);
        if(hover&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){clicked=id;app.picker.focus_road=id;}
    }
    EndScissorMode();
    if(l.max_scroll>0){float ratio=l.list.h/l.content_h;float bar=l.list.h*ratio;DrawRectangle((int)(l.list.x+l.list.w+8),(int)(l.list.y+(l.list.h-bar)*app.road_scroll/l.max_scroll),3,(int)bar,Fade(cyan,.6f));}
    if(app.picker_notice>0)rr_ui_text(&app.ui,"ROAD LOCKED",l.list.x,l.back.y-22,13,1,700,magenta,false);
    bool held=IsMouseButtonDown(MOUSE_BUTTON_LEFT)||IsKeyDown(KEY_ENTER)||pad_down(GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    bool confirm=key_hit(KEY_ENTER)||pad_pressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)||clicked!=0;
    int action=rr_menu_request(&app.menu_policy,IsWindowFocused(),held,confirm,false);
    if(action!=RR_MENU_NO_ACTION){
        int id=clicked?clicked:app.picker.focus_road;
        if(rr_road_picker_choose(&app.picker,&app.track,id)){
            TraceLog(LOG_INFO,"STATE: selected road=%d mode=%d",id,app.picker.mode);
            rr_minimap_init(&app.minimap,&app.track);start_flight(app.picker.mode);return;
        }
        app.picker_notice=1.4;TraceLog(LOG_INFO,"STATE: locked road refused id=%d open=%d",id,app.picker.open_count);
    }
    if(rr_ui_button(&app.ui,l.back,"BACK",false,true,false,5)||key_hit(KEY_ESCAPE)){
        rr_road_picker_close(&app.picker);app.menu_policy=(RRMenuPolicy){0};return;
    }
}

static void menu_ui(void)
{
    if(app.picker.open){road_picker_ui();return;}
    int w=GetScreenWidth(),h=GetScreenHeight();RRUIMenuLayout l=rr_ui_menu_layout((float)w,(float)h);
    DrawRectangleGradientH(0,0,(int)(w*.7f),h,(Color){3,4,20,235},(Color){3,4,20,0});
    rr_ui_text(&app.ui,"ROADSTERS",l.x,l.y,l.brand,4,900,WHITE,true);
    rr_ui_text(&app.ui,"RACE",l.x,l.y+l.brand*.95f,l.brand,4,900,magenta,true);
    const RRRoadRow *road=&app.picker.rows[app.picker.current_road-1];
    rr_ui_text(&app.ui,TextFormat("ROAD %d / %s / %d GATES",app.picker.current_road,rr_road_by_id(app.picker.current_road)->name,road->gates),l.x,l.actions[0].y-24,12,1.5f,700,cyan,false);
    bool trial_mouse=rr_ui_button(&app.ui,l.actions[0],"RACE / OFFLINE TRIAL",true,!app.failed,false,0);
    bool time_mouse=rr_ui_button(&app.ui,l.actions[1],"TIME ATTACK / SOLO",false,!app.failed,false,1);
    rr_ui_button(&app.ui,l.actions[2],"FREE ROAM / ONLINE",false,false,false,2);
    rr_ui_button(&app.ui,l.actions[3],"WORLD RANKING",false,false,false,3);
    rr_ui_button(&app.ui,l.actions[4],"SHOP",false,false,false,4);
    bool enter=key_hit(KEY_ENTER),pad=pad_pressed(GAMEPAD_BUTTON_MIDDLE_RIGHT);
    bool held=IsMouseButtonDown(MOUSE_BUTTON_LEFT)||IsKeyDown(KEY_ENTER)||pad_down(GAMEPAD_BUTTON_MIDDLE_RIGHT);
    int action=app.failed?RR_MENU_NO_ACTION:rr_menu_request(&app.menu_policy,IsWindowFocused(),held,trial_mouse||enter||pad,time_mouse);
    if(action!=RR_MENU_NO_ACTION)open_roads(action==RR_MENU_START_TRIAL?RR_RACE_TWO_LAPS:RR_RACE_TIME_ATTACK);
    float y=l.actions[4].y+l.button_h+14;
    rr_ui_text(&app.ui,app.failed?"SCENE UNAVAILABLE / CHECK LOG":"OFFLINE / ONLINE SERVICES NOT CONNECTED",l.x,y,12,1,500,app.failed?magenta:muted,false);
    rr_ui_text(&app.ui,"MOUSE steer + climb/dive   W thrust   S brake",l.x,y+25,19,0,0,muted,false);
    rr_ui_text(&app.ui,"SHIFT boost   F spin   C camera   R reset",l.x,y+48,19,0,0,muted,false);
    rr_ui_text(&app.ui,"RIVALS / COMBAT / ACCOUNT INTEGRATION PENDING",l.x,y+75,11,0,500,muted,false);
    if(!app.ui.loaded)rr_ui_text(&app.ui,"THEME FONT LOAD FAILED",l.x,y+96,14,0,700,magenta,false);
#if !defined(PLATFORM_WEB)
    if(rr_ui_button(&app.ui,(RRUIRect){(float)w-168,(float)h-76,144,50},"QUIT",false,true,false,6)||key_hit(KEY_ESCAPE))app.quit=true;
#endif
}

static Vector2 map_point(RRVec3 p,float x,float y)
{
    double px,py;rr_minimap_point(&app.minimap,p.x,p.z,&px,&py);
    return (Vector2){x+16+(float)px*168,y+16+(float)py*168};
}
static void flight_ui(void)
{
    int w=GetScreenWidth(),h=GetScreenHeight();
    DrawRectangle(26,24,310,76,Fade(ink,.8f));DrawRectangle(26,24,3,76,cyan);
    DrawText(app.track.road->name,43,38,22,white);
    const RRRace *race=&app.session.race;
    RRVec3 target={0};bool has_target=rr_session_target(&app.session,app.track.gates,app.track.gate_count,&target);
    if(race->finished)DrawText("FINISH / TWO LAPS COMPLETE",43,68,17,cyan);
    else if(race->next_checkpoint==app.track.gate_count)DrawText("RETURN TO FINISH",43,68,17,cyan);
    else DrawText(TextFormat("LAP %d   GATE %d / %d",race->lap,race->next_checkpoint+1,app.track.gate_count),43,68,17,cyan);
    int total=(int)app.session.race.lap_time;
    DrawText(TextFormat("%02d:%02d.%02d",total/60,total%60,(int)(app.session.race.lap_time*100)%100),w/2-65,30,27,white);
    DrawText(app.session.race.mode==RR_RACE_TIME_ATTACK?"TIME ATTACK / OFFLINE":"TWO-LAP SOLO TRIAL",w/2-91,62,14,muted);
    float mx=(float)w-230,my=24;
    DrawRectangle((int)mx,(int)my,204,213,Fade(ink,.8f));
    for(int i=0;i<app.track.sample_count;i+=6){
        int next=(i+6)%app.track.sample_count;
        DrawLineV(map_point(app.track.samples[i],mx,my),map_point(app.track.samples[next],mx,my),Fade(cyan,.6f));
    }
    if(has_target)DrawCircleV(map_point(target,mx,my),5,(Color){143,255,184,255});
    DrawCircleV(map_point(app.car.pos,mx,my),3.5f,WHITE);
    DrawText("SKYLINE CITY",(int)mx+44,216,14,muted);
    DrawText(TextFormat("%03d",(int)fabs(app.car.speed*3.6)),w-226,h-131,68,white);
    DrawText("KM/H",w-92,h-61,18,cyan);
    DrawRectangle(w-228,h-44,202,4,(Color){40,48,70,255});
    DrawRectangle(w-228,h-44,(int)(202*rr_clamp(fabs(app.car.speed)/RR_CAR_BOOST_MAX,0,1)),4,cyan);
    DrawText("BOOST",35,h-110,16,muted);DrawRectangle(35,h-82,210,12,(Color){32,39,61,255});
    DrawRectangle(35,h-82,(int)(210*app.car.boost),12,app.car.boosting?magenta:cyan);
    DrawText(app.spin_cooldown>0?TextFormat("SPIN  %.1f S",app.spin_cooldown):"SPIN  READY [F]",35,h-53,16,app.spin_cooldown>0?muted:white);
    DrawText("ESC PAUSE   C CAMERA   R RESET   F11 FULLSCREEN",w/2-220,h-28,14,muted);
    Vector3 ahead=Vector3Subtract(vector(target),app.camera.position);
    if(has_target && Vector3DotProduct(ahead,Vector3Subtract(app.camera.target,app.camera.position))>0){
        Vector2 point=GetWorldToScreen(vector(target),app.camera);
        if(point.x>40 && point.x<w-40 && point.y>110 && point.y<h-140){
            DrawCircleLines((int)point.x,(int)point.y,22,(Color){143,255,184,255});
            DrawText(TextFormat("%d M",(int)rr_length(rr_sub(target,app.car.pos))),(int)point.x-26,(int)point.y+31,16,white);
        }
    }
    DrawCircleLines(w/2,h/2,6,Fade(WHITE,.5f));
    if(app.session.phase==RR_SESSION_COUNTDOWN)DrawText(TextFormat("%d",app.session.last_count),w/2-20,h/2-60,72,white);
    if(app.warning_time>0)DrawText(app.warning==RR_RECOVERY_SIGNAL_LOST?"SIGNAL LOST":"WRONG WAY",w/2-100,110,22,magenta);
    else if(app.message_time>0)DrawText("CHECKPOINT CLEARED",w/2-151,110,22,(Color){143,255,184,255});
    if(!app.scene.car_loaded)DrawText("FALLBACK VEHICLE",w-212,251,14,muted);
    if(app.session.phase==RR_SESSION_PAUSED){
        DrawRectangle(0,0,w,h,Fade(ink,.86f));int x=w/2-190,y=h/2-110;
        DrawText("FLIGHT PAUSED",x,y,38,white);
        if(app.overloaded)DrawText("Rendering was slow. Flight paused.",x,y+47,17,magenta);
        if(button((Rectangle){(float)x,(float)y+72,380,56},"RESUME",true)){rr_session_resume(&app.session);app.overloaded=false;app.slow_frames=0;if(app.session.phase==RR_SESSION_FINISHED||app.session.phase==RR_SESSION_RESULTS)EnableCursor();else capture();}
        if(button((Rectangle){(float)x,(float)y+144,380,56},"MAIN MENU",false)){rr_session_menu(&app.session);app.menu_policy=(RRMenuPolicy){0};EnableCursor();reset_car(true);}
    }
}

static void results_ui(void)
{
    int w=GetScreenWidth(),h=GetScreenHeight(),x=w/2-240,y=h/2-190;
    bool failed=app.session.phase==RR_SESSION_ERROR;
    DrawText(failed?"SESSION UNAVAILABLE":"TWO-LAP TRIAL FINISHED",x,y,30,white);
    if(failed)DrawText("Lap history could not be stored. Return to menu.",x,y+55,17,magenta);
    else if(app.session.lap_count==2){
        RRResultSummary result=rr_finish_summary(app.session.lap_times,app.session.lap_count);
        double total=result.total;
        DrawText(TextFormat("TOTAL  %.3f S",total),x,y+55,27,cyan);
        DrawText(TextFormat("LAP 1  %.3f S",app.session.lap_times[0]),x,y+100,20,result.best==0?cyan:white);
        DrawText(TextFormat("LAP 2  %.3f S",app.session.lap_times[1]),x,y+132,20,result.best==1?cyan:white);
        DrawText("OFFLINE / NO RIVALS, REWARDS OR PROGRESSION",x,y+177,16,muted);
    }
    if(!failed && key_hit(KEY_ENTER)){start_flight(app.selected_mode);return;}
    if(key_hit(KEY_ESCAPE)||key_hit(KEY_M)){rr_session_menu(&app.session);app.menu_policy=(RRMenuPolicy){0};EnableCursor();reset_car(true);return;}
    if(!failed && button((Rectangle){(float)x,(float)y+225,480,56},"TRIAL AGAIN [ENTER]",true))start_flight(app.selected_mode);
    if(button((Rectangle){(float)x,(float)y+295,480,56},"MAIN MENU",false)){
        rr_session_menu(&app.session);app.menu_policy=(RRMenuPolicy){0};EnableCursor();reset_car(true);
    }
}

static void frame(void)
{
    read_key_events();
    double frame_start=GetTime();
    double raw_dt=app.smoke?1.0/60.0:GetFrameTime();
    double dt=rr_frame_dt(raw_dt);
    app.frame++;
    if(IsWindowMinimized() || GetScreenWidth()<=0 || GetScreenHeight()<=0){
        app.menu_policy.armed=false;
        if(rr_session_pause(&app.session))EnableCursor();
        PollInputEvents();
#if !defined(PLATFORM_WEB)
        WaitTime(.1);
#endif
        return;
    }
    bool focused=IsWindowFocused();
    bool finishing=app.session.phase==RR_SESSION_FINISHED||app.session.phase==RR_SESSION_RESULTS;
    if(finishing && !focused && rr_session_pause(&app.session))EnableCursor();
    target_fps(app.smoke?60:!focused?5:30);
    if(focused && key_hit(KEY_F11))ToggleFullscreen();
    bool pause_key=key_hit(KEY_ESCAPE)||key_hit(KEY_P)||pad_pressed(GAMEPAD_BUTTON_MIDDLE_RIGHT);
    if((app.session.phase==RR_SESSION_ACTIVE || app.session.phase==RR_SESSION_COUNTDOWN) && (pause_key || (!app.smoke && !focused))){rr_session_pause(&app.session);EnableCursor();}
    else if(app.session.phase==RR_SESSION_PAUSED && pause_key && focused){rr_session_resume(&app.session);app.overloaded=false;app.slow_frames=0;if(app.session.phase==RR_SESSION_FINISHED||app.session.phase==RR_SESSION_RESULTS)EnableCursor();else capture();}
    if(app.smoke && app.frame==30)start_flight(RR_RACE_TIME_ATTACK);
    if(app.load_requested && (focused || app.smoke)){
        app.load_requested=false;
        TraceLog(LOG_INFO,"STATE: scene load requested; fallback=%d glow=%d",app.fallback,app.glow);
        if(rr_scene_load(&app.scene,&app.corridor,app.fallback,app.glow)){
            app.scene_ready=true;start_flight(app.selected_mode);
        }else{
            TraceLog(LOG_ERROR,"STATE: scene load failed; returning to 2D menu");
            rr_scene_unload(&app.scene);app.failed=true;app.session.phase=RR_SESSION_MENU;EnableCursor();
        }
    }
#if defined(RR_FINISH_VALIDATION)
    if(app.session.phase==RR_SESSION_ACTIVE){
        if(app.session.race.next_checkpoint<app.track.gate_count){
            const RRGate *gate=&app.track.gates[app.session.race.next_checkpoint];
            rr_car_reset(&app.car,rr_sub(gate->pos,rr_scale(gate->forward,.1)),gate->yaw,35);
            app.car.pitch=atan2(gate->forward.y,hypot(gate->forward.x,gate->forward.z));
        }else rr_car_reset(&app.car,rr_v3(0,118,.1),0,35);
        rr_recovery_rebase(&app.recovery,&app.track,app.car.pos);
    }
#endif
#if defined(RR_COLLISION_VALIDATION)
    if(app.session.phase==RR_SESSION_ACTIVE){
        static bool wall_placed=false;
        if(!wall_placed){rr_car_reset(&app.car,rr_v3(20,131,0),-RR_PI/2,60);rr_recovery_rebase(&app.recovery,&app.track,app.car.pos);wall_placed=true;TraceLog(LOG_INFO,"VALIDATION: scripted start-post approach");}
    }
#endif
    if(app.session.phase==RR_SESSION_ACTIVE){
        RRInput in=app.smoke?(RRInput){1,0,0,.1,false}:controls(dt);
#if defined(RR_FINISH_VALIDATION) || defined(RR_COLLISION_VALIDATION)
        in=(RRInput){0};
#endif
        if(key_hit(KEY_C)||pad_pressed(GAMEPAD_BUTTON_RIGHT_FACE_UP))app.camera_mode=1-app.camera_mode;
        app.spin_cooldown=fmax(0,app.spin_cooldown-dt);
        if((key_hit(KEY_F)||pad_pressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) && app.spin_cooldown==0){rr_car_dodge(&app.car,in.steer<0?-1:1);app.spin_cooldown=RR_SPIN_COOLDOWN;}
        app.message_time=fmax(0,app.message_time-dt);
        unsigned events=rr_session_step(&app.session,&app.car,app.track.gates,app.track.gate_count,in,raw_dt);
#if defined(RR_COLLISION_VALIDATION)
        if(app.car.impact>0){TraceLog(LOG_INFO,"VALIDATION: wall impact=%.6f x=%.6f vx=%.6f speed=%.6f",app.car.impact,app.car.pos.x,app.car.vel.x,app.car.speed);rr_session_pause(&app.session);EnableCursor();}
#endif
        if(events&RR_RACE_CHECKPOINT)app.message_time=2;
        if(events&RR_RACE_LAP)rr_road_picker_lap(&app.picker,app.session.race.last_lap_time);
        if(events&RR_RACE_FINISH){TraceLog(LOG_INFO,"STATE: offline two-lap trial finished in %.3f s",app.session.race.finish_time);app.message_time=0;app.warning_time=0;EnableCursor();}
        if(app.session.phase==RR_SESSION_ACTIVE){
            unsigned recovery=rr_recovery_update(&app.recovery,&app.track,app.car.pos,app.car.vel,34,dt,app.warning_time,app.session.race.mode==RR_RACE_FREE_ROAM);
            if(recovery&RR_RECOVERY_WRONG_WAY){app.warning=RR_RECOVERY_WRONG_WAY;app.warning_time=.8;}
            if(recovery&RR_RECOVERY_SIGNAL_LOST){app.warning=RR_RECOVERY_SIGNAL_LOST;app.warning_time=1.2;}
            if(recovery&RR_RECOVERY_RESPAWN){TraceLog(LOG_INFO,"RECOVERY: signal lost timeout; returning to safe transform");reset_car(false);}
            if(key_hit(KEY_R)||pad_pressed(GAMEPAD_BUTTON_MIDDLE_LEFT))reset_car(false);
        }
    }else if(app.session.phase==RR_SESSION_COUNTDOWN || app.session.phase==RR_SESSION_FINISHED || app.session.phase==RR_SESSION_RESULTS){
        unsigned events=rr_session_step(&app.session,&app.car,app.track.gates,app.track.gate_count,finishing?rr_finish_controls(&app.track,&app.car,app.recovery.sample):(RRInput){0},raw_dt);
        if(finishing){
            unsigned recovery=rr_recovery_update(&app.recovery,&app.track,app.car.pos,app.car.vel,34,dt,app.warning_time,false);
            if(recovery&RR_RECOVERY_RESPAWN)reset_car(false);
        }
        if(events&RR_SESSION_SHOW_RESULTS)TraceLog(LOG_INFO,"STATE: results visible; lap history frozen");
        if(events&RR_SESSION_GO){center_aim();app.message_time=0;TraceLog(LOG_INFO,"STATE: GO");}
    }
    if(app.session.phase==RR_SESSION_ERROR)EnableCursor();
    update_camera(dt);
#if defined(RR_HERO_VIEW)
    if(app.scene_ready){
        static bool logged=false;if(!logged){TraceLog(LOG_INFO,"VALIDATION: camera-only control-tower inspection; no actor/progress changes");logged=true;}
        app.camera=(Camera3D){{-470,180,500},{-72.7f,-40,10},{0,1,0},45,CAMERA_PERSPECTIVE};
    }
#endif
    if((app.session.phase==RR_SESSION_ACTIVE || app.session.phase==RR_SESSION_COUNTDOWN || app.session.phase==RR_SESSION_FINISHED || app.session.phase==RR_SESSION_RESULTS) && (focused || app.smoke) && app.scene_ready){
        if(rr_scene_begin(&app.scene,app.camera)){
            rr_scene_world(&app.scene,&app.track,&app.course,&app.shared,app.camera,app.session.race.finished?-1:app.session.race.next_checkpoint,GetTime());
            rr_scene_car(&app.scene,&app.car);
            rr_scene_finish(&app.scene);
        }else{
            TraceLog(LOG_ERROR,"STATE: render target unavailable; returning to 2D menu");
            rr_scene_unload(&app.scene);app.scene_ready=false;app.failed=true;app.session.phase=RR_SESSION_MENU;EnableCursor();
            BeginDrawing();ClearBackground(ink);
        }
    }else{
        BeginDrawing();ClearBackground(ink);
        DrawRectangleGradientV(0,0,GetScreenWidth(),GetScreenHeight(),ink,(Color){30,19,58,255});
    }
    if(app.session.phase==RR_SESSION_MENU)menu_ui();
    else if(app.session.phase==RR_SESSION_RESULTS || app.session.phase==RR_SESSION_ERROR){DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),Fade(ink,.9f));results_ui();}
    else {flight_ui();if(app.session.phase==RR_SESSION_FINISHED)DrawText("FINISH",GetScreenWidth()/2-80,GetScreenHeight()/2-60,48,cyan);}
    app.picker_notice=fmax(0,app.picker_notice-dt);
    if(app.session.phase!=RR_SESSION_MENU)app.warning_time=fmax(0,app.warning_time-dt);
#if defined(RR_FINISH_VALIDATION)
    DrawText("VALIDATION FIXTURE / SCRIPTED GATES / NO PROGRESS",20,5,14,magenta);
#endif
#if defined(RR_COLLISION_VALIDATION)
    DrawText("COLLISION VALIDATION / SCRIPTED APPROACH / NO PROGRESS",20,5,14,magenta);
#endif
#if defined(RR_HERO_VIEW)
    DrawText("HERO ASSET CAMERA VALIDATION / NO PROGRESS",20,5,14,magenta);
#endif
    EndDrawing();
#if defined(RR_FINISH_VALIDATION) && !defined(PLATFORM_WEB)
    if(app.session.phase==RR_SESSION_RESULTS){
        static bool captured=false;static int result_frames=0;result_frames++;
        if(!captured && result_frames>=3){if(!DirectoryExists("captures"))MakeDirectory("captures");TakeScreenshot("captures/finish-results-fixture.png");captured=true;TraceLog(LOG_INFO,"VALIDATION: scripted gate placements; not a human-driven race");}
    }
#endif

    double frame_seconds=GetTime()-frame_start;
    if(frame_seconds>app.worst_frame)app.worst_frame=frame_seconds;
    if((app.session.phase==RR_SESSION_ACTIVE || app.session.phase==RR_SESSION_COUNTDOWN || app.session.phase==RR_SESSION_FINISHED || app.session.phase==RR_SESSION_RESULTS) && !app.smoke && rr_render_overloaded(frame_seconds,&app.slow_frames)){
        TraceLog(LOG_WARNING,"PERFORMANCE: 10 consecutive slow frames; pausing flight (last %.1f ms)",frame_seconds*1000);
        rr_session_pause(&app.session);app.overloaded=true;app.slow_frames=0;EnableCursor();
    }
    if(GetTime()-app.diagnostic_time>=5){
        TraceLog(LOG_INFO,"HEARTBEAT: frame=%d mode=%d window=%dx%d target=%dx%d worst-frame=%.1fms focused=%d",app.frame,app.session.phase,GetScreenWidth(),GetScreenHeight(),app.scene.target.texture.width,app.scene.target.texture.height,app.worst_frame*1000,focused);
        TraceLog(LOG_INFO,"CITY_CACHE: resident=%d canonical=%d cap=%d",app.scene.city_resident,app.scene.canonical_resident,RR_BUILDING_BUDGET);
        app.diagnostic_time=GetTime();app.worst_frame=0;
    }
    if(app.smoke && (app.frame==20 || app.frame==150)){
#if !defined(PLATFORM_WEB)
        if(!DirectoryExists("captures"))MakeDirectory("captures");
        TakeScreenshot(app.frame==20?(app.fallback?"captures/menu-fallback.png":"captures/menu.png"):(app.fallback?"captures/flight-fallback.png":"captures/flight.png"));
#endif
    }
#if defined(PLATFORM_WEB)
    if(app.smoke && app.frame==151){
        EM_ASM({Module.rrSmoke={};Module.rrSmoke.frame=$0;Module.rrSmoke.carLoaded=!!$1;Module.rrSmoke.speed=$2;Module.rrSmoke.mode=$3;},app.frame,app.scene.car_loaded,app.car.speed,app.session.phase);
        emscripten_pause_main_loop();
    }
#endif
}

int main(int argc,char **argv)
{
    for(int i=1;i<argc;i++){if(strcmp(argv[i],"--smoke-test")==0)app.smoke=true;if(strcmp(argv[i],"--fallback")==0)app.fallback=true;if(strcmp(argv[i],"--glow")==0)app.glow=true;}
#if !defined(PLATFORM_WEB)
    ChangeDirectory(GetApplicationDirectory());
    if(DirectoryExists("logs") || MakeDirectory("logs")==0)fopen_s(&diagnostic_log,"logs/roadsters.log","a");
    SetTraceLogCallback(log_message);
#endif
    TraceLog(LOG_INFO,"SESSION: bounded renderer, 2D startup, 30 FPS, MSAA off, smoke=%d fallback=%d glow=%d",app.smoke,app.fallback,app.glow);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_VSYNC_HINT);
#if defined(RR_HERO_VIEW)
    InitWindow(960,720,"Roadsters Race - HERO CAMERA VALIDATION");
    TraceLog(LOG_WARNING,"VALIDATION: camera-only hero inspection");
#elif defined(RR_COLLISION_VALIDATION)
    InitWindow(960,720,"Roadsters Race - COLLISION VALIDATION FIXTURE");
    TraceLog(LOG_WARNING,"VALIDATION: isolated collision approach, no progression writes");
#elif defined(RR_FINISH_VALIDATION)
    InitWindow(960,720,"Roadsters Race - FINISH VALIDATION FIXTURE");
    TraceLog(LOG_WARNING,"VALIDATION: isolated scripted gate fixture, no progression writes");
#else
    InitWindow(960,720,"Roadsters Race - First Flight");
#endif
    if(!IsWindowReady()){fputs("Cannot create the game window.\n",stderr);return 1;}
    SetWindowMinSize(960,720);SetExitKey(KEY_NULL);
#if defined(PLATFORM_WEB)
    SetTargetFPS(0);
#else
    target_fps(app.smoke?60:30);
#endif
    rlSetClipPlanes(.1,2500);
    app.session.phase=RR_SESSION_MENU;app.camera.up=(Vector3){0,1,0};app.camera.projection=CAMERA_PERSPECTIVE;app.camera_fov=68;
    if(!rr_corridor_init_all(&app.corridor,&app.track)||!rr_road_picker_init(&app.picker,&app.track)){
        TraceLog(LOG_ERROR,"All-route corridor/track initialization failed.");CloseWindow();return 1;
    }
    if(!rr_shared_scenery_plan(&app.shared,&app.track,&app.corridor)){TraceLog(LOG_ERROR,"Shared Skyline scenery initialization failed");CloseWindow();return 1;}
    TraceLog(LOG_INFO,"SHARED: platforms=%d columns=%d trees=%d props=%d colliders=%d",app.shared.platforms,app.shared.columns,app.shared.trees,app.shared.prop_count,app.shared.collider_count);
    if(!rr_canonical_city_plan(&app.city_plan,&app.corridor)){TraceLog(LOG_ERROR,"Canonical core exceeds city budget");CloseWindow();return 1;}
    TraceLog(LOG_INFO,"CITY_CORE: prototype kept=%d removed=%d canonical=%d total=%d",app.city_plan.prototype_kept,app.city_plan.prototype_removed,app.city_plan.canonical_count,app.city_plan.count);
    rr_ui_load(&app.ui);
    reset_car(true);
    TraceLog(LOG_INFO,"WORLD: union ready points=%d coarse=%d gates=%d cells=%d",app.corridor.point_count,app.corridor.coarse_count,app.corridor.gate_count,app.corridor.occupied_cells);
    TraceLog(LOG_INFO,"STARTUP: 2D menu ready; track %.3f m, %d samples; nine-road picker ready; offline open=1; scene deferred",app.track.length,app.track.sample_count);
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(frame,0,1);
#else
    while(!WindowShouldClose() && !app.quit && (!app.smoke || app.frame<151))frame();
    bool asset_failure=app.smoke && !app.fallback && !app.scene.car_loaded;
    if(app.smoke)printf("FIRST_FLIGHT_SMOKE: %d frames, model=%s, speed=%.3f m/s, output=captures/\n",app.frame,app.scene.car_loaded?"loaded":"fallback",app.car.speed);
    rr_session_dispose(&app.session);rr_scene_unload(&app.scene);rr_ui_unload(&app.ui);CloseWindow();
    TraceLog(LOG_INFO,"SESSION: clean shutdown");
    if(diagnostic_log){fclose(diagnostic_log);diagnostic_log=NULL;}
    return (asset_failure || app.failed)?1:0;
#endif
}
