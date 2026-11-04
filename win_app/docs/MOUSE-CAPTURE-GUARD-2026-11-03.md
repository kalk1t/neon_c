# Mouse-capture warmup and release state

The browser ignores movement while unlocked and during the first150ms after pointerlockchange, to avoid leftover capture-click motion. Native previously only drained GetMouseDelta once when centering aim. The native host now tracks capture/release and drops consumed frame deltas while its capture age is below150ms; eligibility begins at exactly150ms. Keyboard/gamepad input mixing and aiming remain active. Capture and all existing cursor-release paths share this state, so pause/menu/error release cannot leave the guard captured. Reacquisition starts a new warmup; ordinary centreAim calls do not restart it.

## Evidence

Native/WASM full application builds each passed23/23 focused checks. The new fixture instantiates the current browser Input class with inert event registration, executes its actual pointerlockchange/mousemove listeners and records48 eligibility states: locked/unlocked, three clock origins, negative/zero age, just below/at/above150ms and long elapsed time. Small events isolate warmup from the separate spike filter. The C test also checks invalid time, release/reacquire and preserved keyboard steering/climb through the shared aiming function. No real cursor capture, desktop control, audio or graphics ran. All59 declared tests were not rerun.

Current local browser version is0.2.11. Exact input.js identity/scope: reference/ui-parity-20261103/mouse-capture-oracle.json. Changed-file hashes, fresh build logs and executable/447 matching asset-copy identities: capture-guard-implementation.json. Five preceding circle-clip helper/fixture/tool files remain unchanged. Original main.c/CMakeLists.txt and launcher: reference/pre-capture-guard-20261103.

## Remaining limits

The eligibility predicate matches the event-listener rule, but native raylib supplies accumulated frame deltas rather than timestamped individual mouse events. A polling frame crossing the cutoff can contain events from both sides and cannot be partitioned here. The native timestamp records the capture API call; actual OS capture completion, focus/warp handling and physical input remain unverified. This is a host warmup improvement, not full raw-event parity.

The browser's separate per-event bogus-jump filter is deliberately not applied to frame totals, which could discard legitimate aggregate motion. Exact spike suppression, stale-button behavior and full event-level input integration remain open. No graphics resources, allocations, saved preferences/account/progression data, production-combat activation or resource-cap changes.

After verification, Run-RoadstersRace.cmd selects build/capture-guard-native/bin/RoadstersRace.exe with that working directory. Preceding clients/launchers remain preserved. Static checks do not run the application. The prior automatic-review graphics rejection under AGENTS.md remains pending explicit execution authorization; no retry or desktop action occurred.
