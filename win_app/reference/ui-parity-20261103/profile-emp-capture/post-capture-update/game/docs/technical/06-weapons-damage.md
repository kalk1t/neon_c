# 6. Weapons and damage

✅ **Done** · [Technical contents](README.md) · Previous: [AI rivals](05-ai-rivals.md) · Next: [Online play](07-online.md)

Tuning: `WEAPON` in `src/ai.js`, `RIVAL_AIM` in `src/race.js`; `KNOCK`, `ROAM_KNOCK` and the weapon code (`playerWeapons`, `damage`,
`wreck` in `src/race.js`, `roamHit` in `src/roam.js`; the player's weapons in `src/aim.js`); projectiles in `src/fx.js`.

## Projectiles (`Weapons` in `fx.js`)
Every shot is a camera-facing glowing quad with: owner, target (or none), velocity, acceleration, max speed, homing
turn rate, lifetime and hit radius. Each frame:
1. **Homing:** with a live target that is still ahead of the shot (or always, for missiles), the velocity turns
   toward it: `dir.lerp(toTarget, 1 − e^(−turn·dt))`.
2. **Move**, then test the **segment traveled this frame** against each racer's position (closest point on the
   segment within the hit radius), so fast shots can't skip through a car.
3. Hit a car → impact effect + `onHit(shot, victim)`. Hit the world (any collider) or below 24 m → impact. Out of
   lifetime → gone.
