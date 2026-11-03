# F04 — Modular hydrodam segment

**Purpose:** Individual modeling reference for R3 — Monsoon Falls.

- PNG: [../../Assets/Environment/F04_dam_section.png](../../Assets/Environment/F04_dam_section.png)
- Selected style reference: [../../References/selected_map.png](../../References/selected_map.png)
- Asset category: architectural
- Native reference resolution: 1024 × 1536 pixels, RGBA

## Scale and coordinates

Local Blender dimension target: **X=40 m width; Y=15 m depth; Z=100 m height/drop**.

The descriptive target is **40 m wide × 15 m deep × 100 m tall**. These are authored production design targets, not measurements extracted from the illustration. Set the dimensions in Blender and validate them against the racing course. Pixel framing and perspective do not encode meter scale.

Blender authoring uses local X for width, Y for depth and Z for up. The game's canonical layout uses its own Y-up convention. Follow the top-level coordinate conversion in the integration guide; do not read the local XYZ size tuple as game XYZ coordinates.

## Origin and pivot

Base center at Z=0. X is width/repetition axis; planar snap ends at X=−20 and +20 m. Water-facing plane is local +Y.

## Preferred production workflow

Prefer Blender modeling for this engineered module. Its snap faces, scale, repeating ribs, material slots and pivots need explicit construction. Use the PNG as a visual target; optional Meshy output can help with exploratory massing but should not define final mounting geometry.

1. Build the primary shape from simple boxes/cylinders at the authored dimensions.
2. Add a limited bevel and only the silhouette-defining mechanical forms. Use materials or baked detail for tiny bolts and panels.
3. Separate glass, dark structure, concrete and cyan emissive trim into deliberate materials where relevant.
4. Apply transforms, establish the prescribed pivot and mounting anchors, and create simple collision proxies.
5. Author LODs and export using the top-level guide. Confirm repetitive modules meet without double-width seams.

### Item-specific construction

Use exactly planar side mounting ends. When repeating segments into a curve, pivot/rotate authored sections along the dam arc and fill only intentional structural joints. This reference is one solid wall section, not an entire dam.

## Collision approach

A simple solid box or a few coarse convex wall/buttress hulls. Keep decorative rails and service detail out of collision unless reachable. Course collision stays separate.

## LOD targets

These are starting triangle budgets for one production render asset, not measured triangle counts of any supplied model. This pack supplies PNG references; no production mesh is implied. Adjust after profiling the actual scene and target devices. Collision geometry and VFX particles are separate budgets.

| Level | Target triangles |
|---|---:|
| LOD0 | 9,000 |
| LOD1 | 3,500 |
| LOD2 | 900 |

## Optional Meshy wording

Use this item's isolated PNG if a Meshy draft is helpful; inspect its result in Blender before integration.

> Optional only for a rough visual study: one straight modular dam wall section with clean planar mounting ends. Production geometry should be rebuilt in Blender for reliable dimensions, snap planes and repetition.

No Meshy generation job was executed for this reference item.

## Exact image-generation prompt

The following is the verbatim prompt used for this PNG. The supplied local source image is the same selected concept included in this ZIP as References/selected_map.png.

~~~text
Create ONE individual isolated PNG modeling-reference asset for R3 MONSOON FALLS, a stylized high-quality realistic 3D browser flying-racing environment. The provided image is STYLE REFERENCE ONLY: use its wet dark basalt, saturated natural rainforest greens, and refined dark industrial materials; do not reproduce its scenery or track. Subject: ONE independent modular hydroelectric dam wall segment, tall and narrow, approximately 40 m wide, 15 m thick, 100 m tall. A coherent vertical dark concrete wall with charcoal metallic buttresses, restrained weathering, a narrow parapet maintenance walkway at the top and one cyan strip near the crest. Its left and right mounting end faces are straight vertical planar faces so the segment can be arrayed to create a dam in Blender; no whole curved dam and no attached cliffs. The front face is a solid concrete panel with ribs and service panels, NO spillway, NO water. Fully visible from top walkway to structural bottom. Clean dark technical engineering compatible with the source image. Single object only, three-quarter studio view with orthographic-like mild perspective, neutral diffuse studio illumination with readable material detail, entire silhouette centered and uncropped, generous 12 percent transparent margin on every side. TRUE TRANSPARENT ALPHA BACKGROUND. No floor, ground plane, cast shadow, environment, sky, rain, fog, track, cars, people, labels, text, UI, frame, watermark, or additional objects. No collection sheet or additional views. No neon unless explicitly specified. The object must have a coherent buildable solid form, not scenery.
~~~

Generation settings: transparent_background=true; one referenced concept image; one output asset; no post-generation raster editing.

## Reference QA and limitations

Standalone straight wall module; no water; planar snap ends require exact rebuilding in Blender.

The PNG was visually inspected, its alpha channel was checked, and its original bytes were copied unchanged. Anti-aliased edges and partially transparent pixels are expected. This illustration defines appearance and silhouette; it does not supply measured orthographic geometry, ready collision, UVs, material maps or finished game models.

Source PNG identifier: exec-dcc6c287-152b-45dc-99bb-7a858b8bb9c8.png

SHA-256: 99a95ab07bd1c134014f1ec2a5300d2a045ecddaf4f30f485f0a502b99a5bb63
