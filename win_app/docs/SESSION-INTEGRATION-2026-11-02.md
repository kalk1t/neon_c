# Session integration and controlled graphics evidence

This supersedes the implementation status in `CONTINUATION-2026-11-02.md`,
`README.md` and older validation records. No entire parity milestone is complete.

## Authorization and ownership

The owner confirmed the earlier Codex CLI was not running and authorized a
controlled game-window test, then explicitly authorized closing and reopening
Roadsters when needed for development tests. That permission applies to this
application and conservative tests; a later physical stop overrides it. It does
not authorize changes outside `win_app`, deployment, security changes, or denied
CIM/Git workarounds. Earlier physical Escape interruption was respected until new
permission arrived. No unrelated desktop app was controlled.

## Implemented

- `rr_race` is now in the shared core/application build, with its browser fixtures.
- New `src/core/rr_session.{h,c}` owns countdown, simulation/race clocks,
  pause/resume state, lap history, two-lap completion, results delay, time attack,
  safe respawn and checkpoint/finish targeting. Core remains independent of raylib.
- The app exposes an explicitly offline two-lap solo trial and endless time
  attack on Skyline. Countdown ignores player flight input and starts clocks only
  on the frame after GO. The final hoop targets the finish gantry safely.
- `rr_car_place` reproduces browser placement: clears motion-related state while
  preserving boost and effect history. Full new-session reset stays separate.
- Lap events are retained in dynamically grown history; allocation failure yields
  a recoverable 2D error screen. Restart clears the session while reusing capacity.
- Finish/results use a 2D offline screen. They award no coins, unlocks or progress.
  The browser's continuing post-finish autopilot/showcase remains a deviation.
- New `rr_menu_policy.h` requires a focused released frame before accepting a
  start input, consumes that arm, and resets on focus loss/minimize/return to menu.
  This mitigates immediate launch/click-through input; it is not proof of the
  first incident's cause or a barrier against deliberate later input.
- Native diagnostics now use UTC timestamps and record smoke/fallback/glow flags
  plus menu-action origin (only app menu mouse/Enter/gamepad activators).
- Deferred world loading, 30 FPS, bounded targets, MSAA/glow defaults, focus pause,
  and overload pause remain. Diagnostic smoke paths were not used in these tests.

## Builds and headless tests

Native app: `build/menu-diagnostics-native/bin/RoadstersRace.exe`.
SHA-256: `b3a8cb4eafe96f938be58bb47195f7fd86ec18f5f74cdaaa71df1393bcf9c81e`.

Full WebAssembly app: `build/session-integration-web/bin/RoadstersRace.html`
plus JS/WASM/data. It was compiled, not launched in a browser.

Both integrated top-level builds passed 5/5 CTest tests:

1. 2,940 recorded browser flight steps, 81 curve/arc/tangent samples and eight
   Skyline gates; worst absolute error 2.61e-12.
2. Headless bounded rendering policy.
3. 1,262 current browser race-rule steps across 32 scenarios/all nine roads;
   worst error 0. These fixtures do not implement C roads 2-9.
4. Session lifecycle, 30/60/144 Hz countdown/GO, long-frame cap, pause/resume,
   respawn, finish targeting, history growth, results delay/cancellation,
   restart and endless time attack.
5. Menu release/focus/consumption/simultaneous-input policy.

Native MSVC C17 builds use warnings as errors for application/core/new tests.
Web build used the existing pinned Emscripten/raylib configuration. It emitted
raylib's existing ES3 override warning and an upstream stb_vorbis pointer-comparison
warning; compilation/link/tests passed. Neither warning establishes GPU behavior.

CTest evidence: each build's `Testing/Temporary/LastTest.log`.
`scripts/Test-Headless.ps1` also discovers the five isolated checks via the updated
`tools/race-rules-check/CMakeLists.txt`; integrated builds above are the latest
executed five-test evidence.

## Interrupted first window test, then verified closure

