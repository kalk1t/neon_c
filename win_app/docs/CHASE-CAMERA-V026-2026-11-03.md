# v0.2.6 chase-camera geometry, 2026-11-03

## Concrete correction

The current browser package is 0.2.6. `game/src/camera.js` SHA256 `5961c2c7d3d81a3412402673f1901f784485b1459a053a77e743cdccd35aba45` remains the same module captured during the preceding raw first-person slice. This turn follows that module directly without browser edits, Git inspection or any release/deployment assumption.

The Windows chase camera previously used an uncapped speed factor for camera pullback/height, whereas the browser caps it at 1.4. Native banking tilted a world-up reference using an approximation tied to smoothed yaw; the browser rolls around the actual lookAt viewing direction. These differ when the car pitches, the camera lags or orbits, and during high speed. The source-derived shared C pose now uses the capped speed factor, exact near/far/fallback offsets, target construction, world-up lookAt basis and local roll. It also preserves Three's parallel-eye/up fallback rather than dividing by a zero cross-product.

Only base chase position and orientation were changed. Existing camera yaw/pitch smoothing, FOV progression and mouse/orbit input ownership stay in the host. Native float conversion occurs after shared double calculations. The earlier three-mode cycle, raw-seat pose/body-hiding and optional dashboard behavior remain intact. The offline adapter still passes alive=true; the shared dead-player pullback case is tested for future integration but does not enable wreck/combat gameplay.

## Actual-source verification

`tools/capture-chase-camera.mjs` executes the actual v0.2.6 chaseCamera function with real Three PerspectiveCamera math in a headless VM. dt=0 isolates base geometry from temporal smoothing and effects. It records source and Three-module hashes, verifies camera.js did not change during capture, and produces `tests/chase_camera_fixture.h` plus `reference/ui-parity-20261103/chase-camera-oracle.json`.

The 1225 cases cover near/far chase, raw-mode fallback during orbit, alive/dead states, negative/zero/base/cap/boost/above-boost speed, roll signs, camera versus car direction/pitch differences, wrapped look angles and one exactly parallel world-up case. Position, forward and up comprise nine compared components per case. Nitro, shake, kick, lens/postprocessing, graphical clipping and actual input delivery are excluded. Mode-2 raw seating remains covered by the preceding fixture, not confused with chase fallback.

The oracle also records differences from an analytic reconstruction of the old native equations using doubles, with an actual lookAt to interpret its old up reference. Those maxima span synthetic boundary inputs, including the exactly parallel edge case; they are not normal-gameplay visual severity or hardware measurements. Native float rounding is excluded from that old-equation comparison.

Fresh full native and WASM application builds passed. All six focused headless tests passed on each target; the geometry maximum component error was 4.4408920985006262e-16 on both, with zero failures. Ten preceding input/first-person implementation, fixture and provenance files were checksum-verified unchanged. Results are recorded in `reference/ui-parity-20261103/chase-camera-implementation.json` and `chase-v026-{native,web}-build.log`. Prior full 42/42 regression results and previous first-person 5/5 results remain separate historical evidence; no full 44-test-suite pass is claimed for this slice.

## Changed paths and preserved work

Implementation: `src/core/rr_chase_camera.h`, `src/app/main.c`, `CMakeLists.txt`, `tests/test_chase_camera.c`, `tests/chase_camera_fixture.h`, `tools/capture-chase-camera.mjs`, `reference/ui-parity-20261103/chase-camera-oracle.json`. Prior app/CMake originals are saved in `reference/pre-chase-20261103`; builds use new isolated `build/chase-v026-native` and `build/chase-v026-web` outputs. Earlier input and first-person shared headers, fixtures, tests and capture tools are preserved.

No asset import, new art, upload/cache cap change, progression/unlock/account/network write, browser edit or paid call occurred. Graphics settings, focus/overload safeguards and default-OFF cockpit candidate remain unchanged.

## Limits and next real checks

The rejected graphics launch remains pending separate explicit authorization; no retry or alternative launch method was used. No graphics window was opened and there is no fresh visual/stability/shutdown result. Raylib's final float view matrix, practical occlusion/clipping and user comfort still require the later controlled native world test; the initial pending test remains menu/track selection only. Camera shake/kick/lens effects, menu/pause showcase and free-look decay outside active player control remain separate open parity gaps.
