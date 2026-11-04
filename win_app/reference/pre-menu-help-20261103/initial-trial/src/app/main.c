#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "rr_scene.h"
#include "rr_session.h"
#include "rr_finish.h"
#include "rr_race_warning.h"
#include "rr_world_plan.h"
#include "rr_canonical_city.h"
#include "rr_menu_policy.h"
#include "rr_recovery.h"
#include "rr_road_picker.h"
#include "rr_key_events.h"
#include "rr_input_mix.h"
#include "rr_first_person.h"
#include "rr_chase_camera.h"
#include "rr_camera_motion.h"
#include "rr_camera_fx.h"
#include "rr_minimap.h"
#include "rr_render_policy.h"
#include "rr_ui.h"
#include "rr_race_ui.h"
#include "rr_cockpit.h"
#include "rr_pause_ui.h"
#include "rr_start_cue.h"
#include "rr_sub_notice.h"
#include "rr_subtext.h"
#include "rr_target_guidance.h"
#include "rr_hud_message.h"
#include "rr_mouse_aim.h"
#include "rr_spin_control.h"
#include "rr_nitro_call.h"
#include "rr_boost_meter.h"
#include "rr_circle_clip.h"
#include "rr_mouse_capture.h"
#include "rr_reticle.h"
#include "rr_best_readout.h"
#include "rr_player_controls.h"
#include "rr_view_controls.h"
#include "rr_minimap_marks.h"
#include "rr_recovery_feedback.h"
#include "rr_ui_layers.h"
#include "rr_pause_style.h"
#include "rr_spin_indicator.h"
#include "rr_css_font_metrics.h"
#include "rr_announcement.h"
#include "rr_minimap_background.h"
#include "rr_target_pulse.h"
#include "rr_arrow_art.h"
#include "rr_capture_prompt.h"
#include "rr_boost_ui.h"
#include "rr_spin_readout.h"
#include "rr_results_layout.h"
#include "rr_showcase.h"
#include "rr_pause_scene.h"
#include "rr_button_text.h"
#include "rr_menu_header.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

typedef struct RRApp {
    RRTrack track; RRCorridor corridor; RRWorld world; RRCityPlan city_plan; RRCourse course; RRSharedScenery shared; RRCar car; RRScene scene; RRSession session; RRRaceMode selected_mode; RRMenuPolicy menu_policy; RRRecovery recovery; RRRoadPicker picker; RRKeyEvents key_events; RRMinimap minimap; RRTargetPulse target_pulse;
    Camera3D camera; RRMouseCapture mouse_capture; RRMouseClickCapture capture_click; RRBoostMeter boost_meter; RRNitroCall nitro_call; RRMouseSettings mouse_settings; bool mouse_sens_drag; RRHUDMessage message; RRNoticeFade message_fade; double message_opacity; RRSubNotice sub_notice; RRNoticeFade sub_fade; double sub_opacity; RRSubArt sub_art; RRArrowArt arrow_art; RRCameraFX camera_fx; RRStartCue start_cue; RRUI ui; float road_scroll; RRCockpitArt cockpit_art;
    bool interior_requested,cockpit_visible,first_person_visible,camera_looking; double cockpit_steer,cockpit_control;
    double aim_yaw,aim_pitch,look_yaw,look_pitch,camera_yaw,camera_pitch,camera_fov;RRVec3 aim_direction;
    RRShowcase showcase;RRSceneClock scene_clock;
    double spin_cooldown,picker_notice;
    int camera_mode,frame; size_t ui_best_lap;
    bool smoke,fallback,failed,scene_ready,load_requested,glow,quit,overloaded;
    int slow_frames,target_fps;
    double diagnostic_time,worst_frame,pause_at_ms;
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
static void release_mouse(void){app.capture_click.armed=false;EnableCursor();rr_mouse_capture_change(&app.mouse_capture,false,GetTime()*1000);}
static void capture(void){DisableCursor();rr_mouse_capture_change(&app.mouse_capture,true,GetTime()*1000);center_aim();}
static RRPlayerControlState player_control_state(void)
{
    return (RRPlayerControlState){&app.session,&app.car,&app.recovery,&app.track,&app.aim_yaw,&app.aim_pitch,&app.spin_cooldown,&app.camera_fx.kick};
}
static void reset_car(bool beginning)
{
    if(beginning){
        rr_car_reset(&app.car,rr_v3(0,115.5,42),0,0);rr_recovery_init(&app.recovery,&app.track,app.car.pos);
        app.camera_yaw=app.car.yaw;app.camera_pitch=0;center_aim();
    }else{RRPlayerControlState state=player_control_state();rr_player_controls_respawn(&state);}
    rr_minimap_init(&app.minimap,&app.track);
}
static void player_recovery_feedback(double dt)
{
    unsigned events=rr_recovery_feedback_update(app.session.phase,&app.recovery,&app.track,&app.car,&app.message,34,dt,app.session.race.mode==RR_RACE_FREE_ROAM);
    if(events&RR_RECOVERY_RESPAWN){TraceLog(LOG_INFO,"RECOVERY: signal lost timeout; returning to safe transform");reset_car(false);}
}
static void start_flight(RRRaceMode mode)
{
    if(app.failed)return;
    app.selected_mode=mode;app.picker.mode=mode;
    if(!app.scene_ready){app.load_requested=true;return;}
    TraceLog(LOG_INFO,"STATE: start offline session mode=%d",mode);
    if(!rr_road_picker_start(&app.picker,&app.track,&app.session,&app.car)){app.failed=true;return;}
    if(!rr_course_plan(&app.course,&app.track)||!rr_world_plan_shared(&app.world,&app.city_plan,&app.course,&app.shared,mode==RR_RACE_FREE_ROAM)){app.failed=true;rr_session_menu(&app.session);release_mouse();return;}
    app.session.world=&app.world;
    TraceLog(LOG_INFO,"COURSE: road=%d dots=%d items=%d colliders=%d beacons=%d barriers=%d signs=%d chevrons=%d",app.track.road->id,app.course.dot_count,app.course.item_count,app.course.collider_count,app.course.beacons,app.course.barriers,app.course.signs,app.course.chevrons);
    TraceLog(LOG_INFO,"COLLISION: native city/start/road world ready shapes=%d road=%d",app.world.count,app.track.road->id);
    rr_minimap_init(&app.minimap,&app.track);
    rr_recovery_init(&app.recovery,&app.track,app.car.pos);
    app.spin_cooldown=0;app.ui_best_lap=0;rr_start_cue_restart(&app.start_cue);
    app.look_yaw=0;app.look_pitch=0;app.camera_looking=false;rr_camera_fx_restart(&app.camera_fx);
    app.camera_yaw=0;app.camera_pitch=0;app.slow_frames=0;app.overloaded=false;
    /* Existing diagnostic smoke path remains distinct from normal countdown. */
    if(app.smoke){app.session.phase=RR_SESSION_ACTIVE;center_aim();}else capture();
}
/* Browser selects the first non-null controller, not necessarily slot zero. */
static int input_pad(void){for(int p=0;p<4;p++)if(IsGamepadAvailable(p))return p;return -1;}
static bool pad_pressed(int button){int p=input_pad();return p>=0&&IsGamepadButtonPressed(p,button);}
static bool pad_down(int button){int p=input_pad();return p>=0&&IsGamepadButtonDown(p,button);}

static RRInput controls(void)
{
    const int codes[]={KEY_W,KEY_S,KEY_D,KEY_RIGHT,KEY_A,KEY_LEFT,KEY_UP,KEY_DOWN,KEY_E,KEY_Q,KEY_LEFT_SHIFT,KEY_RIGHT_SHIFT};
    unsigned held=0;for(unsigned i=0;i<sizeof(codes)/sizeof(codes[0]);i++)if(IsKeyDown(codes[i]))held|=1u<<i;
    RRInputPad pad={0};int p=input_pad();
    if(p>=0){
        pad.connected=true;pad.x=GetGamepadAxisMovement(p,GAMEPAD_AXIS_LEFT_X);pad.y=GetGamepadAxisMovement(p,GAMEPAD_AXIS_LEFT_Y);
        pad.throttle=rr_clamp((GetGamepadAxisMovement(p,GAMEPAD_AXIS_RIGHT_TRIGGER)+1)*.5,0,1);
        pad.brake=rr_clamp((GetGamepadAxisMovement(p,GAMEPAD_AXIS_LEFT_TRIGGER)+1)*.5,0,1);
        pad.boost=IsGamepadButtonDown(p,GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    }
    RRInput in=rr_input_mix(held,pad);
    Vector2 delta=GetMouseDelta();
    if(!rr_mouse_capture_allows(&app.mouse_capture,GetTime()*1000))delta=(Vector2){0};
    RRMouseAim aim={app.aim_yaw,app.aim_pitch,app.look_yaw,app.look_pitch,false,{0}};
    in=rr_mouse_aim_step(&aim,app.mouse_settings,app.car.yaw,app.car.pitch,in,delta.x,delta.y,IsMouseButtonDown(MOUSE_BUTTON_RIGHT));
    app.aim_yaw=aim.yaw;app.aim_pitch=aim.pitch;app.look_yaw=aim.look_yaw;app.look_pitch=aim.look_pitch;app.camera_looking=aim.looking;app.aim_direction=aim.direction;
    return in;
}

/* Native fair side choice only; not a claim of browser-global RNG identity. */
static double spin_random_side(void *context){(void)context;return GetRandomValue(0,1)?.75:.25;}
static RRInput finished_controls(void)
{
    RRFinishAim aim;
    RRInput input=rr_finish_player_controls(&app.track,&app.car,app.recovery.sample,&aim);
    app.aim_yaw=aim.yaw;app.aim_pitch=aim.pitch;app.aim_direction=aim.direction;
    return input;
}
static void motion_camera_feedback(bool playing,bool was_boosting)
{
    if(!playing)return;
    if(rr_camera_fx_ignite(&app.camera_fx,was_boosting,app.car.boosting,app.session.race_time))rr_nitro_call_start(&app.nitro_call,GetTime());
    if(app.session.motion_stepped)rr_camera_fx_impact(&app.camera_fx,app.session.motion.impact);
}

static void pause_showcase_input(void)
{
    if(app.session.phase!=RR_SESSION_PAUSED||!IsWindowFocused()){rr_showcase_up(&app.showcase);return;}
    Vector2 m=GetMousePosition();
    bool inside=m.x>=0&&m.y>=0&&m.x<GetScreenWidth()&&m.y<GetScreenHeight();
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&inside){
        RRUIOverlayLayout l=rr_ui_pause_layout((float)GetScreenWidth(),(float)GetScreenHeight());bool button=false;
        for(int i=0;i<3;i++)if(rr_ui_hit(l.actions[i],m.x,m.y,14))button=true;
        rr_showcase_down(&app.showcase,m.x,m.y,button);
    }
    if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT))rr_showcase_up(&app.showcase);
    else rr_showcase_move(&app.showcase,m.x,m.y);
    /* raylib wheel up is positive; DOM deltaY up is negative. Source uses sign only. */
    float wheel=GetMouseWheelMove();if(inside&&wheel!=0)rr_showcase_wheel(&app.showcase,-wheel);
}

