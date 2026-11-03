# F12 — Hanging vine clump

**Purpose:** Individual modeling reference for R3 — Monsoon Falls.

- PNG: [../../Assets/Environment/F12_hanging_vines.png](../../Assets/Environment/F12_hanging_vines.png)
- Selected style reference: [../../References/selected_map.png](../../References/selected_map.png)
- Asset category: vegetation
- Native reference resolution: 1024 × 1536 pixels, RGBA

## Scale and coordinates

Local Blender dimension target: **X=2.5 m width; Y=1.2 m depth; Z=5 m height/drop**.

The descriptive target is **2.5 m spread × 1.2 m depth × 5 m drop**. These are authored production design targets, not measurements extracted from the illustration. Set the dimensions in Blender and validate them against the racing course. Pixel framing and perspective do not encode meter scale.

Blender authoring uses local X for width, Y for depth and Z for up. The game's canonical layout uses its own Y-up convention. Follow the top-level coordinate conversion in the integration guide; do not read the local XYZ size tuple as game XYZ coordinates.

## Origin and pivot

Top attachment-cluster center at local origin. Vines hang along −Z; X is lateral spread and Y is depth. This is an upper-origin dressing asset.

## Preferred production workflow

Prefer Blender-authored trunk/curve geometry and optimized leaf or frond cards. Meshy can help establish a trunk or overall organic silhouette, but dense generated leaves should be replaced or simplified before browser use. Keep the neutral-lit PNG as a modeling reference rather than using its studio shading as a validated material texture.

1. Establish the authored scale and the correct ground or hanging attachment pivot.
2. Build the main trunk/stem structure, then layer a modest number of leaf clusters or fronds.
3. Author appropriate foliage color/alpha/normal data from the final asset workflow, and check edge appearance against both dark and bright backgrounds.
4. Create lower LOD silhouettes or impostors and only the specified trunk collision where needed.
5. Scatter as instances where supported by the game's existing renderer and keep the racing corridor unobstructed.

### Item-specific construction

Anchor the upper root cluster to cliffs or the arch and let the geometry extend downward. Avoid putting fine dangling strands into collision.

## Collision approach

No collision by default. Vines must not snag or block flying cars; any gameplay rope interaction would require a separate system.

## LOD targets

These are starting triangle budgets for one production render asset, not measured triangle counts of any supplied model. This pack supplies PNG references; no production mesh is implied. Adjust after profiling the actual scene and target devices. Collision geometry and VFX particles are separate budgets.

| Level | Target triangles |
|---|---:|
| LOD0 | 2,200 |
| LOD1 | 700 |
| LOD2 | 100 |

## Optional Meshy wording

Use this item's isolated PNG if a Meshy draft is helpful; inspect its result in Blender before integration.

> One isolated hanging vine clump, connected at a small top root cluster, multiple descending leafy tendrils, no rock, wall or environment. Prefer Blender curves and optimized leaf cards; do not keep dense generated per-leaf topology for the browser asset.

No Meshy generation job was executed for this reference item.

## Exact image-generation prompt

The following is the verbatim prompt used for this PNG. The supplied local source image is the same selected concept included in this ZIP as References/selected_map.png.

~~~text
Create ONE individual isolated PNG modeling-reference asset for R3 MONSOON FALLS, a stylized high-quality realistic 3D browser flying-racing environment. The provided image is STYLE REFERENCE ONLY: use its wet dark basalt, saturated natural rainforest greens, and refined dark industrial materials; do not reproduce its scenery or track. Subject: ONE connected clump of hanging rainforest vines, authored 5 m total drop. A small compact brown root/tendril attachment cluster at the top from which several long curling slender stems descend, densely but irregularly covered with small rich-green heart-shaped tropical leaves. Some exposed bare roots and fine curling tendrils near bottom. Entire hanging silhouette visible with all leaf tips inside the frame. The clump is self-contained; no supporting branch, rock, wall, ground, pot, surrounding plants or scene. Slightly varied stem lengths for a natural uneven lower edge. Wet green foliage, brown fibrous stems. Single object only, three-quarter studio view with orthographic-like mild perspective, neutral diffuse studio illumination with readable material detail, entire silhouette centered and uncropped, generous 12 percent transparent margin on every side. TRUE TRANSPARENT ALPHA BACKGROUND. No floor, ground plane, cast shadow, environment, sky, rain, fog, track, cars, people, labels, text, UI, frame, watermark, or additional objects. No collection sheet or additional views. No neon unless explicitly specified. The object must have a coherent buildable solid form, not scenery.
~~~

Generation settings: transparent_background=true; one referenced concept image; one output asset; no post-generation raster editing.

## Reference QA and limitations

One connected hanging vine clump with top roots and leafed tendrils; no support scene.

The PNG was visually inspected, its alpha channel was checked, and its original bytes were copied unchanged. Anti-aliased edges and partially transparent pixels are expected. This illustration defines appearance and silhouette; it does not supply measured orthographic geometry, ready collision, UVs, material maps or finished game models.

Source PNG identifier: exec-a50c4318-7118-4239-a8e7-0f739e4c8006.png

SHA-256: 2f90cc378d7b6fcf403abac55e3d74760ebb1ca8ad4ad5184c85ceb0de1410b7
