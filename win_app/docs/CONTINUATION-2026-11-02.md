# Windows continuation: 2026-11-02

Scope: read surrounding `neon_c`; add only isolated files under `win_app`.
Claude retains browser/backend/live ownership. No existing source, package,
baseline, launcher or art-task files were replaced by this continuation.

## Recovered architecture and current reference

Repository handoffs (`AGENTS.md`, `PROJECT_DIRECTION.md`, `docs/ARCHITECTURE.md`,
`ROADMAP.md`, `PARITY.md`, `RECOVERY-2026-11-02.md`, `RENDERING.md`, `VALIDATION.md`,
and `PLAYING.txt`) supplied continuity. Private CLI session history was not needed
or opened. C17/raylib, MSVC x64, and Emscripten remain the established architecture.
Core uses double precision and no rendering/platform dependencies. Windows is to
lead after parity; shared C, shared Firebase identity/economy and cross-play remain
requirements. No architecture replacement or gameplay redesign was made.

Read-only examination covered game source and documented behavior, package scripts,
track/car/AI/race logic, Windows core/app/render/build/test code, source manifests,
milestone/technical inventories, asset categories and the surrounding live/asset
layout. Binary art was inventoried rather than exhaustively visually reviewed.
No `.env`, credentials, production logs, or live services were read or changed.

Current `game/package.json` is 0.2.3. The initial saved baseline is 0.2.2 (user's
earlier stated release was 0.2.1). `reference/baseline-drift.json` records hashes:
six of 94 planning inputs differ (`docs/feature-inventory.md`, `index.html`,
`package.json`, `package-lock.json`, `src/ai.js`, `src/main.js`); only `src/main.js`
differs among the nine first-flight fixture inputs. Old baseline/fixtures remain.
Git status/log both returned Permission denied; no Git workaround was attempted.
Current source identity is file hashes, not a verified commit or deployed release.

Current source/changelog confirms 0.2.3 rivals use full player top speed times road
factor, 1.5x damage/firing rate, and Track Flyover was removed. The original parity
and architecture documents still mention tour: treat that as historical scope,
not a feature to reintroduce. A frozen release reference remains an owner decision.

## Browser versus Windows inventory

| Area | Current browser reference | Windows evidence / remaining gap |
|---|---|---|
| Flight | Thrust/brake/reverse, steer/climb, boost, spin; shared AI physics | Existing C flight passes 2,940 recorded steps; physical input and complete camera parity unreviewed |
| Roads | Nine roads; 8-19 ordered gates | Runtime only Skyline; recorded Skyline curve tests pass. New rules test real gate coordinates for all nine; C track generation for roads 2-9 remains absent |
| Race | Countdown, two laps, time attack, finish/results, warnings/reset | New pure C checkpoint/lap/finish foundation passes current browser fixtures; application still practice flight, with no race integration/countdown/results |
| Collision | Spatial OBB/cylinder/torus/world/car contacts | Not ported; practice gate helper has different swept/radius semantics and remains untouched |
| AI/combat/coins | Ten rivals, 0.2.3 tuning, three weapons, wrecks, coins | Not implemented in application; no claims from flight/race tests cover these systems |
| Art/cameras/UI/audio | Full city/scenery/atmosphere, three cars, cockpit, HUD, synthesized sound | Roadster conversion/fallback, temporary city/chase/HUD exist; broad art/UI/camera/audio parity open |
| Rendering/lifecycle | Browser tiers and recovery behavior | Revised bounded renderer compiles historically; headless policy retested. Freeze cause and graphics stability remain unverified |
| Settings/persistence | Controls/quality/audio/local bests/equipped car | Persistent preferences and progress incomplete |
| Identity/economy/ranking | Firebase UID/profile, tickets, server purchases/rewards | Interfaces/plan only; native identity/App Check feasibility still a W0/W1 gate; no service calls made |
| Cross-play/social | Protocol 6 rooms, chat, PvP, auras, reconnect | Not implemented; protocol/backend changes require coordinated handoff |
| WebAssembly | Future shared-C replacement must match desktop/mobile | Existing graphics target remains unrun; new pure-C tests are separately built/run under Node when verified below |
| Distribution | Browser release pipeline owned by Claude | Native development package exists; no installer/signing/clean-launch/performance acceptance |

