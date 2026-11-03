# F10 — Mossy basalt boulder

**Purpose:** Individual modeling reference for R3 — Monsoon Falls.

- PNG: [../../Assets/Environment/F10_moss_boulder.png](../../Assets/Environment/F10_moss_boulder.png)
- Selected style reference: [../../References/selected_map.png](../../References/selected_map.png)
- Asset category: organic
- Native reference resolution: 1536 × 1024 pixels, RGBA

## Scale and coordinates

Local Blender dimension target: **X=8 m width; Y=6 m depth; Z=5 m height/drop**.

The descriptive target is **8 m wide × 6 m deep × 5 m tall**. These are authored production design targets, not measurements extracted from the illustration. Set the dimensions in Blender and validate them against the racing course. Pixel framing and perspective do not encode meter scale.

Blender authoring uses local X for width, Y for depth and Z for up. The game's canonical layout uses its own Y-up convention. Follow the top-level coordinate conversion in the integration guide; do not read the local XYZ size tuple as game XYZ coordinates.

## Origin and pivot

At center of the ground footprint, Z=0. Bottom may be sunk 0.2–0.5 m into terrain to hide the seam.

## Preferred production workflow

Use Blender for exact scale, pivot, placement and collision. Meshy can produce a first-pass organic mesh from this item's isolated PNG, then Blender is responsible for silhouette repair, remeshing/retopology, material cleanup, UVs and LODs. The image is a concept reference with studio lighting baked into its appearance; do not treat it as a physically calibrated albedo map. Build or bake separate material maps during asset production.

1. Block out the authored meter dimensions first and check the module against a 5.6 m vehicle proxy.
2. Use the PNG for shape and surface character, or optionally create one Meshy draft using the wording below.
3. Preserve the important negative space and placement surfaces. Rebuild those in Blender if generation changes them.
4. Simplify the render mesh, assign separate rock and vegetation material regions, and make coarse collision proxies.
5. Make three LODs, inspect silhouette and normals, and export using the top-level integration guide.

### Item-specific construction

Sink the base slightly into supporting terrain to remove a visible cut edge. Small plants in cracks may be material detail or instances of the supplied vegetation.

## Collision approach

One simple convex hull only when close enough to affect gameplay. Background boulders have no collider; moss and small plants never add collision.

## LOD targets

These are starting triangle budgets for one production render asset, not measured triangle counts of any supplied model. This pack supplies PNG references; no production mesh is implied. Adjust after profiling the actual scene and target devices. Collision geometry and VFX particles are separate budgets.

| Level | Target triangles |
|---|---:|
| LOD0 | 4,000 |
| LOD1 | 1,500 |
| LOD2 | 400 |

## Optional Meshy wording

Use this item's isolated PNG if a Meshy draft is helpful; inspect its result in Blender before integration.

> One isolated dark wet basalt boulder, natural angular fractures, moss on upper surfaces, no ground, water or separate pieces. Preserve the low broad silhouette and simplify small moss into material detail.

No Meshy generation job was executed for this reference item.

## Exact image-generation prompt

The following is the verbatim prompt used for this PNG. The supplied local source image is the same selected concept included in this ZIP as References/selected_map.png.

~~~text
Create ONE individual isolated PNG modeling-reference asset for R3 MONSOON FALLS, a stylized high-quality realistic 3D browser flying-racing environment. The provided image is STYLE REFERENCE ONLY: use its wet dark basalt, saturated natural rainforest greens, and refined dark industrial materials; do not reproduce its scenery or track. Subject: ONE solitary large low irregular basalt boulder, authored dimensions 8 m wide, 6 m deep, 5 m high. Dark charcoal wet rock with chunky faceted natural fractures and shallow fissures, soft green moss clinging to the upper half and a few tiny ferns in cracks. A broad stable footprint and readable asymmetric silhouette. No detached fragments, additional rocks, soil base, floor, puddles or ground vegetation. Keep the full rock including its underside edge visible. Single object only, three-quarter studio view with orthographic-like mild perspective, neutral diffuse studio illumination with readable material detail, entire silhouette centered and uncropped, generous 12 percent transparent margin on every side. TRUE TRANSPARENT ALPHA BACKGROUND. No floor, ground plane, cast shadow, environment, sky, rain, fog, track, cars, people, labels, text, UI, frame, watermark, or additional objects. No collection sheet or additional views. No neon unless explicitly specified. The object must have a coherent buildable solid form, not scenery.
~~~

Generation settings: transparent_background=true; one referenced concept image; one output asset; no post-generation raster editing.

## Reference QA and limitations

Single coherent mossy boulder; no detached rock scatter or floor.

The PNG was visually inspected, its alpha channel was checked, and its original bytes were copied unchanged. Anti-aliased edges and partially transparent pixels are expected. This illustration defines appearance and silhouette; it does not supply measured orthographic geometry, ready collision, UVs, material maps or finished game models.

Source PNG identifier: exec-069e0885-232e-4b6f-8443-a593c3f3f9ba.png

SHA-256: 5cc2878b958d14c672d40328be266a772e7a28c43f3d03ba1eac59b0856dd9ad
