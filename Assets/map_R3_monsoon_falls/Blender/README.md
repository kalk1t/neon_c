# Neon Circuit — authored route and Blender source kit

This kit supplies reproducible course data and an editable **starter blockout** for **R1 Neon Downpour** and **R3 Monsoon Falls**. The layouts are new designs inspired by the selected map images. They are not reconstructed or measured from image geometry.

**Blender was not installed in the authoring environment.** The Python files were syntax-checked, the numerical route data was validated, and the pure-Python mesh topology/UV code was tested. No `.blend` scene, GLB export, Blender render, game collision, or browser performance test has been executed here. The commands below create those files on a machine with Blender installed.

## Package placement

For each final map package, copy the selected `R1/` or `R3/` directory's contents into `Layout/`, the files from `scripts/` into `Blender/`, and the schemas from `schemas/` into `Layout/`. This README can live in `Blender/README.md`. Exclude Python cache directories. The complete kit can also be used as-is; replace `Layout/route.json` with `R1/route.json` or `R3/route.json`, and `Blender/` with `scripts/` in the commands.

| Source file | Purpose |
| --- | --- |
| `route.json` | Closed route controls, interpolation rule, gates, spawn/reset poses, sectors, hazards, calculated metrics |
| `placements.json` | Individual asset instances, dimensions, yaw, source-pivot conversion, collision parts, aperture allowances |
| `route_plan.png`, `route_plan.svg` | Exact-data top-down plan plus height profile; a layout diagram, not concept artwork |
| `validation_report.json` | Numerical source-layout checks only |
| `route.schema.json`, `placements.schema.json` | JSON Schema 2020-12 structure definitions |
| `build_map.py` | Blender scene builder and optional glTF exporter; includes `--check` mode |
| `route_tools.py` | Spline, arc-length sampling, coordinate conversion, clearance validation |
| `mesh_geometry.py` | Pure-Python deck, platform, ring, box, and frustum geometry with UVs |
| `validate_routes.py` | Standard-library source-layout CLI |
| `test_layout.py` | Offline geometry/data tests, including deliberately broken layouts |
| `author_routes.py` | Reproducible authoring source; regenerates both layouts and diagrams |
| `module_catalog.json` | Shared and map-specific asset ID mapping |

## Course measurements

| Property | R1 Neon Downpour | R3 Monsoon Falls |
| --- | ---: | ---: |
| Calculated centerline length | 2,464.880 m | 3,109.631 m |
| Course X × Z footprint | 900.000 × 698.425 m | 1,100.000 × 900.125 m |
| Flight-center Y range | 46.001–133.999 m | 36.006–173.994 m |
| Maximum sampled grade | 7.390° | 9.297° |
| Minimum sampled horizontal turn radius | 130.302 m | 209.818 m |
| Controls / checkpoint gates | 24 / 14 | 24 / 16 |
| Checkpoint spacing | 176.063 m | 194.352 m |
| Authored collision parts checked | 15 | 13 |

Length and extrema use **128 samples per span**, or **3,072 chords** around each closed uniform Catmull–Rom curve. These are high-resolution numerical approximations, not exact analytic spline integrals. The last control does not duplicate the first. The closing span and tangent are continuous. The standard validator also rejects non-adjacent projected crossings/touches.

Both courses use an 18 m deck/route width, 22 m inner gate diameter, 26 m outer gate diameter, and a deck top **3 m below the flight center**. Deck thickness is 1.2 m. The start platform is a 24 m wide × 48 m long swept section following the route slope; its top is 0.08 m below the deck, avoiding a coincident overlay. Nominal car size is 5.6 m long × 3.92 m wide. The 2.4 m car-height assumption is for this blockout only; measure the real models and colliders.

The R1 western turnaround is deliberately broad. Its final banking, speed limit, route complexity, and visual hairpin treatment should be decided by playtesting. The R3 path is a similarly approachable first blockout. Both leave room for the supplied landmark assets and later track refinement.

## Coordinate and pivot contract

All layout vectors are **game X right, Y up, Z forward**, in meters. No route coordinate is screen-space or image-relative.

```text
Blender = (game X, -game Z, game Y)
Game    = (Blender X, Blender Z, -Blender Y)
```

This is a proper rotation, so it preserves handedness and face winding. Positive game yaw turns local +Z toward game +X. In Blender it is the same positive angle about Blender +Z. The builder turns a local game position `[0, h, 0]` into Blender `[0, 0, h]`.

