# Nitro call-out timeline and announcement cleanup

An accepted Nitro ignition now starts the native NITRO call-out, using the same strict ignition/retrigger guard as the existing camera effect. Its0.75-second desktop timeline tracks the browser's per-property keyframes: opacity0/1/0.9/0, scale1.6/1/0.95, letter spacing30 to14px and vertical translation-50% to-60%, with CSS ease-out per keyframe interval. Transform interpolation spans18% to100% even though opacity has a separate70% keyframe.

It uses absolute monotonic wall time, so pause/menu, frame-rate limiting or a minimized scheduling gap cannot prolong a CSS animation. A fresh accepted ignition restarts it. There is no fill beyond750ms. The source call-out is outside HUD zoom; desktop placement remains50% width/80% height with36px heavy text, and the native approximation accounts for trailing CSS letter spacing when centering. It draws before pause/menu/results overlays and after the ordinary flight HUD.

During integration, an obsolete cyan64px FINISH draw was found still layered over the newer shared announcement. It is removed, leaving the existing source-tested announcement text/timer as the owner of that message. Earlier headless announcement tests did not establish the appearance of the legacy overlapping layer.

## Evidence

Full native and WASM application builds each passed20/20 focused checks. The new CPU test compares two actual Chromium computed-style samples at135ms and525ms from the existing asset-team v3 package, including opacity, scale, spacing and transform translation. It also checks inactive/start,750ms expiry, wall-clock gaps and strict ignition/retrigger behavior. These are reused engine measurements; no browser was launched for this work.

The audit verifies assets.json and source/style.css against the v3 manifest in both extracted directory and ZIP. The selected Nitro CSS block is byte-identical after newline normalization to the current local v0.2.9 browser block. Exact manifests, hashes, computed states and scope: reference/ui-parity-20261103/nitro-call-oracle.json. Fresh logs, changed-file hashes and executable/447 matching asset-copy identities: nitro-call-implementation.json. Eight preceding spin/font helper/fixture/tool files remain unchanged. Originals and prior launcher: reference/pre-nitro-call-20261103. All56 declared tests were not rerun.

## Remaining limits

Native text is currently upright; browser synthetic italic and the layered cyan/magenta blur are still missing. The existing native glow is an approximation. Actual line-box/baseline, glyph metrics, centering, intermediate CSS animation-engine values, DPI and rendering/layer appearance remain unverified. Touch-specific top30% placement is not part of this desktop slice. This is timing/behavior progress, not pixel-parity acceptance.

No new image assets, textures, shaders, meshes, render targets or per-frame allocation; existing safety/resource caps and cockpit defaultOFF remain. Run-RoadstersRace.cmd selects build/nitro-call-native/bin/RoadstersRace.exe after verification, preserving prior builds and setting that working directory. Static launcher checks do not execute it. The prior automatic-review graphics rejection under AGENTS.md still requires explicit execution authorization; no retry or game launch occurred.