static void update_camera(double dt)
{
    app.cockpit_visible=false;app.first_person_visible=false;
    Vector3 car=vector(app.car.pos);
    if(app.session.phase==RR_SESSION_MENU){
        app.camera.position=Vector3Add(car,(Vector3){10,3.8f,-12});
        app.camera.target=Vector3Add(car,(Vector3){5,.3f,3.8f});app.camera.fovy=42;
        return;
    }
    if(app.session.phase==RR_SESSION_PAUSED){
        RRShowcasePose pose=rr_showcase_pause_step(&app.showcase,app.car.pos,app.car.yaw,
            rr_v3(app.camera.position.x,app.camera.position.y,app.camera.position.z),app.camera.fovy,dt);
        app.camera.position=vector(pose.position);app.camera.target=vector(pose.target);app.camera.up=(Vector3){0,1,0};app.camera.fovy=(float)pose.fov;
        return;
    }
    bool chase_phase=app.session.phase==RR_SESSION_ACTIVE||app.session.phase==RR_SESSION_COUNTDOWN||app.session.phase==RR_SESSION_FINISHED||app.session.phase==RR_SESSION_RESULTS;
    RRCameraMotion motion={app.camera_yaw,app.camera_pitch,app.look_yaw,app.look_pitch};
    rr_camera_motion_step(&motion,app.car.yaw,app.car.pitch,app.camera_looking||!chase_phase,dt);
    app.camera_yaw=motion.yaw;app.camera_pitch=motion.pitch;app.look_yaw=motion.look_yaw;app.look_pitch=motion.look_pitch;
    double orbit=rr_camera_orbit(app.look_yaw,app.look_pitch);
    RRFirstPerson view=rr_chase_pose(&app.car,app.camera_mode,true,app.camera_yaw,app.camera_pitch,app.look_yaw,app.look_pitch);

    double sf=fmin(1.4,fmax(0,app.car.speed)/RR_CAR_MAX);
    app.camera_fov+=(68+15*fmin(1,sf)+9*app.car.nitro-app.camera_fov)*(1-exp(-4*dt));
    app.camera.fovy=(float)app.camera_fov;
    bool flight=rr_view_flight_phase(app.session.phase);
    app.first_person_visible=flight&&rr_first_person_active(app.camera_mode,true,app.look_yaw,app.look_pitch);
    if(app.first_person_visible){
        view=rr_first_person_pose(&app.car,orbit);
    }

#if defined(RR_COCKPIT_PREVIEW)
    app.cockpit_visible=rr_cockpit_visible(app.interior_requested,app.session.phase,app.look_yaw,app.look_pitch,app.cockpit_art.loaded==RR_COCKPIT_LAYER_COUNT&&!app.cockpit_art.failed);
    if(app.cockpit_visible){
        app.first_person_visible=false;RRCKSeat seat=rr_cockpit_seat(&app.car);view.position=seat.position;view.forward=seat.forward;
        app.camera_fov+=(seat.extra_fov)*(1-exp(-4*dt));app.camera.fovy=(float)app.camera_fov;
    }
#endif
    if(chase_phase){RRCameraPulse pulse=rr_camera_fx_step(&app.camera_fx,app.car.nitro,dt,GetTime()*1000);view=rr_camera_fx_apply(view,pulse);app.camera.fovy=(float)(app.camera_fov+pulse.fov_add);}
    app.camera.position=vector(view.position);app.camera.target=vector(rr_add(view.position,rr_scale(view.forward,20)));app.camera.up=vector(view.up);
}

