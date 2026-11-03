# Next-gate guidance

The previous Windows HUD used custom screen margins and omitted guidance when the target fell behind the camera or near the screen edge. It now uses the browser HUD.update branch's strict NDC thresholds (abs(x)<.92, abs(y)<.9, z<=1), rounded metre distance, and directional arrow placement/rotation when the diamond is hidden. Zoom applies to CSS offsets while the physical arrow radius remains 38% of the smaller viewport dimension. Free roam and other existing no-target states remain governed by rr_session_target.

## Fresh verification

Full native and WASM application builds passed, each with 10/10 focused tests. The new test replays 1,296 outputs from the actual browser HUD branch across four viewport/zoom combinations, strict boundary cases, behind-camera and beyond-far NDC values, and distance rounding. The other nine checks cover the preceding submessages/glyphs, start cues, missed-gate warning, UI layout, fonts, session, picker and results. This is not a rerun of all 51 declared tests.

Current local browser package is v0.2.9. The source hash and capture scope are recorded in target-guidance-oracle.json. Full evidence, changed-file hashes, native executable identity and 447 matching asset-copy hashes are recorded in target-guidance-implementation.json under reference/ui-parity-20261103. Eight preceding subtext implementation/fixture files remain unchanged apart from deliberately edited main.c and CMakeLists.txt. Originals are preserved under reference/pre-target-guidance-20261103.

## Remaining limits

No graphics execution occurred. The helper is qualified against supplied NDC inputs, not a full browser-to-raylib world projection. Native .1/2500 clip planes are preserved versus browser .3/22000; rendering safeguards were not expanded. Raylib's float projection can differ at extremely close boundary cases. The new native triangle approximates the browser U+27A4 glyph and its box; pixel shape, glow, CSS diamond box/baseline, pulse easing and real DPI appearance are still unqualified. No new textures, allocations, meshes, render targets or cap increase.

Run-RoadstersRace.cmd is promoted only after these checks to build/target-guidance-native/bin/RoadstersRace.exe, with its working directory set there. Previous launcher/client outputs are preserved; no data or account writes. Launcher verification is static only. The earlier automatic-review rejection of graphics execution under AGENTS.md remains pending explicit execution authorization and has not been retried.
