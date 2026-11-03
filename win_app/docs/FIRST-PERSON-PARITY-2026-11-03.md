# Three-mode camera parity slice, 2026-11-03

## Source boundary

The owner requested catching v0.2.5. During this turn the read-only browser working tree advanced to package 0.2.6 and split main.js into modules including camera.js, aim.js, race.js, render.js, world.js, roam.js and clock.js. The previous turn's statement that 36 indexed runtime files were unchanged described its earlier snapshot; it is not a current whole-source freshness claim. No Git or previously denied process query was retried.

This slice stays grounded in the supplied pre-split v1 source, `components/source/main.js` SHA256 `4dd0cc4594c6849c6b1031520ecfba5eda17fc3c3635f8eba059358aa1a61307`. Its chaseCamera function was compared with the new working camera.js function after removing comments and whitespace: identical. The oracle records the exact newer module hash. This checks that camera behavior survived the split; it does not establish whole-game equivalence of v0.2.5 and v0.2.6 or identify the deployed release.

## Implemented change

Windows previously toggled C between near and far chase. It now cycles near chase, far chase and raw first person, matching the browser's three CAMS entries. Raw first person sits at car position + forward*0.6 + world-Y*0.75, looks along the car's forward vector and applies source roll banking. It uses the source strict orbit <0.05 threshold; orbiting beyond that returns to the near chase geometry while retaining the selected mode. The shared body-hide path suppresses only the local car body, preserving existing thruster rendering and ordinary HUD.

The native application's current offline car has no production wreck/alive state; the adapter passes alive=true. The shared predicate nevertheless checks alive and its dead cases are tested for future qualified gameplay integration. Raw mode is applied in active/countdown/finished phases. Existing results/pause showcase differences, broader C-key phase handling, mouse capture filtering, camera shake/kick, lens effects and full chase lighting/geometry are outside this slice.

The opt-in cockpit artwork remains default OFF. A loaded opt-in dashboard retains precedence if requested; raw mode neither imports/loads dashboard art nor hides ordinary HUD. Existing account progression, locks and asset/resource caps are untouched. No art was generated, converted or imported.

## Verification and evidence

`tools/capture-first-person.mjs` executes the actual pinned chaseCamera function in a restricted headless VM with real Three PerspectiveCamera math. It verifies behavior equality against the new camera module and checks that module did not change during capture. No browser/game renderer, audio, network, storage or account is initialized.

The generated fixture contains 1080 visibility cases across all three modes, alive/dead states, yaw/pitch/roll, just-inside/outside orbit thresholds and wrapped orbit angles. 108 cases select raw first person; C compares nine components of their position, forward and up vectors with a 1e-10 tolerance. Shake, Nitro and camera-punch effects are disabled in the fixture, rather than claimed covered. C tests also verify three-step mode cycles.

Fresh isolated native and WASM application builds passed. The five focused camera/input/cockpit/render-policy tests passed 5/5 on each target; maximum raw-pose component error was 6.0973509486883858e-16 on both. Five preceding input implementation/fixture/provenance files were checksum-verified unchanged. Results are recorded in `reference/ui-parity-20261103/first-person-implementation.json` and `first-person-{native,web}-build.log`. Earlier 42/42 full regression results belong to the preceding input slice and are reused evidence, not re-run full-suite acceptance after this change. No graphics execution or hardware control delivery was tested.

Implementation paths: `src/core/rr_first_person.h`, `src/app/main.c`, `CMakeLists.txt`, `tests/test_first_person.c`, `tests/first_person_fixture.h`, `tools/capture-first-person.mjs`, `reference/ui-parity-20261103/first-person-oracle.json`. Originals of modified app/CMake files are in `reference/pre-first-person-20261103`; preceding candidates/assets remain unchanged. New build output is only `build/first-person-native` and `build/first-person-web`.

## Remaining gate

The previously rejected native graphics launch remains pending separate explicit owner authorization; no retry, desktop control, termination or new graphics instance occurred this turn. Visual correctness, practical first-person clipping/body occlusion, controller/camera input delivery and clean graphics shutdown still require the controlled native check. The initial approved scope should remain menu and road picker; exercising this first-person mode later needs a separately bounded world test, not automatic scope expansion.
