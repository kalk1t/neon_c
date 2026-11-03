# F08 — Tall rainforest palm

**Purpose:** Individual modeling reference for R3 — Monsoon Falls.

- PNG: [../../Assets/Environment/F08_palm_tree.png](../../Assets/Environment/F08_palm_tree.png)
- Selected style reference: [../../References/selected_map.png](../../References/selected_map.png)
- Asset category: vegetation
- Native reference resolution: 1024 × 1536 pixels, RGBA

## Scale and coordinates

Local Blender dimension target: **X=10 m width; Y=10 m depth; Z=22 m height/drop**.

The descriptive target is **10 m frond spread × 10 m depth × 22 m height**. These are authored production design targets, not measurements extracted from the illustration. Set the dimensions in Blender and validate them against the racing course. Pixel framing and perspective do not encode meter scale.

Blender authoring uses local X for width, Y for depth and Z for up. The game's canonical layout uses its own Y-up convention. Follow the top-level coordinate conversion in the integration guide; do not read the local XYZ size tuple as game XYZ coordinates.

## Origin and pivot

At trunk ground-contact center, Z=0. Preserve the natural lean while ensuring bounding dimensions include the full frond spread.

## Preferred production workflow

Prefer Blender-authored trunk/curve geometry and optimized leaf or frond cards. Meshy can help establish a trunk or overall organic silhouette, but dense generated leaves should be replaced or simplified before browser use. Keep the neutral-lit PNG as a modeling reference rather than using its studio shading as a validated material texture.

1. Establish the authored scale and the correct ground or hanging attachment pivot.
2. Build the main trunk/stem structure, then layer a modest number of leaf clusters or fronds.
3. Author appropriate foliage color/alpha/normal data from the final asset workflow, and check edge appearance against both dark and bright backgrounds.
4. Create lower LOD silhouettes or impostors and only the specified trunk collision where needed.
5. Scatter as instances where supported by the game's existing renderer and keep the racing corridor unobstructed.

### Item-specific construction

Keep the whole crown inside the modeled bounds and avoid highly detailed thin leaf geometry at far LODs. Reproduce the trunk lean in the collision proxy only when nearby.

## Collision approach

One or two capsules following the trunk if reachable. Fronds have no collision; distant palms are decorative only.

## LOD targets

These are starting triangle budgets for one production render asset, not measured triangle counts of any supplied model. This pack supplies PNG references; no production mesh is implied. Adjust after profiling the actual scene and target devices. Collision geometry and VFX particles are separate budgets.

| Level | Target triangles |
|---|---:|
| LOD0 | 6,000 |
| LOD1 | 2,000 |
| LOD2 | 300 |

## Optional Meshy wording

Use this item's isolated PNG if a Meshy draft is helpful; inspect its result in Blender before integration.

> One isolated tall palm, one curved ringed trunk and a complete crown of green feather fronds, no ground or scenery. Use a generated trunk as reference and author fronds with optimized leaf cards or low-poly strips in Blender.

No Meshy generation job was executed for this reference item.

## Exact image-generation prompt

The following is the verbatim prompt used for this PNG. The supplied local source image is the same selected concept included in this ZIP as References/selected_map.png.

~~~text
Create ONE individual isolated PNG modeling-reference asset for R3 MONSOON FALLS, a stylized high-quality realistic 3D browser flying-racing environment. The provided image is STYLE REFERENCE ONLY: use its wet dark basalt, saturated natural rainforest greens, and refined dark industrial materials; do not reproduce its scenery or track. Subject: ONE complete tall rainforest palm tree, authored height 22 m with a 10 m spread crown of arching long feather-shaped green palm fronds. One slightly curved ringed dark-brown slender trunk, realistic taper, small natural root flare, coherent single crown. All leaves including fine tips and the full trunk base comfortably inside the frame. Wet tropical dark-to-bright green fronds, neutral studio light. No soil, no earth mound, no grass, no other plants, no coconuts scattered separately, no pot, no landscape. Single object only, three-quarter studio view with orthographic-like mild perspective, neutral diffuse studio illumination with readable material detail, entire silhouette centered and uncropped, generous 12 percent transparent margin on every side. TRUE TRANSPARENT ALPHA BACKGROUND. No floor, ground plane, cast shadow, environment, sky, rain, fog, track, cars, people, labels, text, UI, frame, watermark, or additional objects. No collection sheet or additional views. No neon unless explicitly specified. The object must have a coherent buildable solid form, not scenery.
~~~

Generation settings: transparent_background=true; one referenced concept image; one output asset; no post-generation raster editing.

## Reference QA and limitations

Single palm with full trunk, root flare and frond tips; no ground.

The PNG was visually inspected, its alpha channel was checked, and its original bytes were copied unchanged. Anti-aliased edges and partially transparent pixels are expected. This illustration defines appearance and silhouette; it does not supply measured orthographic geometry, ready collision, UVs, material maps or finished game models.

Source PNG identifier: exec-a1d94de9-61d1-4b3d-99ec-dbf0af3b8f96.png

SHA-256: cb1820ecdb5bc22396c9ba7b2ce90904deecf13c29f21279c6070d0897e703b6
