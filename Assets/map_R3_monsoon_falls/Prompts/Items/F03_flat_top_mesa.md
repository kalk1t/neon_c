# F03 — Flat-top basalt mesa

**Purpose:** Individual modeling reference for R3 — Monsoon Falls.

- PNG: [../../Assets/Environment/F03_flat_top_mesa.png](../../Assets/Environment/F03_flat_top_mesa.png)
- Selected style reference: [../../References/selected_map.png](../../References/selected_map.png)
- Asset category: organic
- Native reference resolution: 1390 × 1132 pixels, RGBA

## Scale and coordinates

Local Blender dimension target: **X=60 m width; Y=55 m depth; Z=75 m height/drop**.

The descriptive target is **60 m wide × 55 m deep × 75 m tall**. These are authored production design targets, not measurements extracted from the illustration. Set the dimensions in Blender and validate them against the racing course. Pixel framing and perspective do not encode meter scale.

Blender authoring uses local X for width, Y for depth and Z for up. The game's canonical layout uses its own Y-up convention. Follow the top-level coordinate conversion in the integration guide; do not read the local XYZ size tuple as game XYZ coordinates.

## Origin and pivot

Base center at Z=0; preserve a separate top-plane anchor at Z=75 m for dressing or structure placement.

## Preferred production workflow

Use Blender for exact scale, pivot, placement and collision. Meshy can produce a first-pass organic mesh from this item's isolated PNG, then Blender is responsible for silhouette repair, remeshing/retopology, material cleanup, UVs and LODs. The image is a concept reference with studio lighting baked into its appearance; do not treat it as a physically calibrated albedo map. Build or bake separate material maps during asset production.

1. Block out the authored meter dimensions first and check the module against a 5.6 m vehicle proxy.
2. Use the PNG for shape and surface character, or optionally create one Meshy draft using the wording below.
3. Preserve the important negative space and placement surfaces. Rebuild those in Blender if generation changes them.
4. Simplify the render mesh, assign separate rock and vegetation material regions, and make coarse collision proxies.
5. Make three LODs, inspect silhouette and normals, and export using the top-level integration guide.

### Item-specific construction

Keep a sufficiently flat top for later structure placement. The image shows rock detail; the production top can use a simplified invisible placement plane.

## Collision approach

One low-sided convex hull or a small set of hulls, plus a broad top placement surface; use coarser geometry than visible rock. Keep silhouette overhangs visually plausible but avoid hidden race-line collision traps.

## LOD targets

These are starting triangle budgets for one production render asset, not measured triangle counts of any supplied model. This pack supplies PNG references; no production mesh is implied. Adjust after profiling the actual scene and target devices. Collision geometry and VFX particles are separate budgets.

| Level | Target triangles |
|---|---:|
| LOD0 | 14,000 |
| LOD1 | 5,500 |
| LOD2 | 1,800 |

## Optional Meshy wording

Use this item's isolated PNG if a Meshy draft is helpful; inspect its result in Blender before integration.

> One solid tall basalt mesa with broad flat top, steep fractured charcoal gray sides, small patches of moss only, no forest, architecture, floating fragments, water or surrounding terrain. Retain a flat upper placement surface.

No Meshy generation job was executed for this reference item.

## Exact image-generation prompt

The following is the verbatim prompt used for this PNG. The supplied local source image is the same selected concept included in this ZIP as References/selected_map.png.

~~~text
Create ONE individual isolated PNG modeling-reference asset for R3 MONSOON FALLS, a stylized high-quality realistic 3D browser flying-racing environment. The provided image is STYLE REFERENCE ONLY: use its wet dark basalt, saturated natural rainforest greens, and refined dark industrial materials; do not reproduce its scenery or track. Subject: one isolated tall rock island or mesa module, a single solid steep-sided basalt mass with a broad almost flat top deck. Authored dimensions 60 m width, 55 m depth, 75 m height. Irregular but roughly circular footprint, blunt near-level top with a thin patchy moss cover and small crevice plants only; NO trees, NO forest, NO structures. Wet charcoal basalt sides show vertical fractures and a few moss streaks. The top must read as an accessible placement surface for later game objects. Entire rock including base visible, no surrounding terrain or water. Single object only, three-quarter studio view with orthographic-like mild perspective, neutral diffuse studio illumination with readable material detail, entire silhouette centered and uncropped, generous 12 percent transparent margin on every side. TRUE TRANSPARENT ALPHA BACKGROUND. No floor, ground plane, cast shadow, environment, sky, rain, fog, track, cars, people, labels, text, UI, frame, watermark, or additional objects. No collection sheet or additional views. No neon unless explicitly specified. The object must have a coherent buildable solid form, not scenery.
~~~

Generation settings: transparent_background=true; one referenced concept image; one output asset; no post-generation raster editing.

## Reference QA and limitations

Broad approximately level top, steep cliff sides, no forest or architecture.

The PNG was visually inspected, its alpha channel was checked, and its original bytes were copied unchanged. Anti-aliased edges and partially transparent pixels are expected. This illustration defines appearance and silhouette; it does not supply measured orthographic geometry, ready collision, UVs, material maps or finished game models.

Source PNG identifier: exec-fb66d4f3-2f01-461f-9e43-db1bf008fe23.png

SHA-256: 2eee41b811a0cbcd3a8c1c116e1e8bfbdc81c8178b088ecc740e6e1954c1f008
