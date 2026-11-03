# F11 — Waterfall curtain look reference

**Purpose:** Shader/VFX reference only for R3 — Monsoon Falls.

- PNG: [../../Assets/Environment/F11_waterfall_curtain.png](../../Assets/Environment/F11_waterfall_curtain.png)
- Selected style reference: [../../References/selected_map.png](../../References/selected_map.png)
- Asset category: vfx
- Native reference resolution: 1024 × 1536 pixels, RGBA

## Scale and coordinates

Local Blender dimension target: **X=25 m width; Y=3 m depth; Z=80 m height/drop**.

The descriptive target is **25 m waterfall width × 3 m nominal visual depth × 80 m vertical drop**. These are authored production design targets, not measurements extracted from the illustration. Set the dimensions in Blender and validate them against the racing course. Pixel framing and perspective do not encode meter scale.

Blender authoring uses local X for width, Y for depth and Z for up. The game's canonical layout uses its own Y-up convention. Follow the top-level coordinate conversion in the integration guide; do not read the local XYZ size tuple as game XYZ coordinates.

## Origin and pivot

Top lip center at local origin. The sheet descends along −Z; X is width and Y is shallow depth. Document this upper-origin exception before placement in the scene.

## Preferred production workflow

This PNG is a shader/VFX look reference, not a solid modeling input. Do not run solid Meshy image-to-3D for this item. It is not a validated PBR texture, seamless tile, normal map, flow map, or animation flipbook.

1. Build a shallow subdivided water sheet in Blender with a clean top lip and a slightly widening lower silhouette. Set its origin at the top lip.
2. Make its texture coordinates and material inputs suitable for downward animated flow in the game renderer.
3. Recreate aerated white streaks, partial transparency and denser foam in engine materials; use separately authored reusable noise/foam data rather than assuming this single reference image tiles.
4. Add a small separate foam/spray effect at the base, with a quality setting and a restricted screen-space footprint.
5. Keep visual water non-solid. Add a separate gameplay trigger only if the game design calls for one, and verify transparency/sorting and performance during integration.

### Item-specific construction

The lower frothy fringe is visual reference for separate foam/spray. The supplied PNG has real alpha and some translucent water strands, but has not been calibrated as a production opacity texture; author and validate the final effect in engine.

## Collision approach

No physical collider for the visual sheet, foam or spray. If gameplay needs a waterfall interaction, create a separate named trigger volume in the course code.

## LOD targets

These are starting triangle budgets for one production render asset, not measured triangle counts of any supplied model. This pack supplies PNG references; no production mesh is implied. Adjust after profiling the actual scene and target devices. Collision geometry and VFX particles are separate budgets.

| Level | Target triangles |
|---|---:|
| LOD0 | 600 |
| LOD1 | 120 |
| LOD2 | 2 |

## Optional Meshy wording

**Do not use solid Meshy generation for this VFX item.**

> DO NOT send this item to solid Meshy image-to-3D. Author a simple waterfall surface in Blender and implement UV flow, opacity/foam and spray in the engine. The PNG is a visual reference, not a validated seamless tile, PBR map, flow map or sprite flipbook.

No Meshy generation job was executed for this reference item.

## Exact image-generation prompt

The following is the verbatim prompt used for this PNG. The supplied local source image is the same selected concept included in this ZIP as References/selected_map.png.

~~~text
Create ONE individual isolated PNG modeling-reference asset for R3 MONSOON FALLS, a stylized high-quality realistic 3D browser flying-racing environment. The provided image is STYLE REFERENCE ONLY: use its wet dark basalt, saturated natural rainforest greens, and refined dark industrial materials; do not reproduce its scenery or track. Subject: ONE isolated vertical WATERFALL CURTAIN visual-effects look reference only, no solid object. Front-facing long narrow waterfall sheet, designed for an 80 m vertical drop and 25 m broad upper lip, white aerated flowing water streaks with natural downward striations, partially translucent watery interior so background alpha can show through, irregular feathered edges, denser white foam near the bottom and a small cohesive frothy splash fringe directly below it. The top of the water sheet has a clean broad lip without any rock or architecture. Full waterfall visible from upper lip to last splash, centered, large transparent margin. No reservoir, river, ground, cliff, rock, gate, pipe, plants, mist cloud environment, scene, lighting halo, floor, shadow, text or watermark. Neutral white water with very restrained pale cool highlights. Real transparent RGBA background, not a checkerboard. This is a single VFX reference and must not look like a solid frozen column, ice, glass block or curtain of fabric.
~~~

Generation settings: transparent_background=true; one referenced concept image; one output asset; no post-generation raster editing.

## Reference QA and limitations

Isolated vertical whitewater sheet and small frothy base. VFX look reference only.

The PNG was visually inspected, its alpha channel was checked, and its original bytes were copied unchanged. Anti-aliased edges and partially transparent pixels are expected. This illustration defines appearance and silhouette; it does not supply measured orthographic geometry, ready collision, UVs, material maps or finished game models.

Source PNG identifier: exec-b6403823-203a-492e-ad89-e782ca12bc70.png

SHA-256: 25b833210398f6785cc6cae8b13fdfc05840dcd84c8531e3a2ee8b832f907ecc
