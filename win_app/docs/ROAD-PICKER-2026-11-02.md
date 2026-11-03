# Connected road picker and discrete-key reliability

Current source/build status, 2026-11-02. Previous manifests, builds and test/log
evidence remain preserved. No complete browser-parity row or milestone is claimed.

## Selection/progress behavior

The native 2D menu now opens a nine-row road picker for trial/time attack. Rows
show the actual browser names, length, gate count, two-lap descriptor, difficulty
bars, current/focused road and the preceding-road first-place requirement.
Arrow/Tab navigation and Enter confirmation are connected. Locked choices remain
in the picker, show refusal feedback and do not initialize a new road/session.

New `src/core/rr_road_picker.{h,c}` handles the browser open-count rule:
clamp the maximum of 1, supplied local open count and supplied profile roadsWon+1
to the nine roads. Integer road IDs preserve the browser's effective fractional
threshold. The current unlocked road is focused; otherwise focus starts at road 1.
Successful selection initializes that road and preserves race/time-attack mode.
Restart reinitializes the selected road/session and resets clocks/flight state.

The current app has **no native persistence/profile adapter**. Its clean offline
inputs are local-open=1, profile-won=0, so only Skyline is selectable. It does not
read/import browser storage or write player data. Trial finishes grant no unlocks,
currency, ranks or progress. The UI explicitly says offline trials do not unlock
roads. A pure unlock-eligibility helper matches browser decisions but is never
called by the offline app to grant progression.

Per-road lap bests in this slice are **session-local memory**, labeled SESSION BEST.
They survive trial restart during the same application run and disappear on exit;
they are not represented as persisted browser/account records.

The picker controller accepts read-only progress inputs for future integration.
Tests inject legitimate progress values to exercise every road; no such test
values are passed into the production UI or saved. Identity/App Check/profile and
atomic local persistence work remain separate integration gates.

`rr_minimap.h` reproduces normalized browser `HUD.setTrack` projection from actual
route bounds (center, max span*1.22, scale .82). The app no longer assumes fixed
Skyline map bounds. Dynamic road names/counts and deferred scene replacement are
connected, while the current clean offline UI still refuses roads 2-9. The native
temporary city's all-route clearance/fidelity remains unfinished.

## Diagnosed input issue and change

The pinned raylib 6.2 source implements `IsKeyPressed` from previous/current
frame-boundary key states. GLFW also records presses in a separate queue. A
press/release pair within a frame can leave current state up and disappear from
the state comparison while remaining in the queue. At the prototype's 30 FPS,
that is a concrete missed-discrete-tap case.

The app now drains `GetKeyPressed` once per frame into `rr_key_events.h`,
deduplicates commands and combines queued presses with the normal state-edge
fallback. Enter/navigation/pause/reset/camera/spin/fullscreen commands use this
path. Held flight inputs still use held state. Focus/release startup guards and
all graphics safeguards remain. Only app command-edge flags are logged; general
text/global input is not recorded. No third-party or system code was changed.

New graphical observations confirm queued commands reach their intended paths.
This addresses the identified code defect; it does not retroactively prove that
every earlier input problem had the same cause. Mouse UI activation remains
state-based, and rapid mouse clicks/physical controller behavior are not fully
qualified. Earlier input-protection rejections and the physical Escape stop
remain accurately recorded, not relabeled as passes.

## Fresh headless/build evidence

`tools/capture-road-picker.mjs` evaluates the actual browser `roadsOpen`,
`openRoads` row callback, `pickRoad`, `unlockNext` and `HUD.setTrack`. DOM and
localStorage are new in-memory adapters per fixture; no user/browser storage is
opened or written. Five current source inputs are hashed before/after capture
and rechecked after compilation; `reference/road-picker.json` records identity.

New coverage:

- 20 actual browser decisions: local/profile maxima and clamps, invalid local
  numeric values, current/first focus, locked refusal, selected mode/road and
  unlock eligibility without granting it.
- 36 actual browser minimap projection cases across all nine roads.
- Selection/initialization/restart across all nine C roads, locked/invalid
  selection preserving current track, focus wrapping, session-local lap bests
  and no progression mutation.
- Existing menu-policy test extended with queued quick-tap, deduplication,
  per-frame clearing and out-of-range key cases.

Both full application targets compile and pass **8/8 CTest tests**:

