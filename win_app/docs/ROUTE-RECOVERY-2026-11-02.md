# Browser route recovery: 2026-11-02

Latest source/build status. Earlier session/menu graphics evidence remains in
`SESSION-INTEGRATION-2026-11-02.md`; this work did not launch or control graphics.

## Behavior changed

The prototype's immediate reset below height 34 or beyond distance 10,000 from
the origin is replaced by the inspected browser player route behavior:

- Search only +/-30 route samples around the previous hint, in browser loop
  order. There is no implicit global fallback during ordinary updates.
- Use the route's arc-mapped tangent to measure velocity along the course.
  Accumulate wrong-way time only while that velocity is strictly below -15.
  Emit WRONG WAY after strictly more than 1.2 seconds, when the current warning
  timer permits it. The app displays it for 0.8 seconds.
- Accumulate lost time only while below the map floor (Skyline 34) or strictly
  more than 650 units from the selected route sample. Reset this timer on return.
- Emit SIGNAL LOST on crossing strictly above 0.05 seconds; request respawn
  after strictly more than 1.1 seconds. The warning displays for 1.2 seconds.
- Reuse the session's last-safe placement and preserve checkpoint progress,
  lap/race clocks, boost and the browser-equivalent placement state. Rebase to a
  global nearest sample after manual/automatic respawn; clear lost time while
  preserving wrong-way time, as browser `respawn` does.
- Free roam suppresses these warnings and automatic route recovery. This core
  behavior does not implement the missing online free-roam mode.

`src/core/rr_recovery.{h,c}` separates pure threshold/timer conditions from route
geometry. Track samples now retain browser-style arc-mapped tangents. The same
arc-length parameter mapping supplies position and tangent evaluation; the
existing curve/flight/race tests still pass. Recovery is connected after the
session's active flight/race step. No rendering limits, shaders, default graphics
settings, services or backend contracts changed.

## Reference and checks

`tools/capture-recovery-rules.mjs` executes the actual browser `raceRules`,
`respawn`, `Car.place` and Skyline Track methods through a CPU-only adapter.
It never imports browser application startup or initializes a renderer/window.
Seven source inputs are hashed before/after capture and rechecked after builds.
Source identity and scope are in `reference/recovery-rules.json`; the earlier
planning/flight/race manifests remain unchanged.

New fixtures cover 357 browser recovery steps across nine scenarios: distance
and floor boundaries, strict warning/respawn thresholds, direction boundaries,
message suppression, intermittent loss, simultaneous warnings and free roam.
There are also 83 sampled tangents and 28 local/global nearest-point cases,
including wraparound and intentionally distant hints. Tests check respawn
position, speed and boost against browser output and verify rebase timer behavior.

Both full application targets compile in new directories:

- Native: `build/recovery-integration-native/bin/RoadstersRace.exe`.
- WebAssembly: `build/recovery-integration-web/bin/RoadstersRace.html` plus
  JS/WASM/data. No browser graphics was launched.

Both integrated builds pass **6/6 headless CTest tests**. New recovery fixtures
have worst measured absolute error **2.3e-13**; recorded flight/track retains
2.61e-12 and the 1,262 race-rule steps still match exactly. Native C17 app/core
builds use `/W4 /WX`; Web compilation retains the prior upstream raylib ES3
configuration and stb_vorbis warnings, with successful link/tests.

Reproducible headless entry point: `scripts/Test-Headless.ps1` (or `-Web`). Its
separate CMake project now includes all six tests. Latest integrated test receipts
are each new build's `Testing/Temporary/LastTest.log`.

## Evidence stays separate

- Successful prior menu-only test: 17:06:01Z-17:06:27Z, normal diagnostic build,
  menu state throughout, target 0x0 and clean shutdown. This was 26 seconds on
  that earlier executable, not graphics approval of the new recovery build.
- Earlier native/Web headless 5/5 receipts are preserved separately from the new
  native/Web 6/6 receipts under `reference/verification-20261102/`.
- The first interrupted flight's original closed log remains
  `reference/graphics-20261102/interrupted-flight-original.log`.
- The later flight and blocked close are still partial/interrupted evidence.
  Window 4133260 closure has not been rechecked or retried. The owner was asked
  by the coordinating thread to close it when finished. Its locked append log
  was not read/copied through an alternate access path.
- The possibly running `build/menu-diagnostics-native/bin/RoadstersRace.exe`
  remains unchanged, hash `b3a8cb4eafe96f938be58bb47195f7fd86ec18f5f74cdaaa71df1393bcf9c81e`.
  New builds use separate paths and have not been run graphically.

No denied desktop action was retried, no process was terminated, and no new
graphics instance was launched during this slice. Existing packages/launcher and
all browser, root, backend, deployment and Doorways files remain untouched.

## Exact changed paths for this slice

Modified: `src/app/main.c`, `src/core/rr_track.c`, `src/core/rr_track.h`,
`CMakeLists.txt`, `tools/race-rules-check/CMakeLists.txt`, and current-status
documentation `README.md`, `PROJECT_DIRECTION.md`, `PLAYING.txt`.

Added: `src/core/rr_recovery.c`, `src/core/rr_recovery.h`, `tests/test_recovery.c`,
`tests/generated_recovery_reference.h`, `tools/capture-recovery-rules.mjs`,
`reference/recovery-rules.json`, this report and separate validation receipts.

Original code/CMake files and hashes are saved in
`reference/pre-recovery-integration-20261102/`; previous status docs are retained
in `reference/pre-recovery-docs-20261102/`.

## Remaining limits

The recovery UI and integrated behavior have compilation/headless evidence only.
No new native/world/WebAssembly graphics, sustained performance or stability
qualification occurred. The prior full-PC freeze cause remains unresolved.

Post-finish autopilot/showcase, missed-gate notice, AI stuck/respawn rules,
collision/wreck/health behavior, roads 2-9 in C, audio, persistent bests/settings,
identity/economy and cross-play remain open. The application still runs only
Skyline offline solo trial/time attack; no complete parity row or milestone is
certified. Frozen browser revision and hardware targets remain owner decisions.
