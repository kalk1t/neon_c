# F01 — Mossy basalt cliff module

**Purpose:** Individual modeling reference for R3 — Monsoon Falls.

- PNG: [../../Assets/Environment/F01_basalt_cliff.png](../../Assets/Environment/F01_basalt_cliff.png)
- Selected style reference: [../../References/selected_map.png](../../References/selected_map.png)
- Asset category: organic
- Native reference resolution: 1024 × 1536 pixels, RGBA

## Scale and coordinates

Local Blender dimension target: **X=60 m width; Y=40 m depth; Z=100 m height/drop**.

The descriptive target is **60 m wide × 40 m deep × 100 m tall**. These are authored production design targets, not measurements extracted from the illustration. Set the dimensions in Blender and validate them against the racing course. Pixel framing and perspective do not encode meter scale.

Blender authoring uses local X for width, Y for depth and Z for up. The game's canonical layout uses its own Y-up convention. Follow the top-level coordinate conversion in the integration guide; do not read the local XYZ size tuple as game XYZ coordinates.

## Origin and pivot

Base center; Z=0 is the placement plane. The large flatter rear face points toward local +Y for burying into terrain.

## Preferred production workflow

Use Blender for exact scale, pivot, placement and collision. Meshy can produce a first-pass organic mesh from this item's isolated PNG, then Blender is responsible for silhouette repair, remeshing/retopology, material cleanup, UVs and LODs. The image is a concept reference with studio lighting baked into its appearance; do not treat it as a physically calibrated albedo map. Build or bake separate material maps during asset production.

1. Block out the authored meter dimensions first and check the module against a 5.6 m vehicle proxy.
2. Use the PNG for shape and surface character, or optionally create one Meshy draft using the wording below.
3. Preserve the important negative space and placement surfaces. Rebuild those in Blender if generation changes them.
4. Simplify the render mesh, assign separate rock and vegetation material regions, and make coarse collision proxies.
5. Make three LODs, inspect silhouette and normals, and export using the top-level integration guide.

### Item-specific construction

Preserve a dominant rock silhouette. Do not model every tiny plant from the reference; bake moss and use a few additional vegetation instances near visible ledges.

## Collision approach

Several coarse convex boxes/hulls for exposed walls; keep gaps and ledges navigable where the course requires them. No triangle collider derived from moss.

## LOD targets

These are starting triangle budgets for one production render asset, not measured triangle counts of any supplied model. This pack supplies PNG references; no production mesh is implied. Adjust after profiling the actual scene and target devices. Collision geometry and VFX particles are separate budgets.

| Level | Target triangles |
|---|---:|
| LOD0 | 12,000 |
| LOD1 | 5,000 |
| LOD2 | 1,500 |

## Optional Meshy wording

Use this item's isolated PNG if a Meshy draft is helpful; inspect its result in Blender before integration.

> One massive isolated vertical wet basalt cliff module, dark gray fractured rock, restrained moss on ledges, fully solid self-contained rock mass, no trees, water, environment or disconnected debris. Preserve the broad silhouette and vertical cracks.

No Meshy generation job was executed for this reference item.

## Exact image-generation prompt

The following is the verbatim prompt used for this PNG. The supplied local source image is the same selected concept included in this ZIP as References/selected_map.png.

~~~text
Create ONE individual isolated PNG modeling-reference asset for R3 MONSOON FALLS, a stylized high-quality realistic 3D browser flying-racing environment. The provided image is STYLE REFERENCE ONLY: use its wet dark basalt, saturated natural rainforest greens, and refined dark industrial materials; do not reproduce its scenery or track. Subject: one monumental steep vertical basalt cliff module. A tall self-contained irregular rectangular rock mass, fully visible from its rough top to its flat concealed base; dimensions designed around 60 m width, 40 m depth, 100 m height. Dark gray wet basalt with vertical column-like fractures and ledges, selective bright-green moss and tiny creeping leaves on upper horizontal ledges; exposed rock remains dominant. No trees, no waterfall, no attached terrain. Broad mass suitable for placement against other modules, not an arch or spire. Single object only, three-quarter studio view with orthographic-like mild perspective, neutral diffuse studio illumination with readable material detail, entire silhouette centered and uncropped, generous 12 percent transparent margin on every side. TRUE TRANSPARENT ALPHA BACKGROUND. No floor, ground plane, cast shadow, environment, sky, rain, fog, track, cars, people, labels, text, UI, frame, watermark, or additional objects. No collection sheet or additional views. No neon unless explicitly specified. The object must have a coherent buildable solid form, not scenery.
~~~

Generation settings: transparent_background=true; one referenced concept image; one output asset; no post-generation raster editing.

## Reference QA and limitations

Complete fractured cliff mass; exposed basalt and restrained greenery; no scene.

The PNG was visually inspected, its alpha channel was checked, and its original bytes were copied unchanged. Anti-aliased edges and partially transparent pixels are expected. This illustration defines appearance and silhouette; it does not supply measured orthographic geometry, ready collision, UVs, material maps or finished game models.

Source PNG identifier: exec-580c2086-9216-4ccd-91f4-acde3ec66693.png

SHA-256: d600f8e5660cb59ee9993ee37856e1eb65bd99251a3663e923979a4f7d52db29
