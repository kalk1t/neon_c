# Boost meter state, geometry and animation

The active native boost meter now uses the browser's strict boost<0.2 low-charge flag, boosting gradient, low-charge blink and boosting brightness pulse. The later low selector takes precedence over boosting animation: burning+low stays magenta but blinks at0.4s instead of applying the0.22s brightness pulse. Both animations alternate using default CSS ease. Their timeline follows monotonic wall time; changing the boosting class while low does not restart the unchanged low animation. Animation state updates with HUD state even when the meter is hidden.

The bar now has the source1px border and skewX(-25deg), outer182x8 CSS pixels and inner180x6, rather than the old unbordered186x8 rectangle. Position and boost-label offset use the measured source positions relative to the speed panel. Fill remains dynamic. Existing UI zoom applies to this HUD element.

## Evidence

Full native and WASM builds each passed21/21 focused checks. The new test uses six reused actual Chromium initial meter states/child rectangles from the asset-team v3 package: full, partial, low, empty, burning and burning+low. Expected geometry combines measured relative rectangles instead of deriving it from the native skew formula. Additional CPU assertions check the exact20% boundary, low/on precedence, animation identity/restart and alternating endpoints. Intermediate easing values are source/spec-based, not newly measured browser frames.

The audit verifies assets.json, source/style.css and source/hud.js against the package manifest in both extracted directory and ZIP. Selected boost CSS and current HUD boost-rule lines match the packaged reference. Exact hashes/samples: reference/ui-parity-20261103/boost-meter-oracle.json. Current local package is v0.2.9. Changed-file hashes, logs and executable/447 matching asset-copy identities: boost-meter-implementation.json. Four preceding Nitro-call helper/fixture/tool files remain unchanged. Original main.c/CMakeLists.txt and launcher: reference/pre-boost-meter-20261103. The complete57-test suite was not rerun.

## Limits and launcher

Native brightness uses a bounded16-strip approximation of CSS filtering after gradient interpolation. Blur/glow, antialiasing, exact saturation/compositing pixels and real frame cost remain unverified. The whole speed-panel height, label/font metrics and missing shield are separate parity gaps. No fresh browser or native graphics execution occurred.

The visible meter adds at most16 gradient fill strips through the existing UI batch. There are no new assets, texture/shader/mesh/render-target resources or per-frame heap allocations; existing resource/safety caps and cockpit defaultOFF remain. Accounts/progression and inactive combat are unchanged.

After verification, Run-RoadstersRace.cmd selects build/boost-meter-native/bin/RoadstersRace.exe with that working directory. All older builds/launcher backups remain. Static target checks do not launch it. The prior automatic-review graphics rejection under AGENTS.md remains pending explicit execution authorization, with no retry and no visual/performance acceptance claim.
