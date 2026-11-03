# Existing control-tower hero asset — 2026-11-02

The native start complex now uses the browser's existing sculpted control-tower
model instead of its two-cylinder approximation, with the source placement,
height fit, yaw, double-sided material, texture filtering and glow setting.
Canonical city layout/crowns and whole-scene PBR/lighting are not complete.

## Reference and visible-difference inventory

Browser package **0.2.3**, source capture **19:16:00Z**, exact eight source/asset
hashes in `reference/control-tower.json`, rechecked after builds. This identifies
the working-tree content used, not a Git commit or deployed-release guarantee.
No browser files or live deployment were changed.

| Remaining visible difference | Current reference | This slice |
|---|---|---|
| City layout and silhouettes | city.js seed 1337, BASE -260, 82-unit grid/extent 2700; multiple tower styles | Native 404-body prototype remains; canonical generation is still a major gap |
| Hero crowns | city.js near-route eligibility/RNG, minimum shaft height, source anchors and city_tower.glb crowns | Asset inventoried (898,516 bytes, 14,357 source vertices); deferred until canonical shaft/layout data is ported, avoiding arbitrary crowns |
| Control landmark | dressScenery source-bounds fit of control_tower.glb | Implemented; highest-impact self-contained existing landmark |
| City/material response | PBR normal/roughness/environment maps, ACES/output pipeline and bloom | Source base texture, sampler and existing sculpt mask/glow reused for this model; full response remains open |
| Wider props/start art | Canonical skyways/props, textured gantry/deck, ads and lighting animation | Existing partial structures remain |

The browser's actual dressScenery function was executed CPU-only with its
controlTowerPrefab geometry and original model position bounds; asynchronous
model loads were controlled data adapters, not renderer/network calls. Twelve
cases vary the old prefab's world bounds and supplied model size and capture
placement plus transformed world bounds. City/tree branches are empty in this
adapter. The captured default old bounds are (-95.399999619,-349,-5.399999619)
to (-50,194.199996948,25.399999619).

## Geometry, source fit and material subset

Original `public/models/props/control_tower.glb`: 942,116 bytes, one identity-node
mesh, 12,746 source vertices and 27,000 triangle-index entries (9,000 triangles).
The established installed-Chrome workflow losslessly decodes embedded WebP images
to PNG while retaining the original BIN. Runtime exports preserve triangle order,
positions, UV and normals; no mesh simplification or new generation occurred.
Only the decoded 1024x1024 base-color map is loaded into VRAM. Converted normal/
roughness images stay in reference rather than bloating runtime/PBR claims.

The exact source formula is used: position at the old bounds' X/Z center and Y
minimum, scale = old height * 1.05 / model height, yaw = pi/2. Default native log:
position (-72.7,-349,10), scale 576.504097543, yaw 1.570796327. Original model
min Y 0.01039352547377348 is retained, not rebased to zero. Its large source fit
therefore remains large; this is not a new visual redesign. Model bounds drive
culling. Browser dressing does not rebuild its original cylinder colliders, so
those physics shapes remain unchanged here too. Visual/collision-envelope
differences remain a reference limitation, not a newly invented larger collider.

The existing source sculpt-mask path is reused with glow=2.6. Source GLTF sampler
magFilter=9729 (linear), minFilter=9987 (linear mip interpolation) and default repeat
wrap are matched with 11 mip levels. Double-sided rendering is retained. This is
base-texture/sampler/mask parity for one asset, not full PBR: normal/roughness,
environment reflection intensity, physical lighting, ACES/output color handling
and bloom remain unqualified. The old structural fallback stays until resources
are ready, or if loading fails.

Manifests: `reference/control-tower.json`, `control-tower-conversion.json`,
`control-tower-geometry.json`. Source and full converted GLBs stay under reference;
they are excluded from Web runtime asset preloads.

## Resource cost and guards

New runtime files: 972,008-byte mesh and 2,327,865-byte PNG, 3,299,873 bytes total.
Mesh attributes use approximately 972,000 bytes each in CPU storage and GPU
buffers. RGBA8 1024-square texture plus its complete mip chain is approximately
5,592,404 GPU bytes; combined incremental GPU storage is about 6.26 MiB, excluding
driver overhead. These are format-based estimates, not GPU memory measurements.
At most one additional 9,000-triangle landmark draw replaces the simple cylinders.

The scene now owns 12 mesh slots; the same two-mesh-uploads-per-frame budget is
shared with buildings. Building/visible-furniture limits, render target, FPS,
focus and overload safeguards remain unchanged. Texture decode/mip creation is a
bounded one-time scene-load operation. Cleanup frees the material map, texture
and mesh resources. No credentials/security/system settings were changed.

