"""Reproduce the two authored course data sets and their precise layout diagrams.

The output is a new design inspired by the selected images. It is not geometry
reconstructed from those images. Matplotlib is only needed for the diagrams.
"""
from __future__ import annotations
import argparse
import json
import math
import os
import tempfile
from pathlib import Path
from route_tools import ClosedRoute, frame, add, mul, round_vec, yaw_for_forward, validate_layout, local_to_game


def save(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False)+"\n", encoding="utf-8")


def pose(route, distance_m):
    point, tangent = route.at_distance(distance_m)
    return dict(distance_m=round(distance_m % route.total_length, 6), position_game=round_vec(point), forward_game=round_vec(tangent))


def create_route(map_id):
    city = map_id == "R1"
    name = "Neon Downpour" if city else "Monsoon Falls"
    points=[]
    for i in range(24):
        t=math.tau*i/24
        x=(425*math.cos(t)+25*math.cos(3*t)) if city else (530*math.cos(t)+20*math.cos(3*t))
        z=(347*math.sin(t)+20*math.sin(2*t)) if city else (448*math.sin(t)+22*math.sin(2*t))
        y=90+44*math.sin(t+.5) if city else 105+69*math.sin(t+.35)
        points.append(dict(id=f"P{i:02d}",position_game=round_vec([x,y,z],6)))
    config=dict(schema_version="1.0", map_id=map_id, map_name=name,
                authored_layout_note="New authored playable blockout inspired by the selected concept; not a reconstruction or measurement of image geometry.",
                coordinate_system="game_x_right_y_up_z_forward_meters", closed=True,
                interpolation=dict(type="closed_uniform_catmull_rom",samples_per_span=128,length_method="Sum of 3072 spline-sampling chords; approximate arc length in meters"),
                route_width_m=18, deck_below_route_m=3, deck_thickness_m=1.2,
                nominal_vehicle=dict(length_m=5.6,width_m=3.92,assumed_height_for_blockout_m=2.4,height_note="Placeholder clearance assumption; measure the actual imported car colliders before final gameplay integration."),
                design_limits=dict(footprint_target_xz_m=[900,700] if city else [1100,900],minimum_elevation_m=40 if city else 30,maximum_elevation_m=140 if city else 180,maximum_grade_degrees=18,minimum_horizontal_turn_radius_m=100),
                environment=dict(weather="rain",water_level_y_m=0,water_extent_xz_m=[1600,1350] if city else [1850,1650],water_center_xz=[0,0],palette={"body":"#151D26","route_light":"#28DDF7","accent":"#EF36BE"} if city else {"body":"#233C39","route_light":"#54E5DC","accent":"#A3C45A"},
                                 rain_note="Runtime camera-local rain particles; no rain mesh in this starter. Waterfall flow is a runtime material/particle effect."),
                control_points=points)
    route=ClosedRoute(config)
    config["computed"]=route.stats()
    count=14 if city else 16
    checkpoints=[]
    for i in range(count):
        s=route.total_length*i/count
        cp=dict(id=f"CP{i:02d}", sequence=i, asset_id="C05" if i==0 else "C04", **pose(route,s),
                inner_diameter_m=22, ring_outer_diameter_m=26, trigger_center_radius_m=9,
                trigger_radius_note="Provisional center trigger: 11 m inner radius minus half the nominal 3.92 m width, rounded down. It does not account for rotated length, height, collider shape, banking, or swept collision; validate the actual car in the host game.",
                required_crossing="negative_to_positive_signed_distance_along_forward_normal",
                respawn=dict(**pose(route,s-18),behind_checkpoint_m=18,zero_linear_velocity=True,next_required_checkpoint=f"CP{i:02d}"))
        checkpoints.append(cp)
    config["checkpoints"]=checkpoints
    config["spawns"]=[dict(id="PLAYER_START", **pose(route,route.total_length-28),next_required_checkpoint="CP00", initial_lap_state="not_started")]
    config["race_order"]=dict(start_gate="CP00",after_start_required_sequence=[c["id"] for c in checkpoints[1:]]+["CP00"],first_cp00_crossing="Start lap; do not increment completed laps",reset_checkpoint_policy="Respawn before the required checkpoint, so that gate must be crossed again in the forward direction")
    sector_specs=([("Rooftop rise",0,.16),("Skybridge run",.16,.32),("Canal descent",.32,.48),("Western turnaround",.48,.62),("Canal sweep",.62,.80),("Return climb",.80,1)] if city else
                  [("Reservoir run",0,.16),("Dam ledge",.16,.32),("Waterfall traverse",.32,.48),("Basalt arch",.48,.64),("Ravine low run",.64,.82),("Return climb",.82,1)])
    config["sectors"]=[dict(id=f"S{i+1:02d}",name=title,start_distance_m=round(a*route.total_length,6),end_distance_m=round(b*route.total_length,6),note=("Broad unbanked turnaround; tune banking and speed after playtesting." if "turnaround" in title else "Art-direction zone; replace placeholders after route clearance is verified.")) for i,(title,a,b) in enumerate(sector_specs)]
    config["hazards"] = ([dict(id="RAIN_VISIBILITY",kind="weather",collision=False,note="Rain and mist are visual effects. Keep checkpoint silhouettes legible."),dict(id="CANAL_WATER",kind="out_of_bounds",water_y_m=0,collision=False,note="Use the host game's out-of-bounds/reset rules; no custom damage logic supplied."),dict(id="SKYBRIDGE_OPENING",kind="clearance",placement_id="N03_01",minimum_opening_width_m=75,collision=True)] if city else
                         [dict(id="RAIN_VISIBILITY",kind="weather",collision=False,note="Keep near-camera rain sparse and race gates readable."),dict(id="RESERVOIR_WATER",kind="out_of_bounds",water_y_m=0,collision=False,note="Route is above water. Reuse the host reset rules."),dict(id="WATERFALL_SPRAY",kind="vfx_zone",placement_id="F11_01",collision=False,note="Do not make the transparent waterfall plane a solid collider."),dict(id="BASALT_ARCH_OPENING",kind="clearance",placement_id="F02_01",minimum_opening_width_m=44,collision=True)])
    return config, route


