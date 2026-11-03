# F02 — Natural basalt arch

**Purpose:** Individual modeling reference for R3 — Monsoon Falls.

- PNG: [../../Assets/Environment/F02_natural_arch.png](../../Assets/Environment/F02_natural_arch.png)
- Selected style reference: [../../References/selected_map.png](../../References/selected_map.png)
- Asset category: organic
- Native reference resolution: 1536 × 1024 pixels, RGBA

## Scale and coordinates

Local Blender dimension target: **X=80 m width; Y=25 m depth; Z=65 m height/drop**.

The descriptive target is **80 m wide × 25 m deep × 65 m tall; clear aperture 30 m wide × 30 m tall**. These are authored production design targets, not measurements extracted from the illustration. Set the dimensions in Blender and validate them against the racing course. Pixel framing and perspective do not encode meter scale.

Blender authoring uses local X for width, Y for depth and Z for up. The game's canonical layout uses its own Y-up convention. Follow the top-level coordinate conversion in the integration guide; do not read the local XYZ size tuple as game XYZ coordinates.

## Origin and pivot

At ground level centered beneath the aperture. Opening runs along local Y; local X spans the opening.

## Preferred production workflow

Use Blender for exact scale, pivot, placement and collision. Meshy can produce a first-pass organic mesh from this item's isolated PNG, then Blender is responsible for silhouette repair, remeshing/retopology, material cleanup, UVs and LODs. The image is a concept reference with studio lighting baked into its appearance; do not treat it as a physically calibrated albedo map. Build or bake separate material maps during asset production.

1. Block out the authored meter dimensions first and check the module against a 5.6 m vehicle proxy.
2. Use the PNG for shape and surface character, or optionally create one Meshy draft using the wording below.
3. Preserve the important negative space and placement surfaces. Rebuild those in Blender if generation changes them.
4. Simplify the render mesh, assign separate rock and vegetation material regions, and make coarse collision proxies.
5. Make three LODs, inspect silhouette and normals, and export using the top-level integration guide.

### Item-specific construction

The playable aperture is the critical feature: construct and measure it in Blender as at least 30 m wide and 30 m high before dressing it with vines. Do not let a convex collision hull span the opening.

## Collision approach

Separate coarse left-foot, right-foot, and overhead bridge hulls; never use one convex hull across the entire arch because it would close the opening. Enforce the 30 × 30 m clear aperture during Blender blockout.

## LOD targets

These are starting triangle budgets for one production render asset, not measured triangle counts of any supplied model. This pack supplies PNG references; no production mesh is implied. Adjust after profiling the actual scene and target devices. Collision geometry and VFX particles are separate budgets.

| Level | Target triangles |
|---|---:|
| LOD0 | 18,000 |
| LOD1 | 7,000 |
| LOD2 | 2,500 |

## Optional Meshy wording

Use this item's isolated PNG if a Meshy draft is helpful; inspect its result in Blender before integration.

> One natural solid basalt rock arch, two thick legs and one thick bridge, large open passage through the center, wet dark stone with moss on top, no scenery, water, track or vegetation forest. Retain the aperture; rebuild it in Blender if generation closes it.

No Meshy generation job was executed for this reference item.

## Exact image-generation prompt

The following is the verbatim prompt used for this PNG. The supplied local source image is the same selected concept included in this ZIP as References/selected_map.png.

~~~text
Create ONE individual isolated PNG modeling-reference asset for R3 MONSOON FALLS, a stylized high-quality realistic 3D browser flying-racing environment. The provided image is STYLE REFERENCE ONLY: use its wet dark basalt, saturated natural rainforest greens, and refined dark industrial materials; do not reproduce its scenery or track. Subject: one monumental natural rainforest basalt rock arch, an independent complete arch-shaped rock formation with two broad feet and a thick curved rock bridge. Overall authored dimensions 80 m wide, 25 m deep, 65 m tall. Large completely open through-aperture approximately 30 m wide and 30 m tall is visibly clear to the transparent background, wide enough for a flying-car racing course. Fractured wet charcoal gray basalt, rugged moss-covered upper surface and restrained hanging vines. Keep most rock surfaces exposed. No trees, no track, no gate, no water, no cliff scenery attached. Single object only, three-quarter studio view with orthographic-like mild perspective, neutral diffuse studio illumination with readable material detail, entire silhouette centered and uncropped, generous 12 percent transparent margin on every side. TRUE TRANSPARENT ALPHA BACKGROUND. No floor, ground plane, cast shadow, environment, sky, rain, fog, track, cars, people, labels, text, UI, frame, watermark, or additional objects. No collection sheet or additional views. No neon unless explicitly specified. The object must have a coherent buildable solid form, not scenery.
~~~

Generation settings: transparent_background=true; one referenced concept image; one output asset; no post-generation raster editing.

## Reference QA and limitations

Complete arch; center passage visibly open; enforce authored clearance in the 3D model.

The PNG was visually inspected, its alpha channel was checked, and its original bytes were copied unchanged. Anti-aliased edges and partially transparent pixels are expected. This illustration defines appearance and silhouette; it does not supply measured orthographic geometry, ready collision, UVs, material maps or finished game models.

Source PNG identifier: exec-1d57a0f8-0343-4a35-8bb8-903fa7ed4a51.png

SHA-256: e7702e3bd2cc9739b1a2f5facdef0088214f0223cfd9768d2a32eaf3b5161b42
