#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "rr_scene.h"
#include "rr_render_policy.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

typedef enum RRMode { RR_MENU, RR_FLYING, RR_PAUSED } RRMode;
typedef struct RRApp {
    RRTrack track; RRCar car; RRScene scene; RRMode mode;
    Camera3D camera;
    double aim_yaw,aim_pitch,look_yaw,look_pitch,camera_yaw,camera_pitch,camera_fov;
    double elapsed,spin_cooldown,message_time;
    int next_gate,passed_gates,camera_mode,frame;
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
    if(localtime_s(&stamp,&now)==0)strftime(date,sizeof(date),"%Y-%m-%d %H:%M:%S",&stamp);
    fprintf(output,"%s [%d] ",date,level);vfprintf(output,message,args);fputc('\n',output);fflush(output);
}
#endif
static const Color ink={7,10,26,255},cyan={67,230,255,255},muted={146,157,186,255};
static const Color magenta={255,69,181,255},white={237,244,255,255};
static Vector3 vector(RRVec3 p){return (Vector3){(float)p.x,(float)p.y,(float)p.z};}

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
    RRVec3 pos=rr_v3(0,118,70);double yaw=0,boost=app.car.boost;
    if(!beginning && app.next_gate>0){
        RRGate gate=app.track.gates[app.next_gate-1];pos=rr_sub(gate.pos,rr_scale(gate.forward,26));yaw=gate.yaw;
    }
    rr_car_reset(&app.car,pos,yaw,beginning?0:35);
    if(!beginning)app.car.boost=boost;
    app.camera_yaw=yaw;app.camera_pitch=0;center_aim();
}
static void start_flight(void)
{
    if(app.failed)return;
    if(!app.scene_ready){app.load_requested=true;return;}
    TraceLog(LOG_INFO,"STATE: start flight");
    app.next_gate=0;app.passed_gates=0;app.elapsed=0;app.spin_cooldown=0;
    app.look_yaw=0;app.look_pitch=0;app.camera_fov=68;
    app.slow_frames=0;app.overloaded=false;
    reset_car(true);app.mode=RR_FLYING;if(app.smoke)center_aim();else capture();
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
    if(app.mode==RR_MENU){
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
static void text_line(const char *key,const char *action,int x,int y)
{
    DrawText(key,x,y,17,cyan);DrawText(action,x+130,y,17,muted);
}

static void menu_ui(void)
{
    int h=GetScreenHeight();float y=(float)h*.5f-250;
    DrawRectangleGradientH(0,0,620,h,(Color){5,8,22,248},(Color){5,8,22,0});
    DrawRectangle(44,(int)y,4,22,magenta);
    DrawText("SKYLINE CITY  /  FIRST FLIGHT",60,(int)y+3,17,cyan);
    DrawText("ROADSTERS",44,(int)y+48,48,white);DrawText("RACE",44,(int)y+100,64,white);
    DrawText("THE SKYLINE IS YOUR RACETRACK.",48,(int)y+180,18,muted);
    DrawText("SKYLINE LOOP",48,(int)y+227,24,white);
    DrawText("5.64 KM   /   8 CHECKPOINTS   /   OFFLINE",48,(int)y+259,16,muted);
    if(!app.failed && (button((Rectangle){48,y+300,380,58},"START FLIGHT     >",true) || IsKeyPressed(KEY_ENTER) || pad_pressed(GAMEPAD_BUTTON_MIDDLE_RIGHT)))start_flight();
    if(app.failed)DrawText("Scene unavailable. Close the game and check the log.",48,(int)y+316,17,magenta);
#if !defined(PLATFORM_WEB)
    if(button((Rectangle){(float)GetScreenWidth()-168,(float)h-84,120,56},"QUIT",false) || IsKeyPressed(KEY_ESCAPE))app.quit=true;
#endif
    text_line("W / S","THRUST / BRAKE",48,(int)y+385);
    text_line("MOUSE","STEER / CLIMB",48,(int)y+412);
    text_line("SHIFT / F","BOOST / SPIN DODGE",48,(int)y+439);
    DrawText("A/D STEER   E/Q CLIMB   C CAMERA   R RESET",48,h-67,15,muted);
    DrawText("FLIGHT PROTOTYPE  /  RACING & ONLINE ARE COMING",48,h-40,14,muted);
    if(app.scene.logo.id)DrawTexturePro(app.scene.logo,(Rectangle){0,0,(float)app.scene.logo.width,(float)app.scene.logo.height},(Rectangle){(float)GetScreenWidth()-110,22,80,80},(Vector2){0,0},0,WHITE);
}

static Vector2 map_point(RRVec3 p,float x,float y)
{
    return (Vector2){x+16+(float)((p.x+750)/1900)*168,y+16+(float)((p.z+950)/1900)*168};
}
static void flight_ui(void)
{
    int w=GetScreenWidth(),h=GetScreenHeight();
    DrawRectangle(26,24,310,76,Fade(ink,.8f));DrawRectangle(26,24,3,76,cyan);
    DrawText("SKYLINE LOOP",43,38,22,white);
    DrawText(TextFormat("CHECKPOINT %d / 8",app.next_gate+1),43,68,17,cyan);
    int total=(int)app.elapsed;
    DrawText(TextFormat("%02d:%02d.%02d",total/60,total%60,(int)(app.elapsed*100)%100),w/2-65,30,27,white);
    DrawText("FIRST FLIGHT / OFFLINE",w/2-91,62,14,muted);
    float mx=(float)w-230,my=24;
    DrawRectangle((int)mx,(int)my,204,213,Fade(ink,.8f));
    for(int i=0;i<app.track.sample_count;i+=6){
        int next=(i+6)%app.track.sample_count;
        DrawLineV(map_point(app.track.samples[i],mx,my),map_point(app.track.samples[next],mx,my),Fade(cyan,.6f));
    }
    DrawCircleV(map_point(app.track.gates[app.next_gate].pos,mx,my),5,(Color){143,255,184,255});
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
    RRVec3 target=app.track.gates[app.next_gate].pos;
    Vector3 ahead=Vector3Subtract(vector(target),app.camera.position);
    if(Vector3DotProduct(ahead,Vector3Subtract(app.camera.target,app.camera.position))>0){
        Vector2 point=GetWorldToScreen(vector(target),app.camera);
        if(point.x>40 && point.x<w-40 && point.y>110 && point.y<h-140){
            DrawCircleLines((int)point.x,(int)point.y,22,(Color){143,255,184,255});
            DrawText(TextFormat("%d M",(int)rr_length(rr_sub(target,app.car.pos))),(int)point.x-26,(int)point.y+31,16,white);
        }
    }
    DrawCircleLines(w/2,h/2,6,Fade(WHITE,.5f));
    if(app.message_time>0)DrawText("CHECKPOINT CLEARED",w/2-151,110,22,(Color){143,255,184,255});
    if(!app.scene.car_loaded)DrawText("FALLBACK VEHICLE",w-212,251,14,muted);
    if(app.mode==RR_PAUSED){
        DrawRectangle(0,0,w,h,Fade(ink,.86f));int x=w/2-190,y=h/2-110;
        DrawText("FLIGHT PAUSED",x,y,38,white);
        if(app.overloaded)DrawText("Rendering was slow. Flight paused.",x,y+47,17,magenta);
        if(button((Rectangle){(float)x,(float)y+72,380,56},"RESUME",true)){app.mode=RR_FLYING;app.overloaded=false;app.slow_frames=0;capture();}
        if(button((Rectangle){(float)x,(float)y+144,380,56},"MAIN MENU",false)){app.mode=RR_MENU;EnableCursor();reset_car(true);}
    }
}

static void frame(void)
{
    double frame_start=GetTime();
    double dt=app.smoke?1.0/60.0:rr_frame_dt(GetFrameTime());
    app.frame++;
    if(IsWindowMinimized() || GetScreenWidth()<=0 || GetScreenHeight()<=0){
        if(app.mode==RR_FLYING){app.mode=RR_PAUSED;EnableCursor();}
        PollInputEvents();
#if !defined(PLATFORM_WEB)
        WaitTime(.1);
#endif
        return;
    }
    bool focused=IsWindowFocused();
    target_fps(app.smoke?60:!focused?5:30);
    if(IsKeyPressed(KEY_F11))ToggleFullscreen();
    bool pause_key=IsKeyPressed(KEY_ESCAPE)||IsKeyPressed(KEY_P)||pad_pressed(GAMEPAD_BUTTON_MIDDLE_RIGHT);
    if(app.mode==RR_FLYING && (pause_key || (!app.smoke && !IsWindowFocused()))){app.mode=RR_PAUSED;EnableCursor();}
    else if(app.mode==RR_PAUSED && pause_key && focused){app.mode=RR_FLYING;app.overloaded=false;app.slow_frames=0;capture();}
    if(app.smoke && app.frame==30)start_flight();
    if(app.load_requested && (focused || app.smoke)){
        app.load_requested=false;
        TraceLog(LOG_INFO,"STATE: scene load requested; fallback=%d glow=%d",app.fallback,app.glow);
        if(rr_scene_load(&app.scene,&app.track,app.fallback,app.glow)){
            app.scene_ready=true;start_flight();
        }else{
            TraceLog(LOG_ERROR,"STATE: scene load failed; returning to 2D menu");
            rr_scene_unload(&app.scene);app.failed=true;app.mode=RR_MENU;EnableCursor();
        }
    }
    if(app.mode==RR_FLYING){
        RRInput in=app.smoke?(RRInput){1,0,0,.1,false}:controls(dt);
        if(IsKeyPressed(KEY_C)||pad_pressed(GAMEPAD_BUTTON_RIGHT_FACE_UP))app.camera_mode=1-app.camera_mode;
        if(IsKeyPressed(KEY_R)||pad_pressed(GAMEPAD_BUTTON_MIDDLE_LEFT))reset_car(false);
        app.spin_cooldown=fmax(0,app.spin_cooldown-dt);
        if((IsKeyPressed(KEY_F)||pad_pressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) && app.spin_cooldown==0){rr_car_dodge(&app.car,in.steer<0?-1:1);app.spin_cooldown=RR_SPIN_COOLDOWN;}
        RRVec3 previous=app.car.pos;
        rr_car_step(&app.car,in,dt);app.elapsed+=dt;
        app.message_time=fmax(0,app.message_time-dt);
        if(rr_gate_crossed(&app.track.gates[app.next_gate],previous,app.car.pos,13.2)){
            app.next_gate=(app.next_gate+1)%RR_TRACK_GATES;app.passed_gates++;app.message_time=2;
        }
        if(app.car.pos.y<34 || rr_length(app.car.pos)>10000)reset_car(false);
    }
    update_camera(dt);
    if(app.mode==RR_FLYING && (focused || app.smoke) && app.scene_ready){
        if(rr_scene_begin(&app.scene,app.camera)){
            rr_scene_world(&app.scene,&app.track,app.camera,app.next_gate,GetTime());
            rr_scene_car(&app.scene,&app.car);
            rr_scene_finish(&app.scene);
        }else{
            TraceLog(LOG_ERROR,"STATE: render target unavailable; returning to 2D menu");
            rr_scene_unload(&app.scene);app.scene_ready=false;app.failed=true;app.mode=RR_MENU;EnableCursor();
            BeginDrawing();ClearBackground(ink);
        }
    }else{
        BeginDrawing();ClearBackground(ink);
        DrawRectangleGradientV(0,0,GetScreenWidth(),GetScreenHeight(),ink,(Color){30,19,58,255});
    }
    if(app.mode==RR_MENU)menu_ui();else flight_ui();
    EndDrawing();
    double frame_seconds=GetTime()-frame_start;
    if(frame_seconds>app.worst_frame)app.worst_frame=frame_seconds;
    if(app.mode==RR_FLYING && !app.smoke && rr_render_overloaded(frame_seconds,&app.slow_frames)){
        TraceLog(LOG_WARNING,"PERFORMANCE: 10 consecutive slow frames; pausing flight (last %.1f ms)",frame_seconds*1000);
        app.mode=RR_PAUSED;app.overloaded=true;app.slow_frames=0;EnableCursor();
    }
    if(GetTime()-app.diagnostic_time>=5){
        TraceLog(LOG_INFO,"HEARTBEAT: frame=%d mode=%d window=%dx%d target=%dx%d worst-frame=%.1fms focused=%d",app.frame,app.mode,GetScreenWidth(),GetScreenHeight(),app.scene.target.texture.width,app.scene.target.texture.height,app.worst_frame*1000,focused);
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
        EM_ASM({Module.rrSmoke={};Module.rrSmoke.frame=$0;Module.rrSmoke.carLoaded=!!$1;Module.rrSmoke.speed=$2;Module.rrSmoke.mode=$3;},app.frame,app.scene.car_loaded,app.car.speed,app.mode);
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
    TraceLog(LOG_INFO,"SESSION: bounded renderer, 2D startup, 30 FPS, MSAA off, glow=%d",app.glow);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_VSYNC_HINT);
    InitWindow(960,720,"Roadsters Race - First Flight");
    if(!IsWindowReady()){fputs("Cannot create the game window.\n",stderr);return 1;}
    SetWindowMinSize(960,720);SetExitKey(KEY_NULL);
#if defined(PLATFORM_WEB)
    SetTargetFPS(0);
#else
    target_fps(app.smoke?60:30);
#endif
    rlSetClipPlanes(.1,2500);
    app.mode=RR_MENU;app.camera.up=(Vector3){0,1,0};app.camera.projection=CAMERA_PERSPECTIVE;app.camera_fov=68;
    if(!rr_track_init(&app.track)){
        TraceLog(LOG_ERROR,"Track initialization failed.");CloseWindow();return 1;
    }
    reset_car(true);
    TraceLog(LOG_INFO,"STARTUP: 2D menu ready; track %.3f m, %d samples; scene deferred",app.track.length,app.track.sample_count);
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(frame,0,1);
#else
    while(!WindowShouldClose() && !app.quit && (!app.smoke || app.frame<151))frame();
    bool asset_failure=app.smoke && !app.fallback && !app.scene.car_loaded;
    if(app.smoke)printf("FIRST_FLIGHT_SMOKE: %d frames, model=%s, speed=%.3f m/s, output=captures/\n",app.frame,app.scene.car_loaded?"loaded":"fallback",app.car.speed);
    rr_scene_unload(&app.scene);CloseWindow();
    TraceLog(LOG_INFO,"SESSION: clean shutdown");
    if(diagnostic_log){fclose(diagnostic_log);diagnostic_log=NULL;}
    return (asset_failure || app.failed)?1:0;
#endif
}
