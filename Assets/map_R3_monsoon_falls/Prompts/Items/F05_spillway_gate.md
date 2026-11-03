# F05 — Hydrodam spillway gate

**Purpose:** Individual modeling reference for R3 — Monsoon Falls.

- PNG: [../../Assets/Environment/F05_spillway_gate.png](../../Assets/Environment/F05_spillway_gate.png)
- Selected style reference: [../../References/selected_map.png](../../References/selected_map.png)
- Asset category: architectural
- Native reference resolution: 1024 × 1536 pixels, RGBA

## Scale and coordinates

Local Blender dimension target: **X=25 m width; Y=12 m depth; Z=80 m height/drop**.

The descriptive target is **25 m wide × 12 m deep × 80 m tall**. These are authored production design targets, not measurements extracted from the illustration. Set the dimensions in Blender and validate them against the racing course. Pixel framing and perspective do not encode meter scale.

Blender authoring uses local X for width, Y for depth and Z for up. The game's canonical layout uses its own Y-up convention. Follow the top-level coordinate conversion in the integration guide; do not read the local XYZ size tuple as game XYZ coordinates.

## Origin and pivot

At center of the bottom sill; Z=0. +Y faces reservoir. Put a distinct gate_panel pivot at its upper hoist attachment if later animation is desired.

## Preferred production workflow

Prefer Blender modeling for this engineered module. Its snap faces, scale, repeating ribs, material slots and pivots need explicit construction. Use the PNG as a visual target; optional Meshy output can help with exploratory massing but should not define final mounting geometry.

1. Build the primary shape from simple boxes/cylinders at the authored dimensions.
2. Add a limited bevel and only the silhouette-defining mechanical forms. Use materials or baked detail for tiny bolts and panels.
3. Separate glass, dark structure, concrete and cyan emissive trim into deliberate materials where relevant.
4. Apply transforms, establish the prescribed pivot and mounting anchors, and create simple collision proxies.
5. Author LODs and export using the top-level guide. Confirm repetitive modules meet without double-width seams.

### Item-specific construction

Keep the visible moving gate panel separate from the stationary frame if any animation is planned. Water, foam and spray are independent objects/effects.

## Collision approach

Static boxes for the frame and, only when needed, one box for the closed gate panel. The waterfall is a separate non-solid VFX asset. Do not derive collision from cable/bolt details.

## LOD targets

These are starting triangle budgets for one production render asset, not measured triangle counts of any supplied model. This pack supplies PNG references; no production mesh is implied. Adjust after profiling the actual scene and target devices. Collision geometry and VFX particles are separate budgets.

| Level | Target triangles |
|---|---:|
| LOD0 | 8,000 |
| LOD1 | 3,000 |
| LOD2 | 800 |

## Optional Meshy wording

Use this item's isolated PNG if a Meshy draft is helpful; inspect its result in Blender before integration.

> Optional visual exploration only: one standalone rectangular mechanical sluice gate frame with a closed ribbed gate panel, no water or surrounding architecture. Build final rails, pivots and mounting geometry in Blender.

No Meshy generation job was executed for this reference item.

## Exact image-generation prompt

The following is the verbatim prompt used for this PNG. The supplied local source image is the same selected concept included in this ZIP as References/selected_map.png.

~~~text
Create ONE individual isolated PNG modeling-reference asset for R3 MONSOON FALLS, a stylized high-quality realistic 3D browser flying-racing environment. The provided image is STYLE REFERENCE ONLY: use its wet dark basalt, saturated natural rainforest greens, and refined dark industrial materials; do not reproduce its scenery or track. Subject: ONE independent tall rectangular industrial spillway sluice gate and its structural frame, nominally 25 m wide, 12 m deep, 80 m tall. Heavy graphite concrete-and-steel vertical frame with two tall rectangular side pillars, overhead mechanical hoist and crossbeam, a recessed ribbed metallic movable gate panel occupying the central opening, guides, visible service bolts, restrained cyan indicator lines near the top. Mechanically plausible hydroelectric architecture. NO water at all, no waterfall, no surrounding dam wall, no cliff, no base slab or landscape. A standalone gate unit only, complete from top beam to frame feet. Single object only, three-quarter studio view with orthographic-like mild perspective, neutral diffuse studio illumination with readable material detail, entire silhouette centered and uncropped, generous 12 percent transparent margin on every side. TRUE TRANSPARENT ALPHA BACKGROUND. No floor, ground plane, cast shadow, environment, sky, rain, fog, track, cars, people, labels, text, UI, frame, watermark, or additional objects. No collection sheet or additional views. No neon unless explicitly specified. The object must have a coherent buildable solid form, not scenery.
~~~

Generation settings: transparent_background=true; one referenced concept image; one output asset; no post-generation raster editing.

## Reference QA and limitations

Standalone gate/frame with hoist and panel; no water or surrounding dam.

The PNG was visually inspected, its alpha channel was checked, and its original bytes were copied unchanged. Anti-aliased edges and partially transparent pixels are expected. This illustration defines appearance and silhouette; it does not supply measured orthographic geometry, ready collision, UVs, material maps or finished game models.

Source PNG identifier: exec-6ae512d2-83cc-41c4-b406-458960c0ae26.png

SHA-256: 72aa5f406594686edd85d1656b4633140637e015516936c9c8de32d33948a7df
