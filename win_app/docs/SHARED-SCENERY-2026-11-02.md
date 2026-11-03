# Shared platforms, trees and source material inputs — 2026-11-02

Native scenery now includes the browser's immutable shared Skyline-based service
platform/tree layout, cleared against all nine roads and all checkpoints. It
contains 18 platforms (nine with columns), 21 trees, 39 transforms and 48 solid
cylinders. Four trees occupy the exact start-deck positions. Shared objects stay
solid in roam and do not move when another road is selected. Selected-road race
furniture still switches/off-flags separately; no unlock or progress is granted.

## Current browser reference

Package 0.2.3 identified by source hashes in `reference/shared-scenery.json`,
including course/track/materials/main/skyprops/car and the existing cyan-tree GLB.
The actual shared course block executes CPU-only using real Track/Corridor/rng
and Three matrices with render sinks. Full source hashes were rechecked after
builds. No browser working-tree or deployment write occurred, and no Git identity
is asserted. Seeded RNG order, all-road exclusions, optional columns/trees,
scale, rotations and collider insertion order match the reference.

New RRSharedScenery is prepared once with Skyline as its explicit required base;
other roads cannot silently replace it. World order is city/start/shared/road,
matching construction order in the browser. Skyline now has 575 solid shapes;
Apex with this native prototype city has 716. Existing 1,024-shape capacity,
16-nearest-furniture draw limit and two shared mesh uploads per frame remain.

## Existing cyan-tree asset, fallback and material limits

The current browser's dressScenery replaces both procedural tree colors with
`public/models/props/cyan_tree.glb` after loading. Native now follows that rule;
the four exported procedural platform/tree meshes retain the browser fallback.
No paid generation or new artwork was used.

Existing GLB: one identity-transform mesh, 10,786 vertices, 15,000 triangle-index
entries. The original BIN was retained during the established Chrome lossless
WebP-to-PNG conversion. The runtime export flattens the original triangle order,
positions, normals and UV without geometry approximation and uses the decoded
1,024x1,024 base-color PNG. Original normal/roughness maps and full PBR rendering
remain unported; they are not loaded into VRAM merely to leave them unused.
Full converted GLB stays in reference, outside WASM runtime asset preloads.

The same 13 / 0.9980993866920471 = 13.024755022729025 height fit is applied on top
of each existing tree transform, and the material's double-sided behavior is
retained. Tree collision cylinders remain the browser's 4*s radius / 11*s height;
they were not enlarged to a new visual envelope. Model/texture failure retains
the procedural tree variants; there is no startup dependency on a paid service.

The tree shader now uses the source saturation/value neon thresholds and 2.4
glow intensity, with sRGB texture decoding for the mask. Fog uses the current
0.00062 FogExp2 density, camera view depth instead of radial distance, and the
source fog color adapted through Three's sRGB conversion into the current native
RGB pipeline. This corrects concrete source inputs/calculations, not full material
equivalence: native lighting/ACES/output color pipeline, normal/roughness maps,
environment reflections, foliage emissive response, bloom and dynamic course
lights still differ. The source sculpt mask/glow is scoped to the cyan-tree draw;
other materials retain their previous behavior.

Manifests: `reference/cyan-tree-conversion.json` (source/output hashes and lossless
image conversion), `reference/cyan-tree-geometry.json` (mesh/PNG/fit/double-sided
details), `reference/shared-scenery.json` (placement/material source and assets).

## Fresh builds and tests

Full MSVC native and Emscripten/raylib WASM applications build in
`build/shared-scenery-native` and `build/shared-scenery-web`; each passes **13/13**
suites. New coverage compares 39 browser transforms and 48 ordered colliders,
all-road clearance, base-road enforcement, selection independence, shared solidity
in roam, ten actual Three FogExp2 cases and 15 actual source neon-mask cases.
Worst numerical error is 1.81e-11. Existing twelve suites rerun on this core.

