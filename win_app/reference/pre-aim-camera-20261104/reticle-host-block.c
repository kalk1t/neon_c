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
