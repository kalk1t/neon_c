# v0.2.5 input parity correction, 2026-11-03

## Reference and implemented behavior

Read-only browser `game/package.json` reports 0.2.5. All 36 runtime source files in the prepared Windows asset index remain byte-identical to its snapshot. Package v1-v6 index plus v7 addendum supplies the existing art; this input change needs no new art or asset import. This identifies the current local source, not the deployed website or a Git revision.

Browser `src/input.js` SHA-256: `835bff688cd8afa1f4d958ad003dec0429cade1f8f2d0c1fdff2ee3ca088c753`.

The Windows input adapter previously grouped duplicate directional bindings with logical OR. Browser Input.state adds every held direction and clamps after combining. Holding D + Right + A therefore gives rightward steer 1 in the browser but previously gave 0 on Windows. Up + E + Q has the analogous climb difference. Opposing and duplicate inputs now pass through shared C `rr_input_mix`, matching browser summation, clamp, keyboard boost, controller deadzone, controller axis precedence, maximum trigger blending and controller boost. Existing mouse aim/orbit handling and flight simulation follow the mixer as before.

The native adapter previously checked gamepad slot 0 only. It now chooses the first available among raylib's four supported slots for continuous control and existing one-shot/pad-held UI actions. This aligns the source's first-non-null selection within the native backend's four-slot limit. Actual controller connection, mapping, hotplug, edge delivery and disconnect behavior remain untested on hardware. Combat/fire features remain inactive and were not enabled by adding this input mixer.

## Verification

`tools/capture-input-state.mjs` imports the actual browser Input class with inert event registration and a supplied gamepad snapshot. It executes Input.state directly without rendering, WebGL, network, storage or accounts. It covers all 4096 combinations of the twelve relevant digital keys twice: once without a controller, once with one of seven controller profiles. The seven profiles rotate across masks rather than forming a complete Cartesian product. The 8192 generated expected states cover exact/below 0.15 deadzone values, signed axes, analog trigger blending and boost. A leading null controller slot is included in each connected browser snapshot. The native slot-selection adapter itself is compiled, not hardware-tested by that browser fixture.

The committed fixture is `tests/input_state_fixture.h`, verified by `tests/test_input_mix.c` on both targets. Source provenance and profile definitions are in `reference/ui-parity-20261103/input-state-oracle.json`. Output builds are isolated at `build/input-mix-v025-native` and `build/input-mix-v025-web`, default cockpit OFF. Existing candidate executables and assets were not overwritten. Originals of modified app/CMake files were saved in `reference/pre-input-mix-20261103` before editing.

Fresh native and WASM application builds succeeded, and full headless suites each passed 42/42 (8192 source-generated state cases in the new suite). No test failed. Native suite duration was 107.47 seconds; WASM was 75.00 seconds. Final build/test results are recorded in the accompanying receipt and `reference/ui-parity-20261103/input-mix-v025-{native,web}-build.log`. No native game or WASM graphics window was launched for this work.

## Exact implementation paths

- `src/core/rr_input_mix.h`: shared pure key/controller state mixer.
- `src/app/main.c`: native keyboard/pad snapshot adapter and first-connected slot selection.
- `CMakeLists.txt`: headless source-equivalence test registration.
- `tests/test_input_mix.c` and `tests/input_state_fixture.h`: C fixture comparison and generated browser states.
- `tools/capture-input-state.mjs`: reproducible current-source oracle.
- `reference/ui-parity-20261103/input-state-oracle.json`: source hash and test scope.

## Remaining acceptance gates

The graphics launch rejection remains in force; no retry occurred. Process absence was already confirmed by the owner and is distinct from the pending explicit graphics execution authorization. The next visual check remains one bounded menu/track-picker instance followed by a verified exit. Do not treat this input correction or headless results as appearance, hardware input or whole-game acceptance.

Highest-impact remaining gaps are native menu/HUD/cockpit/garage appearance qualification; full faithful city residency under current caps (1947 missing canonical grid towers and 97 poster planes remain outside qualified production coverage); native shader/material/lighting and effect/audio integration; production player/pack/combat/coins coupling and human race acceptance; and agreed shared authentication/progression/cross-play service integration. Prepared assets and offline deterministic modules reduce implementation work but do not complete those gates. Browser mouse capture-event filtering versus raylib frame-aggregated motion also remains a separate input-parity difference; it was not approximated in this patch.