static void open_roads(RRRaceMode mode)
{
    /* Persistence/profile adapters are not connected. Do not import or grant progress. */
    rr_road_picker_open(&app.picker,mode,1,0);
    app.menu_policy=(RRMenuPolicy){0};app.picker_notice=0;app.road_scroll=0;release_mouse();
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
        char hint[64];rr_ui_best_row(hint,&app.picker,id);
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

static void menu_brand_line(RRUIMenuLayout l,const char *text,int line,Color body,Color shadow)
{
    float baseline=(float)rr_menu_brand_baseline(l,line);
    /* Retain bounded four-offset glow passes; exact18/22px CSS blur is open. */
    for(int i=0;i<4;i++){float dx=i==0?-2.f:i==1?2.f:0,dy=i==2?-2.f:i==3?2.f:0;rr_ui_text_css(&app.ui,text,l.x+dx,baseline+dy,l.brand,4,900,Fade(shadow,.12f),false);}
    rr_ui_text_css(&app.ui,text,l.x,baseline,l.brand,4,900,body,false);
}

static void menu_help_ui(RRUIMenuLayout l,int height)
{
    Font body=app.ui.loaded?app.ui.body:GetFontDefault();float pixels=app.ui.loaded?(float)rr_css_font_size(15,0):15;
    float line=rr_menu_help_line_height((float)height);
    for(int row=0;row<3;row++){
        float baseline=(float)rr_menu_help_baseline(l.footer_y+61+row*line,line),pen=l.x;
        for(int i=0;i<rr_menu_help_count(row);i++){
            const RRHelpRun *run=rr_menu_help_run(row,i);const RRHelpKey *key=rr_menu_help_key(run->key);
            if(key){
                RRHelpBox b=rr_menu_help_key_box(run->key,pen,baseline);Rectangle box={b.x,b.y,b.w,b.h};float rounding=6/fminf(b.w,b.h);
                DrawRectangleRounded(box,rounding,6,(Color){25,230,255,31});
                DrawRectangleRoundedLinesEx(box,rounding,6,1,cyan);
                rr_ui_text_css(&app.ui,key->text,b.x+7,baseline,12,0,700,WHITE,false);
                pen+=(float)rr_menu_help_advance(run,0);
            }else{
                rr_ui_text_css(&app.ui,run->text,pen,baseline,15,0,0,(Color){153,187,221,255},false);
                pen+=(float)rr_menu_help_advance(run,MeasureTextEx(body,run->text,pixels,0).x);
            }
        }
    }
}

static void menu_ui(void)
{
    if(app.picker.open){road_picker_ui();return;}
    int w=GetScreenWidth(),h=GetScreenHeight();RRUIMenuLayout l=rr_ui_menu_layout((float)w,(float)h);
    DrawRectangleGradientH(0,0,(int)(w*.7f),h,(Color){3,4,20,235},(Color){3,4,20,0});
    menu_brand_line(l,"ROADSTERS",0,WHITE,cyan);
    menu_brand_line(l,"RACE",1,magenta,magenta);
    const RRRoadRow *road=&app.picker.rows[app.picker.current_road-1];
    Font menu_face=app.ui.loaded?app.ui.bold:GetFontDefault();
    float track_baseline=(float)rr_menu_track_baseline(l),track_pixels=app.ui.loaded?(float)rr_css_font_size(13,700):13;
    float track_origin=app.ui.loaded?(float)rr_css_raylib_origin(track_baseline,13,700,menu_face.baseSize):track_baseline-13;
    rr_ui_text_fit(&app.ui,TextFormat("ROAD %d / %s / %d GATES",app.picker.current_road,rr_road_by_id(app.picker.current_road)->name,road->gates),l.x,track_origin,l.width,track_pixels,3,700,cyan);
    float button_size=(float)rr_menu_button_css_size(h);
    bool trial_mouse=rr_ui_button_ex(&app.ui,l.actions[0],"RACE / OFFLINE TRIAL",true,!app.failed,false,0,button_size,24);
    bool time_mouse=rr_ui_button_ex(&app.ui,l.actions[1],"TIME ATTACK / SOLO",false,!app.failed,false,1,button_size,24);
    rr_ui_button_ex(&app.ui,l.actions[2],"FREE ROAM / ONLINE",false,false,false,2,button_size,24);
    rr_ui_button_ex(&app.ui,l.actions[3],"WORLD RANKING",false,false,false,3,button_size,24);
    rr_ui_button_ex(&app.ui,l.actions[4],"SHOP",false,false,false,4,button_size,24);
    bool enter=key_hit(KEY_ENTER),pad=pad_pressed(GAMEPAD_BUTTON_MIDDLE_RIGHT);
    bool held=IsMouseButtonDown(MOUSE_BUTTON_LEFT)||IsKeyDown(KEY_ENTER)||pad_down(GAMEPAD_BUTTON_MIDDLE_RIGHT);
    int action=app.failed?RR_MENU_NO_ACTION:rr_menu_request(&app.menu_policy,IsWindowFocused(),held,trial_mouse||enter||pad,time_mouse);
    if(action!=RR_MENU_NO_ACTION)open_roads(action==RR_MENU_START_TRIAL?RR_RACE_TWO_LAPS:RR_RACE_TIME_ATTACK);
    char record_clock[32];rr_ui_best_clock(record_clock,&app.picker,app.picker.current_road);
    float best_baseline=(float)rr_menu_best_baseline(l);
    rr_ui_text_css(&app.ui,"BEST LAP ",l.x,best_baseline,14,2,700,(Color){153,170,187,255},false);
    float best_pixels=app.ui.loaded?(float)rr_css_font_size(14,700):14;
    float best_x=l.x+MeasureTextEx(menu_face,"BEST LAP ",best_pixels,2).x+2;
    rr_ui_text_css(&app.ui,record_clock,best_x,best_baseline,14,2,900,magenta,false);
    float y=l.footer_y;
    rr_ui_text(&app.ui,app.failed?"SCENE UNAVAILABLE / CHECK LOG":"OFFLINE / ONLINE SERVICES NOT CONNECTED",l.x,y,12,1,500,app.failed?magenta:muted,false);
    rr_ui_text(&app.ui,"MOUSE SENS",l.x,y+28,10,1,500,muted,false);
    RRUIRect sens={l.x+102,y+22,150,24};Vector2 mouse=GetMousePosition();
    if(!IsWindowFocused()||!IsMouseButtonDown(MOUSE_BUTTON_LEFT))app.mouse_sens_drag=false;
    if(IsWindowFocused()&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mouse,(Rectangle){sens.x,sens.y,sens.w,sens.h}))app.mouse_sens_drag=true;
    if(app.mouse_sens_drag)
        app.mouse_settings.sensitivity=rr_mouse_sensitivity_at((mouse.x-sens.x)/sens.w);
    DrawRectangle((int)sens.x,(int)(sens.y+11),(int)sens.w,3,Fade(cyan,.3f));
    float knob=sens.x+(float)((app.mouse_settings.sensitivity-.3)/2.2)*sens.w;
    DrawCircleV((Vector2){knob,sens.y+12},5,cyan);
    rr_ui_text(&app.ui,TextFormat("%.2f",app.mouse_settings.sensitivity),sens.x+sens.w+8,y+27,11,0,700,cyan,false);
    RRUIRect invert={l.x+312,y+22,140,24};
    if(IsWindowFocused()&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mouse,(Rectangle){invert.x,invert.y,invert.w,invert.h}))app.mouse_settings.invert=!app.mouse_settings.invert;
    DrawRectangleLines((int)invert.x,(int)(invert.y+5),14,14,cyan);
    if(app.mouse_settings.invert)DrawRectangle((int)(invert.x+3),(int)(invert.y+8),8,8,cyan);
    rr_ui_text(&app.ui,"INVERT Y",invert.x+22,y+28,10,1,500,muted,false);
    menu_help_ui(l,h);
    float pending_y=y+rr_menu_help_pending((float)h),error_y=y+rr_menu_help_error((float)h);
    rr_ui_text_css(&app.ui,"RIVALS / COMBAT / ACCOUNT INTEGRATION PENDING",l.x,(float)rr_css_line_baseline(pending_y,14,11,500),11,0,500,muted,false);
    if(!app.ui.loaded)rr_ui_text_css(&app.ui,"THEME FONT LOAD FAILED",l.x,(float)rr_css_line_baseline(error_y,18,14,700),14,0,700,magenta,false);
#if !defined(PLATFORM_WEB)
    if(rr_ui_button(&app.ui,(RRUIRect){(float)w-168,(float)h-76,144,50},"QUIT",false,true,false,6)||key_hit(KEY_ESCAPE))app.quit=true;
#endif
}

static Vector2 map_point(RRVec3 p,RRUIRect box)
{
    double px,py;rr_minimap_point(&app.minimap,p.x,p.z,&px,&py);
    return (Vector2){box.x+(float)px*box.w,box.y+(float)py*box.h};
}
static void ui_map_line(Vector2 a,Vector2 b,RRUIRect box,float thickness,Color color)
{
    double ax=a.x,ay=a.y,bx=b.x,by=b.y;
    if(rr_ui_circle_segment(&ax,&ay,&bx,&by,box.x+box.w*.5f,box.y+box.h*.5f,box.w*.5f-thickness))
        DrawLineEx((Vector2){(float)ax,(float)ay},(Vector2){(float)bx,(float)by},thickness,color);
}
static void ui_map_triangle(const RRCirclePoint triangle[3],RRUIRect canvas,Color color)
{
    RRCirclePoint clipped[RR_CIRCLE_CLIP_MAX];
    int count=rr_circle_clip_triangle(triangle,canvas.x+canvas.w*.5,canvas.y+canvas.h*.5,canvas.w*.5,clipped);
    if(count<0){static bool logged=false;if(!logged){TraceLog(LOG_ERROR,"MINIMAP: invalid primitive clipping input/capacity");logged=true;}return;}
    for(int i=1;i+1<count;i++)DrawTriangle((Vector2){(float)clipped[0].x,(float)clipped[0].y},(Vector2){(float)clipped[i].x,(float)clipped[i].y},(Vector2){(float)clipped[i+1].x,(float)clipped[i+1].y},color);
}
static void pause_backdrop_ui(int width,int height)
{
    const float stops[3]={0,.32f,.5f};
    for(int i=0;i<2;i++){
        uint32_t a=rr_pause_backdrop_rgba(stops[i]),b=rr_pause_backdrop_rgba(stops[i+1]);
        Color left={(unsigned char)(a>>24),(unsigned char)(a>>16),(unsigned char)(a>>8),(unsigned char)a};
        Color right={(unsigned char)(b>>24),(unsigned char)(b>>16),(unsigned char)(b>>8),(unsigned char)b};
        DrawRectangleGradientEx((Rectangle){stops[i]*width,0,(stops[i+1]-stops[i])*width,(float)height},left,left,right,right);
    }
}
static void pause_ui(void)
{
    int w=GetScreenWidth(),h=GetScreenHeight();RRUIOverlayLayout l=rr_ui_pause_layout((float)w,(float)h);
    pause_backdrop_ui(w,h);rr_ui_pause_panel(l.panel,cyan);
    rr_ui_text_center(&app.ui,"PAUSED",l.panel.x+l.panel.w*.5f,l.panel.y+32,40,4,900,white,true);
    if(app.overloaded)rr_ui_text_center(&app.ui,"Rendering was slow. Flight paused.",l.panel.x+l.panel.w*.5f,l.panel.y-30,18,0,0,magenta,false);
    if(rr_ui_button_ex(&app.ui,l.actions[0],"RESUME",true,true,false,0,20,24)){
        rr_session_resume(&app.session);app.overloaded=false;app.slow_frames=0;
        if(app.session.phase==RR_SESSION_FINISHED||app.session.phase==RR_SESSION_RESULTS)release_mouse();else capture();return;
    }
    if(rr_ui_button_ex(&app.ui,l.actions[1],"RESTART",false,true,false,1,20,24)){start_flight(app.selected_mode);return;}
    if(rr_ui_button_ex(&app.ui,l.actions[2],"MAIN MENU",false,true,false,2,20,24)){
        rr_session_menu(&app.session);app.menu_policy=(RRMenuPolicy){0};release_mouse();reset_car(true);return;
    }
    rr_ui_subtext_spaced(&app.sub_art,&app.ui,"DRAG TO ROTATE \xc2\xb7 SCROLL TO ZOOM",l.panel.x+l.panel.w*.5f,
        (float)rr_css_line_baseline(l.panel.y+309,15,11,700),11,2,(Color){127,169,196,255});
    rr_ui_text_center(&app.ui,"OFFLINE TRIAL / NO REWARDS",l.panel.x+l.panel.w*.5f,l.panel.y+344,14,0,0,(Color){153,187,221,255},false);
}
static Color boost_color(Color c,double brightness,double opacity)
{
    c.r=(unsigned char)fmin(255,floor(c.r*brightness+.5));c.g=(unsigned char)fmin(255,floor(c.g*brightness+.5));c.b=(unsigned char)fmin(255,floor(c.b*brightness+.5));c.a=(unsigned char)floor(255*opacity+.5);return c;
}
static void boost_meter_ui(RRUIRect panel,float z)
{
    double now=GetTime();
    RRBoostStyle style=rr_boost_meter_present_style(&app.boost_meter,now);
    RRBoostUI box=rr_boost_ui(panel,z);
    Rectangle track={box.track.x,box.track.y,box.track.w,box.track.h};float border=box.border;
    float cx=track.x+track.width*.5f,cy=track.y+track.height*.5f;
    float shear[16]={1,0,0,0,-.466307658f,1,0,0,0,0,1,0,0,0,0,1};
    rlPushMatrix();rlTranslatef(cx,cy,0);rlMultMatrixf(shear);rlTranslatef(-cx,-cy,0);
    DrawRectangleRec(track,(Color){25,230,255,31});
    float width=(float)((track.width-2*border)*app.boost_meter.fill);
    Color left=app.boost_meter.on?magenta:cyan,right=app.boost_meter.on?(Color){255,196,244,255}:(Color){143,247,255,255};
    /* Fixed16 strips approximate post-gradient CSS brightness saturation.
     * Glow/antialias/pixel equivalence remain pending native visual testing. */
    if(width>0)for(int i=0;i<16;i++){
        double a=(double)i/16,b=(double)(i+1)/16;
        Color ca={(unsigned char)round(left.r+(right.r-left.r)*a),(unsigned char)round(left.g+(right.g-left.g)*a),(unsigned char)round(left.b+(right.b-left.b)*a),255};
        Color cb={(unsigned char)round(left.r+(right.r-left.r)*b),(unsigned char)round(left.g+(right.g-left.g)*b),(unsigned char)round(left.b+(right.b-left.b)*b),255};
        ca=boost_color(ca,style.brightness,style.opacity);cb=boost_color(cb,style.brightness,style.opacity);
        DrawRectangleGradientEx((Rectangle){track.x+border+width*(float)a,track.y+border,width/16,track.height-2*border},ca,ca,cb,cb);
    }
    DrawRectangleLinesEx(track,border,(Color){25,230,255,128});rlPopMatrix();
}

