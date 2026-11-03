# v4 native integration readiness — 2026-11-03

Inspected the completed `RoadstersSharedAssets-v4` manifest, component metadata, source specifications, coverage and predecessor errata. Prepared a reproducible read-only import audit and a native integration contract. No additional hidden screen/controller prototype, runtime asset import, browser edit, network/account action or graphics launch was added.

## Fresh verification and actual compatibility

- All1,004 manifest file sizes/SHA256s pass in both the completed local package and the ZIP archive; their manifests match. Fresh IHDR checks find950 PNG files:937 RGBA8 and13 RGB8, all noninterlaced. The930 state layers are a subset; additional originals/previews explain the difference from the package's930-layer count. Current pinned raylib PNG support covers these format classes, but this audit does not newly decode every image or certify native blending/rendering.
- Metadata contains494 state/scale records and247 state IDs. All chrome/reference paths resolve within the verified package. Exact dimensions, file hashes, format counts and resource estimates are in `reference/ui-parity-20261103/v4-package-audit.json`.
- The asset team's28 behavior checks and browser PNG/font validation are reused evidence. They did not launch the native application or validate pixel parity. No new native/WASM build was needed for this read-only audit/documentation slice; prior41/41 results remain prior scoped evidence.
- A representative `shop/cards/roadster/equipped/unselected/default/chrome@1x.png` was visually inspected as a local image: it contains the background/border shell without the thumbnail or dynamic label. Its522×248 pixel image has logical control content410×120 at offset(48,64). That offset is metadata-specific; assuming uniform64px padding would shift the native control. Runtime placement must subtract the content offset, and hit testing must use the logical clipped control rather than the padded image.
- All individual garage card records have410×120 logical content at the supplied component viewport1600×1000. Source CSS at native width960 selects a300px garage column with6px right padding, analytically implying294px card content. This is a CSS-derived expectation, **not a fresh measured native/browser screenshot**. Individual action assets are400×76, while source's≤1180 compact rule uses a300px minimum action width. The1280×720 preview is a screen fixture, not proof that each individual sprite matches960×720 component geometry.
- Arbitrarily compressing410px artwork to294px would shrink its fixed14px cut and border/glow thickness horizontally. Use source-recipe drawing or request source-rendered294px card/300px action variants at the target viewport/DPI. Do not silently nine-slice or distort the export and claim exact parity.
- Preloading every supplied PNG would require1,175,735,504 decoded RGBA8 bytes (about1.095GiB), before textures/driver overhead. That includes reference images, all states/scales and previews and is **not** a runtime proposal. Import a bounded set of needed chrome/original files at the chosen scale; keep data, text, focus and continuous state native. Existing renderer budgets remain unchanged.

## What can be reused safely

| Asset/category | Use | Required native binding |
|---|---|---|
| Original thumbnails/coins/fonts | Existing source pixels and static font faces; no regeneration needed. | Hash-verified selective copy once an implementation is ready; correct contain fit, font weight and alpha/filtering. |
| Garage card chrome | Background shell for active product/status/selection/interaction. | Source-matched dimensions, logical content offsets, clipped hit regions, separate thumbnail/name/status/price, hover/focus transition. |
| Shop actions/dialog chrome | Source endpoint appearance. | Disabled/busy state and dynamic text; preview selection never implies purchase/equip. Reference fixture outcomes must not become production data. |
| Ranking rows/panels/tabs | Reusable presentation templates. | Dynamic name/rank/score/road selection, loading/empty/error/scroll state. Only1/5/9 road-label raster examples exist; all nine labels remain dynamic. |
| Account/avatar/nickname | Frames and state chrome. | Real authorized data/remote photo and native text input/validation; no bundled synthetic user identity. |
| Live showroom | v1 car GLBs and source material/fit/camera recipes. | Native model switching, resource accounting, source material overrides, orbit/input and cleanup. v4 supplies no static fake showroom or qualified native3D preview. |

