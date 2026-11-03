# Browser-to-C parity checklist


Latest course evidence: [selected-road furniture](COURSE-FURNITURE-2026-11-02.md) — native/WASM 12/12; existing prefab geometry, normal Skyline render/start/pause and clean shutdown. Shared platforms/trees, city/materials and manual race acceptance remain open; locks/progress unchanged.


Latest collision evidence: [scoped solid-world collision](COLLISION-2026-11-02.md) — native/WASM 11/11; scripted post push-out/rebound/pause and clean shutdown verified. Full furniture/city and human-driven race acceptance remain open; no progress changes.


Latest finish evidence: [solo finish/results](FINISH-RESULTS-2026-11-02.md) — native/WASM 10/10; scripted finish/result/restart/menu visual checks and clean shutdown. Full human-driven race, rivals/collision/art remain open; no progression changes.


Latest city evidence: [all-route city clearance](CITY-CLEARANCE-2026-11-02.md) — native/WASM 9/9, bounded Skyline rendering and clean shutdown; complete city art/collision and race acceptance remain open. Locks and progress are unchanged.

Reference: `../reference/browser-baseline.json`. Initial status for every feature is **not implemented**. This is a tracking inventory; it does not certify the browser reference is bug-free or that a feature has been ported.

| Feature group | Browser reference | C acceptance |
|---|---|---|
| Road content | `roads.js`, `track.js`, `maps.js` | Same nine roads, centripetal spline/arc-length sampling, gate placement, two-lap race, road unlocks |
| Flight | `car.js`, `input.js`, `main.js` | Same acceleration/brake/reverse, mouse aim, steer/climb, drift, boost, spin/surge/invulnerability, reset |
| Collision | `physics.js`, `course.js`, `city.js` | Same OBB/cylinder/torus behavior, car contacts, floor/bounds and safe transforms |
| AI | `ai.js`, `main.js` | Ten named/liveried rivals, shared car physics, gate progress, difficulty, rubber band, coin seeking, firing and respawn |
| Combat | `ai.js`, `fx.js`, `main.js` | Blaster heat, lock/homing, seeker charges, EMP, health/stun/smoke, wreck/respawn, rewards and streaks |
| Race modes | `main.js`, `hud.js` | Race, endless time attack, tour, countdown, wrong-way/lost warnings, rankings/results, local bests |
| Coins | `coins.js`, `main.js`, account backend | Placement/respawn, coin race, podium bonus; trusted rewards come from server |
| Cars/shop | `car.js`, `shop.js`, GLBs | ROADSTER/VIPER/TITAN, 11 paint choices, preview/equip, matching materials and fallback, shared ownership and prices |
| City/art | `city.js`, `course.js`, `props.js`, `skyprops.js`, `ads.js` | Corridor clearance, skyline, gates/beacons/signs, scenery/traffic, animated ads; same art direction |
| Atmosphere | `env.js`, `materials.js`, postprocessing in `main.js` | Moon/sky/clouds, fog, emissive light, bloom, tone mapping, speed effects; portable quality presets |
| Cameras | `main.js`, `cockpit.js` | Chase distances, first person/cockpit, RMB look, menu/pause showcase and tour |
| HUD/UI | `index.html`, `style.css`, `hud.js`, `shop.js`, `chat.js` | Menus, road picker, race position/timing, minimap, target/lock, weapons, health, coins, results and error messages |
| Audio | `audio.js` | Engine/thrust/boost and countdown/weapon/impact/coin/finish cues, volume/mute persistence |
| Controls | `input.js`, `touch.js` | Keyboard/mouse/gamepad mappings, capture and focus behavior; browser replacement retains mobile touch/tilt |
| Settings | `main.js`, `sysreq.js`, storage sites | Sensitivity/invert, quality, volume, color/equipped car, local bests and recovery; accurate capability reporting |
| Identity/profile | `account.js`, `online/firebase.js`, Firestore rules | Same Firebase UID, Google sign-in, nickname uniqueness, account switching/sign-out, fresh profile data |
| Economy | `functions/index.js`, `functions/rules.js` | Server-authorized tickets/rewards/shop, timing/caps/retry/idempotency behavior, shared wallet/progress |
| Rankings | `account.js`, `online/firebase.js` | Coin, per-road time and kill rankings; same field names and ordering |
| Free roam | `online/live.js`, `online/protocol.js`, `server/live.js` | Shared room, version/rate/size limits, interpolation, spawn, roster, player count and ping/time display |
| Online combat | `main.js`, `server/live.js` | Both hit directions, EMP, wreck/respawn/protection/regen, confirmed kills, lifetime save and known trust boundaries |
| Social/auras | `chat.js`, `killer.js`, `server/live.js` | Chat limits/feed, session kills, CRIMSON FLAME/VOID CROWN, reset lock and `/pkclear` |
| Lifecycle | loading/error/state code | Asset loading and fallback, recoverable errors, pause, reconnect, update required, clean exit and persistent settings |
| Browser replacement | current manual and feature inventory | Desktop + mobile regressions, storage/text/audio/identity bridges, loading/caching and rollback review |

## Compatibility is not a redesign

Do not add private rooms, stronger server-authoritative combat, new shop items, changed weapon balance, extra maps, or Windows-exclusive gameplay as incidental port work. Existing limitations, such as how remote car appearance is transmitted, must be tracked and changed deliberately across clients rather than silently fixed on one side.

For each completed row, append its C module paths, reference hashes, test evidence, visual review, known deviations, and the milestone that accepted it. A row becomes complete only when native C and the relevant C browser build both meet its gate; old-browser interoperability is additionally required during transition.


## Partial execution evidence, 2026-11-02

Shared C now reproduces all nine road geometries and gate placement, with
18,052 sample/tangent pairs, 117 gates and coupled two-lap rules checked on
every road. Full native/WebAssembly builds pass 7/7 headless tests. The UI still
starts Skyline; other-road selection/unlocks, all-route city clearance, per-road
persistence and visuals remain open. This does not complete the road-content
row or any whole milestone. See `ALL-ROADS-2026-11-02.md` for fresh versus reused
evidence, hashes, changed paths and current graphics-test limits.

## Road-picker connection, 2026-11-02

The native menu connects a nine-row picker to road/session selection and restart.
Browser lock/open/focus/eligibility and minimap decisions have CPU reference tests;
both full targets pass 8/8 headless tests. The clean offline UI opens only road 1,
with no profile/storage import or unlock writes. Native queued Enter/navigation,
locked refusal, Skyline countdown/start, pause/resume/reset and clean shutdown
were observed. This does not qualify other-road worlds, full results, progression,
physical inputs or Web graphics. See `ROAD-PICKER-2026-11-02.md`.