Every placement wrapper uses a **base-center origin** with local size `[X width, Y height, Z depth]`. `size_xyz_m` is the target envelope, not a promise that an AI-generated mesh will have those dimensions. Normalize imported meshes deliberately before replacing a placeholder. Final placement envelopes and clear openings take precedence over a reference image's nominal proportions.

**Source pivots are not all base-center.** Keep each placement wrapper at its authored base-center pose. After scaling the asset to the target envelope, parent its source root to that wrapper and apply the provided local offset. Top-pivot assets extend downward from their own origin:

| Item | Target height | Child offset in game coordinates | Child offset in Blender local coordinates |
| --- | ---: | --- | --- |
| N05 rooftop platform | 4 m | `[0, 4, 0]` | `[0, 0, 4]` |
| N07 bridge deck slab | 5 m | `[0, 5, 0]` | `[0, 0, 5]` |
| R1 C15 service platform | 1.5 m | `[0, 1.5, 0]` | `[0, 0, 1.5]` |
| R3 C15 service platform | 4 m | `[0, 4, 0]` | `[0, 0, 4]` |
| F11 waterfall | 87 m | `[0, 87, 0]` | `[0, 0, 87]` |
| F12 hanging vine | 60 m | `[0, 60, 0]` | `[0, 0, 60]` |

This places a top-pivot mesh's bottom at the wrapper's base and its top at base + target height. Do not add this offset to a mesh that has already been re-originated to base-center.

Two mounting pivots need a different conversion. N10's rear mounting center receives game `[0, 11, -2.5]`, or Blender local `[0, 2.5, 11]`, for its 32 × 22 × 5 m target envelope; its screen faces game +Z / Blender −Y. F06's rear bracket receives game `[0, 6, 9.5]`, or Blender local `[0, -9.5, 6]`, for its 20 × 12 × 19 m envelope; the cabin extends toward game −Z / Blender +Y. These offsets assume a vertically centered source mounting pivot. Measure the actual imported mount and bounds; if the vertical mount is asymmetric, use the measured offset that places the bottom at wrapper Y=0 rather than blindly using half the height.

Other assets default to zero source-root offset. Pivot type and offset are in `placements.json` and on Blender replacement roots. The illustrations do not establish exact geometric bounds; imported mesh measurement is the final authority for normalization.