The pure RRC1 validator now runs before allocation for all prefab/hero meshes:
magic, complete length, nonzero triangle count, the existing 100,000-vertex cap,
and finite position/normal/UV payload. Tests exercise valid input, truncation,
bad magic/count, huge counts and non-finite UV. The cap was not raised.

## Fresh build/headless evidence

Full native MSVC and Emscripten/raylib WebAssembly builds succeeded in
`build/control-hero-native` and `build/control-hero-web`; both pass **14/14** suites.
New hero cases match 12 actual browser transforms/world bounds, worst error
5.68e-14, plus invalid-size and mesh-resource/finite guards. Existing thirteen
suites reran unchanged. Final sampler-only renderer refinement was then rebuilt
on native/WASM before graphical checks; it did not alter core/test behavior.
Known upstream Web ES3 override/stb_vorbis warnings remain. No WASM graphics run
occurred. Exact binaries, receipt hashes and source checks:
`reference/verification-20261102/control-hero-builds.json`.

## Separate controlled native views

Normal build: **19:21:06Z–19:21:56Z**, window 788822. Observed preserved locked
picker, Skyline startup and the sculpted landmark at the left of the normal
world view. Log confirms loaded=1, 27,000 vertices, 1024-square texture, 11 mips,
source transform/glow, successful deferred upload and normal GO. Paused before
closing, then verified clean shutdown and absent window before the next launch.
Original log: `reference/graphics-20261102/control-hero-normal-192106-original.log`.

Camera-only build: **19:22:13Z–19:23:24Z**, window 29560818,
`build/control-hero-view`. `RR_HERO_VIEW` defaults OFF and is labeled in window,
logs and overlay. It changes only the inspection camera, leaving actor state,
road eligibility and progress unchanged. Observed sculpted structure, cyan/magenta
texture details and smoothed texture at distance. Existing prototype towers
obscure part of the source-fitted model; that is a visible canonical-city gap,
not evidence of whole-scene acceptance. Paused/closed only this app; verified
clean shutdown and no Roadsters window remaining.
Original log: `reference/graphics-20261102/control-hero-view-192213-original.log`.

Both used normal startup without smoke/fallback flags, 960x720 window, 800x600
world, 30 FPS target, MSAA/glow off. Loading peaks were 253.1ms normal/244.3ms
inspection; active heartbeats were about 33.4–33.9ms including pacing. No physical
stop/input-protection rejection occurred. No sustained performance, freeze
resolution, full material comparison or complete race qualification follows.
No reporting screenshot was regenerated or uploaded. Existing finish screenshot
and successful Library receipt remain byte-identical. Earlier interrupted trials
and prior scoped 8–13-suite evidence remain separately preserved.

## Remaining work and decisions

Next high-impact independent world work is canonical city layout/shaft/style
generation before hero crowns, then skyways/props and full start-complex art.
Whole-scene material/lighting, manual full-route acceptance, AI/combat/audio,
physical devices, accounts/persistence and cross-play remain open. No product
decision is required for that independent world work. Native same-UID identity/
App Check and local data scope still need a coordinated plan before account writes.
Unlocks/progress/user data are unchanged. No paid generation, publishing,
credential changes or Doorways interference occurred.

## Exact changes and preservation

Modified: `src/presentation/rr_scene.c`, `src/presentation/rr_scene.h`,
`src/app/main.c`, `CMakeLists.txt`, `tools/race-rules-check/CMakeLists.txt`,
`README.md`, `PROJECT_DIRECTION.md`, `PLAYING.txt`, `docs/PARITY.md`.
Added: `src/core/rr_hero_asset.h`, `rr_control_tower_data.h`, `rr_mesh_asset.h`,
`src/presentation/control_tower_asset.h`, `tools/capture-control-tower.mjs`,
`tools/prepare-control-tower.mjs`, `tools/export-control-tower.mjs`,
`tests/test_hero_asset.c`, `tests/generated_hero_reference.h`,
`assets/course/control-sculpt.rrmesh`, `assets/models/control-tower-base.png`,
source/converted reference GLBs, three hero/geometry/conversion manifests, this
report, two fourteen-test logs, artifact/hash receipt and two original graphics
logs. Source originals: `reference/pre-control-hero-20261102/`; status originals:
`reference/pre-control-hero-docs-20261102/`. All project writes stayed win_app;
browser files, earlier binaries/reports and user progress are preserved.
No current denial, pending action, uncertain outcome or development blocker.
