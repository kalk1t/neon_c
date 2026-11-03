# v3 UI corrections and next garage gate — 2026-11-03

Reviewed the current browser garage and the completed asset-team v3 handoff. Implemented two concrete corrections to existing native UI. Did **not** add another hidden garage prototype: the main unresolved UI acceptance step is now supported native graphical verification.

## Completed v3 review and corrections

- Verified695 v3 package file sizes/SHA256s against its manifest, with zero failures. Package root: `C:/Users/User/Documents/Codex/2026-11-03/task/RoadstersSharedAssets-v3`; selected release `28a4e302e63d2e797db65f62d042025b97a7de84` (v0.2.3). Its213 component/state IDs,578 PNG layers,13 browser-harness behavior checks and11 cockpit image pairs are asset-team evidence, not new native rendering acceptance.
- All five current native Roadster PNGs are **byte-identical** to v3's corresponding compatible exports. No replacement, conversion, extra texture upload or new runtime asset was needed. Earlier v1/v2/v3 packages remain unchanged.
- Browser `main.js` records `performance.now()` when paused and allows Escape/P resume only when elapsed time is **strictly greater than400ms**. Native previously resumed immediately. New `rr_pause_ui.h` stamps only actual pause transitions and enforces that guard for keyboard/mapped-controller resume. Focus, minimize, overload and validation pauses use the same timestamp. Repeated pause attempts do not reset it. Mouse RESUME remains immediate, matching the separate browser button behavior; restart/menu and previous-phase restoration remain unchanged.
- v3's actual cockpit DOM measurements confirm empty boost text `#c4ccd7` and overheated attack text `#ffb15a`, versus normal inherited text `#f6f9ff` and normal attack `#ff5ade`. The existing candidate renderer now uses those exact packed RGBA values. The offline host still marks combat NOT CONNECTED; no heat/weapon gameplay was activated. Cockpit remains defaultOFF.

New headless checks cover four resumable phases,400ms boundary/just-over timing, repeated requests, focus/no-input suppression, immediate mouse resume, restart/menu cancellation, invalid clock values and v3 palette constants. Full native/WASM builds/tests run in fresh `build/v3-ui-corrections-native` and `build/v3-ui-corrections-web` folders. Counts, artifacts and full logs are recorded in `reference/ui-parity-20261103/v3-ui-corrections.json`; no graphics was executed. Pre-slice source is preserved under `reference/pre-v3-ui-corrections-20261103`.

## Current garage contract, without executing account code

Read-only reference: `game/src/shop.js`, `game/src/online/index.js`, `game/src/car.js`, shop CSS in `game/src/style.css`, and shop markup in `game/index.html`.

| Item | Exact source behavior | Windows integration requirement |
|---|---|---|
| Catalog | ROADSTER0, VIPER2500, TITAN7500 virtual coins; cosmetic models share speed/weapons. | Read the selected reference catalog; no economy redesign or invented prices/entitlements. |
| Browse/preview | Shop opens on equipped car, choosing first unowned sale car on a first visit from Roadster; selecting another card changes preview only. Closing restores equipped car. | Maintain distinct `preview_id` and `equipped_id`; preview Viper/Titan must never equip, unlock or modify profile. Offline browsing itself needs no product/account decision. |
| Ownership | All free cars plus `profile.cars`; persisted equipped ID is accepted only if owned, otherwise Roadster. | Actual shared profile adapter is needed for equip/purchases. Do not import browser local storage or fabricate a native wallet/owned list. |
| States | `equipped`, `owned`, `signin`, `closed`, `sale`, `expensive`, and optional null `soon`. CSS comment's five-state summary does not replace these seven actual logic states. | Keep source-derived state presentation and disabled controls. The existing native SHOP entry remains disabled until a real screen is qualified. |
| Purchase | Confirm preview ID, set busy, await backend `buyCar`, then show success; errors restore retry/cancel. Escape/close cannot interrupt busy state. `justBought` supports a confirmed purchase before profile snapshot arrives. | No new purchase/equip/sign-in actions without the agreed shared service plan. Test local intents with synthetic data only; backend confirmation remains authoritative. |
| Preview scene | Browser main/showcase camera, original three GLBs, fitted source car offsets/material/shader overrides; drag/zoom controls. | Native renderer currently supports Roadster-specific resources/material behavior. Viper/Titan loading, material overrides, model swap cleanup/fallback and peak resource costs need qualification. A thumbnail is not the source live3D preview. |
| Layout | Header/wallet across top; left scrolling cards; live stage center; detail/action bottom; width≤1180 compact rule. | Match actual viewports/DPI and interaction hitboxes. Preserve selected/hover/focus/disabled distinctions; labels and prices stay dynamic. |

