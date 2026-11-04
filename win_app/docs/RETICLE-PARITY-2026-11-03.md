# Heading crosshair and mouse-aim reticle

Windows now draws the source white26px/2px mouse-aim ring and4px center dot while captured, independently of the cyan car-heading crosshair. Aim direction comes from the existing source-tested shared updateAim math, initialized to(0,0,-1) and preserved until the next aim update as in the browser. The marker projects90m along that aim from the current car position. Cursor release hides it; production remains solo/invulnerable, without activating combat or lock-on.

The heading circle now has the source44px outer diameter/2px border, and its pseudo-element ticks follow the40px padding box: +/-30px endpoints and +/-12px horizontal mask stops. HUD zoom scales geometry. Paint order follows source DOM: heading/aim before the later gate target and messages. Fully offscreen marks are culled only when their complete extents cannot affect the viewport.

A shared presentation projection retains clip-space z and follows HUD.screen's strict z>1 rejection, rather than only testing positive camera depth. Gate guidance now reuses it. The host projects through the actual bounded world target and stretches coordinates with the displayed texture, preventing aspect drift from integer render-size rounding. Existing native clip planes and render-size caps remain unchanged. Zero camera-plane/nonfinite output is rejected to avoid invalid GPU vertices.

## Fresh evidence

Full native and WASM builds each passed24/24 focused checks. The new test captures1620 actual HUD.screen outputs using real Three perspective/lookAt/roll matrices:3 direct viewport/zoom configurations plus3 native bounded-target display configurations,3FOVs/views, both browser/native clip configurations and front/behind/beyond-far points. It also executes the current HUD aim branch for8 projected/captured/alive combinations. The native initial test differed from Three by at most0.002180142118275441px, within0.02px tolerance, and matched raylib's CPU GetWorldToScreenEx exactly. Final native/WASM summaries are in the implementation receipt. No window/GPU context was created; rlSetClipPlanes in the math-only test only updates CPU globals.

Two existing Chromium crosshair/aim border-box measurements are verified through v3 assets.json and source/style.css, in directory and ZIP. The selected current CSS matches the packaged reference. Inner/outer ring geometry is checked against those measurements at three scales; pseudo-element tick/dot placement follows source CSS. This is not a fresh browser raster capture.

Local browser package v0.2.11. Exact source/library identities and scope: reference/ui-parity-20261103/reticle-oracle.json and reticle-box-oracle.json. Changed-file hashes, final logs, context hashes and executable/447 matching asset-copy identities: reticles-implementation.json. Four preceding capture-guard helper/fixture/tool files remain unchanged. Originals and prior launcher: reference/pre-reticles-20261103. The first candidate executable/source and pre-layer-order logs are preserved separately; final builds include corrected paint order. All60 declared tests were not rerun.

## Limits and launcher

Native float projection is not bit-identical to Three doubles. Bounded-target aspect is an explicit native adaptation, not browser runtime sizing; the CPU.3/22000 tests do not raise production.1/2500 planes. Cyan aim glow, antialiasing, pixel compositing, real capture/steering and human race appearance remain unverified. The current offline host has no wreck/alive state; future combat integration must supply it, without double-stepping existing aim/input. No firing, targeting locks, rewards/accounts or network actions were enabled.

No new texture/shader/mesh/render-target assets or per-frame heap; only a fixed aim vector, matrix arithmetic and existing UI primitives. Run-RoadstersRace.cmd selects build/reticles-native/bin/RoadstersRace.exe after verification, preserving older clients and setting that working directory. Static launcher checks do not execute it. The prior automatic-review graphics rejection under AGENTS.md remains pending explicit authorization; no retry or desktop action occurred.