def create_placements(config, route):
    items=[]
    def at(fraction, offset=0):
        p,t=route.at_distance(route.total_length*fraction)
        right,_,_=frame(t)
        return add(p,mul(right,offset)), yaw_for_forward(t)
    def box(asset_id,fraction,offset,size,base_y=0,kind="box",notes="",parts=None,collision=True,position=None,yaw=None,parent_id=None):
        p,angle=at(fraction,offset)
        p[1]=base_y
        if position is not None: p=position
        if yaw is not None: angle=yaw
        pivot,source_offset="base_center",[0,0,0]
        if asset_id in ("N05","N07","C15","F11","F12"):
            pivot,source_offset="top_center",[0,size[1],0]
        elif asset_id=="N10":
            pivot,source_offset="rear_mount_center",[0,size[1]/2,-size[2]/2]
        elif asset_id=="F06":
            pivot,source_offset="rear_bracket_center",[0,size[1]/2,size[2]/2]
        item=dict(id=f"{asset_id}_{sum(1 for i in items if i['asset_id']==asset_id)+1:02d}",asset_id=asset_id,placeholder_kind=kind,position_game=round_vec(p),yaw_degrees=round(angle,6),size_xyz_m=size,origin="base_center",source_asset_pivot=pivot,source_root_offset_game=source_offset,collision_mode="parts" if collision else "none",collision_parts=(parts if parts is not None else [dict(id="solid",center_local=[0,size[1]/2,0],size_xyz_m=size)]) if collision else [],notes=notes)
        if parent_id: item["anchor_instance_id"]=parent_id
        items.append(item)
        return item
    if config["map_id"]=="R1":
        box("N01",.08,82,[42,175,45],kind="tower",notes="Cyan vertical accents; neutral dark facade.")
        stepped=box("N02",.30,100,[48,155,58],kind="stepped_tower")
        # Wide hollow landmark: collision is the actual two piers and high beam.
        parts=[dict(id="left_tower",center_local=[-53.75,95,0],size_xyz_m=[32.5,190,42]),dict(id="right_tower",center_local=[53.75,95,0],size_xyz_m=[32.5,190,42]),dict(id="high_skybridge",center_local=[0,175,0],size_xyz_m=[140,16,34])]
        box("N03",.23,0,[140,190,42],kind="skybridge",parts=parts,notes="Hollow opening 75 m wide. Treat the three collision boxes separately; never use one filled bounding box. Skybridge beam bottom is Y=167 m.")
        box("N04",.39,-95,[58,90,65],kind="midrise")
        roof_p,roof_yaw=at(.025,0)
        roof=box("N05",.025,0,[52,4,45],base_y=roof_p[1]-10,kind="platform",notes="Underlying rooftop slab sits 6 m below flight center; deck is 3 m below. Starter geometry, not final roof detailing.")
        box("N06",.70,45,[15,30,180],kind="canal_wall")
        box("N06",.70,-45,[15,30,180],kind="canal_wall")
        bridge_p,_=at(.72)
        box("N07",.72,0,[110,5,28],base_y=bridge_p[1]-14,kind="bridge",notes="Deck-only canal bridge proxy below the flying course. The 110 x 5 x 28 m box describes only the deck slab, not a filled overall bridge. Build supports separately below it with a hollow underpass and validate each support collider.")
        vent_position=local_to_game([20,4,0],roof["position_game"],roof_yaw)
        box("N08",0,0,[5,4,6],position=vent_position,yaw=roof_yaw,kind="vent",parent_id=roof["id"])
        antenna_position=local_to_game([0,155,0],stepped["position_game"],stepped["yaw_degrees"])
        box("N09",0,0,[4,38,4],position=antenna_position,yaw=0,kind="antenna",parent_id=stepped["id"])
        billboard_p,_=at(.87,-65)
        box("N10",.87,-65,[32,22,5],base_y=billboard_p[1]-12,kind="billboard",notes="Blank panel. Add game text in the engine or Blender; do not bake generated letters.")
        cabinet_p,cabinet_yaw=at(.02,24)
        platform=box("C15",.02,24,[12,1.5,16],base_y=cabinet_p[1]-5,kind="service_platform")
        box("N11",.02,24,[2.5,3,2],base_y=cabinet_p[1]-3.5,kind="cabinet",parent_id=platform["id"])
        box("N12",0,0,[280,225,145],position=[0,0,-615],yaw=0,kind="cluster",collision=False,notes="Non-colliding distant skyline placeholder; replace with low-detail merged silhouettes or billboards.")
    else:
        cliff=box("F01",.42,-110,[100,155,80],kind="cliff",notes="Organic silhouette may be rebuilt or generated with Meshy. Use a simple collision hull after cleanup.")
        arch_p,_=at(.565)
        arch_h=round(arch_p[1]+40,3)
        parts=[dict(id="left_rock_pier",center_local=[-33.5,arch_h/2,0],size_xyz_m=[23,arch_h,38]),dict(id="right_rock_pier",center_local=[33.5,arch_h/2,0],size_xyz_m=[23,arch_h,38]),dict(id="arch_crown",center_local=[0,arch_p[1]+31,0],size_xyz_m=[90,18,38])]
        box("F02",.565,0,[90,arch_h,38],kind="rock_arch",parts=parts,notes="44 m wide clear opening; crown bottom is 22 m above local flight center. Keep this aperture in every replacement mesh and collider.")
        mesa=box("F03",.71,125,[125,112,110],kind="mesa")
        dam=box("F04",.24,-120,[140,110,38],kind="dam",notes="Precise modular dam structure belongs in Blender. Do not ask Meshy to determine the route clearance.")
        spill_position=local_to_game([0,92,25],dam["position_game"],dam["yaw_degrees"])
        spill=box("F05",0,0,[42,14,26],position=spill_position,yaw=dam["yaw_degrees"],kind="spillway",parent_id=dam["id"])
        observation_p,_=at(.37,66)
        platform=box("C15",.37,66,[25,4,24],base_y=observation_p[1]-16,kind="service_platform")
        box("F06",.37,66,[20,12,19],base_y=observation_p[1]-12,kind="observation_pod",parent_id=platform["id"])
        for asset_id,local,size,kind in [("F07",[-27,112,-18],[24,33,24],"tree"),("F08",[26,112,10],[16,30,16],"palm"),("F09",[-35,112,24],[7,4,7],"fern")]:
            box(asset_id,0,0,size,position=local_to_game(local,mesa["position_game"],mesa["yaw_degrees"]),yaw=mesa["yaw_degrees"],kind=kind,parent_id=mesa["id"],notes="Anchored to mesa top in this blockout. Scatter additional instances only after terrain surfaces exist.")
        box("F10",.83,-90,[24,24,22],kind="boulder")
        waterfall_pos=local_to_game([0,-87,15],spill["position_game"],spill["yaw_degrees"])
        box("F11",0,0,[38,87,.4],position=waterfall_pos,yaw=spill["yaw_degrees"],kind="waterfall",collision=False,parent_id=spill["id"],notes="VFX placement plane only. Replace material with scrolling waterfall texture and spray; keep collision disabled.")
        vine_pos=local_to_game([40,95,-40],cliff["position_game"],cliff["yaw_degrees"])
        box("F12",0,0,[6,60,1],position=vine_pos,yaw=cliff["yaw_degrees"],kind="vine",collision=False,parent_id=cliff["id"],notes="Small transparent-plane proxy. Final hanging vines should stay outside the route.")
    return dict(schema_version="1.0",map_id=config["map_id"],coordinate_system=config["coordinate_system"],layout_status="authored_blockout",origin_convention="All instance origins are base-center; sizes are local [X width, Y height, Z depth] in meters. Positive yaw rotates local +Z toward game +X.",
                reserved_flight_corridor=dict(half_width_m=11,half_height_m=2,method="Conservative local-XZ AABB inflation of each collision part against every dense route chord. 11 m is route half-width 9 m plus 2 m clearance; vertical envelope is +/-2 m. This is a source-layout check, not swept vehicle collision or engine physics.",replacement_warning="Recheck final mesh colliders, banks, route changes, and all spawn points. Empty space in arches/skybridges must stay empty."),
                instances=items)