There is no asset-creation or visual-style decision needed. A view-only garage can be implemented without account writes after the existing UI/renderer is qualified. The graphics gate is separate from the later service-integration gate for purchase/equip/sign-in/progression.

## Source contracts and corrections to earlier planning

1. Preview and equipped IDs are independent. Arrow navigation changes **focus**, not preview; click/Enter selects. Closing restores the equipped car. While equipped remains Roadster, every `open()` previews the first unowned paid car—not only a first visit, despite the source comment. Opening only marks the cosmetic shop news notice seen.
2. Active catalog has14 reachable product/status combinations: Roadster equipped/owned; Viper/Titan equipped/owned/signin/closed/sale/expensive. The optional `soon` branch has no active product or `car-soon.png`, and selecting null is unsupported. Do not request/invent future-car art merely to fill the older seven-state planning table. The v4 active matrix supersedes that earlier broad request.
3. Successful purchase promise shows success but neither auto-equips nor optimistically subtracts coins. Busy state blocks buy/cancel/close/select/Escape. A15s purchase timeout does not cancel the backend request or prove failure. `justBought` permits an equip intent before profile arrival; the equipped getter still independently verifies ownership. These contracts are future integration requirements; no transaction code runs here.
4. Shop numbers use explicit `en-US`; account/ranking use ambient locale. Preserve dynamic numeric formatting and ellipsis; native ASCII-only menu fonts are not universal profile/locale coverage. Nicknames themselves are trimmed3–16 ASCII letters/digits/_/-, while arbitrary displayed provider/account strings still need a text/fallback plan.
5. The selected release's actual `RACE_GAP_MS` is60,000ms. V4 records that v3's result-only retry fixture used120,000ms from an outdated comment. Windows currently has no online award/pending-save integration using either constant. Keep errata as a contract note, not a reason to alter offline rules or historical packages.
6. Ranking has a documented stale-response limitation: same-board requests lack a token guard and old failures can overwrite a new board. Nickname saving disables only SAVE; input/cancel/Escape remain active, with no nickname busy guard/timeout. Record source behavior and intended integration decisions before changing it; no browser fix or new native account behavior is inferred from this audit.

## Concrete next integration sequence

First resolve the supported process-absence check and qualify existing isolated menu/road, HUD/pause/results and cockpit builds, with equal viewport/DPI comparisons and clean shutdown. Those already contain unverified native rendering; adding another disabled garage prototype would not resolve that dependency.

Then implement actual view-only garage browsing: source catalog plus separated focus/preview state; source-sized dynamic card layout; a bounded resource-managed live3D preview; restore equipped Roadster on close. Keep purchase/equip/account controls inactive. Use matched-size chrome only where it improves fidelity over source-recipe C drawing. Review peak memory/upload costs before loading Viper/Titan; do not preload all models or raise resource caps silently.

Through parent, request only native-target component sizes/state metadata not already supplied:960×720 desktop compact rendering (including294px cards/300px action where confirmed), chosen DPI/scales, and exact content/hit rectangles. V4 already covers active states; no broad extraction or future-car generation is needed.

Account/purchase/ranking production actions follow only after the shared backend/identity plan is agreed. The fixture adapters are local specification harnesses, not production authentication or transaction implementations. Synthetic profile values are never imported into user progress.

## Outstanding blocker, unchanged

Action pending: native UI/resource/input/GPU verification of existing isolated builds. Earlier Task Manager input failed to establish process absence because its higher integrity limited supported access. Owner-reported closure and absent window remain observations, not independent PID verification. This turn made **no** process/control retry, alternate lookup, protection bypass, game launch, termination or security change.

The next required resolution is an allowed process identity/absence check or fresh explicit owner evidence addressing that specific blocker, followed by reconciliation/verified shutdown before one controlled instance. No new product/art approval is requested. Native pixel parity, input delivery, GPU cost, graphics stability and whole-race acceptance remain unqualified.
