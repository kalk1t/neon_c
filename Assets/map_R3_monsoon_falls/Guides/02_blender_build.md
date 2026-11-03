# Blender build workflow

## 1. Validate and create the starter

Work from this extracted pack's root. Keep the Blender scripts together because they import shared geometry and route helpers.

~~~bash
python3 Blender/validate_routes.py --route Layout/route.json --placements Layout/placements.json
python3 Blender/build_map.py --check --config Layout/route.json --placements Layout/placements.json
blender --factory-startup --background --python Blender/build_map.py -- --config Layout/route.json --placements Layout/placements.json --output-dir Build --export-glb
~~~

The first two commands validate source data without Blender. The third requires a local Blender installation and creates the procedural blockout. Use a new output directory when iterating; the script refuses to overwrite named outputs unless you deliberately add --force.

The script is delivered as **source**. It was not run inside Blender in the preparation environment because Blender and bpy were unavailable. Run it locally and address any Blender-version API difference before calling its output verified. The source includes a version-tolerant approach where practical, but that does not replace execution.

The output is a starter scene: a dimensioned route/deck, gates, placeholders, simple materials, water reference surface, spawn/reset markers and cameras. A generated blockout is not a finished environment. The PNG references are not automatically converted into meshes by this script.

## 2. Coordinate and scale contract

Use meters. The layout JSON uses game coordinates **X right, Y up, Z forward**. Blender is Z up. The explicit conversion in the source kit is:

~~~text
Blender X = Game X
Blender Y = -Game Z
Blender Z = Game Y

Game X = Blender X
Game Y = Blender Z
Game Z = -Blender Y
~~~

A game point (10, 20, 30) therefore becomes Blender (10, -30, 20). The glTF export uses Y-up; do not apply the same conversion again in the game loader. Check a named origin and a forward marker after import.

The route describes the **car's flight centerline**. The deck surface is three meters below it in the starter. The reference vehicle is 5.6 m long, with a maximum planned width of 3.92 m; its height in the blockout is an assumption until Claude measures the actual game model and collider.

Individual item dimensions in asset_catalog.json are local Blender width/depth/height targets. Layout/placements.json uses local game width/height/depth envelopes and records the origin convention. Do not copy a tuple between these conventions unchanged. Layout envelopes and clearance requirements take priority when making the starter; rebuild repeated panels or structural sections when dimensions change, rather than stretching detailed textures and trim.

## 3. Build precise parts first

Use Blender for the continuous deck, module seams, start arch, checkpoint rings, barriers, supports and buildings/dam structural massing. Their connections and clear openings must be controlled.

Construct one useful cross-section: neutral deck surface, structural underside, thin cyan edge inlays and small magenta marks. Sweep it along the route, or generate modules with shared boundary vertices. The starter demonstrates the continuous approach. Make expansion joints/decorative panels separate and keep them out of the car's collision path.

Treat the beginning/end of the loop as one continuous seam. Check normal direction, slope and width there. Do not scale a finished 90-degree bend until it approximately meets the next segment; derive the curve and resample it instead.

Keep an 18 m useful deck width and 22 m clear checkpoint diameter at the initial blockout scale. These are proposed level-design values, not verified limits of the existing game. Test overtaking, attacks, steering, camera clearance and emergency resets with all three cars.

## 4. Replace placeholders with individual items

Use the matching asset ID on every placeholder. Model/import the corresponding item, normalize its transform, establish its local origin and fit its intended envelope. Preserve the instance's world position and heading. Rebuild a custom collision proxy from the item notes.

Use a base-center placement wrapper when the source item's pivot differs from the layout origin. Fit the accepted mesh to its intended size, measure its bounds, then offset it as a child of the wrapper; preserve the wrapper's world transform. Top-pivot items move upward inside the wrapper by their fitted height and extend downward. Rear-mounted items need a measured depth and vertical offset. See the per-instance source-pivot metadata and Blender/README.md for concrete exceptions. Never assume that the illustrated pivot and the layout envelope origin are interchangeable.

Keep instances of repeated scenery linked to shared geometry and material resources. Large one-off cliffs can use several low-cost modules, with the closest section receiving the most detail. For building facades, repeat small panel/window elements or use an atlas. Avoid thousands of independent bolts and windows.

Place vines, trees, roof equipment and sign faces after the collision corridor is stable. Do not obstruct the ring opening or fill an arch with one large convex hull. Keep the visible mesh and collider separate.

## 5. Export and inspect

Export a Y-up GLB, apply required static transforms, preserve meaningful node names and export metadata only when the engine reads it. Convert non-mesh geometry needed at runtime to meshes. Do not assume that every Blender node network, procedural material or modifier will reproduce in the browser.

Use simple metal/rough materials and the intended texture channels. Check the exported file in the actual game loader, including alpha, backfaces, emissive strips and scale. Add compression only after the loader's support is confirmed, then compare the compressed and uncompressed result.

For production, split distant scenery from the racing corridor if that helps streaming or culling. Export collision/metadata through the real project's established convention. Never display collision proxies as opaque map scenery.

Official references: Blender glTF export operator parameters and the maintained glTF-Blender-IO material/export guide are linked in SOURCES.md.
