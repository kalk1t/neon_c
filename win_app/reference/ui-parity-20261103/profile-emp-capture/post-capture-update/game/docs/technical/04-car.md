# 4. Car: models, flight model, boost, spin dodge

✅ **Done** · [Technical contents](README.md) · Previous: [World](03-world.md) · Next: [AI rivals](05-ai-rivals.md)

`src/car.js`, class `Car`. The same class flies the player, the rivals and (from network data) other FREE ROAM players.
It is an **arcade** model: the car has a scalar `speed` along its nose, and its velocity eases toward
`nose × speed`, which gives a little drift.

## Constants
| Constant | Value | Meaning |
|---|---|---|
| `CAR.MAX` | `TOP` 135 m/s (**486 km/h**, 302 mph; 95 before v0.2.15) | Top speed on thrust |
| `CAR.BOOST_MAX` | 138 × `K` = 196 m/s (**706 km/h**, 439 mph; 138 before) | Top speed while boosting |
| `CAR.ACCEL` | 36 × `K` = 51 m/s² | Thrust acceleration (fades as speed nears the top) |
| `CAR.BOOST_ACCEL` | 75 × `K` = 107 m/s² | Boost acceleration |
| `CAR.RADIUS` | 2.1 m | Collision sphere against the world |
| `K` | `TOP / 95` = 1.42 | Scales nitro and both accelerations with the top speed, so 90% of top speed still takes ~3.9 s |
| `SPIN.ROLL` | 0.55 s | Barrel roll duration (no damage while > 0) |
| `SPIN.SURGE` | 2 s | Speed surge after a spin |
| `SPIN.MUL` | 1.5 | Surge multiplier (speed and top speed) |
| `SPIN.JINK` | 22 m/s | Sideways hop |
| `SPIN.COOLDOWN` | 6 s | Between spins (`g.spinCD` in `aim.js`) |

The HUD and the cockpit show `speed × 3.6` as km/h, or `× 2.237` as mph (Settings **SPEED**, `neon.units`, v0.2.15).

## Shop cars (`CAR_STATS`, v0.2.17)
Each shop car flies and fights differently: `CAR_STATS[id]` holds multipliers on `CAR` / `SPIN` / `WEAPON` and its own
health. `car.useStats(id)` sets `car.stats` (the player's equipped car at each `startRace`; other players' cars in
FREE ROAM from their `car` id). Rivals fly the ROADSTER's.

| | ROADSTER (free) | VIPER (2,500) | TITAN (7,500) |
|---|---|---|---|
| Role | All-rounder | Fast and agile, fragile | Heavy tank, slower |
| Top speed | 135 m/s · 486 km/h | 146 m/s · 525 km/h (+8%) | 127 m/s · 457 km/h (−6%) |
| To 90% of top speed | 3.9 s | 3.3 s | 4.6 s |
| Handling (turn rate) | 100% | 112% | 90% |
| Health | 100 | 75 (4 bolts wreck it) | 140 (7 bolts) |
| Knock-back from hits | normal | ×1.3 | ×0.4, stuns 40% shorter |
| Car-to-car bumps | equal | lighter (0.8) | heavy (×2): pushes the other car away |
| Spin dodge | every 6 s | every 4 s | every 6 s |
| Seekers | 3 | 3 | 4 |
| EMP reach | 95 m | 95 m | 119 m (+25%) |

| Field | Used by |
|---|---|
| `top`, `accel`, `turn` | `Car.update`: top speed and nitro top × `top`; both accelerations × `accel`; `turnRate` × `turn` |
| `hp` | `Racer.maxHp`: reset / respawn health, the SHIELD bar and nameplate bars as a share of it, smoke under 60% |
| `knock`, `stun` | `damage` (`race.js`) and `roamHit` (`roam.js`): speed lost × `knock`, push × `knock`, stun × `stun`; EMP push × the victim's `knock` |
| `mass` | `carContacts`: the push between two cars is split by mass (equal cars: half each) |
| `spinCD` | the spin dodge's cooldown (`aim.js`, the SPIN ring) |
| `missiles` | seeker charges (`aim.js`, takedown refills, the HUD pips) |
| `emp` | EMP radius × `emp` (`aim.js`; other players' EMPs in `roam.js` use their car's) |

## Inputs
Every controller produces the same shape each frame:
`{ throttle 0..1, brake 0..1, steer −1..1, strafe −1..1, climb −1..1, boost bool, fire bool, look bool }`
(`strafe`: A / D while the mouse is captured, v0.2.13; then `steer` comes from the arrows only)
(from `input.js` for keyboard/mouse/gamepad, `touch.js` for phones, `aiInput` for rivals, `autopilot` for tests and
the finish).