static void nitro_call_ui(void)
{
    RRNitroStyle s=rr_nitro_call_sample(&app.nitro_call,GetTime());if(!s.shown||s.opacity<=0)return;
    float scale=(float)s.scale,spacing=(float)s.spacing;
    /* Source is outside #hud zoom. Include its trailing CSS letter spacing in
     * centering; native glyph/glow/italic appearance remains unqualified. */
    float cx=GetScreenWidth()*.5f-spacing*scale*.5f;
    float cy=GetScreenHeight()*.8f+(float)(.5+s.translate_y/100)*45;
    rr_ui_text_center(&app.ui,"NITRO",cx,cy-22.5f*scale,36*1.254f*scale,spacing*scale,900,Fade((Color){234,255,255,255},(float)s.opacity),true);
}

/* Project through the actual bounded world target, then stretch with the
 * displayed texture. Integer render-size rounding must not drift HUD marks. */
static RRHUDPoint hud_point(Vector3 point)
{
    int w=GetScreenWidth(),h=GetScreenHeight();RRRenderSize size=rr_render_size(w,h);
    RRHUDPoint p=rr_reticle_project(point,app.camera,size.width,size.height,rlGetCullDistanceNear(),rlGetCullDistanceFar());
    if(p.valid){p.position.x*=(float)w/size.width;p.position.y*=(float)h/size.height;}return p;
}

