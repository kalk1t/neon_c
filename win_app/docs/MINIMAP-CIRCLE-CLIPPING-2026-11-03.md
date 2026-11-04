# Minimap content box and player-arrow clipping

The native player arrow previously disappeared completely when any vertex crossed the minimap rim. The browser instead clips the visible portion through its circular overflow mask. Windows now clips each of the source arrow's two triangles and draws the surviving convex polygons. The ordinary inside path still draws the same two triangles. The content box also changes from the old240px canvas assumption to the source236px interior inside a240px frame with2px border; track, gates, start and player coordinates all use that interior.

The clip uses an inscribed128-gon instead of a new stencil/texture/render target. Its maximum inward radial deviation is r*(1-cos(pi/128)):0.035539393847898504 CSS pixels at the desktop118px interior radius. This explicit geometric approximation is separate from antialias/pixel fidelity. A convex triangle clipped by128 halfplanes has at most131 distinct vertices; capacity is132 with overflow/invalid-input failure reported rather than accepting truncated geometry. Adjacent numerical duplicates are removed. Winding is preserved.

## Fresh evidence

Native and WASM full application builds each passed22/22 focused checks. The new test covers516 cases:504 triangles captured from the current browser's actual HUD.drawMap arrow commands with inert Canvas recording, plus12 inside/outside/tangent/whole-circle controls. The independent Python reference uses exact segment/circle roots and oriented sector integrals, not polygon clipping. Output area stays within the exact intersection and the proved approximation bounds; vertices remain within both circle and source triangle. Translation, zero radius, invalid coordinates and capacity are checked. Prior race-UI arrow partition/road projection tests passed again.

Sampled player triangles produced at most6 vertices (4 fan triangles each); a control enclosing the whole circle produced128. The largest sampled marker-area loss was1.0361180619379411 square pixels across radii94.4,118 and236, within its analytic bound. This is a geometry measurement, not a native raster/visual observation. The complete58-test suite was not rerun.

Current local browser version is0.2.10. Source/method/CSS hashes and scope: reference/ui-parity-20261103/circle-clip-oracle.json. Fresh logs, changed-file hashes, bounds and executable/447 matching asset-copy identities: minimap-clip-implementation.json. Four preceding boost-meter helper/fixture/tool files remain unchanged. Originals and previous launcher: reference/pre-minimap-clip-20261103.

## Resources and remaining gaps

Static normals use2048 bytes. Per call, one132-point temporary buffer plus caller output is roughly4224 transient bytes; no heap. The theoretical fan bound is129 triangles per input triangle; sampled markers needed at most4. Ordinary inside arrows retain2 total triangles. No new texture, shader, mesh, render target or changes to scene resource/safety caps.

Track strokes still use the prior conservative circular segment clipping. Checkpoint-dot/rim clipping, label/glow/complete minimap styling, actual antialias/DPI/native appearance and hardware frame cost remain open. No full minimap or graphics acceptance is claimed.

After verification, Run-RoadstersRace.cmd selects build/minimap-clip-native/bin/RoadstersRace.exe with that working directory, preserving all preceding clients. Static identity checks do not execute it. The prior automatic-review rejection under AGENTS.md still requires explicit graphics authorization; no retry or game launch occurred. Accounts/progression and inactive combat remain unchanged.
