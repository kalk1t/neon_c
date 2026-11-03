# Browser missed-gate warning, 2026-11-03

## Missing notification restored

The current local browser package reports0.2.8. In race.js, the player crossing the start aperture with some ordered checkpoints passed but more required, after raceTime>5, receives MISSED GATE for1.4 seconds. Windows already refused that incomplete lap but displayed no corresponding warning. This slice restores the notification without changing accepted laps, checkpoint progress, car motion, economy, rewards or network contracts.

Shared `rr_race_missed_gate` checks source strict boundaries: previous.z>0, current.z<=0, abs(x)<23, y>109 and y<142, race clock>5, non-finished/non-roam state and ordered progress strictly between zero and checkpoint count. It is used only by the offline player host. The host records the previous car position before session stepping and evaluates the notification afterward, so same-frame checkpoint advancement is considered and an accepted lap's reset progress does not create a false warning.

The existing warning display receives the new type and1.4-second payload before recovery processing. Wrong-way gating sees that active warning, while the existing signal-lost event may override it later in the same rule order. No new message priority policy was invented. Other HUD differences remain: native checkpoint announcements are still large text instead of source subtext, and final-lap/GO/submessage integration and exact display-expiry frame timing need separate work. This is not whole-HUD message parity.

## Fresh source oracle and tests

`tools/capture-missed-gate.mjs` extracts and executes the actual current raceRules function with real Three vector math and an inert HUD recorder that updates msgTimer. Checkpoint movement/recovery is held neutral; completeLap and respawn are inert so no rewards/account path executes. The1248 cases cover race/time/roam modes, finished flags, zero/partial/full checkpoint progress, strict5-second boundary, aperture x/y edges and both sides/directions of z crossing. Forty-eight cases emit the source warning with duration1.4. Ordered passed.size is supplied consistently with nextCP; arbitrary corrupt progression is not inferred covered.

Generated expected booleans and durations are compared by `tests/test_race_warning.c`; source SHA256 is `b80df69a3a81becfe5f924c703033fe3e1c76b257c0eb534d8213395594cc140` and capture verifies the file did not change mid-run. This is actual source trigger/payload equivalence, not a replay of the native predicate. It does not qualify simultaneous full-world checkpoint/collision sequences or native visual expiry/input behavior.

Fresh native and WASM full application builds passed, with 7/7 focused missed-warning/race/session/recovery/finish/UI/pause tests on each target. Both matched1248 source cases and48 warnings with zero failures. Five prior minimap implementation/fixture/provenance files were verified unchanged. Actual source GATE/DECK_Y extraction confirmed23/109/142; strengthening that provenance left the tested fixture byte-identical. Results are recorded in `missed-gate-{native,web}-build.log` and `missed-gate-implementation.json`. Earlier full42-test and subsequent focused results remain separate; the complete46-test suite is not claimed freshly rerun.

## Paths and limits

Added: `src/core/rr_race_warning.h`, `tests/test_race_warning.c`, `tests/missed_gate_fixture.h`, `tools/capture-missed-gate.mjs`, `reference/ui-parity-20261103/missed-gate-oracle.json`. Modified: `src/app/main.c`, `CMakeLists.txt`. Originals saved in `reference/pre-missed-gate-20261103`; isolated builds are `build/missed-gate-native` and `build/missed-gate-web`. Prior inputs/camera/lap-history/minimap implementation and evidence are preserved.

No art, GPU resource, cache/upload limit, framerate/focus safety setting, progression/unlock/account service, browser-owned file or paid service changed. The rejected graphics launch remains pending separate explicit authorization; no retry, desktop interaction, process inspection or termination occurred. Warning appearance, final frame timing and real driving acceptance remain unverified. The initial pending graphics scope remains menu/track selection, followed only later by an appropriately bounded race/HUD trial.