static void flight_ui(void)
{
    int w=GetScreenWidth(),h=GetScreenHeight();RRUIHudLayout l=rr_ui_hud_layout((float)w,(float)h);float z=l.scale;
    RRUIHudBoxes boxes=rr_ui_hud_boxes(l);
    double target_now=GetTime();bool target_visible=false;
    const RRRace *race=&app.session.race;char clock[32];RRVec3 target={0};bool has_target=rr_session_target(&app.session,app.track.gates,app.track.gate_count,&target);
    rr_ui_text_fit(&app.ui,app.track.road->name,22*z,22*z,210*z,15*z,1*z,700,cyan);
    rr_ui_text(&app.ui,"SOLO / OFFLINE",22*z,49*z,12*z,1*z,500,muted,false);
    if(!app.cockpit_visible){
    rr_ui_hud_panel(l.timer,cyan,z);
    float lap_baseline=(float)rr_css_line_baseline(boxes.lap_row.y,boxes.lap_row.h,34*z,900);
    rr_ui_text_css(&app.ui,"LAP",boxes.lap_row.x,lap_baseline,15*z,2*z,700,cyan,true);
    const char *lap=race->mode==RR_RACE_TIME_ATTACK?TextFormat("%d",race->lap):TextFormat("%d/2",race->lap>2?2:race->lap);
    rr_ui_text_right_css(&app.ui,lap,boxes.lap_row.x+boxes.lap_row.w,lap_baseline,34*z,0,900,WHITE,false);
    rr_ui_time(clock,race->lap_time);rr_ui_text_css(&app.ui,clock,boxes.time.x,(float)rr_css_line_baseline(boxes.time.y,boxes.time.h,40*z,900),40*z,1*z,900,WHITE,true);
    float checkpoint_baseline=(float)rr_css_line_baseline(boxes.checkpoint_row.y,boxes.checkpoint_row.h,20*z,900);
    rr_ui_text_css(&app.ui,"CHECKPOINT",boxes.checkpoint_row.x,checkpoint_baseline,15*z,2*z,700,cyan,false);
    rr_ui_text_right_css(&app.ui,TextFormat("%d/%d",race->next_checkpoint,app.track.gate_count),boxes.checkpoint_row.x+boxes.checkpoint_row.w,checkpoint_baseline,20*z,0,900,WHITE,false);

    }
    Vector2 center={l.map.x+l.map.w*.5f,l.map.y+l.map.h*.5f};float radius=l.map.w*.5f;
    RRUIRect canvas={l.map.x+2*z,l.map.y+2*z,l.map.w-4*z,l.map.h-4*z};
    uint32_t inner=rr_minimap_background_vertex(false),outer=rr_minimap_background_vertex(true);
    Color bg_inner={(unsigned char)(inner>>24),(unsigned char)(inner>>16),(unsigned char)(inner>>8),(unsigned char)inner};
    Color bg_outer={(unsigned char)(outer>>24),(unsigned char)(outer>>16),(unsigned char)(outer>>8),(unsigned char)outer};
    DrawCircleGradient(center,radius,bg_inner,bg_outer);
    DrawRing(center,radius-2*z,radius,0,360,128,cyan);
    /* Canvas strokes the entire outer path before the entire inner path. */
    for(int pass=0;pass<2;pass++)for(int i=0;i<app.track.sample_count;i++){
        Vector2 a=map_point(app.track.samples[i],canvas),b=map_point(app.track.samples[(i+1)%app.track.sample_count],canvas);
        ui_map_line(a,b,canvas,(pass?4:14)*canvas.w/440,pass?cyan:Fade(cyan,.25f));
    }
    for(int i=0;i<app.track.gate_count;i++){
        const RRGate *gate=&app.track.gates[i];
        RRMapMark mark=rr_minimap_checkpoint(&app.minimap,gate->pos.x,gate->pos.z,canvas.x,canvas.y,canvas.w,i==race->next_checkpoint,i<race->next_checkpoint);
        Color color={(unsigned char)(mark.rgba>>24),(unsigned char)(mark.rgba>>16),(unsigned char)(mark.rgba>>8),(unsigned char)mark.rgba};
        for(int layer=0;layer<(mark.next?2:1);layer++)for(int n=0;n<(layer?128:64);n++){
            RRCirclePoint triangle[3];if(rr_minimap_mark_triangle(mark,layer!=0,n,triangle))ui_map_triangle(triangle,canvas,layer?WHITE:color);
        }
    }
    for(int i=0;i<2;i++){RRCirclePoint triangle[3];if(rr_minimap_start_triangle(&app.minimap,canvas.x,canvas.y,canvas.w,i,triangle))ui_map_triangle(triangle,canvas,WHITE);}
    double player_points[4][2];rr_minimap_player_points(&app.minimap,app.car.pos.x,app.car.pos.z,app.car.yaw,canvas.x,canvas.y,canvas.w,canvas.h,player_points);
    const int indices[2][3]={{0,2,1},{0,3,2}};
    for(int side=0;side<2;side++){
        RRCirclePoint triangle[3],clipped[RR_CIRCLE_CLIP_MAX];
        for(int i=0;i<3;i++)triangle[i]=(RRCirclePoint){player_points[indices[side][i]][0],player_points[indices[side][i]][1]};
        int count=rr_circle_clip_triangle(triangle,center.x,center.y,canvas.w*.5,clipped);
        if(count<0){static bool logged=false;if(!logged){TraceLog(LOG_ERROR,"MINIMAP: invalid clipping input/capacity; marker unqualified");logged=true;}continue;}
        for(int i=1;i+1<count;i++)DrawTriangle((Vector2){(float)clipped[0].x,(float)clipped[0].y},(Vector2){(float)clipped[i].x,(float)clipped[i].y},(Vector2){(float)clipped[i+1].x,(float)clipped[i+1].y},WHITE);
    }
    rr_ui_text_center(&app.ui,TextFormat("ROAD %d / %s",app.track.road->id,app.track.road->name),center.x,l.map.y+l.map.h-29*z,8*z,2*z,700,cyan,false);

    RRUIHistory history=rr_ui_history_layout(l.history,z,(float)GetScreenHeight(),app.session.lap_count,app.ui_best_lap);
    DrawRectangleRec((Rectangle){history.panel.x,history.panel.y,history.panel.w,history.panel.h},(Color){4,8,30,184});
    DrawRectangleRec((Rectangle){history.panel.x,history.panel.y,3*z,history.panel.h},magenta);
    float title_baseline=(float)rr_css_line_baseline(l.history.y+10*z,19*z,15*z,700);
    rr_ui_text_css(&app.ui,"LAP TIMES",l.history.x+19*z,title_baseline,15*z,2*z,700,cyan,true);
    for(size_t i=0;i<history.visible_rows;i++){
        bool best=i==history.best_index;Color color=best?magenta:(Color){207,239,255,255};
        float y=rr_ui_history_row_y(history,i),font=rr_ui_history_row_font(history,i),line=rr_ui_history_row_height(history,i);
        float baseline=(float)rr_css_line_baseline(y,line,font,700);
        rr_ui_time(clock,app.session.lap_times[i]);
        char number[32];snprintf(number,sizeof(number),"%zu.",i+1);
        rr_ui_text_right_css(&app.ui,number,l.history.x+35*z,baseline,font,0,700,color,best);
        rr_ui_text_css(&app.ui,clock,l.history.x+39*z,baseline,font,0,700,color,best);
    }
    if(history.best_visible){
        rr_ui_best_clock(clock,&app.picker,app.track.road->id);
        float baseline=(float)rr_css_line_baseline(history.best_y,16*z,13*z,700);
        rr_ui_text_css(&app.ui,"BEST ",l.history.x+19*z,baseline,13*z,0,700,(Color){153,170,204,255},false);
        Font face=app.ui.loaded?app.ui.bold:GetFontDefault();
        float size=app.ui.loaded?(float)rr_css_font_size(13*z,700):13*z;
        float prefix=MeasureTextEx(face,"BEST ",size,0).x;
        rr_ui_text_css(&app.ui,clock,l.history.x+19*z+prefix,baseline,13*z,0,700,WHITE,false);
    }

    if(!app.cockpit_visible){
    rr_ui_hud_panel(l.speed,cyan,z);float sx=l.speed.x+22*z,sy=l.speed.y;
    rr_ui_text_css(&app.ui,"SPEED",boxes.speed_label.x,(float)rr_css_line_baseline(boxes.speed_label.y,boxes.speed_label.h,15*z,700),15*z,2*z,700,cyan,true);
    rr_ui_text_css(&app.ui,TextFormat("%d",rr_ui_speed(app.car.speed)),boxes.speed_value.x,(float)rr_css_line_baseline(boxes.speed_value.y,boxes.speed_value.h,64*z,900),64*z,0,900,(Color){242,254,255,255},true);
    rr_ui_text_css(&app.ui,"km/h",boxes.unit.x,(float)rr_css_line_baseline(boxes.unit.y,boxes.unit.h,18*z,700),18*z,0,700,cyan,false);
    boost_meter_ui(l.speed,z);
    RRBoostUI boost_box=rr_boost_ui(l.speed,z);uint32_t boost_rgba=rr_boost_label_color();
    Color boost_label={(unsigned char)(boost_rgba>>24),(unsigned char)(boost_rgba>>16),(unsigned char)(boost_rgba>>8),(unsigned char)boost_rgba};
    rr_ui_text_css(&app.ui,"BOOST [SHIFT]",boost_box.label.x,(float)rr_css_line_baseline(boost_box.label.y,boost_box.label.h,10*z,700),10*z,2*z,700,boost_label,false);
    rr_ui_text(&app.ui,"COMBAT NOT CONNECTED",sx,sy+180*z,9*z,0,500,muted,false);
    }
    RRSpinReadout spin_text=rr_spin_readout(z);
    rr_ui_hud_panel(l.spin,magenta,z);Vector2 spin={l.spin.x+l.spin.w-(float)spin_text.right_inset,l.spin.y+(float)spin_text.center_y};
    float spin_baseline=l.spin.y+(float)spin_text.label_baseline;
    RRSpinIndicator indicator=rr_spin_indicator(app.spin_cooldown);
    for(int layer=0;layer<3;layer++){
        uint32_t rgba=rr_spin_indicator_color(indicator,layer);
        Color color={(unsigned char)(rgba>>24),(unsigned char)(rgba>>16),(unsigned char)(rgba>>8),(unsigned char)rgba};
        for(int i=0;i<rr_spin_indicator_sectors(indicator,layer);i++){
            RRSpinPoint points[3];if(!rr_spin_indicator_triangle(indicator,layer,i,points))continue;
            DrawTriangle((Vector2){spin.x+(float)points[0].x*z,spin.y+(float)points[0].y*z},(Vector2){spin.x+(float)points[1].x*z,spin.y+(float)points[1].y*z},(Vector2){spin.x+(float)points[2].x*z,spin.y+(float)points[2].y*z},color);
        }
    }
    uint32_t text_rgba=rr_spin_indicator_text_delta(indicator,false),shadow_rgba=rr_spin_indicator_text_delta(indicator,true);
    Color text={(unsigned char)(text_rgba>>24),(unsigned char)(text_rgba>>16),(unsigned char)(text_rgba>>8),(unsigned char)text_rgba};
    Color shadow={(unsigned char)(shadow_rgba>>24),(unsigned char)(shadow_rgba>>16),(unsigned char)(shadow_rgba>>8),(unsigned char)shadow_rgba};
    BeginBlendMode(BLEND_ADDITIVE);
    for(int i=0;i<4;i++){float dx=i==0?-2*z:i==1?2*z:0,dy=i==2?-2*z:i==3?2*z:0;rr_ui_text_center_css(&app.ui,"SPIN",spin.x+dx,spin_baseline+dy,14*z,0,900,shadow);}
    rr_ui_text_center_css(&app.ui,"SPIN",spin.x,spin_baseline,14*z,0,900,text);
    EndBlendMode();
    rr_ui_subtext_spaced(&app.sub_art,&app.ui,RR_SPIN_KEY_TEXT,spin.x,l.spin.y+(float)spin_text.key_baseline,(float)spin_text.key_font,(float)spin_text.key_spacing,(Color){201,166,217,255});

    /* Source DOM paints heading/aim before the later gate target. */
    RRReticleShape marks=rr_reticle_shape(z);
    Vector3 heading=Vector3Add(vector(app.car.pos),Vector3Scale(vector(app.car.fwd),90));
    RRHUDPoint cross=hud_point(heading);
    if(cross.shown&&cross.position.x>=-marks.tick_end&&cross.position.x<=w+marks.tick_end&&cross.position.y>=-marks.tick_end&&cross.position.y<=h+marks.tick_end){
        Vector2 p=cross.position;
        DrawRing(p,(float)marks.cross_inner,(float)marks.cross_outer,0,360,64,Fade(cyan,.5f));
        DrawRectangleRec((Rectangle){p.x-z,p.y-(float)marks.tick_end,2*z,2*(float)marks.tick_end},Fade(cyan,.7f));
        DrawRectangleRec((Rectangle){p.x-(float)marks.tick_end,p.y-z,(float)(marks.tick_end-marks.tick_gap),2*z},Fade(cyan,.7f));
        DrawRectangleRec((Rectangle){p.x+(float)marks.tick_gap,p.y-z,(float)(marks.tick_end-marks.tick_gap),2*z},Fade(cyan,.7f));
    }
    Vector3 aim_point=Vector3Add(vector(app.car.pos),Vector3Scale(vector(app.aim_direction),90));
    RRHUDPoint aim=hud_point(aim_point);
    /* This solo host has no wreck/alive state; production combat remains off. */
    if(rr_reticle_aim_visible(aim.shown,app.mouse_capture.captured,true)&&aim.position.x>=-marks.aim_outer&&aim.position.x<=w+marks.aim_outer&&aim.position.y>=-marks.aim_outer&&aim.position.y<=h+marks.aim_outer){
        DrawRing(aim.position,(float)marks.aim_inner,(float)marks.aim_outer,0,360,48,WHITE);
        DrawCircleV(aim.position,(float)marks.dot,WHITE);
    }
    if(has_target){
        RRHUDPoint projected=hud_point(vector(target));Vector2 point=projected.position;double nz=projected.ndc_z;
        if(projected.valid){
            RRTargetGuidance t=rr_target_guidance(2*point.x/w-1,1-2*point.y/h,nz,w,h,z,rr_length(rr_sub(target,app.car.pos)));
            Color green={43,255,122,255};Vector2 at={(float)t.x,(float)t.y};
            if(t.on){
                target_visible=true;double pulse=rr_target_pulse_observe(&app.target_pulse,true,target_now);
                RRTargetMarker marker=rr_target_marker(at.x,at.y,z,pulse);
                DrawPolyLinesEx((Vector2){(float)marker.cx,(float)marker.cy},4,(float)marker.radius,0,(float)marker.thickness,green);
                for(int i=0;i<4;i++){
                    float dx=i==0?-2*z:i==1?2*z:0,dy=i==2?-2*z:i==3?2*z:0;
                    rr_ui_text_center_css(&app.ui,TextFormat("%.0f m",t.distance),(float)marker.cx+dx,(float)marker.text_baseline+dy,(float)marker.font,0,700,Fade(green,.12f));
                }
                rr_ui_text_center_css(&app.ui,TextFormat("%.0f m",t.distance),(float)marker.cx,(float)marker.text_baseline,(float)marker.font,0,700,(Color){208,255,224,255});
            }else{
                if(!rr_arrow_art_draw(&app.arrow_art,at.x,at.y,z,t.angle)){
                /* Native vector placeholder for source edge-arrow glyph;
                 * placement/rotation are source-defined, pixel shape is pending. */
                Vector2 arrow_center={at.x+20*z,at.y+25*z},tip={(float)cos(t.angle),(float)sin(t.angle)};
                Vector2 side={-tip.y,tip.x};
                Vector2 v0=Vector2Add(arrow_center,Vector2Scale(tip,15*z));
                Vector2 back=Vector2Subtract(arrow_center,Vector2Scale(tip,10*z));
                DrawTriangle(v0,Vector2Subtract(back,Vector2Scale(side,10*z)),Vector2Add(back,Vector2Scale(side,10*z)),green);
                }
            }
        }
    }
    if(!target_visible)rr_target_pulse_observe(&app.target_pulse,false,target_now);
    if(app.message_opacity>0){
        RRAnnouncementLayout notice=rr_announcement_layout(w,h,z,app.message.warn);
        uint32_t rgba[]={notice.near_glow,notice.far_glow,notice.color};
        for(int layer=0;layer<3;layer++){
            uint32_t packed=rgba[layer];Color color={(unsigned char)(packed>>24),(unsigned char)(packed>>16),(unsigned char)(packed>>8),(unsigned char)lround((layer<2?.12:1)*app.message_opacity*255)};
            /* Bounded native shadow approximation; source20/50px (warn20/40)
             * Gaussian blur and isolated group opacity remain unqualified. */
            int passes=layer<2?4:1;float offset=(layer?4.f:2.f)*z;
            for(int pass=0;pass<passes;pass++){
                float dx=layer<2?(pass==0?-offset:pass==1?offset:0):0,dy=layer<2?(pass==2?-offset:pass==3?offset:0):0;
                rr_ui_text_center_css(&app.ui,app.message.text,(float)notice.cx+dx,(float)notice.baseline+dy,(float)notice.font,(float)notice.tracking,900,color);
            }
        }
    }
    if(app.sub_opacity>0){
        /* Cached fallback baseline offsets; mixed/native pixel alignment is pending. */
        float ascent=strstr(app.sub_notice.text,"\xc2\xb7")?29.f:strstr(app.sub_notice.text,"\xe2\x98\x85")?28.f:26.484f;
        rr_ui_subtext_opacity(&app.sub_art,&app.ui,app.sub_notice.text,(float)w*.5f,(float)h*.24f+(100+ascent)*z,z,app.sub_opacity);
    }
    if(rr_capture_hint_visible(app.session.phase,app.mouse_capture.captured,false)){
        RRCaptureHintLayout hint=rr_capture_hint_layout(w,h,z);
        for(int i=0;i<4;i++){
            float dx=i==0?-2*z:i==1?2*z:0,dy=i==2?-2*z:i==3?2*z:0;
            rr_ui_text_center_css(&app.ui,RR_CAPTURE_HINT_TEXT,(float)hint.cx+dx,(float)hint.baseline+dy,(float)hint.font,(float)hint.spacing,700,Fade(cyan,.12f));
        }
        rr_ui_text_center_css(&app.ui,RR_CAPTURE_HINT_TEXT,(float)hint.cx,(float)hint.baseline,(float)hint.font,(float)hint.spacing,700,cyan);
    }
    rr_ui_text_center(&app.ui,"ESC PAUSE / C CAMERA / R RESET / F11 FULLSCREEN",(float)w*.5f,(float)h-18,11*z,0,500,muted,false);
#if defined(RR_COCKPIT_PREVIEW)
    rr_ui_text(&app.ui,app.interior_requested?"I / EXTERIOR VIEW":"I / COCKPIT VIEW",(float)w-220,(float)h*.5f,11,0,500,muted,false);
    if(app.interior_requested&&app.cockpit_art.loaded!=RR_COCKPIT_LAYER_COUNT)rr_ui_text_center(&app.ui,"COCKPIT NOT READY / PAUSE TO FINISH LOADING",(float)w*.5f,(float)h*.6f,12,0,700,magenta,false);
#endif
    if(!app.scene.car_loaded)rr_ui_text(&app.ui,"FALLBACK VEHICLE",l.map.x,l.map.y+l.map.h+12*z,11*z,0,500,muted,false);

}