W0/W1 acceptance is still incomplete. W2 has one new tested rules slice; no entire
milestone or parity row is certified complete. W3-W7 remain pending.

## Implemented slice

`src/core/rr_race.{h,c}` implements ordered checkpoints, browser endpoint aperture
tests (strict radius 15), +0.3 capped boost, last-safe transform (+12 forward),
lap/finish boundary tests, two-lap completion, endless time attack and free-roam
gate bypass. It emits events so a future caller can maintain history/results.
The caller owns countdown/pause, time/dt, warnings, respawn, rewards and UI.

The module intentionally does not call the existing `rr_gate_crossed`: that helper
intersects the movement segment, while browser `raceRules` checks the current
endpoint projected into the gate plane. Changing the browser behavior would be a
redesign. No existing practice-flight behavior has been silently replaced.

`tools/capture-race-rules.mjs` extracts the actual `completeLap` and `raceRules`
functions without importing browser main/application startup. A VM supplies only
vectors, track geometry and non-rendering state. Nonplayer rules avoid UI effects;
out-of-scope respawn throws. References use all nine actual browser tracks and
synthetic boundary cases. Input hashes are checked before/after capture. This
executes source rule behavior, not browser UI or deployed-build behavior.

32 scenarios / 1,262 steps cover missed/out-of-order/repeated/reverse gates,
plane/radius/start-boundary cases, endpoint-versus-segment differences, boost caps,
safe transforms, two laps, post-finish immutability of race progress, time attack
and roam. `tests/test_race.c` compares state and events with tolerance 1e-8.

## Reproduction

From `win_app`, run `powershell -NoProfile -ExecutionPolicy Bypass -File
scripts/Test-Headless.ps1`, or append `-Web` for WebAssembly/Node. These use a
separate CMake project with no raylib dependency or game target. They do not
launch graphics. To deliberately update only the new race fixtures, run
`node tools/capture-race-rules.mjs`; it does not replace the initial baseline.

## Verification and limits

- Native MSVC 19.51 C17: three headless tests passed. Race: 1,262 steps, worst
  absolute error 0. Existing flight/track: 2,940 steps, 81 curve samples, eight
  Skyline gates; worst absolute error 2.61e-12. Bounded renderer policy passed.
- Initial CMake Debug compiler probe failed with C1902 (PDB manager mismatch).
  Reconfiguring with embedded `/Z7` Debug information passed; the new script
  records this build-local option. No system/compiler installation was changed.
- PowerShell helper parser validation passed.
- WebAssembly result is recorded in `reference/continuation-validation.txt`.
- No graphics, sustained performance, native UI, audio, real input, identity,
  backend, multiplayer or release checks ran. Graphics restriction from
  `AGENTS.md` remains: new explicit owner authorization is required.

Process command-line inspection through CIM was denied. The permitted name-only
process query showed no matching Codex CLI/Claude/node writer, but this cannot
prove writer inactivity in this environment. Existing core files remained stable
during inspection. This continuation therefore adds new files only and leaves
existing CMake/application/package files untouched, avoiding concurrent edits.

## Next work

1. Coordinate writer ownership before changing existing app/build files. Add the
   new race module to the shared core and replace practice progression with an
   explicit race/time-attack state machine, countdown and results; headless-test
   lifecycle/time/reset behavior before any graphics authorization.
2. Port parameterized road data/sampling for roads 2-9 against pinned fixtures;
   the present all-road rule tests do not implement those C tracks.
3. Port warnings, last-safe respawn and collision behavior, then AI/combat with
   the current 0.2.3 tuning. Keep freeze investigation and identity/App Check
   feasibility visible as independent acceptance gates.
4. Resolve frozen browser reference and Windows/hardware/performance targets
   with the owner; leave backend/deployment integration with Claude.
