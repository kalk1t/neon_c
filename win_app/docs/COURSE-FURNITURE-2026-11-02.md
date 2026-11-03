# Selected-road course furniture — 2026-11-02

Native roads now use the browser's guidance dot spacing, sloped chevron frames,
paired beacons, outside-turn barriers/signs and checkpoint station supports.
The renderer reuses six existing browser prefab meshes exported CPU-only:
beacon, barrier, both sign directions, trail chevron and full ring station.
No new art was generated or purchased. Shared service platforms/trees and the
complete browser city/skyway layout remain separate gaps.

## Reference and implementation

Browser package 0.2.3, seven source hashes in `reference/course-furniture.json`,
captured 18:42:03Z and rechecked after builds. The actual setRoad function runs
in an isolated CPU adapter: real Track/rng/Three matrices and physics constructors,
render-object sinks in place of renderer/UI. Actual props.js/geo.js prefab
builders supply triangle positions/normals/UV. No browser renderer, storage,
account/network or deployment operation executes. Three package version/hash
is also retained in the build receipt. No Git commit identity is asserted.

Placement preserves near-checkpoint/start exclusion, turn thresholds,
per-road seeded Mulberry32 RNG, both sign directions and the browser's lastSign
update after each accepted barrier. Collision order is exact per road: five
shapes per ring (including the previously missing support boxes), beacons, then
interleaved barriers/signs. Road changes rebuild rather than append; roam disables
all race furniture colliders. Locks remain intact; only Skyline is selectable.

Skyline: 805 dots, 131 prefab instances (64 beacons, eight barriers, two signs,
57 chevrons), 114 road colliders and 527 total city/start/road shapes.
Apex: 1,485 dots, 261 instances, 255 road colliders; 668 total with native city.
The existing 1,024-shape collision capacity is sufficient; it was not raised.
The app reuses its prepared course plan for collision assembly. Compatibility
callers use checked temporary heap storage, avoiding a large stack allocation.

Six .rrmesh assets contain 31,884 vertices, 1,147,872 total file bytes, with
individual counts/digests in the manifest. Runtime validates header/length/count,
finite positions/normals and allocation success before accepting a mesh. GPU
uploads share the existing two-per-frame budget with buildings, not an additional
budget. Draw limits: nearest 16 visible furniture items, up to 256 visible
guidance dots, existing gate frustum/distance checks and building budgets.
Dots use small native sphere approximation at the exact browser positions;
prefab triangle geometry is retained. Active gate cue uses a green inner circle.

Source base RGB colors are retained, but HDR emissive/PBR maps, reflections,
browser bloom/material response and runtime ring/dot color animations are not
equivalent. Limited visible instances can pop at budget boundaries. This is a
bounded portable rendering implementation, not complete browser artwork acceptance.
Start gantry/deck/control-tower presentation remains the earlier simple structural
approximation. Full ring-station mesh replaces its earlier cylinder approximation
after deferred upload; fallback remains while meshes wait for their upload slot.

## Fresh verification and corrected failures

Full native MSVC and Emscripten/raylib WebAssembly builds succeed in
`build/course-furniture-native` and `build/course-furniture-web`; each passes
**12/12** headless suites. New fixture coverage compares all nine roads:
10,307 Float32 dot matrices, 1,782 prefab transforms and 1,650 ordered colliders,
plus road replacement, off flags and contacts. Worst double error 1.63e-11;
browser instance-matrix Float32 quantization reaches 0.000122 and is checked
separately with a 0.0005 bound. Existing eleven suites rerun on this core;
the old collision test verifies its original shape subset at their new offsets,
while the new test covers the full expanded collider list.

An initial native /WX build caught an int-to-float culling-distance literal;
explicit float literals corrected it. The first WASM run failed the collision
suite with a stack overflow from temporary course data. The application now
reuses its cached plan and compatibility calls allocate/free checked heap data;
the complete native and WASM suites then pass. Original failure receipt:
`reference/course-first-wasm-stack-failure.log`. Stack size/safety settings were
not enlarged/disabled. Known upstream ES3 and stb_vorbis warnings remain.
Final hashes/receipts: `reference/verification-20261102/course-furniture-builds.json`.

## Controlled native observation

Normal executable, no smoke/fallback/validation flags, 18:48:03Z–18:48:39Z,
window 266042. Inventory showed no overlapping Roadsters instance.
Observed locked picker, Skyline load, GO, denser green guidance line, magenta
chevrons and detailed paired beacons in the real world view. Logs confirm all six
mesh uploads succeeded, 805/131/114 course counts and 527 solid world shapes.
The normal car remained stationary without throttle; this is a rendering/start
check, not a completed race or a manually flown furniture contact course.
Paused with P, closed only Roadsters, verified absent window and clean shutdown.
No new physical stop or input-protection denial occurred.

30 FPS target, 960x720 window, 800x600 world target, MSAA/glow off and existing
overload/focus protections remain. Loading heartbeat reached 176.2ms; subsequent
active intervals were about 33.8–34.9ms including pacing. No sustained performance
qualification or claim the prior PC freeze cannot recur follows. No Web graphics
run or all-route visual acceptance occurred. Original log:
`reference/graphics-20261102/course-furniture-normal-184803-original.log`.

Earlier 8/8 controls, 9/9 city, 10/10 finish and 11/11 collision reports/logs/builds
remain separate. Interrupted trials remain interrupted. The existing results
screenshot and successful Library receipt are unchanged; no reupload or newly
generated reporting screenshot occurred.

## Remaining work and decisions

Next independent W2 gaps: shared service platforms/trees, full city/skyways and
prefab start-complex artwork, then manual full-route race/collision acceptance.
Rendering material/animation differences above remain open. AI, combat, audio,
car/shop/cockpit, physical input devices, persistence/accounts and cross-play
remain later parity gates. No immediate product decision is needed for continued
course/art work. Native same-UID identity/App Check and local account/device data
scope remain unresolved before account/progress integration; no choice was guessed.

No unlock/progress/profile/storage/reward, security/backend or Claude-owned files
were changed. No paid calls, publishing or Doorways interference occurred.

## Changed paths and preservation

Modified: `src/core/rr_world_plan.c`, `src/core/rr_world_plan.h`, `src/app/main.c`,
`src/presentation/rr_scene.c`, `src/presentation/rr_scene.h`,
`tests/test_physics.c`, `CMakeLists.txt`, `tools/race-rules-check/CMakeLists.txt`,
`README.md`, `PROJECT_DIRECTION.md`, `PLAYING.txt`, `docs/PARITY.md`.
Added: `src/core/rr_course_plan.c`, `src/core/rr_course_plan.h`,
`tools/capture-course-furniture.mjs`, `tests/test_course_furniture.c`,
`tests/generated_course_furniture_reference.h`, `reference/course-furniture.json`,
six `assets/course/*.rrmesh` files, this report, preserved WASM failure log,
two twelve-test receipts, artifact/hash receipt and normal graphics log.
Source originals: `reference/pre-course-furniture-20261102/`; status-doc originals:
`reference/pre-course-furniture-docs-20261102/`. All project writes stayed win_app;
prior originals/builds/reports and screenshot Library receipt are preserved.
No current denial, pending action, uncertain outcome or development blocker.