static void results_ui(void)
{
    RRUIOverlayLayout l=rr_ui_results_layout((float)GetScreenWidth(),(float)GetScreenHeight());bool failed=app.session.phase==RR_SESSION_ERROR;
    rr_ui_pause_panel(l.panel,cyan);float cx=l.panel.x+l.panel.w*.5f;char clock[32];
    rr_ui_text_center_css_glow(&app.ui,failed?"SESSION UNAVAILABLE":"FINISH",cx,l.panel.y+(float)rr_results_baseline(0,failed?24:34,900),failed?24.f:34.f,4,900,white,true);
    rr_ui_text_center_css_glow(&app.ui,"TWO-LAP OFFLINE TRIAL",cx,l.panel.y+(float)rr_results_baseline(1,24,900),24,0,900,magenta,true);
    if(!failed&&app.session.lap_count==2){
        RRResultSummary r=rr_finish_summary(app.session.lap_times,app.session.lap_count);rr_ui_time(clock,r.total);
        rr_ui_text_center_css_glow(&app.ui,clock,cx,l.panel.y+(float)rr_results_baseline(2,32,900),32,0,900,white,true);
        char laps[2][64];float widths[2];Font font=app.ui.loaded?app.ui.bold:GetFontDefault();
        for(int i=0;i<2;i++){rr_ui_time(clock,app.session.lap_times[i]);snprintf(laps[i],sizeof(laps[i]),"LAP %d  %s",i+1,clock);widths[i]=MeasureTextEx(font,laps[i],app.ui.loaded?(float)rr_css_font_size(16,700):16,0).x;}
        float lap_x=(float)rr_results_lap_left(cx,widths[0],widths[1]);
        for(int i=0;i<2;i++){rr_ui_text_css(&app.ui,laps[i],lap_x,l.panel.y+(float)rr_results_baseline(3,16,700),16,0,700,r.best==(size_t)i?magenta:(Color){207,239,255,255},false);lap_x+=widths[i]+28;}
        rr_ui_text_css(&app.ui,"STANDINGS",l.left.x,(float)rr_css_line_baseline(l.left.y,19,15,700),15,2,700,cyan,true);
        rr_ui_text_css(&app.ui,"SOLO TRIAL",l.left.x,(float)rr_css_line_baseline(l.left.y+35,23,18,700),18,0,700,white,false);
        rr_ui_text_css(&app.ui,"NO RIVALS IN THIS SESSION",l.left.x,(float)rr_css_line_baseline(l.left.y+67,16,12,500),12,0,500,muted,false);
        rr_ui_text_css(&app.ui,"COIN RACE",l.right.x,(float)rr_css_line_baseline(l.right.y,19,15,700),15,2,700,(Color){255,210,90,255},false);
        rr_ui_text_css(&app.ui,"NO REWARDS",l.right.x,(float)rr_css_line_baseline(l.right.y+35,23,18,700),18,0,700,muted,false);
        rr_ui_text_css(&app.ui,"ACCOUNT NOT CONNECTED",l.right.x,(float)rr_css_line_baseline(l.right.y+67,16,12,500),12,0,500,muted,false);
    }else rr_ui_text_center(&app.ui,"Lap history unavailable. Return to menu.",cx,l.panel.y+178,20,0,0,magenta,false);
    rr_ui_text_center_css(&app.ui,"OFFLINE / NO REWARDS OR PROGRESSION",cx,l.panel.y+(float)rr_results_baseline(5,12,500),12,1,500,muted);
    if(!failed&&(key_hit(KEY_ENTER)||rr_ui_button(&app.ui,l.actions[0],"RACE AGAIN [ENTER]",true,true,false,0))){start_flight(app.selected_mode);return;}
    if(rr_ui_button(&app.ui,l.actions[1],"MAIN MENU",false,true,false,1)||key_hit(KEY_ESCAPE)||key_hit(KEY_M)){
        rr_session_menu(&app.session);app.menu_policy=(RRMenuPolicy){0};release_mouse();reset_car(true);
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
        app.menu_policy.armed=false;app.capture_click.armed=false;rr_showcase_up(&app.showcase);
        rr_boost_meter_render(&app.boost_meter,false,GetTime());
        if(rr_ui_pause_stamp(&app.session,&app.pause_at_ms,GetTime()*1000))release_mouse();
        PollInputEvents();
#if !defined(PLATFORM_WEB)
        WaitTime(.1);
#endif
        return;
    }
    bool focused=IsWindowFocused();
    bool finishing=app.session.phase==RR_SESSION_FINISHED||app.session.phase==RR_SESSION_RESULTS;
    if(finishing && !focused && rr_ui_pause_stamp(&app.session,&app.pause_at_ms,GetTime()*1000))release_mouse();
    target_fps(app.smoke?60:!focused?5:30);
    if(focused && key_hit(KEY_F11))ToggleFullscreen();
    /* Source C then I dispatch precedes pause/resume, even in menus/results. */
    RRViewControls view_controls={app.camera_mode,app.interior_requested};
#if defined(RR_COCKPIT_PREVIEW)
    bool interior_enabled=true;
#else
    bool interior_enabled=false;
#endif
    rr_view_controls_step(&view_controls,app.session.phase,focused,interior_enabled,key_hit(KEY_C)||pad_pressed(GAMEPAD_BUTTON_RIGHT_FACE_UP),key_hit(KEY_I));
    app.camera_mode=view_controls.mode;app.interior_requested=view_controls.interior;
    bool pause_key=key_hit(KEY_ESCAPE)||key_hit(KEY_P)||pad_pressed(GAMEPAD_BUTTON_MIDDLE_RIGHT);
    if((app.session.phase==RR_SESSION_ACTIVE || app.session.phase==RR_SESSION_COUNTDOWN) && (pause_key || (!app.smoke && !focused))){rr_ui_pause_stamp(&app.session,&app.pause_at_ms,GetTime()*1000);release_mouse();}
    else if(rr_ui_resume_key(&app.session,app.pause_at_ms,GetTime()*1000,pause_key,focused)){app.overloaded=false;app.slow_frames=0;if(app.session.phase==RR_SESSION_FINISHED||app.session.phase==RR_SESSION_RESULTS)release_mouse();else capture();}
    Vector2 capture_point=GetMousePosition();
    bool capture_inside=capture_point.x>=0&&capture_point.y>=0&&capture_point.x<(float)GetScreenWidth()&&capture_point.y<(float)GetScreenHeight();
    if(rr_capture_click_step(&app.capture_click,app.session.phase,focused,app.mouse_capture.captured,IsMouseButtonPressed(MOUSE_BUTTON_LEFT),IsMouseButtonReleased(MOUSE_BUTTON_LEFT),capture_inside)&&!app.smoke)capture();
    if(app.smoke && app.frame==30)start_flight(RR_RACE_TIME_ATTACK);
    if(app.load_requested && (focused || app.smoke)){
        app.load_requested=false;
        TraceLog(LOG_INFO,"STATE: scene load requested; fallback=%d glow=%d",app.fallback,app.glow);
        if(rr_scene_load(&app.scene,&app.corridor,app.fallback,app.glow)){
            app.scene_ready=true;start_flight(app.selected_mode);
        }else{
            TraceLog(LOG_ERROR,"STATE: scene load failed; returning to 2D menu");
            rr_scene_unload(&app.scene);app.failed=true;app.session.phase=RR_SESSION_MENU;release_mouse();
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
#if defined(RR_COCKPIT_PREVIEW)
    if(focused&&(app.session.phase==RR_SESSION_MENU||app.session.phase==RR_SESSION_PAUSED)&&!app.load_requested)
        rr_cockpit_load_next(&app.cockpit_art);
#endif
    bool boost_was=app.car.boosting;bool boost_fx_playing=app.session.phase==RR_SESSION_ACTIVE||app.session.phase==RR_SESSION_FINISHED||app.session.phase==RR_SESSION_RESULTS;
    if(app.session.phase==RR_SESSION_ACTIVE){
        RRInput in=app.smoke?(RRInput){1,0,0,.1,false}:controls();
#if defined(RR_FINISH_VALIDATION) || defined(RR_COLLISION_VALIDATION)
        in=(RRInput){0};
#endif
        app.cockpit_control=in.steer;
        bool spin_requested=key_hit(KEY_F)||pad_pressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
        bool reset_requested=key_hit(KEY_R)||pad_pressed(GAMEPAD_BUTTON_MIDDLE_LEFT);
        RRVec3 warning_previous=app.car.pos;double checkpoint_clock=app.session.race.lap_time+dt;
        unsigned events=rr_session_step(&app.session,&app.car,app.track.gates,app.track.gate_count,in,raw_dt);
#if defined(RR_COLLISION_VALIDATION)
        if(app.car.impact>0){TraceLog(LOG_INFO,"VALIDATION: wall impact=%.6f x=%.6f vx=%.6f speed=%.6f",app.car.impact,app.car.pos.x,app.car.vel.x,app.car.speed);rr_ui_pause_stamp(&app.session,&app.pause_at_ms,GetTime()*1000);release_mouse();}
#endif
        if(events&RR_RACE_CHECKPOINT)rr_sub_notice_checkpoint(&app.sub_notice,events&RR_RACE_LAP?app.track.gate_count:app.session.race.next_checkpoint,app.track.gate_count,checkpoint_clock);
        if(events&RR_RACE_LAP){
            if(!(events&RR_RACE_FINISH))rr_hud_message_lap(&app.message,app.session.race.lap,app.session.race.mode==RR_RACE_TIME_ATTACK?0:2);
            double previous_best=app.picker.rows[app.track.road->id-1].best_lap;
            if(!(events&RR_RACE_FINISH))rr_sub_notice_lap(&app.sub_notice,app.session.race.last_lap_time,previous_best<=0||app.session.race.last_lap_time<previous_best);
            rr_road_picker_lap(&app.picker,app.session.race.last_lap_time);app.ui_best_lap=rr_ui_best_lap_on_append(app.session.lap_times,app.session.lap_count,app.ui_best_lap);
        }
        if(events&RR_RACE_FINISH){rr_hud_message_finish(&app.message);TraceLog(LOG_INFO,"STATE: offline two-lap trial finished in %.3f s",app.session.race.finish_time);release_mouse();}
        if(app.session.phase==RR_SESSION_ACTIVE){
            if(rr_race_missed_gate(&app.session.race,app.track.gate_count,&app.car,warning_previous,app.session.race_time)){
                rr_hud_message_set(&app.message,"MISSED GATE",1.4,true);
            }
        }
        /* raceRules recovery also runs on the frame that completes the race. */
        player_recovery_feedback(dt);
        /* Source playerWeapons follows player movement/rules/recovery and
         * Nitro/impact feedback. A finish produced above suppresses this step. */
        motion_camera_feedback(boost_fx_playing,boost_was);
        RRPlayerControlState state=player_control_state();
        /* Solo host is alive and has no connected account-role restriction. */
        RRPlayerControlEvents actions=rr_player_controls_step(&state,(RRPlayerControlInput){dt,in.steer,spin_requested,reset_requested,true,false},spin_random_side,NULL);
        if(actions.spin.accepted)rr_sub_notice_spin(&app.sub_notice);
    }else if(app.session.phase==RR_SESSION_COUNTDOWN || app.session.phase==RR_SESSION_FINISHED || app.session.phase==RR_SESSION_RESULTS){
        unsigned events=rr_session_step(&app.session,&app.car,app.track.gates,app.track.gate_count,finishing?finished_controls():(RRInput){0},raw_dt);
        if(finishing)player_recovery_feedback(dt);
        motion_camera_feedback(boost_fx_playing,boost_was);
        int previous_count=app.start_cue.count;
        rr_start_cue_observe(&app.start_cue,app.session.last_count);
        if(app.start_cue.count!=previous_count)rr_hud_message_set(&app.message,app.start_cue.text,.9,false);
        if(events&RR_SESSION_SHOW_RESULTS)TraceLog(LOG_INFO,"STATE: results visible; lap history frozen");
        if(events&RR_SESSION_GO){center_aim();TraceLog(LOG_INFO,"STATE: GO");}
    }
    if(app.session.phase==RR_SESSION_ERROR)release_mouse();
#if defined(RR_COCKPIT_PREVIEW)
    if(app.session.phase==RR_SESSION_ACTIVE||app.session.phase==RR_SESSION_COUNTDOWN||app.session.phase==RR_SESSION_FINISHED)
        app.cockpit_steer=rr_cockpit_steer(app.cockpit_steer,app.cockpit_control,dt,app.session.phase==RR_SESSION_ACTIVE);
#endif
    pause_showcase_input();
    update_camera(dt);
    if(app.session.phase!=RR_SESSION_MENU){rr_boost_meter_observe(&app.boost_meter,app.car.boost,app.car.boosting,GetTime());rr_hud_message_tick(&app.message,dt);rr_start_cue_tick(&app.start_cue,dt);rr_sub_notice_tick(&app.sub_notice,dt);}
#if defined(RR_HERO_VIEW)
    if(app.scene_ready){
        static bool logged=false;if(!logged){TraceLog(LOG_INFO,"VALIDATION: camera-only control-tower inspection; no actor/progress changes");logged=true;}
        app.camera=(Camera3D){{-470,180,500},{-72.7f,-40,10},{0,1,0},45,CAMERA_PERSPECTIVE};
    }
#endif
    double scene_time=rr_scene_frame_time(&app.scene_clock,app.session.phase,GetTime());bool scene_rendered=false;
    if(rr_scene_frame_requested(app.session.phase,focused,app.smoke,app.scene_ready,app.overloaded)){
        if(rr_scene_begin(&app.scene,app.camera)){
            scene_rendered=true;
            rr_scene_world(&app.scene,&app.track,&app.course,&app.shared,app.camera,app.session.race.finished?-1:app.session.race.next_checkpoint,scene_time);
            rr_scene_car_view(&app.scene,&app.car,app.cockpit_visible||app.first_person_visible);
            rr_scene_finish(&app.scene);
        }else{
            TraceLog(LOG_ERROR,"STATE: render target unavailable; returning to 2D menu");
            rr_scene_unload(&app.scene);app.scene_ready=false;app.failed=true;app.session.phase=RR_SESSION_MENU;release_mouse();
            BeginDrawing();ClearBackground(ink);
        }
    }else{
        BeginDrawing();ClearBackground(ink);
        DrawRectangleGradientV(0,0,GetScreenWidth(),GetScreenHeight(),ink,(Color){30,19,58,255});
    }
    /* Snapshot the phase once: overlay actions may change it during this paint. */
    RRUIPaintPlan paint=rr_ui_paint_plan(app.session.phase);
    double presentation_now=GetTime();bool hud_rendered=paint.count>0&&paint.layers[0]==RR_UI_LAYER_HUD;
    if(!hud_rendered)rr_target_pulse_observe(&app.target_pulse,false,presentation_now);
    rr_boost_meter_render(&app.boost_meter,hud_rendered&&!app.cockpit_visible,presentation_now);
    app.message_opacity=rr_notice_observe(&app.message_fade,app.message.shown,hud_rendered,presentation_now);
    app.sub_opacity=rr_notice_observe(&app.sub_fade,app.sub_notice.shown,hud_rendered,presentation_now);
    rr_ui_buttons_begin(&app.ui,(int)app.session.phase+(app.picker.open?64:0),presentation_now);
    for(int i=0;i<paint.count;i++){
        RRUIPaintLayer layer=paint.layers[i];
        if(layer==RR_UI_LAYER_HUD){
#if defined(RR_COCKPIT_PREVIEW)
        if(app.cockpit_visible){
            RRCKReadout values;int laps=app.session.race.mode==RR_RACE_TIME_ATTACK?-1:2;
            rr_cockpit_readout(&values,0,fabs(app.car.speed)*3.6,app.car.boost,app.session.race.lap_time,0,false,laps==2?(app.session.race.lap>2?2:app.session.race.lap):app.session.race.lap,laps,app.cockpit_steer);
            rr_cockpit_draw(&app.cockpit_art,&app.ui,GetScreenWidth(),GetScreenHeight(),&values,app.car.boost,false);
        }
#endif
            flight_ui();
        }else if(layer==RR_UI_LAYER_NITRO)nitro_call_ui();
        else if(layer==RR_UI_LAYER_MENU)menu_ui();
        else if(layer==RR_UI_LAYER_PAUSE)pause_ui();
        else if(layer==RR_UI_LAYER_RESULTS){
            uint32_t rgba=rr_ui_results_backdrop();
            Color shade={(unsigned char)(rgba>>24),(unsigned char)(rgba>>16),(unsigned char)(rgba>>8),(unsigned char)rgba};
            DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),shade);results_ui();
        }else if(layer==RR_UI_LAYER_ERROR){DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),Fade(ink,.9f));results_ui();}
    }
    rr_ui_buttons_end(&app.ui);
    app.picker_notice=fmax(0,app.picker_notice-dt);

