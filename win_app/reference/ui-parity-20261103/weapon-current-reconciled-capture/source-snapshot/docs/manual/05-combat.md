# 5. Combat

✅ **Done** · [Manual contents](README.md) · Previous: [Flying](04-flying.md) · Next: [Free Roam online](06-free-roam.md)

🎬 **Video:** [02_Combat.mp4](<../../../tiktok videos/02_Combat.mp4>) (recorded before v0.1; spin dodge isn't in it).

Every car carries four tools. The attack panel (bottom right) shows their state.

| Weapon | Key | Phone | Gamepad | Good for |
|---|---|---|---|---|
| **Pulse blaster** | hold left mouse / J | FIRE | RB | Steady damage up close |
| **Seeker missile** | Space / middle mouse | SEEKER | X | Big hits on a locked target |
| **EMP** | X / V | EMP | LB | Everyone around you at once; wiping incoming shots |
| **Spin dodge** | F | SPIN | B | Escaping: no damage during the roll, then extra speed |

## Pulse blaster
Hold to fire a stream of bolts from alternating sides of your car's front bumper (about 5 a second). The bolts are
long electric streaks in **your car's paint colour**, and reach about **1.1 km** (further while you're moving).

- Each bolt does **21 damage**: **5 bolts wreck** a car (everyone has 100).
- The blaster **heats up**: the **PULSE** bar fills, and after about **22 bolts** (4.6 seconds of non-stop fire) it **overheats**.
  It then can't fire until the bar has cooled to a quarter. Fire in bursts to avoid overheating.
- With a **lock** (below), bolts aim ahead of the target to meet it, and curve slightly toward it.

## Seeker missiles and lock-on
### Locking on
Point your aim dot at a car ahead of you (within about **2 km**). A **bracket** appears on it, spins and closes in:
**LOCKING VOLT 212m**. After **half a second** it turns red: **LOCKED**. A tone plays when the lock starts and
another when it completes. The lock holds while the target stays roughly in front of you; turn too far away and it
drops.

![Lock-on](../media/v0.1/combat-lock-on.jpg)

### Firing
- You carry **3 seekers** (the pips under the **SEEKER** ring). A used one recharges in **4.5 seconds**.
- With a **full lock**, the missile homes in hard on the target. Each hit does **48 damage**: about half a rival's health.
- Without a full lock it flies straight (**NO LOCK · DUMBFIRE** / **NO TARGET · DUMBFIRE**).
- With no seekers left, you hear a "denied" buzz.

## EMP
A shockwave around your car, about **95 m** across in every direction.

- Damages every car in range: **25** at the center, down to 12 at the edge.
- Shoves cars away and stuns them briefly. Rivals hit by it also hold their fire for 3 extra seconds.
- **Destroys incoming enemy shots** inside the wave: *EMP · 2 HITS · 3 SHOTS NEUTRALISED*.
- Recharges in **11 seconds** (the **EMP** ring fills up).

## Spin dodge
Press **F** for a fast **barrel roll** with a **sideways hop**, then a burst of speed.

- **While rolling (about half a second), no shot can hurt you.** Shots that reach you show **DODGED**.
- You hop sideways in the direction you're steering (or a random side if you fly straight).
- For **2 seconds** after the roll, your speed and top speed are **1.5×**: *SPIN · 1.5× SPEED*.
- Recharges in **6 seconds** (the **SPIN** ring, labelled *DODGE · F*).
- Time it: spin just before a seeker reaches you, or to get out of a blaster stream.

🎬 *To add: a short clip of a spin dodge taking a seeker hit (`docs/media/v0.1/combat-spin-dodge.mp4`).*

## Shield and wrecks
Your **SHIELD** bar (bottom left) is your health: 100 at the start.

| Hit on you | Damage | Effect |
|---|---|---|
| Rival's shot (RACE) | 30 | The 4th hit wrecks you |
| Blaster bolt (FREE ROAM) | 21 | Small knock; the 5th wrecks you |
| Seeker (FREE ROAM) | 48 | Big knock, stunned for a second |
| EMP (FREE ROAM) | 12–25 | Pushed away, stunned |

- Hits flash the screen red, shake the camera and cost speed.
- Below 60% shield your car **trails smoke**. The bar blinks under 35%.
- At 0 you are **WRECKED** (*BY VOLT*): the car explodes, the camera pulls back, and after **2.4 seconds** you
  respawn at your last checkpoint with a full shield (FREE ROAM: after **3 seconds**, at your spawn point).
- In RACE your shield is refilled only by a wreck; in FREE ROAM it regenerates
  ([Free Roam](06-free-roam.md#health)).

## Takedowns
Wrecking a car is a **TAKEDOWN**. Rewards:
- **+50% boost** and **+1 seeker**.
- In RACE: you **steal 25% of the rival's coins**: *VEX WRECKED · +BOOST +SEEKER · +3 COINS*.
- Takedowns within 8 seconds of each other build a streak: **DOUBLE TAKEDOWN**, **TRIPLE TAKEDOWN**, **4x RAMPAGE**…
- Your takedown count appears on the results screen. In FREE ROAM, takedowns are **kills**.

## How rivals fight (RACE)
- Rivals don't shoot in the first 6 seconds of a race.
- Each rival fires roughly every 4 to 8 seconds at a car 15 to 230 m in front of it, usually another rival, sometimes you.
- A rival is wrecked by 2 hits from other rivals, 5 of your bolts or 3 of your seekers (or a mix).
- Wrecked rivals return 2.4 seconds later at their last checkpoint.
