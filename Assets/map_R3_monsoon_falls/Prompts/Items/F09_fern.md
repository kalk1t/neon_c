# F09 — Rainforest fern

**Purpose:** Individual modeling reference for R3 — Monsoon Falls.

- PNG: [../../Assets/Environment/F09_fern.png](../../Assets/Environment/F09_fern.png)
- Selected style reference: [../../References/selected_map.png](../../References/selected_map.png)
- Asset category: vegetation
- Native reference resolution: 1374 × 1145 pixels, RGBA

## Scale and coordinates

Local Blender dimension target: **X=2 m width; Y=2 m depth; Z=2 m height/drop**.

The descriptive target is **2 m spread × 2 m depth × 2 m height**. These are authored production design targets, not measurements extracted from the illustration. Set the dimensions in Blender and validate them against the racing course. Pixel framing and perspective do not encode meter scale.

Blender authoring uses local X for width, Y for depth and Z for up. The game's canonical layout uses its own Y-up convention. Follow the top-level coordinate conversion in the integration guide; do not read the local XYZ size tuple as game XYZ coordinates.

## Origin and pivot

At root crown ground-contact center, Z=0. Local Z aligns with the average upright direction; keep frond tips above the placement surface.

## Preferred production workflow

Prefer Blender-authored trunk/curve geometry and optimized leaf or frond cards. Meshy can help establish a trunk or overall organic silhouette, but dense generated leaves should be replaced or simplified before browser use. Keep the neutral-lit PNG as a modeling reference rather than using its studio shading as a validated material texture.

1. Establish the authored scale and the correct ground or hanging attachment pivot.
2. Build the main trunk/stem structure, then layer a modest number of leaf clusters or fronds.
3. Author appropriate foliage color/alpha/normal data from the final asset workflow, and check edge appearance against both dark and bright backgrounds.
4. Create lower LOD silhouettes or impostors and only the specified trunk collision where needed.
5. Scatter as instances where supported by the game's existing renderer and keep the racing corridor unobstructed.

### Item-specific construction

Use a small number of convincing frond layers and preserve the central crown. Place groups outside the playable lane; the fern has no default collider.

## Collision approach

No collision by default. Use a tiny root proxy only if the game explicitly needs interaction; do not block vehicles with foliage.

## LOD targets

These are starting triangle budgets for one production render asset, not measured triangle counts of any supplied model. This pack supplies PNG references; no production mesh is implied. Adjust after profiling the actual scene and target devices. Collision geometry and VFX particles are separate budgets.

| Level | Target triangles |
|---|---:|
| LOD0 | 2,500 |
| LOD1 | 900 |
| LOD2 | 200 |

## Optional Meshy wording

Use this item's isolated PNG if a Meshy draft is helpful; inspect its result in Blender before integration.

> One isolated tropical fern with a single crown and arching compound fronds, no soil or surrounding objects. Prefer building optimized frond cards in Blender; a generated mesh is only a shape reference and should not be exported as dense per-leaf geometry.

No Meshy generation job was executed for this reference item.

## Exact image-generation prompt

The following is the verbatim prompt used for this PNG. The supplied local source image is the same selected concept included in this ZIP as References/selected_map.png.

~~~text
Create ONE individual isolated PNG modeling-reference asset for R3 MONSOON FALLS, a stylized high-quality realistic 3D browser flying-racing environment. The provided image is STYLE REFERENCE ONLY: use its wet dark basalt, saturated natural rainforest greens, and refined dark industrial materials; do not reproduce its scenery or track. Subject: ONE lush tropical fern plant rosette, authored approximately 2 m high with a 2 m spread. Multiple arching compound green fronds radiate from one compact coherent central crown, delicate individual pinnate leaflets, damp rich rainforest greens, a few unfurling young fiddleheads. Complete crown and all frond tips visible, tiny root base only. No soil, pot, grass, surrounding plants, mossy rock, detached leaves or landscape. Clean 3/4 botanical game-asset reference. Single object only, three-quarter studio view with orthographic-like mild perspective, neutral diffuse studio illumination with readable material detail, entire silhouette centered and uncropped, generous 12 percent transparent margin on every side. TRUE TRANSPARENT ALPHA BACKGROUND. No floor, ground plane, cast shadow, environment, sky, rain, fog, track, cars, people, labels, text, UI, frame, watermark, or additional objects. No collection sheet or additional views. No neon unless explicitly specified. The object must have a coherent buildable solid form, not scenery.
~~~

Generation settings: transparent_background=true; one referenced concept image; one output asset; no post-generation raster editing.

## Reference QA and limitations

Single fern rosette with all fronds and small root base; no ground.

The PNG was visually inspected, its alpha channel was checked, and its original bytes were copied unchanged. Anti-aliased edges and partially transparent pixels are expected. This illustration defines appearance and silhouette; it does not supply measured orthographic geometry, ready collision, UVs, material maps or finished game models.

Source PNG identifier: exec-8819e08d-aac2-4fb9-b31c-131dc90bf330.png

SHA-256: 44693fe07839eb1cf36fe8462a2e87f3998bc0b996d7b0046439027a4894d5f5