## Update step (`Car.update`)
1. **Timers:** stun, roll and surge count down.
2. **Boost** is on when boost is held, the tank has more than 2%, throttle > 0 and the car isn't stunned.
3. **Speed**
   - top = (boosting ? `BOOST_MAX` : `MAX`) × `maxMul` (rivals: `rivalTop(road) / CAR.MAX` and the rubber band; 1 for players) × (surge ? 1.5 : 1)
   - thrust: `speed += acc × maxMul × throttle × dt × (1 − (speed/top)²)` (smooth approach to the top; × `maxMul` since v0.2.15)
   - no thrust: coast down at 9 m/s² and stop at exactly 0 (v0.2.13: overshooting flipped the sign, and the
     steering with it, every frame); brake: −70 m/s² (−14 when reversing); over the top: −28 m/s²
   - minimum −18 m/s (reverse, ~65 km/h)
4. **Steering:** yaw rate 1.9 rad/s at low speed down to 1.15 rad/s at top speed, ×1.25 while braking, reduced
   below ~8 m/s. **Pitch** eases toward `climb × 0.62 rad` (±35.5°) at rate 3.2/s.
5. **Slide:** `side` (m/s, + right) eases toward `strafe × STRAFE.SPEED` (25) at `STRAFE.RESPONSE` (6/s); 0 while
   stunned; reset by `place`.
6. **Velocity** eases toward `nose × speed + right × side` with grip **3.4/s**, or **1.6/s** when braking and steering together
   (a deliberate drift).