def draw_plan(config, placements, directory):
    os.environ.setdefault("MPLCONFIGDIR",str(Path(tempfile.gettempdir())/"neon_circuit_matplotlib_cache"))
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from matplotlib.patches import Polygon
    from route_tools import local_to_game
    route=ClosedRoute(config)
    sampled=route.samples(4)
    p=[s["position_game"] for s in sampled]+[sampled[0]["position_game"]]
    d=[s["distance_m"] for s in sampled]+[route.total_length]
    colors=["#42D9DD","#EF77C7","#FFCF78","#91ACFF","#84D495","#DE9AF4"]
    plt.rcParams.update({"font.family":"DejaVu Sans","font.size":9,"svg.fonttype":"none"})
    fig=plt.figure(figsize=(12,10),facecolor="#101923")
    grid=fig.add_gridspec(2,1,height_ratios=[3.5,1],hspace=.28,left=.09,right=.91,top=.86,bottom=.08)
    ax=fig.add_subplot(grid[0],facecolor="#14212C")
    for item in placements["instances"]:
        w,h,z=item["size_xyz_m"]
        corners=[local_to_game([a,0,b],item["position_game"],item["yaw_degrees"]) for a,b in [(-w/2,-z/2),(w/2,-z/2),(w/2,z/2),(-w/2,z/2)]]
        poly=Polygon([[a[0],a[2]] for a in corners],facecolor="#344653",edgecolor="#78909B",alpha=.6,linewidth=.5)
        ax.add_patch(poly)
        if item["asset_id"] in ("N01","N02","N03","N04","N10","N12","F01","F02","F03","F04","F06","F10"):
            ax.text(item["position_game"][0],item["position_game"][2],item["asset_id"],color="#A7B7C4",fontsize=6,ha="center",va="center")
    ax.plot([q[0] for q in p],[q[2] for q in p],color="#536773",linewidth=7,alpha=.8)
    for sector,color in zip(config["sectors"],colors):
        group=[s for s in sampled if sector["start_distance_m"] <= s["distance_m"] <= sector["end_distance_m"]]
        ax.plot([s["position_game"][0] for s in group],[s["position_game"][2] for s in group],color=color,linewidth=3,label=sector["name"])
    for cp in config["checkpoints"]:
        x,y,z=cp["position_game"]
        ax.scatter(x,z,color="white",s=17,zorder=5)
        ax.annotate(cp["id"],(x,z),xytext=(5,6),textcoords="offset points",color="white",fontsize=7)
    for f in [.06,.29,.53,.78]:
        a,t=route.at_distance(route.total_length*f)
        ax.annotate("",xy=(a[0]+t[0]*38,a[2]+t[2]*38),xytext=(a[0],a[2]),arrowprops=dict(arrowstyle="-|>",color="white",lw=1.3))
    ax.set_aspect("equal","datalim")
    ax.set_xlabel("Game X · meters · right",color="#DCE9EE")
    ax.set_ylabel("Game Z · meters · forward",color="#DCE9EE")
    ax.grid(alpha=.13,color="#9FB4C0")
    ax.tick_params(colors="#9FB4C0")
    for spine in ax.spines.values(): spine.set_color("#425461")
    legend=ax.legend(loc="lower center",bbox_to_anchor=(.5,1.02),ncol=3,frameon=False,fontsize=8)
    for text in legend.get_texts(): text.set_color("#E1ECF0")
    elev=fig.add_subplot(grid[1],facecolor="#14212C")
    elev.fill_between(d,[q[1] for q in p],0,color="#41BFC9",alpha=.12)
    elev.plot(d,[q[1] for q in p],color="#64DFE2",linewidth=2,label="Flight center")
    elev.plot(d,[q[1]-3 for q in p],color="#A6B7C4",linewidth=1,linestyle="--",label="Deck top (−3 m)")
    for cp in config["checkpoints"]:
        elev.axvline(cp["distance_m"],color="#607F91",alpha=.3,linewidth=.7)
    elev.set_xlim(0,route.total_length)
    elev.set_ylim(0,config["design_limits"]["maximum_elevation_m"]+10)
    elev.set_xlabel("Distance along the closed course · meters",color="#DCE9EE")
    elev.set_ylabel("Game Y · meters",color="#DCE9EE")
    elev.tick_params(colors="#9FB4C0")
    elev.grid(alpha=.13,color="#9FB4C0")
    for spine in elev.spines.values(): spine.set_color("#425461")
    legend=elev.legend(loc="upper right",frameon=False,fontsize=8)
    for text in legend.get_texts(): text.set_color("#E1ECF0")
    fig.suptitle(f"{config['map_id']}  /  {config['map_name'].upper()}",x=.09,y=.97,ha="left",fontsize=21,color="white",weight="bold")
    stats=config["computed"]
    fig.text(.09,.927,f"NEW AUTHORED BLOCKOUT  ·  {stats['length_m']/1000:.2f} km loop  ·  18 m deck  ·  {len(config['checkpoints'])} gates  ·  {stats['maximum_grade_degrees']:.1f}° maximum grade",color="#A9C0CE",fontsize=10)
    fig.text(.09,.022,"Concept-inspired layout, not image-measured geometry. Gray shapes are replaceable landmark envelopes; hollow openings are defined in placements.json.",color="#8FA9B8",fontsize=8)
    fig.savefig(directory/"route_plan.png",dpi=160,facecolor=fig.get_facecolor())
    fig.savefig(directory/"route_plan.svg",facecolor=fig.get_facecolor())
    plt.close(fig)


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--output-root",type=Path,default=Path(__file__).resolve().parents[1])
    parser.add_argument("--no-diagrams",action="store_true")
    args=parser.parse_args()
    for map_id in ("R1","R3"):
        config,route=create_route(map_id)
        placements=create_placements(config,route)
        folder=args.output_root/map_id
        save(folder/"route.json",config)
        save(folder/"placements.json",placements)
        report=validate_layout(config,placements)
        save(folder/"validation_report.json",report)
        if not report["ok"]:
            raise SystemExit(json.dumps(report,indent=2))
        if not args.no_diagrams:
            draw_plan(config,placements,folder)
        print(map_id,json.dumps(report["computed"]),"clearance parts",report["placeholder_collision_parts_checked"])


if __name__=="__main__":
    main()