- `build/road-picker-native/bin/RoadstersRace.exe`: MSVC C17, app/core/new tests
  with warnings as errors. SHA256:
  `0aacff678eab1c9d3582adb37597715456d785980f4e7780c1b8a8186118688a`.
- `build/road-picker-web/bin/RoadstersRace.html` plus JS/WASM/data: pinned
  Emscripten/raylib. Existing upstream ES3 configuration/stb_vorbis warnings
  remain; compile/link/tests pass. Web graphics was not launched.

The other seven suites were freshly rerun against their existing fixtures,
including all nine road geometries/two-lap rules. Earlier five/six/seven-test
receipts remain separate. Latest receipts are preserved at
`reference/verification-20261102/native-eight-tests.log` and
`wasm-eight-tests.log`, in addition to each build's CTest log.

## Fresh native functional observation

One uniquely identified instance of the native executable above was launched
normally, without smoke/fallback/glow flags. The session ran **17:50:01Z to
17:52:42Z**, window 960x720, 30 FPS target, MSAA/glow off; world target was 800x600.
No overlapping native instance or browser graphics test was launched.

Observed visually and reconciled against the released native log:

| Action | Result / evidence |
|---|---|
| Enter on menu | Queued Enter logged at 17:50:10Z; nine-road picker appeared |
| Down, Enter on Violet Run | Focus moved to locked row; log refused road 2 with open=1 at 17:50:32Z; picker stayed open |
| Up, Enter on Skyline | Selected road 1, deferred scene loaded, countdown 4 visible; GO logged at 17:50:50Z |
| P | Pause UI appeared; queued press logged at 17:50:59Z; timer held at 00:09.33 across observations |
| P again | Resumed flight at 17:51:19Z; timer advanced |
| R | Queued reset logged at 17:51:36Z; movement/speed visibly changed from stopped to the safe-reset/coast behavior |
| Close while captured | Initial title-bar close did not complete; active flight remained. This attempt is not counted as shutdown |
| P, then close | Paused/released mouse capture, close succeeded; inventory contained no Roadsters window and log ended cleanly at 17:52:42Z |

The original Roadster, Skyline scene, improved bounds-based minimap, picker rows,
focus outlines and locked hints were visible. This is functional evidence for
these paths, not a complete art/reference comparison or input-device certification.
Reset's exact transform/boost/timing values are covered by headless fixtures;
the visual observation is supporting evidence, not a numerical measurement.

The closed original log is copied at
`reference/graphics-20261102/road-picker-functional-175001-original.log`.
SHA256: `0d8e206b6a146310255cc73c1a98148bd7dd2d8a4e3528c51a881b14a38b17c7`.
It records menu/lock/selection/GO/queued command states and clean shutdown.
No input-protection rejection or new physical stop occurred in this session.

Frame-duration heartbeats include pacing/presentation, not isolated GPU time.
No sustained performance target, complete two-lap/results visual run, automatic
signal-loss visual check, other-road world review, mouse/gamepad qualification,
Web/mobile graphics or proof that the former PC freeze cannot recur is claimed.

## Changed paths and next work

Modified: `src/app/main.c`, `CMakeLists.txt`,
`tools/race-rules-check/CMakeLists.txt`, `tests/test_menu_policy.c`, plus current
status docs `README.md`, `PROJECT_DIRECTION.md`, `PLAYING.txt`, `docs/PARITY.md`.

Added: `src/core/rr_road_picker.c`, `src/core/rr_road_picker.h`,
`src/core/rr_key_events.h`, `src/core/rr_minimap.h`,
`tools/capture-road-picker.mjs`, `tests/test_road_picker.c`,
`tests/generated_road_picker_reference.h`, `reference/road-picker.json`, this
report and distinct verification/graphics receipts.

Pre-edit code/build/test files and hashes are in
`reference/pre-road-picker-20261102/`; prior status docs are in
`reference/pre-road-picker-docs-20261102/`. Previous binaries, package, launcher,
evidence and Claude-owned files remain unchanged. No deployment, credentials,
security, backend or Doorways changes occurred.

Next independent gap: native city clearance around all nine routes before other
roads can be exposed safely/fairly, then collision/presentation and post-finish
autopilot. Production progression additionally needs the planned native
identity/profile and local persistence adapters; preserve actual shared UID/data
and unlock rules rather than turning solo prototype finishes into wins. Full
race/results visual acceptance and physical input checks remain open.
