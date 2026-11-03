# Browser-timed start cues, 2026-11-03

## Concrete correction

Windows previously displayed a continuous4/3/2/1 label through countdown and cleared it when GO fired. The current local0.2.8 browser initializes lastCount=4 but only publishes messages when that count changes: the initial interval is blank, then3/2/1/GO! each receive a0.9-second message, leaving the source-defined gaps between them.

The native host now synchronizes shared RRStartCue state from the existing session's last_count, ticks its timer before painting and renders GO! during active play. Session timing, race physics, input activation, rewards and progression are unchanged. Source HUD timers continue during pause, so the cue timer does too. A countdown restart resets the count to4 while retaining any start cue already visible, matching the source reset of lastCount without clearing its HUD text. Resetting a fresh test/initial state remains explicit. Legacy checkpoint big text is suppressed while a start cue occupies that main-message slot, avoiding double-painted large messages; checkpoint submessage parity remains open.

Native warning overlays still take precedence. This isolated start-cue state is not a complete shared global HUD-message engine: mixed warning/restart leftovers, finish/lap announcements, sound, touch calibration and source fade transitions are not claimed covered. Minimized-window early-return safety remains unchanged and is not modeled as browser hidden-tab timing.

## Fresh source oracle and tests

`tools/capture-start-cues.mjs` executes the actual countdown branch from current main.js and the actual two timer-update statements from HUD.update, with inert sound/input/touch/render bindings. Main SHA256 is `970b45c22f8f56d8ca5f9e8a68016248700c1092947af5d776cb3472d96ce36a`; HUD SHA256 is `d0f8e0621b7601ad00f5c5c15f32ccbe210ad9fb573d9f6d1debf6f1f111b7bc`. Both sources are checked unchanged during capture.

The final fixture contains1680 frames in four sequences: steady dt, zero/stall/capped dt, pause/resume and mid-countdown restart preserving its visible cue. The C test drives the actual RRSession state machine and independently carries RRStartCue state across those frames. It compares session phase, current count, message visibility/text and remaining timer with a1e-12 tolerance. No C state is reset to each frame's expected output. Native car physics runs headlessly as part of RRSession; renderer/input/audio/network/accounts do not.

Initial native1260-frame validation passed; the initial WASM build stopped on a test-only signed-enum comparison under warnings-as-errors before running tests. The comparison was corrected and the restart case added before final validation. Initial logs are preserved as `start-cue-pre-restart-{native,web}-build.log`. Fresh final native and WASM application builds passed. Both targets passed6/6 focused countdown/missed-gate/race/session/UI/pause tests, including1680 frames with exactly zero timer difference in the measured fixture. Five preceding missed-gate implementation/fixture/provenance files were verified unchanged. Results are recorded in `start-cue-{native,web}-build.log` and the implementation receipt. Earlier full42 and focused results remain separate; the full47-test suite was not rerun.

## Exact paths and preservation

Added: `src/presentation/rr_start_cue.h`, `tests/test_start_cue.c`, `tests/start_cue_fixture.h`, `tools/capture-start-cues.mjs`, `reference/ui-parity-20261103/start-cue-oracle.json`. Modified: `src/app/main.c`, `CMakeLists.txt`. Originals: `reference/pre-start-cue-20261103`; isolated outputs: `build/start-cue-native` and `build/start-cue-web`. Prior warning, minimap, lap-history and camera/input implementation and evidence remain preserved. No art import/generation, resource cap, graphics/framerate/focus safeguard, account/progression/unlock or browser-owned source changed.

## Next typography gap and graphics limit

Current checkpoint and best-lap submessages contain U+00B7 (middle dot) and U+2605 (black star). Font cmap inspection found neither character in the supplied Orbitron500/700/900 fonts; Rajdhani Medium contains the dot but not the star. The current native atlases are ASCII-only. Exact message typography therefore needs a defined browser-equivalent fallback glyph/asset strategy before importing those strings blindly. This turn adds only source ASCII start cues and changes no font atlases. Machine-readable requirements and exact font hashes are in reference/ui-parity-20261103/ui-fallback-glyph-requirements.json. Share these two codepoint requirements with the asset team; do not regenerate complete UI skins or treat fixture text as live artwork.

The rejected graphics launch remains pending separate explicit owner authorization; no retry, desktop interaction, process inspection or termination occurred. Native glyph appearance, practical screen placement, fades, input delivery and start/race comfort remain unverified. Initial pending graphics scope remains menu/track selection; the start sequence later needs a separately bounded world/HUD check.
