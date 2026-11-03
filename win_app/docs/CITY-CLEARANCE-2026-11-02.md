# Shared all-route city clearance — 2026-11-02

The native city now clears the union of all nine browser routes, independent of
the selected road. Previously it removed towers using the selected route only.
This ports browser Corridor and default candidate fitting policy onto the existing
native prototype candidates; it does not port the complete browser city layout.

## Reference and implementation

Browser package 0.2.3, identified by six source hashes in
`reference/city-clearance.json`, captured at 18:09:24Z and rechecked after builds.
No browser source was changed; Git history was unavailable, so no commit identity
is asserted. CPU-only capture executes actual Track/Corridor and extracts the
default candidate fitting block from city.js with injected fixed randomness.
No browser renderer, account adapter or persistent storage was executed.

The immutable union contains 18,052 full samples, 3,615 coarse samples, 117 gates
and 337 occupied 160-unit grid cells. Queries preserve strict distance/height
boundaries, flat-order nearest ties and negative cell handling. Fitting preserves
start-area height cap, checkpoint exclusion, conditional random lowering and the
40-step height-lowering limit. Synthetic high/lower crossings check that nearest
XZ alone cannot hide a lower road. City generation no longer reloads per road.

Native seed/grid/dimensions/colors/base remain prototype content. The resulting
404 accepted towers include 104 lowered towers; 221 candidates are rejected.
Conditional lowering draws affect subsequent prototype RNG consumption, so the
resulting arrangement changes. Full browser seed/layout/style, elevated buildings,
traffic/skyways/decor, collision responses and post-finish autopilot remain open.
Renderer budgets, 30 FPS target, 800x600 world target, MSAA/glow defaults and
overload pause remain unchanged.

## Fresh verification

Full MSVC native and Emscripten 6.0.13/raylib 6.2 WebAssembly builds succeeded in
`build/city-clearance-native` and `build/city-clearance-web`. Both pass **9/9**
headless tests, including existing road-picker/lifecycle/geometry coverage rerun
on this source. The new suite matches 1,534 actual browser corridor queries and
651 candidate-fitting cases (worst error 3.87e-12), checks all-route-independent
generation and finds zero sampled route/body-roof AABB intersections across
18,052 centers, with roof overhang/height and a 4-unit expansion. This finite
sample audit is not a continuous clearance or collision/vehicle-envelope proof.

Web build retains upstream ES3 override and stb_vorbis comparison warnings;
application and tests link successfully. No Web graphics execution occurred.
Receipts and exact artifact hashes: `reference/verification-20261102/city-clearance-builds.json`.

Controlled native test: 18:15:19Z–18:15:46Z, executable from the native build
above, normal startup without smoke/fallback flags. Inventory showed no prior
Roadsters window. Observed normal menu, picker with only Skyline unlocked,
shared-city load/countdown, GO and P pause (timer 00:02.16). Log confirms union
counts and 404/104/221 city counts, 960x720 window and 800x600 world target.
Observed Roadster, gates, cyan route beacons and neon towers; this is a local
rendering check, not a browser side-by-side art acceptance or all-route visual run.
One heartbeat included 146.9ms around scene load; no sustained performance or
freeze-resolution claim follows. Paused before title-bar close; a subsequent
inventory verified window 4198532 absent and log confirms clean shutdown.
No input-protection rejection or new physical stop occurred.
Original log: `reference/graphics-20261102/city-clearance-181519-original.log`.

Prior road-picker 8/8 and Enter/navigation/locked refusal/pause/resume/reset visual
evidence remain separately preserved in `docs/ROAD-PICKER-2026-11-02.md`; they
are not relabeled as this session or whole-game acceptance. Earlier interrupted
trials remain interrupted evidence. Full race/results, other-road visuals,
physical input devices and sustained performance remain unqualified.

## Progress boundary and next work

Only road 1 remains selectable; fixtures initialize locked roads in memory for
analysis only. No profile import, unlock grant, local progress storage, reward,
backend/account, credential or security changes occurred. Native integration
still needs an agreed same-UID sign-in/token/App Check approach, profile refresh
and account-switch handling, and versioned/atomic local best/settings storage
with explicit account-versus-device scope. Shared server-authorized reward and
progress rules must be preserved; offline prototype finishes cannot become wins.
Resolve these before production progress changes, not by inventing clean-account
unlocks. Independent next work: browser collision shapes/responses and remaining
city presentation; preserve locks while verifying with isolated fixtures.

## Exact changed paths

Modified: `src/app/main.c`, `src/presentation/rr_scene.c`,
`src/presentation/rr_scene.h`, `CMakeLists.txt`,
`tools/race-rules-check/CMakeLists.txt`, `README.md`, `PROJECT_DIRECTION.md`,
`PLAYING.txt`, `docs/PARITY.md`.

Added: `src/core/rr_corridor.c`, `src/core/rr_corridor.h`,
`src/core/rr_city_plan.c`, `src/core/rr_city_plan.h`,
`tools/capture-city-clearance.mjs`, `tests/test_city_clearance.c`,
`tests/generated_city_clearance_reference.h`, `reference/city-clearance.json`,
this report, two nine-test logs, artifact/hash receipt and original graphics log.
Originals: `reference/pre-city-clearance-20261102/`; status-doc originals:
`reference/pre-city-clearance-docs-20261102/`. Earlier binaries/packages/launchers
and evidence remain unchanged. All writes stayed win_app; no deployment or
Doorways work was touched. No current action is blocked.
