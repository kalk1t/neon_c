# Collision-driven camera shake

The native client already resolved wall impacts but did not feed them to the chase camera's shake. RRSession now exposes the collision report from its one existing physics step, alongside a motion_stepped flag. Each rr_session_step call clears this report first, including paused/zero-time/menu calls; start resets it with the session. No extra physics step was added.

The host consumes that report only in the previously active/finished/results playing branch, after Nitro ignition and before camera update. The source rule is strict impact>4, then shake=min(1.2,shake+impact/25), followed by existing camera decay. Countdown contacts do not trigger playing-branch camera shake. Collision report values survive any later recovery placement without consulting a stale car.impact field. Paused/zero-time frames cannot replay an old impact.

## Fresh evidence

Native and WASM full application builds each passed15/15 focused checks: new collision-shake test, preceding Nitro camera fixture, collision primitives/flight regression, and the12 previous UI/input/session/result checks. The new fixture executes the current browser's actual Nitro-then-impact branch and camera shake decay across336 boundary/order cases. A native physics/session seam fixture checks one-step collision output equality, normal and movement, active/countdown/finished/results reporting, paused/zero-time/menu clearing and restart. It does not replace the earlier source collision trajectories. The full54 declared tests were not rerun.

The browser package remains local v0.2.9. main.js changed since earlier camera fixtures, so this slice records its current exact hash and parses the selected current branches; camera.js retains its prior hash. Current source identities and scope: reference/ui-parity-20261103/impact-fx-oracle.json. Changed-file hashes, fresh build logs and client/447 matching asset-copy identities: impact-fx-implementation.json. Four preceding mouse-aim helper/fixture/tool files remain unchanged. Originals and prior launcher: reference/pre-impact-fx-20261103.

## Limits and continuation

This slice ports impact camera shake only. Hit audio, sparks, hit statistics and complete collision feedback remain open. The separate spin-dodge phase-order/random-side/feedback gap identified during inspection remains unchanged; porting it needs a bounded ordering change rather than enabling the full unqualified combat pack. Production rivals/combat/coins remain inactive and accounts/progression untouched.

No new graphics resources, uploads, allocations or cap increase. Native camera appearance, comfort, stability and performance remain unobserved. Run-RoadstersRace.cmd is promoted after verification to build/impact-fx-native/bin/RoadstersRace.exe, preserving prior builds and setting that working directory. Static launcher verification does not execute it. The earlier automatic-review rejection under AGENTS.md still requires explicit graphics execution authorization; no retry occurred.