First executable: `build/session-integration-native/bin/RoadstersRace.exe`, hash
`19f38280e1d2059fdeb8ac1f52c069ad81b8071d0c3385992c53a0a36cad01cb`.
Normal startup was requested without flags. The first screenshot already showed
a time-attack countdown, then flight. Agent actions had not selected a flight
button. User-input protection blocked close attempts; physical Escape stopped
Computer Use. This was reported as interrupted, not a menu-only pass.

After renewed permission, the exact window was closed with Alt-F4. Its absence
was verified in window inventory, its log became readable, and it ended with
`Window closed successfully` / `SESSION: clean shutdown`.

The original log is preserved byte-for-byte at
`reference/graphics-20261102/interrupted-flight-original.log`. It uses unzoned
Windows local timestamps (11:59:14-12:02:54); do not label them UTC. It confirms
2D startup, world load roughly three seconds later, time attack and GO. It records
NVIDIA GeForce RTX 5080 / OpenGL 3.3 NVIDIA 610.88 as reported by the context.
This is not a fresh hardware/CIM query or a driver-health diagnosis.

No input-origin logging existed then. The activating input/actor is unresolved.
The observed countdown differs from the smoke path, which skips countdown.
The log also shows focus pauses/resumes and resize events outside the intended
isolated test. Recorded long frames therefore are not a controlled benchmark.

## Completed normal menu-only retry

New diagnostic build, normal startup, no flags. From **17:06:01Z to 17:06:27Z**,
screenshots showed the 960x720 2D menu. A blank-area click was used, no flight
button or Enter. Five heartbeats reported mode 0 and target **0x0**; there was
no scene-load request or menu-start action. Window closure was verified in
inventory and log. Log ended cleanly at 17:06:27Z.

The log at completion had SHA-256
`31ba339d5324a6d078a493fa53eb2081f16ecb39d9017abfbc39007733164fe4`.
The executable appends later sessions to the same file, so that hash identifies
the completed first-session log prefix, not necessarily the later whole file.

This establishes a short normal menu/OpenGL path and clean shutdown on this
machine. It does not establish flight stability, sustained performance, absence
of all freezes, hardware minimums, or WebAssembly graphics behavior.

## Subsequent short normal trial: partial / interrupted

A fresh normal launch was uniquely identified. A menu mouse attempt did not
produce a confirmed transition; Enter then entered the offline two-lap trial.
Screenshots confirmed active flight, original vehicle and checkpoint HUD progress.
No smoke/fallback/glow/fullscreen flag or action was used by the agent.

Concurrent user input was detected and blocked the agent's pause, then close
request. After refreshed observation, one close retry was also blocked with
`user input was detected in this window; call get_window_state before continuing`.
Desktop actions stopped at that point. Closure of this third window is **not
verified**. It was window **4133260**, executable path as above. Its append log
must be reconciled after closure; no locked-file access workaround was attempted.
Checkpoint observations with concurrent input are not an isolated controlled
flight or complete-race acceptance test. No complete race/results visual check
has passed.

## Changed files and preservation

Modified existing files:

- `CMakeLists.txt`
- `src/app/main.c`
- `src/core/rr_flight.c`
- `src/core/rr_flight.h`
- `tools/race-rules-check/CMakeLists.txt`

New integration files:

- `src/core/rr_session.c`, `src/core/rr_session.h`, `tests/test_session.c`
- `src/core/rr_menu_policy.h`, `tests/test_menu_policy.c`
- this report and preserved graphics log

Existing-file originals and hashes are under
`reference/pre-session-integration-20261102/` and
`reference/pre-menu-diagnostics-20261102/`. Earlier isolated race additions remain.
Existing `dist/first-flight` and `Run-RoadstersRace.cmd` were not repackaged or
changed; that launcher still points to the older development package. Use the
identified build path for further authorized testing.

No browser, shared root, backend, deployment, account, economy, other art task,
or system/security settings changed. Git/CIM denials remain respected.

## Next work

Reconcile/close the remaining test window when input protection permits;
preserve its released log and any app input origin evidence. Then verify pause,
safe reset and complete two-lap/results behavior without concurrent interaction.
Port parameterized road generation for roads 2-9, warnings/collision/respawn
semantics and AI/combat in subsequent slices. Identity/App Check and frozen
browser reference decisions remain open. No full milestone is complete.