4. Shots grow slightly with distance from the camera to stay visible (looks only; the hit radius doesn't change).

`purge(pos, radius, owner)` deletes other owners' shots in a sphere (EMP).

## Pulse blaster
| Property | Value | Constant |
|---|---|---|
| Damage per bolt | **21** (5 wreck a car; 6 before v0.2.14) | `WEAPON.BOLT_DMG` |
| Fire interval | 0.21 s (~4.8 per second; 0.09 s before v0.2.14), alternating left/right of the front bumper | `BOLT_RATE` |
| Muzzle | per car model, on its front bumper (`CAR_MODELS[id].muzzle`, x mirrored; `car.muzzle(side)`): roadster ±0.7 / −0.74 / −2.9, viper ±0.25 / −1.22 / −2.85, titan ±0.7 / −1.0 / −2.9 (x / y / z, car space, nose toward −z) | `muzzle` |
| Speed | 750 m/s + the car's speed (430 before v0.2.14) | `BOLT_SPEED` |
| Life / range | 1.5 s (1.3 before): ~1,125 m standing still, more at speed | `BOLT_LIFE` |
| Homing / hit radius | turn 2.5, 3.6 m | – |
| Look | a streak 45 m long and 7 m wide (24 / 5 before) that grows out of the muzzle, in the shooter's paint colour (lightened, ×3 bright); the muzzle flash too | `BOLT_LEN`, `BOLT_WID` |
| Heat per bolt | +0.08 (overheats after ~22 bolts, ~4.6 s; 0.06 / ~17 bolts / ~1.5 s before) | `HEAT_PER_BOLT` |
| Cooling | 0.45 per second; ×0.35 while still firing; ×1.4 while overheated | `COOL_RATE` |
| Overheat ends | when heat ≤ 0.25 | – |
| Spread | ±0.004 random per axis | – |
| With a lock | aims where the target will be (`leadDir`: 90% of its velocity × flight time) and homes on it | – |

## Seeker missiles
| Property | Value | Constant |
|---|---|---|
| Damage | **48** | `MISSILE_DMG` |
| Charges | 3; one recharges every **4.5 s** (continuous, fractional) | `MISSILES`, `MISSILE_RECHARGE` |
| Launch | from the nose, aim direction tilted up 0.08 | – |
| Speed | starts at 60 m/s + car speed, accelerates 420 m/s² up to car speed + 330 m/s | – |
| Homing | turn **7** with a full lock (and keeps homing even when the target is behind); **0** without (dumbfire) | – |
| Life / hit radius | 3.6 s, 5.5 m, smoke trail | – |

### Lock-on (`updateLock`)
| Step | Rule |
|---|---|
| Candidates | Rivals (RACE) or other players (FREE ROAM) that are alive, 8–2000 m away (`LOCK_RANGE`; 400 before v0.2.14) |
| Acquire | the smallest angle between the **aim direction** and the target, if under **0.3 rad** (17°) |
| Keep | while under **0.5 rad** (29°) and within 2100 m (`LOCK_KEEP`; 460 before; sticky lock) |
| Lock time | `lockT` rises from 0 to 1 over **0.5 s** (`LOCK_TIME`); a new target restarts it |
| Sounds | a tick on acquire, a tone at full lock |
| Missile | homes only if `lockT ≥ 1` at launch |

## EMP
| Property | Value | Constant |
|---|---|---|
| Radius | 95 m | `EMP_RADIUS` |
| Damage | `25 × (1 − 0.5 × distance/radius)`: 25 at the center, 12.5 at the edge | `EMP_DMG` |
| Cooldown | 11 s | `EMP_COOLDOWN` |
| Push | `30 × (1 − d/R) + 8` m/s away from you (8–38) | – |
| Other | deletes enemy shots in range; rivals hit wait 3 s more before firing | – |
| Keys | X or V | – |

## Damage and wrecks
### RACE (`damage`)
| Hit | Damage | Speed kept | Stun |
|---|---|---|---|
| Your bolt | 21 | 98.5% | 0.12 s |
| Your seeker | 48 | 50% | 1.0 s |
| Your EMP | 12.5–25 | 65% | 1.6 s |
| Rival bolt on you | 21 (v0.2.15; a homing shot of 30 before) | 98.5% | 0.12 s |
| Rival bolt on a rival | 21 (51 before v0.2.15) | 98.5% | 0.12 s |
| Rival seeker (v0.2.16) | 48 | 50% | 1.0 s |
| Rival EMP (v0.2.16) | 12.5–25 | 65% | 1.6 s |

### FREE ROAM (`roamHit`, applied by the victim's own game)
| Hit | Damage | Speed kept | Stun | Push |
|---|---|---|---|---|
| Bolt | 21 | 90% | 0.25 s | 6 m/s |
| Seeker | 48 | 45% | 1.0 s | 26 m/s |
| EMP (×k = 1 − 0.5 d/R) | 25k | 1 − 0.4k | 1.2k s | 34k m/s |

Health is the car's own (`CAR_STATS`, v0.2.17): ROADSTER **100**, VIPER **75**, TITAN **140** (rivals 100). Speed lost,
push and stun from every hit are scaled by the victim's `knock` / `stun` ([Car](04-car.md#shop-cars-car_stats-v0217)). Stun blocks boosting and wobbles the car. No damage is taken while **dodging**
(`car.dodging`), in **spawn protection** (FREE ROAM, 3 s after a spawn) or when not in the `race` state.

### Wreck (`wreck`)
At 0 health: explosion, the car hides, speed and velocity zeroed, `wreck = 2.4 s` (FREE ROAM: `ROAM_WRECK` = **3 s**, counted down on screen),
then `respawn`. The lock is dropped if it was on that car. Under 60% health a car trails smoke out of its engine nozzles (more as health falls; [Car](04-car.md#thruster-flames-v0218)).

## Takedowns
When the player wrecks a car:
| Reward | Value |
|---|---|
| Boost | +0.5 |
| Seeker | +1 (up to 3) |
| Coins (RACE) | `ceil(25% of the victim's coins)` move to the player |
| Streak | another takedown within **8 s** of race time: TAKEDOWN → DOUBLE → TRIPLE → *n*x RAMPAGE |

In FREE ROAM the same rewards apply when the server confirms the kill (`roamKill`), and the kill is banked
([Accounts](08-accounts-data.md#free-roam-kills)).

## Tuning cheatsheet
| To change | Edit |
|---|---|
| Any weapon number | `WEAPON` in `src/ai.js` (bolt speed and life also in `server/hits.js`: `tests/constants.test.mjs` checks they match) |
| Where shots leave a car | `muzzle` in `CAR_MODELS` (`src/car.js`) |
| Bolt colour | `shotColor` / `shotMat` in `showPaint` (`src/main.js`) |
| Shop car differences | `CAR_STATS` in `src/car.js` |
| Rival weapons | the player's `WEAPON` values; when rivals use them: `RIVAL_AIM` in `src/race.js` (`CONE` 35°, `SLOWER` bolt rate ×3, `SPREAD` aim error, `SEEKER_RANGE` 600 m, `SEEKER_GAP` 1.5 s, `DODGE` 50%, `EMP_CARS` 2) |
| Rival speed | `RIVAL_SPEED` in `src/roads.js`; corners `CORNER` in `src/ai.js` |
| Knock-back | `KNOCK` in `src/race.js` / `ROAM_KNOCK` in `src/roam.js` |
| Spin dodge | `SPIN` in `src/car.js` |
| FREE ROAM wreck / protection / regen | `ROAM_WRECK`, `ROAM_SAFE`, `ROAM_REGEN` in `src/roam.js` |

After changing FREE ROAM behavior that other players' games must agree on, bump `LIVE_PROTOCOL`
([Online play](07-online.md#protocol-versions)).