With `export_yup=True`, the glTF exporter rotates the Blender scene back into the canonical game Y-up convention. Do not add a second 90° correction in the game loader. Check one point such as game `[3, 7, 11]` → Blender `[3, -11, 7]` → game `[3, 7, 11]` before integrating a large scene. Official exporter options: [Blender glTF export operator](https://docs.blender.org/api/4.4/bpy.ops.export_scene.html).

## Run the data checks

From the extracted map package root:

```bash
python3 Blender/validate_routes.py --route Layout/route.json --placements Layout/placements.json
python3 Blender/test_layout.py --layout-dir Layout
python3 Blender/build_map.py --check --config Layout/route.json --placements Layout/placements.json
```

These commands need only Python's standard library. `--check` writes nothing and does not import Blender. `validate_routes.py --report new_report.json` can write a new report and refuses to overwrite an existing report. JSON schemas describe structure; the Python validator additionally checks geometry and semantics. If you edit control points, regenerate dependent checkpoint/reset poses and metrics rather than retaining stale coordinates.

`author_routes.py --output-root ./Regenerated` reproduces both layouts. It uses Matplotlib only when drawing the diagrams; add `--no-diagrams` to use the standard library alone. The authoring script intentionally writes its generated data into the chosen directory, so use a separate directory for regeneration.

## Build in Blender

Use a new Blender process from the extracted package directory:

```bash
blender --factory-startup --background --python Blender/build_map.py -- \
  --config Layout/route.json \
  --placements Layout/placements.json \
  --output-dir Build \
  --export-glb
```

Omit `--export-glb` to save only the editable scene and build report. A successful execution writes `r1_starter.blend`/`r3_starter.blend`, an optional matching `.glb`, and a matching `_build_report.json`. These files are **not included as prebuilt outputs** in this source kit.

The script refuses to overwrite those named files unless `--force` is supplied. It creates a new marked scene rather than clearing an arbitrary open scene. If rerun inside the same session, it only replaces its marked scene and refuses to remove objects shared with another scene. The documented fresh-process command is the predictable path. `--force` never deletes an output directory wholesale.

The builder checks required glTF exporter options at runtime and records the options actually used. An unavailable or changed exporter may still need a small version-specific adjustment; that has not been executed here. Geometry construction follows the official [Mesh API](https://docs.blender.org/api/current/bpy.types.Mesh.html), including `from_pydata`, validation, and per-loop UV data.

## What the Blender starter creates

The new scene separates course, gates, route markers, supports, landmark placeholders, collider guides, water/VFX, spawn/reset empties, and lights/cameras into named collections. The flight deck follows the actual spline rather than connecting control points with straight chords. Course side/bottom faces, ring walls, and platform end caps have consistent winding. The deck has distance-based UVs; primitives and rings have starter UVs suitable for replacement or later atlasing.

Checkpoints have a visible ring, a separate named trigger empty, and a named reset empty. **CP00 uses a circular C05-tagged ring as a blockout marker**, not the final start/finish arch silhouette. Replace it with the approved C05 arch while preserving its trigger plane, 22 m clear opening, center, and direction. The other gate rings also remain procedural placeholders for the final style treatment.

N03 skybridge and F02 basalt arch are built as separate solid piers plus an overhead beam. These preserve hollow flight openings. Other landmarks use primitive silhouettes and simple materials. Organic trees, cliffs, boulders, and vines are intentionally coarse replacement proxies. No Meshy service is contacted. The waterfall is a VFX placement plane, and the flat water surface is a simple opaque starter material. Rain, splash, mist, waterfall scrolling, fog, reflections, and engine materials remain runtime work.

The collection of wireframe collider guides is hidden from rendering and explicitly excluded from the selected-object GLB export. It documents geometry intent; it is not automatically converted into the game's collision representation. Lights and cameras stay in the editable `.blend` and are omitted from this GLB export. The exported empties carry useful custom properties, but the host engine must decide how to consume them.

Many small route markers remain separate editable objects. Merge compatible static meshes, instance repeated props, and establish draw-call/material/texture budgets before shipping this blockout in a browser.

## Race logic and clearance

Checkpoint order is CP00 → CP01 → … → last gate → CP00. The first forward CP00 crossing starts the lap; only a later crossing after all intervening gates completes it. Compare the car's previous/current position against the gate's signed plane using `forward_game`, require a negative-to-positive crossing, and require the next expected checkpoint. Fast cars require swept or segment-plane crossing logic; a per-frame point-in-sphere test can skip gates.

`trigger_center_radius_m: 9` is provisional. It is approximately the 11 m hole radius minus half the nominal car width. It does **not** account for rotated vehicle length, height, banking, actual collider shape, or collision with the curved ring. Recheck with all three final cars. Do not turn the ring's full bounding box into a filled solid obstacle.

Every gate has a reset pose sampled **18 m before that gate**, so the same gate remains required after reset. The start spawn is 28 m before CP00. The JSON includes forward directions and reset velocity intent; the host game must apply the actual camera, orientation, physics, lap, and invulnerability behavior.

`placements.json` reserves a conservative route corridor using 11 m horizontal half-width and 2 m vertical half-height. The validator inflates each local collision box and tests every dense route chord against it. This passes for the supplied 15 R1 / 13 R3 solid parts. The non-colliding distant skyline and waterfall/vine effects are excluded. The conservative local-XZ inflation protects the full deck width plus a margin, but is not a swept car-collider proof. It does not validate camera clipping, deck support collisions, art replacements, free-flight excursions, or playable speed.

For hollow landmarks, use `collision_parts`, not the full size envelope:

- N03 has a 75 m clear opening between its towers; its overhead beam starts at game Y=167 m.
- F02 has a 44 m clear opening between the rock piers; the crown starts 22 m above the local flight center.

N07's 110 × 5 × 28 m placement is a **deck slab only**, not the filled envelope of an entire canal bridge. Build final piers/supports separately below the deck with a hollow underpass, then validate each solid support. N12 distant skyline has no collision.

When a final model is larger, shifted, slanted, banked, or regenerated, recheck its real bounds and collisions. Expand the terrain and skyline around the course only after the route and all vehicle clearances have been accepted in the game.

## Verification performed

Eleven offline tests passed during authoring. They cover the supplied routes, coordinate roundtrip/handedness/yaw, C1 spline closure, gate crossing direction/reset positions, non-adjacent crossings including exact touches, rejected stale lengths/bad gate spacing/excessive grades, rejected course-blocking objects, rejected filled arch/skybridge envelopes, and closed mesh manifold/winding/UV checks. The tests intentionally fail bad layouts rather than merely copying implementation formulas.

The PNG/SVG route diagrams were rendered from JSON data and visually reviewed. All source files passed Python compilation. These results establish a usable source handoff; they do not establish Blender execution, an integrated game map, art-to-mesh consistency, or a browser frame-rate target.