#if defined(RR_FINISH_VALIDATION)
    DrawText("VALIDATION FIXTURE / SCRIPTED GATES / NO PROGRESS",20,5,14,magenta);
#endif
#if defined(RR_COLLISION_VALIDATION)
    DrawText("COLLISION VALIDATION / SCRIPTED APPROACH / NO PROGRESS",20,5,14,magenta);
#endif
#if defined(RR_HERO_VIEW)
    DrawText("HERO ASSET CAMERA VALIDATION / NO PROGRESS",20,5,14,magenta);
#endif
#if defined(RR_COCKPIT_PREVIEW)
    rr_ui_text(&app.ui,"COCKPIT CANDIDATE / VISUAL VALIDATION PENDING",20,5,11,0,700,magenta,false);
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
    if(scene_rendered&&rr_scene_phase(app.session.phase)&&!app.smoke&&rr_render_overloaded(frame_seconds,&app.slow_frames)){
        TraceLog(LOG_WARNING,"PERFORMANCE: 10 consecutive slow frames; suspending scene rendering (last %.1f ms)",frame_seconds*1000);
        rr_ui_pause_stamp(&app.session,&app.pause_at_ms,GetTime()*1000);app.overloaded=true;app.slow_frames=0;release_mouse();
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
    rr_showcase_init(&app.showcase);
    app.mouse_settings=(RRMouseSettings){1,false};app.aim_direction=rr_v3(0,0,-1);
    app.session.phase=RR_SESSION_MENU;app.camera.up=(Vector3){0,1,0};app.camera.projection=CAMERA_PERSPECTIVE;app.camera_fov=70;
    if(!rr_corridor_init_all(&app.corridor,&app.track)||!rr_road_picker_init(&app.picker,&app.track)){
        TraceLog(LOG_ERROR,"All-route corridor/track initialization failed.");CloseWindow();return 1;
    }
    if(!rr_shared_scenery_plan(&app.shared,&app.track,&app.corridor)){TraceLog(LOG_ERROR,"Shared Skyline scenery initialization failed");CloseWindow();return 1;}
    TraceLog(LOG_INFO,"SHARED: platforms=%d columns=%d trees=%d props=%d colliders=%d",app.shared.platforms,app.shared.columns,app.shared.trees,app.shared.prop_count,app.shared.collider_count);
    if(!rr_canonical_city_plan(&app.city_plan,&app.corridor)){TraceLog(LOG_ERROR,"Canonical core exceeds city budget");CloseWindow();return 1;}
    TraceLog(LOG_INFO,"CITY_CORE: prototype kept=%d removed=%d canonical=%d total=%d",app.city_plan.prototype_kept,app.city_plan.prototype_removed,app.city_plan.canonical_count,app.city_plan.count);
    rr_ui_load(&app.ui);rr_sub_art_load(&app.sub_art);rr_arrow_art_load(&app.arrow_art);
    reset_car(true);
    TraceLog(LOG_INFO,"WORLD: union ready points=%d coarse=%d gates=%d cells=%d",app.corridor.point_count,app.corridor.coarse_count,app.corridor.gate_count,app.corridor.occupied_cells);
    TraceLog(LOG_INFO,"STARTUP: 2D menu ready; track %.3f m, %d samples; nine-road picker ready; offline open=1; scene deferred",app.track.length,app.track.sample_count);
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(frame,0,1);
#else
    while(!WindowShouldClose() && !app.quit && (!app.smoke || app.frame<151))frame();
    bool asset_failure=app.smoke && !app.fallback && !app.scene.car_loaded;
    if(app.smoke)printf("FIRST_FLIGHT_SMOKE: %d frames, model=%s, speed=%.3f m/s, output=captures/\n",app.frame,app.scene.car_loaded?"loaded":"fallback",app.car.speed);
    rr_session_dispose(&app.session);rr_scene_unload(&app.scene);rr_cockpit_unload(&app.cockpit_art);rr_sub_art_unload(&app.sub_art);rr_arrow_art_unload(&app.arrow_art);rr_ui_unload(&app.ui);CloseWindow();
    TraceLog(LOG_INFO,"SESSION: clean shutdown");
    if(diagnostic_log){fclose(diagnostic_log);diagnostic_log=NULL;}
    return (asset_failure || app.failed)?1:0;
#endif
}


