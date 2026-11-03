# Solo race finish/results continuation — 2026-11-02

The native offline two-lap session now continues the browser's racing-line
autopilot after finish and under the results overlay. The finish HUD stays over
the world for two seconds, then shows total, both laps and the exact best lap.
Enter starts another trial; M/Escape or the existing button returns to menu.
Lap/finish time and history remain frozen while the global race clock and car
continue. Completed gates cannot be collected again. World gates lose their
active highlight, the HUD labels FINISH rather than lap 3, and finish clears the
stale checkpoint message. No online results, rank, rivals or rewards are invented.

## Current reference and scope

Browser package 0.2.3, four source hashes in `reference/finish.json`, captured at
18:19:57Z and rechecked after the final builds. Existing browser files and live
deployment were read-only. No Git identity is asserted. The capture executes the
actual browser autopilot function and solo showResults in an isolated VM, with
no-op account/unlock/telemetry adapters. It does not call real storage/network.

The line follower targets `(sample+16)%n`, wraps yaw through atan2(sin,cos), uses
the same steering/climb clamps and strict brake/boost turn thresholds. Recovery
sample tracking/respawn also continues after finish. The native focus/minimize
and ten-slow-frame protections extend to finished/results: they pause that phase
until explicit resume, preserving an accessible cursor for results. This is a
native safety extension; browser P/Escape does not pause its finished phase.
The two-second results delay stays session-owned, so menu/start synchronously
cancel it; there is no delayed callback from the previous race.

This completes the bounded solo finish slice, not whole browser race parity.
Ranked rivals, standings, coin race/economy, browser collision responses, full
city art, sound and complete human-driven race qualification remain open.

## Fresh headless/build evidence

Full native MSVC and WebAssembly Emscripten/raylib applications build in
`build/finish-native` and `build/finish-web`, with visual fixture disabled by
default. Each passes **10/10** headless suites. The new test checks 270 actual
browser autopilot cases across nine routes and six actual solo result summaries
(including equal-time first-best ties), worst absolute error 6.22e-15.
It drives the actual session/car/gate code through both laps on all nine routes,
continues autopilot through results, checks exactly one results event and frozen
lap/finish data, pauses/resumes results, restarts cleanly, and cancels a pending
result delay on start/menu. Gate placements in tests are synthetic inputs rather
than proof a player can fly each complete route. Existing flight/race/recovery,
picker and city tests were freshly rerun with the new core. Final later app-only
HUD/capture refinements were rebuilt on both platforms; core/tests did not change.

Known upstream Web ES3 override and stb_vorbis comparison warnings remain;
linking and tests passed. No WASM graphics run occurred. One incremental native
build attempt lacked the Visual Studio developer environment (nmake unavailable);
it was rerun successfully with VsDevCmd. This was a build setup failure, not a
test failure. Artifact hashes and original ten-test receipts are in
`reference/verification-20261102/finish-builds.json`.

## Scoped native visual evidence

Two dedicated builds with `RR_FINISH_VALIDATION=ON` exercise real session/gate
crossings via scripted car placements. This option defaults OFF, is isolated in
separate build directories, and identifies itself in title, logs and watermark.
It uses only the normally selectable road 1 and writes no progress. It is not a
production/demo unlock or proof of a physically flown race.

First fixture: 18:22:01Z–18:23:06Z, `build/finish-visual-fixture`.
Observed menu/picker/countdown, first solo results, Enter restart with cleared
clock/countdown, a second result and M return to menu. Log shows GO 18:22:17,
finish 18:22:18, results 18:22:20; restart 18:22:39, GO 18:22:43,
finish 18:22:44, results 18:22:46. Clean shutdown and absent window 9704202
were verified. The initial screenshot read the preceding FINISH frame;
it is preserved accurately as `finish-first-fixture-finish-frame.png`, along
with the exact first executable, log and hash receipt under `reference/graphics-20261102`.

Final fixture: 18:24:13Z–18:25:05Z, `build/finish-visual-final`.
Observed locked picker, countdown, solo result overlay over moving world,
highlighted best lap, M return and clean shutdown. Log: GO 18:24:31,
finish 18:24:32, results 18:24:34. Window 6034174 was absent after close;
no other Roadsters instance remained. The screenshot was delayed three results
frames, then inspected from disk and confirmed to show the result panel.

Development QA screenshot, ready to reuse in the hourly report:
`reference/graphics-20261102/finish-final-results-fixture.png`.
It is visibly marked SCRIPTED GATES / NO PROGRESS. Its 0.600-second total is an
artificial fixture duration, not a genuine race time. Do not present it as a
human-driven completion. Original final log: `finish-final-182413-original.log`.

Both checks used normal startup, no smoke/fallback flags, 960x720 window,
800x600 world, 30 FPS target, MSAA/glow off and existing bounded renderer.
No new physical stop/input-protection denial occurred. Final heartbeats include
145.8–160ms around startup/loading and one 63ms results interval; this is not a
sustained performance or freeze-resolution qualification. No full-route manual
flight, physical mouse/gamepad acceptance or continuous collision proof follows.

Prior 8/8 picker/control and 9/9 city evidence remains separate in earlier reports;
previous interrupted tests remain interrupted, not relabeled passed.

## Account boundary and next work

Road locks remain unchanged: only road 1 is selectable in the offline app.
No identity/profile/persistence import, progress grant, storage schema, reward,
backend protocol, credentials/security, publishing or deployment changes occurred.
No account product decision was needed for this independent finish work.
Before progress integration, resolve the same-UID native sign-in/token/App Check
adapter, account-switch/profile refresh behavior and account/device scope of
versioned atomic local settings/bests. Offline prototype laps must not become
server-authorized wins. These decisions remain open as documented in the city
report. Next independent gaps are browser collision/responses and visual city/
course parity; full normal race acceptance needs a physically flown test.

## Exact changes

Modified: `src/core/rr_session.c`, `src/core/rr_session.h`, `src/app/main.c`,
`CMakeLists.txt`, `tools/race-rules-check/CMakeLists.txt`, `README.md`,
`PROJECT_DIRECTION.md`, `PLAYING.txt`, `docs/PARITY.md`.
Added: `src/core/rr_finish.c`, `src/core/rr_finish.h`, `tools/capture-finish.mjs`,
`tests/test_finish.c`, `tests/generated_finish_reference.h`, `reference/finish.json`,
this report, ten-test receipts, artifact/hash receipts and preserved graphical
logs/screenshots/first fixture executable. Code originals are in
`reference/pre-finish-20261102/`; status-doc originals are in
`reference/pre-finish-docs-20261102/`. Prior binaries and evidence are preserved.
All project writes stayed win_app; Claude-owned areas and Doorways were untouched.
No current action is denied, pending or uncertain; no current blocker remains.
