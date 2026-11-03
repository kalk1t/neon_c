"""Editable Blender starter for R1/R3; no downloaded assets or paid API calls.

Blender invocation (run from the extracted map folder):
  blender --factory-startup --background --python Blender/build_map.py -- \
    --config Layout/route.json --placements Layout/placements.json \
    --output-dir Build --export-glb

Plain Python data check (does not require Blender):
  python3 Blender/build_map.py --check --config Layout/route.json \
    --placements Layout/placements.json

The script was syntax-checked and its pure-Python geometry/data were validated,
but Blender is not installed in the authoring environment. Run and inspect the
generated scene before calling it production-ready.
"""
from __future__ import annotations
import argparse
import json
import math
import sys
from pathlib import Path

SCRIPT_DIR=Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0,str(SCRIPT_DIR))
from route_tools import ClosedRoute, read_json, validate_layout, game_to_blender, frame, add, mul, yaw_for_forward
from mesh_geometry import deck_mesh, ring_mesh, edge_strip_mesh, start_platform_mesh, box_mesh, frustum_mesh


def arguments():
    argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else sys.argv[1:]
    parser=argparse.ArgumentParser(description=__doc__,formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--config",required=True,type=Path,help="Path to Layout/route.json")
    parser.add_argument("--placements",type=Path,help="Defaults to placements.json beside the route")
    parser.add_argument("--output-dir",type=Path,default=Path("Build"))
    parser.add_argument("--export-glb",action="store_true")
    parser.add_argument("--force",action="store_true",help="Overwrite only this map's named output files and replace its named starter scene")
    parser.add_argument("--check",action="store_true",help="Validate source data and exit without importing bpy or creating files")
    return parser.parse_args(argv)


def main():
    args=arguments()
    config=read_json(args.config)
    placement_path=args.placements or args.config.with_name("placements.json")
    placements=read_json(placement_path)
    report=validate_layout(config,placements)
    if args.check or not report["ok"]:
        print(json.dumps(report,indent=2))
    if not report["ok"]:
        raise SystemExit("Source layout validation failed; no scene was created")
    if args.check:
        return
    try:
        import bpy
        from mathutils import Vector, Matrix
    except ImportError as error:
        raise SystemExit("Blender's bpy is unavailable. Use the blender executable, or pass --check for data validation only.") from error

    map_id=config["map_id"]
    if map_id not in ("R1","R3"):
        raise SystemExit("Only the authored R1 and R3 map IDs are supported")
    stem=f"{map_id.lower()}_starter"
    args.output_dir=args.output_dir.resolve()
    targets=[args.output_dir/(stem+".blend"),args.output_dir/(stem+"_build_report.json")]
    if args.export_glb: targets.append(args.output_dir/(stem+".glb"))
    existing=[str(p) for p in targets if p.exists()]
    if existing and not args.force:
        raise SystemExit("Refusing to overwrite existing outputs. Review them or pass --force:\n"+"\n".join(existing))
    scene_name=f"NC_{map_id}_STARTER"
    if scene_name in bpy.data.scenes:
        if not args.force:
            raise SystemExit(f"Scene {scene_name} already exists; use a fresh file or --force")
        old_scene=bpy.data.scenes[scene_name]
        # Only remove a scene created and marked by this script.
        if old_scene.get("source_kit")!="neon_circuit_map_starter_v1":
            raise SystemExit("Existing scene is not marked as this kit's output; it will not be replaced")
        if any(len(obj.users_scene)>1 for obj in old_scene.objects):
            raise SystemExit("Starter objects are shared with another scene. Use a fresh file instead of --force.")
        if bpy.context.window and bpy.context.window.scene==old_scene:
            bpy.context.window.scene=bpy.data.scenes.new("NC_SAFE_TEMP")
        for obj in list(old_scene.objects):
            bpy.data.objects.remove(obj,do_unlink=True)
        bpy.data.scenes.remove(old_scene)
    scene=bpy.data.scenes.new(scene_name)
    scene["source_kit"]="neon_circuit_map_starter_v1"
    scene["map_id"]=map_id
    scene["layout_note"]=config["authored_layout_note"]
    scene["game_coordinate_mapping"]="Blender (X,Y,Z) = game (X,-Z,Y); export glTF Y-up enabled"
    scene["source_layout_json"]=str(args.config.resolve())
    scene["is_placeholder_blockout"]=True
    scene.unit_settings.system="METRIC"
    scene.unit_settings.scale_length=1
    if bpy.context.window:
        bpy.context.window.scene=scene
    else:
        raise SystemExit("No Blender window context is available to select the new scene. Run the documented --factory-startup --background command.")
    route=ClosedRoute(config)
    collections={}
    for name in ("01_COURSE","02_GATES","03_ROUTE_MARKERS","04_SUPPORTS","05_LANDMARK_PLACEHOLDERS","06_COLLIDER_GUIDES","07_WATER_VFX","08_SPAWN_RESET","09_LIGHTS_CAMERAS"):
        c=bpy.data.collections.new(f"{map_id}_{name}")
        scene.collection.children.link(c)
        collections[name]=c

    def srgb_to_linear(value):
        return value/12.92 if value<=.04045 else ((value+.055)/1.055)**2.4

    def rgba(hex_color):
        hex_color=hex_color.lstrip("#")
        return tuple(srgb_to_linear(int(hex_color[i:i+2],16)/255) for i in (0,2,4))+(1,)

    def material(name,color,metallic=0,roughness=.6,emission=0):
        mat=bpy.data.materials.new(f"{map_id}_{name}")
        mat.use_nodes=True
        shader=mat.node_tree.nodes.get("Principled BSDF")
        shader.inputs["Base Color"].default_value=rgba(color)
        shader.inputs["Metallic"].default_value=metallic
        shader.inputs["Roughness"].default_value=roughness
        if emission:
            socket=shader.inputs.get("Emission Color")
            if socket is None: socket=shader.inputs.get("Emission")
            if socket is not None: socket.default_value=rgba(color)
            strength=shader.inputs.get("Emission Strength")
            if strength is not None: strength.default_value=emission
        mat.diffuse_color=rgba(color)
        return mat

    palette=config["environment"]["palette"]
    mats={"deck":material("WetDeck","#28323B",.25,.34),
          "frame":material("DarkStructure","#171F29",.4,.5),
          "cyan":material("RouteLight",palette["route_light"],.1,.35,2.2),
          "accent":material("AccentLight",palette["accent"],.1,.35,1.6),
          "rock":material("RockProxy","#334A49",0,.9),
          "leaf":material("FoliageProxy","#315E45",0,.85),
          "bark":material("BarkProxy","#413E30",0,.9),
          "concrete":material("Concrete","#64716F",.05,.75),
          "water":material("WaterProxy","#153B49",.1,.22),
          "spray":material("WaterfallProxy","#8ED0CF",0,.45),
          "panel":material("BlankPanel","#274354",.2,.35)}

    def mesh_object(name,data,collection,materials,parent=None):
        vertices,faces,uvs,indices=data
        mesh=bpy.data.meshes.new(name+"_MESH")
        mesh.from_pydata([game_to_blender(v) for v in vertices],[],faces)
        mesh.update(calc_edges=True)
        if mesh.validate(verbose=False):
            raise RuntimeError(f"Mesh validation changed {name}; inspect topology before export")
        layer=mesh.uv_layers.new(name="UVMap")
        for polygon,face_uv,mat_index in zip(mesh.polygons,uvs,indices):
            if len(polygon.loop_indices)!=len(face_uv):
                raise RuntimeError("UV loop count mismatch: "+name)
            for loop_index,uv in zip(polygon.loop_indices,face_uv):
                layer.data[loop_index].uv=uv
            polygon.material_index=mat_index
            polygon.use_smooth=False
        for mat in materials: mesh.materials.append(mat)
        obj=bpy.data.objects.new(name,mesh)
        collection.objects.link(obj)
        if parent is not None:
            obj.parent=parent
        obj["starter_geometry"]=True
        obj["uv_note"]="Starter UVs: swept deck distance UVs; primitive face/cylindrical UVs. Final asset atlasing and PBR baking remain production work."
        return obj

    def box(name,center,size,collection,mat,parent=None):
        return mesh_object(name,box_mesh(center,size),collection,[mat],parent)

    def empty(name,position,collection,forward=None,yaw=None,parent=None):
        obj=bpy.data.objects.new(name,None)
        collection.objects.link(obj)
        obj.empty_display_type="ARROWS"
        obj.empty_display_size=4
        obj.location=game_to_blender(position)
        if forward is not None:
            right,up,tangent=frame(forward)
            # Local Blender X=right, local -Y=forward, local Z=up.
            matrix=Matrix((Vector(game_to_blender(right)),Vector(mul(game_to_blender(tangent),-1)),Vector(game_to_blender(up)))).transposed()
            obj.rotation_euler=matrix.to_euler()
        elif yaw is not None:
            obj.rotation_euler[2]=math.radians(yaw)
        if parent is not None: obj.parent=parent
        return obj

    course=collections["01_COURSE"]
    deck=mesh_object("COURSE_FLIGHT_DECK_C01_C02_C03",deck_mesh(route),course,[mats["deck"],mats["frame"]])
    deck["asset_ids"]="C01,C02,C03"
    deck["deck_below_flight_center_m"]=3.0
    deck["route_width_m"]=18.0
    deck["collision_note"]="Visible swept deck. Author engine collision independently; game may fly above this surface."
    for side in (-1,1):
        mesh_object(f"ROUTE_EDGE_LIGHT_{side:+d}",edge_strip_mesh(route,side*8.75),course,[mats["cyan"]])
    spawn=config["spawns"][0]
    start_platform=mesh_object("C10_START_PLATFORM_24x48",start_platform_mesh(route,spawn["distance_m"]),course,[mats["deck"],mats["frame"]])
    start_platform["asset_id"]="C10"
    start_platform["design_dimensions_m"]=[24,1.4,48]
    start_platform["note"]="Swept along the local slope; top is 0.08 m below the main deck to avoid coplanar surfaces."

    # Checkpoint geometry, named trigger origin, and respawn marker.
    for cp in config["checkpoints"]:
        gate=mesh_object(cp["id"]+"_"+cp["asset_id"]+"_RING",ring_mesh(cp["position_game"],cp["forward_game"],cp["inner_diameter_m"]/2,cp["ring_outer_diameter_m"]/2),collections["02_GATES"],[mats["accent"] if cp["sequence"]==0 else mats["cyan"],mats["frame"]])
        gate["checkpoint_id"]=cp["id"]
        gate["asset_id"]=cp["asset_id"]
        gate["sequence"]=cp["sequence"]
        gate["collision_note"]="Do not use the gate's full bounding box as a solid collider."
        trigger=empty(cp["id"]+"_TRIGGER",cp["position_game"],collections["08_SPAWN_RESET"],forward=cp["forward_game"])
        trigger["inner_diameter_m"]=cp["inner_diameter_m"]
        trigger["provisional_center_trigger_radius_m"]=cp["trigger_center_radius_m"]
        trigger["forward_game"]=cp["forward_game"]
        trigger["required_crossing"]=cp["required_crossing"]
        r=cp["respawn"]
        reset=empty(cp["id"]+"_RESET",r["position_game"],collections["08_SPAWN_RESET"],forward=r["forward_game"])
        reset["next_required_checkpoint"]=r["next_required_checkpoint"]
        reset["zero_linear_velocity"]=True
    for spawn in config["spawns"]:
        obj=empty(spawn["id"],spawn["position_game"],collections["08_SPAWN_RESET"],forward=spawn["forward_game"])
        obj["next_required_checkpoint"]=spawn["next_required_checkpoint"]
        obj["initial_lap_state"]=spawn["initial_lap_state"]

    # Modest route markers/supports, not a high-detail scatter or rain simulation.
    for i in range(math.ceil(route.total_length/64)):
        p,t=route.at_distance(i*64)
        top=p[1]-4.2
        if top>0:
            obj=box(f"C06_SUPPORT_{i:03d}",[p[0],top/2,p[2]],[3.8,top,3.8],collections["04_SUPPORTS"],mats["frame"])
            obj["asset_id"]="C06"
            obj["collision_note"]="Support below the deck. Simplify or remove distant supports for browser budgets."
    for i in range(math.ceil(route.total_length/36)):
        p,t=route.at_distance(i*36)
        right,_,_=frame(t)
        for side in (-1,1):
            q=add(p,mul(right,side*11.6)); q[1]-=2.6
            root=empty(f"C09_BEACON_{i:03d}_{side:+d}",q,collections["03_ROUTE_MARKERS"],yaw=yaw_for_forward(t))
            box(root.name+"_POST",[0,0,0],[.6,.8,.6],collections["03_ROUTE_MARKERS"],mats["frame"],root)
            box(root.name+"_LIGHT",[0,.48,0],[.65,.16,.65],collections["03_ROUTE_MARKERS"],mats["cyan"],root)
            root["asset_id"]="C09"
    # Dotted center guide is geometry only; merge/instance it in the final game.
    for i in range(math.ceil(route.total_length/22)):
        p,t=route.at_distance(i*22)
        p[1]-=2.94
        root=empty(f"C14_DOT_{i:03d}",p,collections["03_ROUTE_MARKERS"],yaw=yaw_for_forward(t))
        box(root.name+"_MESH",[0,0,0],[.24,.035,1.2],collections["03_ROUTE_MARKERS"],mats["cyan"],root)
        root["asset_id"]="C14"
    # A few barrier and blank directional-sign modules outside the clear deck.
    for fraction in (.18,.52,.78):
        p,t=route.at_distance(route.total_length*fraction)
        right,_,_=frame(t)
        q=add(p,mul(right,12.8));q[1]-=2.0
        root=empty(f"C07_BARRIER_{fraction:.2f}",q,collections["03_ROUTE_MARKERS"],yaw=yaw_for_forward(t))
        box(root.name+"_MESH",[0,0,0],[1.1,2,8],collections["03_ROUTE_MARKERS"],mats["frame"],root)
        root["asset_id"]="C07"
        sign=empty(f"C08_SIGN_{fraction:.2f}",add(q,mul(right,3.5)),collections["03_ROUTE_MARKERS"],yaw=yaw_for_forward(t))
        box(sign.name+"_POST",[0,-.5,0],[.3,3,.3],collections["03_ROUTE_MARKERS"],mats["frame"],sign)
        box(sign.name+"_BLANK_PANEL",[0,1.5,0],[4,1.6,.2],collections["03_ROUTE_MARKERS"],mats["panel"],sign)
        sign["asset_id"]="C08"
        sign["note"]="Blank sign proxy; replace with the supplied chevron asset or mesh."

    # One editable root per replacement item. Local axes use +Y up before mapping.
    for item in placements["instances"]:
        coll=collections["07_WATER_VFX"] if item["placeholder_kind"] in ("waterfall","vine") else collections["05_LANDMARK_PLACEHOLDERS"]
        root=empty(item["id"]+"_REPLACE_ME",item["position_game"],coll,yaw=item["yaw_degrees"])
        root["asset_id"]=item["asset_id"]
        root["instance_id"]=item["id"]
        root["placeholder_kind"]=item["placeholder_kind"]
        root["size_xyz_game_m"]=item["size_xyz_m"]
        root["collision_mode"]=item["collision_mode"]
        root["notes"]=item["notes"]
        root["source_asset_pivot"]=item.get("source_asset_pivot","base_center")
        root["source_root_offset_game"]=item.get("source_root_offset_game",[0,0,0])
        if "anchor_instance_id" in item: root["anchor_instance_id"]=item["anchor_instance_id"]
        w,h,d=item["size_xyz_m"]
        kind=item["placeholder_kind"]
        organic=kind in ("cliff","mesa","rock_arch","boulder")
        mat=mats["rock"] if organic else mats["frame"]
        if kind in ("skybridge","rock_arch"):
            for part in item["collision_parts"]:
                box(root.name+"_"+part["id"],part["center_local"],part["size_xyz_m"],coll,mat,root)
        elif kind=="stepped_tower":
            for j,scale in enumerate((1,.78,.55)):
                box(root.name+f"_TIER_{j}",[0,h*(j+.5)/3,0],[w*scale,h/3,d*scale],coll,mat,root)
        elif kind=="cluster":
            for j in range(9):
                x=(j%3-1)*w*.29;z=(j//3-1)*d*.29
                height=h*(.48+.055*j)
                box(root.name+f"_TOWER_{j}",[x,height/2,z],[w*.21,height,d*.21],coll,mat,root)
        elif kind in ("tree","palm"):
            trunk_height=h*.62 if kind=="tree" else h*.82
            mesh_object(root.name+"_TRUNK",frustum_mesh([0,0,0],trunk_height,w*.06,w*.035,8),coll,[mats["bark"]],root)
            mesh_object(root.name+"_CANOPY",frustum_mesh([0,h*.43,0],h*.57,w*.5,w*.10,8),coll,[mats["leaf"]],root)
        elif kind=="fern":
            mesh_object(root.name+"_FOLIAGE",frustum_mesh([0,0,0],h,w*.5,w*.08,7),coll,[mats["leaf"]],root)
        elif kind in ("cliff","mesa","boulder"):
            mesh_object(root.name+"_FACETED_PROXY",frustum_mesh([0,0,0],h,min(w,d)*.5,min(w,d)*.43,8),coll,[mats["rock"]],root)
        elif kind=="waterfall":
            data=([[-w/2,0,0],[w/2,0,0],[w/2,h,0],[-w/2,h,0]],[[0,1,2,3]],[[(0,0),(1,0),(1,1),(0,1)]],[0])
            obj=mesh_object(root.name+"_VFX_PLANE",data,coll,[mats["spray"]],root)
            obj["collision"]=False
            obj["runtime_work"]="Scrolling UV material, alpha handling, and spray particles remain to be implemented."
        elif kind=="vine":
            box(root.name+"_VINE_PROXY",[0,h/2,0],[w,h,d],coll,mats["leaf"],root)
        elif kind=="antenna":
            box(root.name+"_MAST",[0,h/2,0],[w*.2,h,d*.2],coll,mat,root)
            box(root.name+"_CROSSBAR",[0,h*.8,0],[w,.4,d*.5],coll,mat,root)
        else:
            mat=mats["concrete"] if kind in ("dam","spillway","canal_wall","bridge") else mat
            box(root.name+"_BODY",[0,h/2,0],[w,h,d],coll,mat,root)
        if kind in ("tower","midrise","stepped_tower","billboard","cabinet","observation_pod"):
            box(root.name+"_ACCENT",[0,h*.77,d/2+.03 if kind=="billboard" else -d/2-.03],[w*.65,.3,.08],coll,mats["cyan"],root)
        for part in item["collision_parts"]:
            obj=box("COLLIDER_GUIDE_"+item["id"]+"_"+part["id"],part["center_local"],part["size_xyz_m"],collections["06_COLLIDER_GUIDES"],mats["accent"],root)
            obj.display_type="WIRE"
            obj.hide_render=True
            obj["collision_guide_only"]=True
            obj["do_not_render_or_export_as_visible_mesh"]=True
    collections["06_COLLIDER_GUIDES"].hide_render=True

    water=config["environment"]
    w,d=water["water_extent_xz_m"]
    x,z=water["water_center_xz"]
    y=water["water_level_y_m"]
    data=([[x-w/2,y,z-d/2],[x-w/2,y,z+d/2],[x+w/2,y,z+d/2],[x+w/2,y,z-d/2]],[[0,1,2,3]],[[(0,0),(0,d/40),(w/40,d/40),(w/40,0)]],[0])
    water_obj=mesh_object("WATER_LEVEL_PROXY",data,collections["07_WATER_VFX"],[mats["water"]])
    water_obj["collision"]=False
    water_obj["note"]="Flat opaque starter material. Engine water animation, reflection strategy, and out-of-bounds behavior remain implementation work."
    fx=empty("RAIN_SPLASH_MIST_RUNTIME_ONLY",[0,0,0],collections["07_WATER_VFX"])
    fx["asset_ids"]="C11,C12,C13"
    fx["note"]="No particle system built. Use supplied sprites in a camera-local, pooled runtime effect."

    world=bpy.data.worlds.new(map_id+"_STUDIO_WORLD")
    world.use_nodes=True
    world.node_tree.nodes["Background"].inputs[0].default_value=(.055,.07,.09,1)
    world.node_tree.nodes["Background"].inputs[1].default_value=.55
    scene.world=world
    sun_data=bpy.data.lights.new(map_id+"_KEY_SUN","SUN")
    sun_data.energy=2.0
    sun=bpy.data.objects.new(map_id+"_KEY_SUN",sun_data)
    collections["09_LIGHTS_CAMERAS"].objects.link(sun)
    sun.rotation_euler=(math.radians(28),math.radians(-20),math.radians(-35))
    def camera(name,position,target,ortho=None):
        data=bpy.data.cameras.new(name)
        data.clip_start=.5;data.clip_end=5000
        if ortho is not None: data.type="ORTHO";data.ortho_scale=ortho
        else: data.lens=42
        obj=bpy.data.objects.new(name,data)
        collections["09_LIGHTS_CAMERAS"].objects.link(obj)
        obj.location=game_to_blender(position)
        direction=Vector(game_to_blender(target))-obj.location
        obj.rotation_euler=direction.to_track_quat("-Z","Y").to_euler()
        return obj
    scene.camera=camera("CAM_OVERVIEW",[1150,1020,-1280],[0,70,0])
    camera("CAM_TOP_PLAN",[0,1600,0],[0,0,0],1450 if map_id=="R1" else 1700)
    start_pos,start_tangent=route.at_distance(config["spawns"][0]["distance_m"])
    camera("CAM_SPAWN_REVIEW",add(add(start_pos,mul(start_tangent,-24)),[0,8,0]),add(start_pos,mul(start_tangent,40)))
    scene.render.resolution_x=1600;scene.render.resolution_y=1000;scene.render.resolution_percentage=100
    scene.view_settings.view_transform="Standard"
    bpy.context.view_layer.update()

    args.output_dir.mkdir(parents=True,exist_ok=True)
    # glTF selects the active starter scene; hidden collision-guide meshes are
    # explicitly excluded by selection rather than relying on renderer visibility.
    exported=None
    if args.export_glb:
        for obj in scene.objects:
            obj.select_set(obj.type in ("MESH","EMPTY") and not obj.get("collision_guide_only",False))
        selected=[obj for obj in scene.objects if obj.select_get()]
        if not selected:
            raise RuntimeError("No starter geometry selected for export")
        bpy.context.view_layer.objects.active=next(o for o in selected if o.type=="MESH")
        requested=dict(filepath=str(args.output_dir/(stem+".glb")),export_format="GLB",use_selection=True,use_active_scene=True,export_yup=True,export_extras=True,export_cameras=False,export_lights=False,export_animations=False)
        supported={p.identifier for p in bpy.ops.export_scene.gltf.get_rna_type().properties}
        # These keys govern safety and coordinates; never silently omit them.
        for required in ("filepath","export_format","use_selection","export_yup"):
            if required not in supported:
                raise RuntimeError("This Blender glTF exporter lacks required option "+required)
        options={k:v for k,v in requested.items() if k in supported}
        with bpy.context.temp_override(scene=scene):
            result=bpy.ops.export_scene.gltf(**options)
        if "FINISHED" not in result:
            raise RuntimeError("glTF export did not finish: "+str(result))
        exported=dict(path=str(args.output_dir/(stem+".glb")),settings=options)
    with bpy.context.temp_override(scene=scene):
        result=bpy.ops.wm.save_as_mainfile(filepath=str(args.output_dir/(stem+".blend")))
    if "FINISHED" not in result:
        raise RuntimeError("Blend save did not finish: "+str(result))
    result=dict(map_id=map_id,blender_version=bpy.app.version_string,scene_name=scene_name,blend_path=str(args.output_dir/(stem+".blend")),glb=exported,
                object_count=len(scene.objects),mesh_object_count=sum(o.type=="MESH" for o in scene.objects),
                source_validation=report,status="Blender starter creation completed; no host-game integration, visual QA, or performance certification implied",
                limitations=["Replace primitive landmarks with approved assets","Rebuild final collision and test all vehicles","Optimize draw calls/materials and VFX for target devices","Starter UVs are present but not production atlases","No game-specific scripts, physics, rain system, or Meshy generation included"])
    (args.output_dir/(stem+"_build_report.json")).write_text(json.dumps(result,indent=2)+"\n",encoding="utf-8")
    print(json.dumps(result,indent=2))


if __name__=="__main__":
    main()