The first full runs failed three fog RGB assertions because the test assumed
Three's sRGB round-trip equals plain byte/255 exactly (difference up to 6.14e-6).
The reference now captures Three's actual converted values and tests those at the
original tolerance; no tolerance was weakened. Initial native/web failure logs
are preserved as `reference/shared-first-native-fog-color-test-failure.log` and
`reference/shared-first-web-fog-color-test-failure.log`.
Known upstream ES3 override/stb_vorbis warnings remain. No Web graphics run occurred.
Receipts/artifact hashes: `reference/verification-20261102/shared-scenery-builds.json`.

## Conservative normal native check

Normal native executable, no smoke/fallback/validation flags, 19:05:48Z–19:06:43Z,
window 2165020. Verified no overlapping Roadsters instance. Observed preserved
locks, Skyline load/countdown/GO, existing cyan-tree model near the start deck,
guidance/beacons and pause. Logs confirm 18/9/21 shared counts, all 11 mesh uploads,
cyan tree loaded=1, 15,000 vertices, 13.024755023 fit, 1,024x1,024 texture and 575
solid world shapes. This is a local normal-start/render check; it does not show
every distant platform or establish a manually flown contact/race acceptance.

Paused with P, closed only Roadsters, verified window absent and clean shutdown.
No new physical stop/input-protection rejection occurred. 30 FPS, 960x720 window,
800x600 world target, MSAA/glow off, existing focus/overload protections remain.
Loading heartbeat reached 249.2ms; active intervals were about 33.7–36ms including
pacing. No sustained performance qualification or freeze-resolution claim follows.
Original log: `reference/graphics-20261102/shared-scenery-normal-190548-original.log`.

Earlier test/build/report identities remain separate. Interrupted trials remain
interrupted. Existing results screenshot and successful Library receipt are
byte-identical; no reupload or newly generated reporting screenshot occurred.

## Remaining gaps and decisions

This bounded shared-scenery slice is complete to the scopes above. Next independent
world/art gaps are canonical browser city/hero towers and skyways/props, sculpted
control tower, full start-deck/gantry presentation, source material/animation and
lighting pipeline. Manual full-route race/collision acceptance remains open.
AI/combat/audio/shop/cockpit, physical-device controls, persistence/accounts and
cross-play remain later parity gates. No immediate product decision is required
for continued world/art work. Native same-UID identity/App Check and account/device
storage scope still require a coordinated plan before account/progress writes.

No locks/progress/profile/storage/reward, backend/security/credential, paid calls,
publishing or Claude-owned changes occurred. Doorways work was untouched.

## Changed paths and preservation

Modified: `src/core/rr_world_plan.c`, `src/core/rr_world_plan.h`, `src/app/main.c`,
`src/presentation/rr_scene.c`, `src/presentation/rr_scene.h`,
`src/presentation/rr_shaders.h`, `CMakeLists.txt`,
`tools/race-rules-check/CMakeLists.txt`, `README.md`, `PROJECT_DIRECTION.md`,
`PLAYING.txt`, `docs/PARITY.md`.
Added: `src/core/rr_shared_scenery.c`, `.h`, `rr_atmosphere.h`,
`rr_shared_material.h`, `src/presentation/cyan_tree_asset.h`,
`tools/capture-shared-scenery.mjs`, `tools/prepare-cyan-tree.mjs`,
`tools/export-cyan-tree.mjs`, `tests/test_shared_scenery.c`,
`tests/generated_shared_scenery_reference.h`, five `assets/course/*.rrmesh`
(platform, platform-column, tree-magenta, tree-cyan, cyan-sculpt),
`assets/models/cyan-tree-base.png`, source/converted cyan-tree reference GLBs,
three source/conversion/geometry manifests, this report, initial failure logs,
two thirteen-test receipts, artifact/hash receipt and normal graphics log.
Initial source originals: `reference/pre-shared-scenery-20261102/`;
intermediate material-test sources: `reference/pre-shared-cyan-enhancement-20261102/`;
status originals: `reference/pre-shared-scenery-docs-20261102/`.
All project writes stayed win_app. No current denial, pending action, uncertain
outcome or development blocker remains.
