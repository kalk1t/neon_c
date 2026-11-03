# Porting milestones

No calendar promises or v0.3 deadline. Each milestone produces evidence and a reviewable artifact. Completing an early playable milestone is not full parity. All stages below are planned, not completed.

| Stage | Deliverable | Acceptance gate |
|---|---|---|
| W0: reference and dependencies | Source manifest, parity inventory, pinned raylib, native/web toolchain checks | Browser commit/content identity recorded; official dependency verifies; C window and minimal WebAssembly frame build/run without editing `game/` |
| W1: shared simulation and first flight | C math, Skyline Loop, car motion, input, camera, loading fallback | Native flyable course; headless traces compare thrust/brake/steer/climb/boost/spin with reference; first flight also runs as a browser artifact |
| W2: world and race | Procedural city/corridor, colliders, gates, countdown, reset, lap/result state | Complete valid two-lap race; missed/out-of-order gates do not count; nine roads validate; collision/respawn cases pass |
| W3: AI, combat, presentation | Ten rivals, all weapons, lock-on, wrecks, coins, cockpit, menus/HUD, effects/audio | Player/AI share physics; damage/cooldowns/rewards match fixtures; screenshots and audio cues reviewed; input/settings work on both targets |
| W4: accounts and economy | Real desktop sign-in, profile/nickname, tickets, wallet, shop, ranking | Existing UID/data preserved; server rejects invalid requests; real race saves once; same purchases/progress appear in browser; refresh/sign-out tested |
| W5: cross-play | Existing live protocol, shared room, interpolation, chat, combat, killer status | Old JavaScript browser + native C play together; browser C + native C play together; both hit directions, reconnect, account conflicts, chat limits, and protocol mismatch verified |
| W6: parity and distribution | Complete parity report, packaged Windows build, candidate C browser bundle | Every required parity row passes; installation/clean launch/save recovery/uninstall policy reviewed; browser regression and measured performance targets pass |
| W7: C-led transition | Claude integrates approved C browser artifact; shared release process | Browser replacement reviewed and rollback tested; one gameplay/content revision serves both builds; future features originate in shared C |

## Early integration work

Run the identity/App Check and transport feasibility spikes during W0/W1, alongside the first C build. W4 completes online integration; it must not be the first time desktop sign-in or enforcement is examined. Establish whether a backend change is needed while gameplay work can still proceed independently.

Build a tiny WebAssembly artifact during W0/W1 to expose graphics, loading, loop, and toolchain incompatibilities early. Do not postpone the first browser compilation until Windows parity.

## W0 execution order

1. Install and verify the pinned raylib archive, or obtain the pinned source and build it with the installed compiler. Retain licenses. Record any network blocker accurately.
2. Use the observed browser revision and file hashes captured in `reference/browser-baseline.json`; inspect its dirty status before selecting the implementation reference. Label its actual package version instead of silently calling 0.2.2 v0.2.1. A dirty tree is identified by hashes as well as its commit, never by commit alone.
3. Add CMake targets for a native window, shared C library, and headless tests. Pin actual compiler/library/toolchain choices in build documentation.
4. Validate one existing GLB, coordinate conventions, shader basics, resize/fullscreen, keyboard/mouse/gamepad input, and asset paths in a native spike.
5. Build a minimal C/raylib browser artifact inside `win_app/` with a matching frame loop and content. Hand off integration requirements to Claude.
6. Produce a desktop identity/App Check feasibility note with the exact account/configuration/backend dependencies; do not alter production to prove the spike.
7. Capture first flight and track fixtures before porting behavior, then start W1.

## Browser changes during catch-up

Use a stable captured reference per milestone. Browser development can continue; it does not silently change what a partly completed milestone must reproduce.

At a milestone boundary, read the browser commit and dirty status again. Compare hashes for the captured files, log gameplay/content/protocol/economy/UI changes, and classify each as already covered, new parity work, intentional future change, or a coordination blocker. For a dirty working tree, record file hashes and do not claim a commit fully identifies its content. Never discard Claude's changes to recreate the baseline.

Do not continually re-export assets or road data from a moving working tree. Copy/convert against a recorded revision and retain a manifest of inputs and conversion settings. Baseline capture is a source reference, not proof that it matches the currently deployed release.

## Evidence required for parity

- Headless numerical reports for car/track/collision/combat cases, with tolerances and dt sequences.
- Two-lap completion and checkpoint validation on every road, plus reset/wrong-way/floor behavior.
- Comparable menu/city/race/cockpit/combat screenshots; no new art direction during the port.
- Keyboard/mouse and gamepad checks on Windows; touch/tilt and mobile layout checks for the future browser replacement.
- Signed-in integration results for nickname ownership, profile refresh, purchases, ticket timing, retry, and ranking queries.
- Two distinct accounts crossing clients in the same room; test native-to-browser and browser-to-native shots, damage, wreck/respawn, kill confirmation, chat, and auras.
- Reports for missing assets, offline startup, token expiry, connection loss, incompatible protocol, device/graphics failure, and settings recovery.
- Frame-time measurements on identified hardware/browser settings. A successful build or a headless software-rendered test is not performance proof.

## Handoffs outside this folder

Keep proposed auth-broker/App Check changes, profile/protocol adjustments, browser shell integration, and deployment procedures in reviewable notes here. Claude implements approved work in its owned directories. No backend contract, release switch, or browser replacement is implied by creating a Windows milestone artifact.

