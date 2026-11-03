# Offline coin-field gameplay parity - 2026-11-03

Shared C now implements the source coin field used by rival AI: placement,
ahead-selection, local pickup ordering, respawn/pop timers and reset/visibility
state. This is a bounded W3 gameplay slice from the existing ROADMAP/PARITY plan.
**The application still activates no coin field or live rivals.** No account,
wallet, progression, ticket, unlock, backend or persistent-data behavior changed.

## Source behavior preserved

Reference is browser working-tree package 0.2.3, identified by the content hashes
in reference/coin-reference.json, not a deployed version/commit claim. The adapter
executes the actual Coins constructor's CPU placement prefix and actual ahead,
reset, setVisible and update methods. Three Group/Track/Vector math remains actual
source code. Texture creation/loading is excluded; matrix/halo calls are render-only
sinks. Browser files are read-only, with source hashes rechecked at completion.

New src/core/rr_coins.{h,c} preserves:

- Six coins in each sorted checkpoint-sample gap plus the final gap, source sample
  wrapping/rounding, lane offset, alternating lateral placement and cosine height.
  The list-length-based lane expression is retained exactly, including its current
  six-per-gap pattern; it is not redesigned into different lane coverage.
- Nearest active target ahead within default caller distance90, along>=15 and
  along^2>=distance^2*0.85. Equal-distance matches choose the later list entry.
  The helper takes RRCar state and returns a coin index for the existing AI input.
- Strict pickup radius6.5. First eligible racer in supplied order collects; dead
  and finished racers skip. Each coin emits at most one collection event/update.
  Only the supplied local RRLocalRacer.coins counter increments. Source onCoin
  audio/impact/HUD behavior is left to future event consumers, with no account call.
- Inactive timer decay and reactivation at<=0, with no same-update collection after
  reactivation; pop resets then advances by dt*3 capped at1. Timers continue when
  collection is inactive, as in the source menu/finished-state caller policy.
- Visibility is separate from collection eligibility. Reset reactivates all coins,
  clears timers/pop, preserves placement/hidden state, and does not reset a racer's
  counters. Pause/phase policy remains with the caller, matching source main.js.

## Fresh headless evidence

| Check | Actual-source coverage |
|---|---:|
| Road layouts | 9 |
| Placement/sample pairs across those layouts | 756 |
| Largest selected-road field | 120 coins |
| Ahead-selection cases | 324 plus four tie/distance variants |
| Independent pickup/timer/visibility updates | 108 |
| Multi-update replay frames | 1,440 |
| Reset/hidden-state cases | 9 |
| Ordered collection events compared | 81 |

Checks include dead/finished and competing collectors, strict-radius exclusion,
timer expiry, delayed recollection, inactive collection, equal target distances,
pop progression and state reset. Actual-source placement errors are compared at
1e-8 tolerance; maximum observed numerical difference is **5.9117e-12**.
Both targets produced identical summaries. Full native and WASM application builds
compile in isolated offline-coins output folders; final headless suites each pass
**28/28**. Existing AI/lifecycle/race/world/material suites remain passed; culling
suites ran only as unchanged regressions, with no speculative renderer work.

The initial native compile rejected an event-aggregate field-order mistake in the
fixture generator (C2078); initial WASM compilation warned and its coin-event
comparison failed. The generator, not gameplay code, was corrected. The original
fixture/tool and 27-pass/1-fail initial WASM log are preserved separately; final
28/28 results use the corrected fixtures. No failed result was relabeled passed.

## Bounds and implementation status

One field is selected per road, not all756 coins resident at once. The fixed pool
supports 6*(32+1)=198 coins, aligned with the existing track checkpoint capacity;
actual source roads need at most120. Pool size is **9,512 bytes**;
the bounded collection event buffer is **1,588 bytes**. There
is no per-coin heap allocation, new world collider, asset, texture, GPU upload or
draw. Active local racer input is bounded to eleven. These are CPU/data bounds,
not measured native game frame-time or graphics acceptance.

The module is compiled into shared core for native/WASM, ready for an offline pack
orchestrator. No app/presentation code calls it yet. Existing runtime source files,
assets, copied native meshes and Web app preload were rechecked unchanged, and all
new build outputs are isolated from the potentially running Roadsters executable.
All project edits are within win_app; Claude-owned files, locks, profile/progress
and prior evidence/Library receipt/finish screenshot remain preserved.

## Desktop and graphics gate unchanged

No desktop/control call, process inspection, timed-out/denied retry, active-log
read, termination or graphics launch occurred in this slice. The last supported
inventory showed window3148188 absent, but did not establish process shutdown.
Task Manager app approval previously timed out. Owner confirmation/process check
is still pending; absence of a window is not evidence that RoadstersRace.exe has
stopped. reference/desktop-reconciliation-20261102.json is preserved, not replaced
with a fabricated fresh observation. No native material/visual/GPU/performance
result or human-driven race acceptance is claimed.

Exact next graphical prerequisite: obtain the pending owner process answer or
fresh supported approval, verify the intended old process is stopped, then resume
bounded source Canvas capture/native GPU checks without duplicate instances.

## Remaining useful gameplay work

Independent work is not exhausted: source start-grid/reaction/frame ordering can
now connect existing AI, lifecycle, contacts and coin modules in a headless local
pack orchestrator before rendering. Weapon targeting/heat/charges/projectiles,
damage/wreck reward semantics, pack ranking/results and audio still remain. No
account or progression decision is needed for that offline numerical work.
Source-authorized backend integration is a separate gate. City status remains413
canonical meshes and **1,947** unrepresented grid towers; poster pixel/native
material work remains pending graphics, with historical evidence preserved.

## Deliverables

Core: src/core/rr_coins.h and rr_coins.c. Fixtures/tests: tests/test_coins.c and
generated_coin_reference.h. Read-only source adapter:
tools/capture-coins.mjs WIN_APP_ROOT OUTPUT.json; regenerate staging C fixtures
with tools/prepare-coins-reference.py STAGING_DIRECTORY.

Full source output: reference/coin-reference.json. Final summaries:
reference/offline-coins-results.json. Hashes/complete final logs:
reference/verification-20261103/offline-coins-builds.json and the two twentyeight
logs. The initial rejected WASM test log and source backups in reference/pre-offline-
coins* are retained, along with all earlier reports and evidence.
