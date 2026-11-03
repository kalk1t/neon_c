# F07 — Broadleaf rainforest tree

**Purpose:** Individual modeling reference for R3 — Monsoon Falls.

- PNG: [../../Assets/Environment/F07_tropical_tree.png](../../Assets/Environment/F07_tropical_tree.png)
- Selected style reference: [../../References/selected_map.png](../../References/selected_map.png)
- Asset category: vegetation
- Native reference resolution: 1374 × 1145 pixels, RGBA

## Scale and coordinates

Local Blender dimension target: **X=14 m width; Y=14 m depth; Z=18 m height/drop**.

The descriptive target is **14 m canopy width × 14 m canopy depth × 18 m height**. These are authored production design targets, not measurements extracted from the illustration. Set the dimensions in Blender and validate them against the racing course. Pixel framing and perspective do not encode meter scale.

Blender authoring uses local X for width, Y for depth and Z for up. The game's canonical layout uses its own Y-up convention. Follow the top-level coordinate conversion in the integration guide; do not read the local XYZ size tuple as game XYZ coordinates.

## Origin and pivot

At ground contact under trunk center; Z=0. Root tips should intersect terrain after placement; keep the canopy centered approximately over the origin.

## Preferred production workflow

Prefer Blender-authored trunk/curve geometry and optimized leaf or frond cards. Meshy can help establish a trunk or overall organic silhouette, but dense generated leaves should be replaced or simplified before browser use. Keep the neutral-lit PNG as a modeling reference rather than using its studio shading as a validated material texture.

1. Establish the authored scale and the correct ground or hanging attachment pivot.
2. Build the main trunk/stem structure, then layer a modest number of leaf clusters or fronds.
3. Author appropriate foliage color/alpha/normal data from the final asset workflow, and check edge appearance against both dark and bright backgrounds.
4. Create lower LOD silhouettes or impostors and only the specified trunk collision where needed.
5. Scatter as instances where supported by the game's existing renderer and keep the racing corridor unobstructed.

### Item-specific construction

Keep the main branching silhouette and a few buttress roots. Use root overlap with terrain rather than a soil disk. The crown should read well at distance.

## Collision approach

One tapered trunk capsule or 2–3 coarse trunk hulls if players can reach it. Canopy and leaves have no collision. Background trees may have no collider.

## LOD targets

These are starting triangle budgets for one production render asset, not measured triangle counts of any supplied model. This pack supplies PNG references; no production mesh is implied. Adjust after profiling the actual scene and target devices. Collision geometry and VFX particles are separate budgets.

| Level | Target triangles |
|---|---:|
| LOD0 | 10,000 |
| LOD1 | 3,500 |
| LOD2 | 500 |

## Optional Meshy wording

Use this item's isolated PNG if a Meshy draft is helpful; inspect its result in Blender before integration.

> One isolated tropical broadleaf tree with a single coherent branching trunk, broad layered canopy, natural green leaves, visible base roots, no ground or surrounding objects. Use generation for trunk/branch massing; rebuild leaf clusters as optimized cards or low-poly foliage in Blender.

No Meshy generation job was executed for this reference item.

## Exact image-generation prompt

The following is the verbatim prompt used for this PNG. The supplied local source image is the same selected concept included in this ZIP as References/selected_map.png.

~~~text
Create ONE individual isolated PNG modeling-reference asset for R3 MONSOON FALLS, a stylized high-quality realistic 3D browser flying-racing environment. The provided image is STYLE REFERENCE ONLY: use its wet dark basalt, saturated natural rainforest greens, and refined dark industrial materials; do not reproduce its scenery or track. Subject: ONE complete mature broadleaf rainforest tree, authored about 18 m high and a broad 14 m canopy. Strong brown branching trunk with visible buttress roots that terminate cleanly at the root tips, layered dense but airy evergreen leaf canopy with varied natural rich green leaves and fine wet-surface highlights. The entire trunk from root tips to all top leaves must be visible and clear, no cut branches at frame edges. No soil disk, no mound, no grass, no secondary trees, no hanging scene elements. This is an isolated plant modeling reference, not a photo of a forest. Single object only, three-quarter studio view with orthographic-like mild perspective, neutral diffuse studio illumination with readable material detail, entire silhouette centered and uncropped, generous 12 percent transparent margin on every side. TRUE TRANSPARENT ALPHA BACKGROUND. No floor, ground plane, cast shadow, environment, sky, rain, fog, track, cars, people, labels, text, UI, frame, watermark, or additional objects. No collection sheet or additional views. No neon unless explicitly specified. The object must have a coherent buildable solid form, not scenery.
~~~

Generation settings: transparent_background=true; one referenced concept image; one output asset; no post-generation raster editing.

## Reference QA and limitations

Single complete tree with trunk, root flare and crown; no ground.

The PNG was visually inspected, its alpha channel was checked, and its original bytes were copied unchanged. Anti-aliased edges and partially transparent pixels are expected. This illustration defines appearance and silhouette; it does not supply measured orthographic geometry, ready collision, UVs, material maps or finished game models.

Source PNG identifier: exec-3d1da07e-a904-4c4e-a673-cf5daeac1172.png

SHA-256: 4499d20f83d6688ffa6fcbbc4ec5eacd85c0b1a0c3d2b85f04e614a13824f8d5
