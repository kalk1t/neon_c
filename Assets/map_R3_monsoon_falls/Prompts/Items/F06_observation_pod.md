# F06 — Cantilever observation pod

**Purpose:** Individual modeling reference for R3 — Monsoon Falls.

- PNG: [../../Assets/Environment/F06_observation_pod.png](../../Assets/Environment/F06_observation_pod.png)
- Selected style reference: [../../References/selected_map.png](../../References/selected_map.png)
- Asset category: architectural
- Native reference resolution: 1536 × 1024 pixels, RGBA

## Scale and coordinates

Local Blender dimension target: **X=20 m width; Y=16 m depth; Z=12 m height/drop**.

The descriptive target is **20 m wide × 16 m deep × 12 m tall, including short bracket**. These are authored production design targets, not measurements extracted from the illustration. Set the dimensions in Blender and validate them against the racing course. Pixel framing and perspective do not encode meter scale.

Blender authoring uses local X for width, Y for depth and Z for up. The game's canonical layout uses its own Y-up convention. Follow the top-level coordinate conversion in the integration guide; do not read the local XYZ size tuple as game XYZ coordinates.

## Origin and pivot

Center of rear bracket mounting plate; local +Y points out from the cliff toward the cabin, local Z is up. Add a separate balcony center anchor.

## Preferred production workflow

Prefer Blender modeling for this engineered module. Its snap faces, scale, repeating ribs, material slots and pivots need explicit construction. Use the PNG as a visual target; optional Meshy output can help with exploratory massing but should not define final mounting geometry.

1. Build the primary shape from simple boxes/cylinders at the authored dimensions.
2. Add a limited bevel and only the silhouette-defining mechanical forms. Use materials or baked detail for tiny bolts and panels.
3. Separate glass, dark structure, concrete and cyan emissive trim into deliberate materials where relevant.
4. Apply transforms, establish the prescribed pivot and mounting anchors, and create simple collision proxies.
5. Author LODs and export using the top-level guide. Confirm repetitive modules meet without double-width seams.

### Item-specific construction

The bracket attachment point must be easy to align to a cliff face. Keep cabin glass and opaque supporting structure separate so transparency can be controlled by the game.

## Collision approach

Coarse cylinder/box hull for cabin plus a bracket box if reachable; omit the small rail tubes from collision. Treat glass as opaque proxy collision if players can approach it.

## LOD targets

These are starting triangle budgets for one production render asset, not measured triangle counts of any supplied model. This pack supplies PNG references; no production mesh is implied. Adjust after profiling the actual scene and target devices. Collision geometry and VFX particles are separate budgets.

| Level | Target triangles |
|---|---:|
| LOD0 | 10,000 |
| LOD1 | 4,000 |
| LOD2 | 1,200 |

## Optional Meshy wording

Use this item's isolated PNG if a Meshy draft is helpful; inspect its result in Blender before integration.

> Optional visual study: one rounded glass observation cabin on a short cantilever metal bracket, dark graphite metal and cyan trim, no cliff or scene. Rebuild glazing frames and bracket in Blender for modular mounting.

No Meshy generation job was executed for this reference item.

## Exact image-generation prompt

The following is the verbatim prompt used for this PNG. The supplied local source image is the same selected concept included in this ZIP as References/selected_map.png.

~~~text
Create ONE individual isolated PNG modeling-reference asset for R3 MONSOON FALLS, a stylized high-quality realistic 3D browser flying-racing environment. The provided image is STYLE REFERENCE ONLY: use its wet dark basalt, saturated natural rainforest greens, and refined dark industrial materials; do not reproduce its scenery or track. Subject: ONE compact futuristic rainforest hydrodam observation outpost, a circular or rounded glass viewing cabin with a low dark metallic roof and continuous wraparound tinted glazing, two restrained thin cyan light strips, narrow exterior safety railing and a clean compact silhouette. The cabin sits on ONE SHORT triangular dark steel cantilever bracket that protrudes behind it for attachment to a cliff, with a visible rear mounting plate; no cliff included. Nominal overall dimensions 20 m width, 16 m depth, 12 m height including bracket. Match the source image's refined round glass outpost. No furnishings visible beyond simple dim interior silhouettes, no people, antennas, roads, vegetation or extra buildings. Single object only, three-quarter studio view with orthographic-like mild perspective, neutral diffuse studio illumination with readable material detail, entire silhouette centered and uncropped, generous 12 percent transparent margin on every side. TRUE TRANSPARENT ALPHA BACKGROUND. No floor, ground plane, cast shadow, environment, sky, rain, fog, track, cars, people, labels, text, UI, frame, watermark, or additional objects. No collection sheet or additional views. No neon unless explicitly specified. The object must have a coherent buildable solid form, not scenery.
~~~

Generation settings: transparent_background=true; one referenced concept image; one output asset; no post-generation raster editing.

## Reference QA and limitations

Round glazed cabin, cyan trim, rear cantilever bracket; no cliff.

The PNG was visually inspected, its alpha channel was checked, and its original bytes were copied unchanged. Anti-aliased edges and partially transparent pixels are expected. This illustration defines appearance and silhouette; it does not supply measured orthographic geometry, ready collision, UVs, material maps or finished game models.

Source PNG identifier: exec-8afd2490-7866-4b88-b597-b7e7e970d9d6.png

SHA-256: e1c7c3d4ebed59503c1043fd7e4d574f434b60b526f76d1212a1688fb8e8cb88
