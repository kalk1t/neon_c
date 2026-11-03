# Source-exact minimap player path, 2026-11-03

## Source boundary and implementation

The browser's local package now reports0.2.8. Its race HUD.drawMap method was compared with the supplied v1 reference after removing comments/whitespace and is unchanged. This slice follows that common method, rather than importing unreviewed newer economy/network behavior. Current HUD source and reference hashes are in minimap-marker-oracle.json; no deployed version or whole-version parity is claimed.

Windows used a smaller solid triangle for the player marker, while source Canvas fills tip(0,-16), right(11,12), notch(0,6), left(-11,12), rotated by minus car yaw on its440px canvas. The shared minimap helper now applies that exact four-point path through the existing world projection and viewport scaling. Native rendering fills it as two non-overlapping triangles with raylib winding, preserving the concave notch. This adds one triangle to the previous marker and no mesh/texture/render-target allocation or raised cap.

The existing conservative circular-edge policy remains an explicit parity limit: draw only when all four vertices lie inside the minimap disk. A convex disk contains the full polygon whenever its vertices are inside, so this avoids drawing outside it. Source CSS can show a partially clipped arrow at the rim; native partial-circle polygon clipping remains unresolved. This change does not claim edge behavior, magenta shadow/glow, rasterization or final float placement matches the browser. No sampled approximation was introduced to call that clipping solved.

## Fresh verification

`tools/capture-minimap-marker.mjs` invokes actual HUD.drawMap with an inert Canvas command/transform recorder and normal source HUD.setTrack projection. It captures the emitted path, rather than supplying expected arrow vertices to the oracle. Fiftyfour cases cover three world positions (including off-map), six headings including cardinal/wrapped angles, and three square/non-square viewports. Current source is checked unchanged during capture, and method equality with the preserved reference is asserted before capture. No actual Canvas renderer, graphics window, DOM account UI, network or storage is used.

`tests/test_race_ui.c` compares all four transformed vertices in each case with1e-10 tolerance and checks both native fill triangles' winding. Existing nine-road minimap checks, chronological lap-history viewport bounds,142 source clock cases and race/pause state tests remain present. After the name correction, final native and WASM application builds passed with 3/3 focused race-HUD/render-policy/font tests on each target. Three preceding lap-history implementation/provenance files were verified unchanged. Results are recorded in minimap-marker-{native,web}-build.log and minimap-marker-implementation.json. The full45-test suite was not rerun; earlier full42 results remain separate. The first native attempt stopped at a warnings-as-errors local-name shadowing diagnostic in main.c, before running tests; the minimap array was renamed and that failure log was preserved separately before final validation.

Modified: `src/core/rr_minimap.h`, `src/app/main.c`, `tests/test_race_ui.c`. Added: `tests/minimap_marker_fixture.h`, `tools/capture-minimap-marker.mjs`, `reference/ui-parity-20261103/minimap-marker-oracle.json`, implementation receipt and this report. Originals are in `reference/pre-minimap-marker-20261103`. Isolated outputs: `build/minimap-marker-native` and `build/minimap-marker-web`; prior candidates and records remain preserved.

## Remaining limits

The rejected graphics launch remains pending separate explicit owner authorization; no retry, desktop input, process query or termination occurred. Actual arrow visibility, clipping, glow and native input/hardware/GPU stability remain unverified. The first pending graphics check stays menu/track selection; a later HUD fixture or world trial requires bounded scope. No browser-owned source, art, progression/unlock/account/network behavior, cache/upload caps or graphics safeguards changed. The source-provided art was reused without generation or paid calls.
