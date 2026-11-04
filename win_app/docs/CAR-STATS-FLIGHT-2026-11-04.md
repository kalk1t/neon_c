# Current-source per-car flight parity

Current flight now consumes source-defined top-speed, acceleration and turning multipliers for Roadster, Viper and Titan through RRCar.profile / rr_car_use_stats. This is the existing current flight/session path, not a separate movement prototype. Production selection remains Roadster; no ownership, equipment, unlock or progress operation was added. The source table also records hp, knock, stun, mass, spin cooldown, missile capacity and EMP multiplier, but those behaviors are explicitly NOT ported by this slice.

| Profile | Top / nitro top | Both accelerations | Turning | Recorded max HP |
|---|---:|---:|---:|---:|
| Roadster | 1 | 1 | 1 | 100 |
| Viper | 1.08 | 1.276 | 1.12 | 75 |
| Titan | 0.94 | 0.797 | 0.9 | 140 |

Source details preserved: the speed fraction used for turn remains relative to base135, not profile-adjusted top; maxMul and surge still combine in source order; boost scales both maximum and acceleration; dodge retains shared135m/s floor rather than multiplying by profile top. Place/respawn preserves the selected stats; constructor-style reset selects Roadster. Unknown/null IDs and invalid profile enums fall back to Roadster. Frozen old flight entry points intentionally ignore profiles and retain historical fixtures. No source-owned files changed.

Fresh inert-node capture calls original browser Car.update/place/dodge and actual Colliders (box/cylinder/torus), without constructing a Car/model/renderer, browser UI or audio device.7,200 frames span3cars x10scenarios x240frames: coast/reverse, boosts, strafe/aim-steering, brake/turn, pitch, stun, surge/dodge, variable dt, speed scaling, placement reset and primitive contacts.33 numerical channels per frame plus boost/contact/normal are checked. New source snapshots/hashes and fixture are in reference/ui-parity-20261103/car-stats-capture. Original old fixtures remain unchanged. Additional native session tests cover every profile through capped-dt movement, countdown, pause/resume, nonfinite dt, finished/results movement, shared dodge floor, placement/fallback/reset, and legacy invariance.

Fresh full native and WebAssembly builds/CPU suites passed149/149 each. Exact errors, contact/reset counts, RRCar sizes and client hashes are in car-stats-implementation.json. Static table cost240bytes plus a profile enum per car (actual resulting struct sizes recorded); no new heap/GPU/upload budget, graphics instance or cap change.455 copied assets, main-source identity and prior client were verified before launcher promotion. Prior builds and before-files remain preserved.

## Working source versus released behavior

Local package.json and42 docs/tech-plan baseline files remain0.2.16/unchanged. Current car/aim/race/main/HUD/style/index/AI/protocol source already contains v0.2.17 CAR_STATS changes, and LIVE_PROTOCOL is11 (previous10). Relevant sources were stable against the previous13:52 pin during this slice; this is evidence of a working source revision, not a verified release, tag or deployed build. No Git/deployment/live-server inspection occurred.

Protocol11's source comment ties it to differing shop-car stats and percentage health. roam.js sends100*hp/maxHp (zero while wrecked); HUD likewise derives percentage from each car's own maximum. Racer construction/respawn use maxHp. Damage scales lost speed and stun by profile; contacts use reciprocal mass shares; player/rival resources use per-car missile capacity, spin cooldown and EMP reach, with victim knock on EMP push. These are recorded next parity gaps; no native online protocol/serialization/backend/account behavior was modified.

## Remaining qualification

Next useful offline slices: profile-aware health/damage/respawn and contact mass, then player/rival arsenal capacity/cooldown/EMP interactions with source-seeded full-pack fixtures. Preserve old fixtures through explicit versioning/default Roadster behavior. Equipment selection/persistence must follow the shared account plan and must not grant cars. HUD health percentage and online protocol11 require their own integration qualification.

Production rivals/combat/coins/rewards/audio remain inactive. Graphics/real-mouse feel/current visuals remain untested: the explicit execution gate in AGENTS.md is still pending, and no game, graphics test, protected process inspection or denied-action retry occurred. A future authorized menu/road-picker trial must reconcile instance state first and verify clean shutdown.