## Reusable assets and compatibility requirements

- `game/public/ui/car-{roadster,viper,titan}.png`: transparent400×200 thumbnails,3×320,000 decoded RGBA bytes (960,000 total). Existing coin PNGs32/64/256px and static font faces are already available through v1/v2/v3. No new artwork/generation is identified as necessary.
- `game/public/models/{car,car02,car03}.glb`: original3D vehicles. Existing native Roadster conversion is not proof that raw Viper/Titan embedded textures, mesh names, source fit or emission/clearcoat override will render correctly. Keep source `car.js` fits and material recipe. Quantify peak memory/upload work before defining a native swap strategy; do not silently raise caps or preload all three cars.
- v2/v3 deliver chrome/reference pairs with transparent shadow padding and explicit content rectangles. Use chrome for dynamic native components, not labeled fixture images. Baked state opacity must be applied once; padded image bounds are not hit bounds. Fixed clips/glows/gradients do not automatically support nine-slice stretching.
- v3 explicitly leaves the complete garage/purchase matrix incomplete. Through parent, request individual garage card/action/dialog chrome for all seven semantic states, selected/default/hover/focus/pressed/disabled variants, busy/error/success states, exact padding/content/hit rectangles and desktop/compact layout metadata. Keep synthetic fixture values labeled and do not bake a real account/wallet into reusable assets. Do not duplicate the asset team's broad extraction project.
- v3 distinguishes HUD rounded fractional time from cockpit floored centiseconds and discrete font mapping:600→700,750/800→900. Preserve those recipes when binding further UI; current typography/glow/spacing still needs visual comparison.
- v3's selected live account-result contract uses numeric awards, while newer mutable working source can support structured responses. Offline Windows does not process either contract yet. Pin the actual shared backend/release interface before connecting saves/rewards; do not blindly port newer fixture response shapes into live integration.

## Exact next meaningful step

The owner has authorized controlled tests, but the earlier higher-integrity Task Manager/input limitation left **independent process absence unverified**. Owner-reported closure and absent native window were preserved as observations, not a successful process check. No retry or alternate process lookup, protection bypass, launch, termination or change to a potentially in-use output occurred here.

Required supported resolution: establish whether `RoadstersRace.exe` is running through an allowed identity/process check or fresh explicit owner evidence that resolves that specific blocker; reconcile/close only the intended test application and verify shutdown before launching one isolated build. Do not retry the denied/higher-integrity route without new relevant evidence and parent direction. No security changes are proposed.

After that, qualify **existing** menu/road-picker first, then HUD/pause/results and finally the opt-in cockpit: font/texture loading, equal-viewport/DPI comparison, clipped input, repeated Escape timing, focus/overload pause, restart/return and clean shutdown. Native GPU appearance/input/performance is the gate; further synthetic scene variants will not answer it.

Then implement view-only garage browsing using source catalog, reused thumbnails and a bounded actual3D preview path, with purchase/equip/account controls inactive until service integration is agreed. This needs no new visual-style or artwork decision. It does need the native renderer/resource/interaction measurements above. A new headless garage controller or static screen would add another unreviewed layer now; none was created.

All browser/asset-team files remain read-only, progression/locks unchanged, and prior compiled evidence retained. Native visual parity, GPU stability, human race acceptance and shared account/economy integration remain open.
