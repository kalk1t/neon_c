# v0.2.6 camera timing and finish transition, 2026-11-03

## User-visible correction awaiting native observation

Windows previously eased a released RMB free-look orbit back only in `controls`, which runs during active driving. Finishing stopped that function, freezing any residual orbit angle. Browser v0.2.6 applies the return in chaseCamera after player updates, so an already-released orbit continues settling during finished/autopilot frames. This matters particularly when the residual orbit crosses the raw first-person visibility threshold.

The application now records the last processed look flag and calls shared `rr_camera_motion_step` once in the camera update. That step retains the source shortest-angle yaw follow, pitch follow and released-orbit exponential return. Its dt is the host's existing clamped frame dt. Active/countdown/finished/results camera phases can return the orbit; menu and pause do not newly consume camera-input updates or decay the orbit. Existing pause/showcase presentation remains a separate parity gap. No raw mouse event approximation, new setting, input capture action or additional simulation tick was added.

The last processed look flag is deliberately retained after finishing, just as browser autopilot does not call updateAim. If looking was true when active control ended, the orbit remains held rather than inventing a new release rule. A new native session resets its already-reset orbit and the flag together. Native mode, base chase pose, raw first-person body hiding and default-OFF cockpit candidate remain preserved.

## Source and fresh test scope

Reference is current `game/src/camera.js`, SHA256 `5961c2c7d3d81a3412402673f1901f784485b1459a053a77e743cdccd35aba45`, unchanged from the prior v0.2.6 camera capture. `aim.js` was inspected to establish that updateAim owns cam.looking; `main.js` was inspected to establish that finished autopilot bypasses updateAim and chaseCamera still runs. Browser files were read only. No whole-source/deployed-release equivalence is claimed.

`tools/capture-camera-motion.mjs` executes the actual chaseCamera function with real Three PerspectiveCamera math, inert presentation bindings and no renderer/network/storage/account. It captures six independent 160-tick sequences: 120 ticks with active look changes and 40 finished ticks retaining the last look flag. Each sequence uses zero, 1/60, 1/30 and 0.05 dt, wrapped angles, changing car yaw/pitch, held/released/re-held orbit and differing initial angles. C carries its own camera state across all 160 ticks rather than resetting to the reference each frame.

The 960 generated states compare four camera timing values and nine resulting pose components after the existing shared chase-geometry calculation. Results therefore cover temporal/geometry coupling, rather than only isolated exponential constants. Hardware mouse capture, raw event filtering, native input delivery, actual graphics, pause showcase, effects/Nitro/shake/punch, FOV and account/progression are excluded. The fixture uses already-clamped dt and does not claim to test clock acquisition.

Fresh full native and WASM application builds passed. Seven focused tests passed 7/7 on each target with zero failures; maximum timing/pose component error was 1.8207657603852567e-14 native and 3.5527136788005009e-15 WASM, each below the 1e-10 tolerance. Fifteen preceding input/first-person/chase implementation, fixture and provenance files were checksum-verified unchanged. Results are recorded in `reference/ui-parity-20261103/camera-motion-implementation.json` and `camera-motion-v026-{native,web}-build.log`. Previous full 42/42 input runs and prior five/six-test camera runs remain historical; a full 45-test suite was not rerun for this bounded timing correction.

## Exact paths and preservation

Implementation paths: `src/core/rr_camera_motion.h`, `src/app/main.c`, `CMakeLists.txt`, `tests/test_camera_motion.c`, `tests/camera_motion_fixture.h`, `tools/capture-camera-motion.mjs`, `reference/ui-parity-20261103/camera-motion-oracle.json`.

Prior app/CMake originals: `reference/pre-camera-motion-20261103`. Isolated new builds: `build/camera-motion-v026-native` and `build/camera-motion-v026-web`. Earlier input, first-person and chase headers/tests/fixtures/tools remain preserved. No executable from an earlier trial or its assets was replaced. No assets, cache/upload caps, framerate/graphics safeguards, wallet/progression/unlocks, accounts or browser-owned files changed.

The prior automatic approval rejection still blocks graphics launch pending the separately requested explicit authorization. No launch retry, desktop input, process query or termination occurred this turn. Native view comfort, clipping, input behavior and actual finish transition remain unobserved. The first pending graphics scope stays menu/track selection; a later world/finish trial needs its own bounded scope.