7. **Collision** with the world ([World](03-world.md#collisions)).
8. **Look:** roll into turns (up to 0.62 rad, plus slip, plus `STRAFE.LEAN` 42° at a full slide, v0.2.14; 17° before), hover bob, nose lift on nitro, thruster pods tilt with
   throttle, brake and steering (and all toward a slide), fans spin with thrust, nozzle glow brightens, a white-hot flash on ignition, and a
   wobble while stunned.

## Thruster flames (v0.2.18)
Every car has three flames: two side nozzles and a middle one. Their spots were marked by the owner on rear pictures
(`Assets/01_Vehicle/ROADSTER_flame_spots.png`, `car02/VIPER_flaming_spots.png`, `car03/TITAN_flaming_spots.png`) and
measured on each model (the face of the nozzle, so no flame is drawn over the car):

| Car | Side flames (`exhaust`) | Middle flame (`turbineFlame`) | `flames` [length, width]: side / middle |
|---|---|---|---|
| ROADSTER | ±1.38, −0.79, 2.70: the small slot low on the back of each rear pod | 0, −0.69, 2.59: the dark core of the pink ring | [0.85, 0.25] / [0.85, 0.4] |
| VIPER | ±0.61, −0.885, 2.665: the two side turbines | 0, −0.845, 2.70: the big middle turbine | [0.85, 0.34] / [0.85, 0.6] |
| TITAN | ±0.48, −0.975, 2.64: the two smaller turbines | 0, −0.94, 2.65: the middle turbine's hub | [0.85, 0.25] / [0.85, 0.34] |

All three flames of a car are the same length; the middle one is thicker (`controls.test.mjs` checks both).

- `updateFX` places the nozzles from the **model's** matrix (`car.model`), so the flames follow the hover bob and the
  nitro nose lift. Each flame points out of its nozzle (`n.d`) and starts right at it.
- The ROADSTER's side nozzles sit on its rear pods (`podNozzles: true`): `useModel` stores them in the nearest pod's
  own space (`nozzleMount`, measured against the untilted pod), so the side flames tilt with the pods.
- `car.nozzles` (`{p, d, s, w}`: world position, facing, length scale, width scale) is shared: the nitro ignition
  ring, flash, sparks and burn-out puffs (`nitro.js`), the light ribbons and the damage smoke all start there.
- **Damage smoke** (under 60% health, `stepRacer` in `main.js`): puffs come out of the three nozzles in turn
  (`weapons.exhaustSmoke`), every 0.012 s + health share / 20 (about 30-40 a second when badly damaged), each
  1.3-3.5 m wide, keeping 93% of the car's speed and slowing with air drag (×e^(−1.6 t)), so they stream back from the
  engine. All puffs due in a frame come out, each placed where it would be by now, so the plume stays unbroken at any
  frame rate. Before v0.2.18 the smoke left a fixed point 2.5 m behind the car's centre and stayed where it was let go.

## Mouse aim
`updateAim` in `aim.js` turns mouse motion into steering:
- Each pixel moves the aim by `0.0021 rad × sensitivity` (sensitivity 0.3–2.5).
- The aim may be at most **1.1 rad** left/right of the car's heading and **±0.62 rad** up/down.
- `steer = clamp(−yawDifference × 2.8)`, `climb = clamp(aimPitch / 0.62)` (the nose settles at the aim's height;
  × 1.25 before v0.2.12). Mouse steering sets `inp.aimSteer`: the car turns its nose toward the aim even while
  reversing (`car.update` flips the steering for negative speed only for keys/gamepad; with the mouse that flip made the
  nose turn away from the aim and the car spin at full lock, fixed in v0.2.12).
- Keyboard or gamepad steering overrides the mouse and drags the aim along with the car.
- With RMB held, mouse motion goes to the free-look camera instead and the aim stays put.
- **Mouse-aim camera** (v0.2.12, `g.camAim` while the mouse is captured): `chaseCamera` follows the aim's yaw (and 0.85 ×
  its pitch) instead of the car's, so the aim ring stays near the screen centre; keys, touch, gamepad and the autopilot
  keep the car-following camera.
- Buttons: `Input` re-reads the real button state (`e.buttons`) on every mouse movement while captured, so a release the
  browser never delivered can't leave free look (RMB) or the blaster (LMB) stuck on.

## Boost
| Property | Value |
|---|---|
| Tank drain | 0.32 per second (a full tank lasts **~3.1 s**) |
| Natural refill | 0.06 per second (empty to full in **~17 s**) |
| Checkpoint | +0.30 |
| Takedown | +0.50 |
| Start of a race or mode | full (a respawn keeps the current level) |

On ignition (and not again within 1 s) the main loop (`main.js`) adds the "nitro kick": camera FOV punch, shake, an edge flash,
the **NITRO** call-out and a sound. `nitro` (0..1, eased) drives the flame effects (`nitro.js`), speed lines and FOV.

## Spin dodge
`Car.dodge(side)` (side +1 right, −1 left; `playerWeapons` in `aim.js` picks the steering direction, or random when flying straight):
1. `rollT = 0.55 s`: the model does one full roll around its nose, fastest in the middle (smoothstep).
2. `surge = 2 s`: top speed ×1.5.
3. `speed = max(speed, 47.5 m/s) × 1.5`: always a real burst, even from slow flight.
4. A sideways velocity kick of 22 m/s.

While `car.dodging` (`rollT > 0`): `damage()` (race) and `roamHit()` (FREE ROAM) ignore hits and show **DODGED**.
In FREE ROAM the spin is broadcast as a `spin` fire event; other players see the roll (`barrelRoll`) but no surge or
hop: their game draws your position from the network anyway.

## Network-driven cars
`Car.follow(d, dt)` poses another player's car from interpolated network state
`[x, y, z, yaw, pitch, roll, speed, thrust, nitro]`, so their thrusters, nitro flames and fans animate like a local car
([Online play](07-online.md#movement)).

## KILLER auras (CRIMSON FLAME, VOID CROWN)
`src/killer.js`, `car.setKiller(on, tier)` / `car.isKiller`. One `KillerAura` per car while its player is a Killer
([KILLER STATUS](07-online.md#killer-status)); `car.useModel` re-attaches it to the new model, so switching cars keeps
it.

Two styles (`AURA_STYLES`), one per Killer rank:

| Style | Look | Art |
|---|---|---|
| `crimson` **CRIMSON FLAME** (3+ Killer kills) | Crimson underglow, side flames, pod fire wreaths, orange-red embers | `Assets/06_Effects/car0N_killeraura.png`, textures `killeraura_01..03.webp` |
| `void` **VOID CROWN** (10+ Killer kills) | The same parts in dark purple and neon blue, plus a crystal crown above the roof and crystal shards circling the car | `Assets/06_Effects/killeraura2.png`, textures `voidaura_01..03.webp`, `voidaura_crown.webp` |

**Which one:** `auraStyleFor(kc)`: the VOID CROWN from `VOID_KILLS` (10, `src/online/protocol.js`) Killer kills
(`killerKillCount`), the CRIMSON FLAME before. Our own car reads the count from the profile (`syncKiller`, so the 10th
kill swaps it at once); remote cars get it from `join.kc` and every `kill.kc` (`roamKill` rebuilds the aura when the
rank changes). `/pkclear` resets the count, so the next Killer starts with the CRIMSON FLAME again. Dev: `?mock&killer=void`
gives the mock profile 10 kills.

| Part | Texture (`public/fx`) | Detail |
|---|---|---|
| Underglow ring | `*_01.webp` | Flat under the body, sized from the measured body box (pods left out) |
| Flank tongues | `*_03.webp` | Low on both sides, stretch back and flatten with speed |
| Pod wreaths | `*_02.webp` | A sprite parented to each of the 4 pods, so they follow the pod tilt |
| Crown (void) | `voidaura_crown.webp` | A sprite above the roof (anchored at its bottom edge), hovering and pulsing |
| Shards (void) | geometry | Stretched octahedra with per-face violet / blue / white vertex colours, circling the car on an ellipse (faster with speed), bobbing and spinning |
| Ember trail | `*_03.webp` | From the turbine, billboarded about the car's axis; only while moving |
| Embers | canvas spark (per style) | A recycled `THREE.Points` pool in the scene |

- **VOID CROWN textures** are made by `node tools/fx/voidaura.mjs` (Playwright Chromium): `voidaura_01..03` recolour the flame
  shapes of `Assets/06_Effects/killeraura_01..03.png` from crimson to violet / neon blue; `voidaura_crown` is cut from
  the sheet's implementation-reference panel (on black: brightness becomes alpha, edges faded). The sheet's shard and
  particle tiles sit on a drawn checkerboard, so shards are geometry and sparks a canvas texture instead.
- Per-car sizes in `KILLER_AURA` (`roadster`, `viper`, `titan`): `body`, `flank`, `pod`, `crown` [width × body width,
  height above the roof in m, offset along the car], `shards` [gap beyond the sides in m, × half the body length],
  `trail`.
- **Tiers** (`TIERS`): `high` 28 embers, 7 shards + trail (the player racing), `menu` 14 embers, 7 shards, no trail
  (menu, shop), `remote` 10 embers, 5 shards + trail (other players), `low` no embers, 3 shards, no trail (LOW
  graphics). The crown shows on every tier. `main.js` picks the tier (`playerAuraTier`).
- `car.setKiller(on, tier, style)`: a new tier or style rebuilds the aura.
- Everything is additive, `depthWrite: false`, no lights; textures shared per style, one shard geometry for all;
  materials per aura (each flickers on its own). Speed response is clamped (`speed / 95`, 0–1), so boost doesn't blow
  it up.
- `dispose()` frees the parts, embers and materials; `KillerAura.count` (live auras) is checked by the tests for leaks.
  Remote cars are pooled: `setKiller(false)` when their player leaves.

## Model and colors
- **Three cars** (`CAR_MODELS` in `car.js`), all with the same physics and weapons:

  | Id | Name | File | Notes |
  |---|---|---|---|
  | `roadster` | ROADSTER | `models/car.glb` (5–6 MB) | Free; also the model of rivals (FREE ROAM players show their own shop car since v0.2.4) |
  | `viper` | VIPER | `models/car02.glb` | Shop, 2,500 coins. Meshy multi-image model, built by `meshy/build_game_car.py` |
  | `titan` | TITAN | `models/car03.glb` (7.3 MB) | Shop, 7,500 coins. Meshy model retextured with a text prompt (image-guided texturing lost the neon strips) |

  Each entry holds the URL, a `y` drop (so every underside sits at the same hover height) and attachment points
  measured in Blender: `exhaust` (side flame nozzles, x mirrored), `turbine` (centre + radius of the glowing ring),
  `tail` (tail-light ribbons) and `podTilt`; since v0.2.18 also `turbineFlame` (middle flame nozzle) and `flames`
  ([length, width] scale of the side and middle flames), see [Thruster flames](#thruster-flames-v0218). `loadCarModel(url, envMap, onProgress, spec)` stores the spec in `userData.spec`, which
  `car.useModel` reads. `loadCar(id)` in `main.js` loads a car the first time it's shown and forgets a failed load so
  it can retry. The player shows `shop.previewId ?? shop.equipped` (`paintPlayer`).
- The ROADSTER is loaded at start-up into a **high** version (the
  player) and a **low** version (rivals and remote players). Until it loads, a **code-built** faceted car is used: a
  hull lofted through cross-sections, four thruster pods, a nose cannon, magenta and cyan light lines.
- **Colors:** 11 choices: ORIGINAL (paint `#1a1030`, accent `#ff26d4`) plus the 10 rival liveries (`LIVERIES` in
  `ai.js`, each with a paint color, an accent and for some a metalness). `liveryModel` repaints a copy of the model.
  The accent colors the glow, nozzle and the car's projectiles.
- The player's choice is `localStorage['neon.paint']` (−1 … 9) and is sent to the FREE ROAM server, so everyone sees it.
- `car.muzzle(side)` is where shots start, in the world: the model's own front-bumper point (`CAR_MODELS[id].muzzle`,
  v0.2.14), `side` −1 / +1 for the blaster's left / right bolts, 0 for seekers. The code-built car shown until a
  model loads keeps its old cannon under the nose, 1.3 m to each side.
