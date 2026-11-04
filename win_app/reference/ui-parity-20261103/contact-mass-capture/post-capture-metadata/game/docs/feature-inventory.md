# Feature inventory (v0.2)

🔄 **Changing** · Everything Roadsters Race does in version 0.2 (in testing), in one flat list. Each line links to where it is
explained. *Player* = Player Manual, *Tech* = Technical Reference.

## Platform
| Feature | Player | Tech |
|---|---|---|
| Runs in the browser (WebGL 2), nothing to install | [Getting started](manual/01-getting-started.md) | [Overview](technical/01-overview.md) |
| Desktop (mouse + keyboard), phone (tilt + touch buttons) and gamepad | [Controls](manual/02-controls.md) | [HUD and UI](technical/09-hud-ui.md) |
| System requirements panel with a device check on the main menu | [Getting started](manual/01-getting-started.md#system-requirements) | [HUD and UI](technical/09-hud-ui.md#system-requirements) |
| Graphics quality HIGH / LOW, automatic LOW on slow devices and phones | [Settings](manual/08-settings.md) | [Overview](technical/01-overview.md#rendering) |
| Loading screen with car-model progress; built-in fallback car after 20 s | – | [Overview](technical/01-overview.md#loading) |
| Error screens: no WebGL 2, graphics driver reset, start-up failure | [Troubleshooting](manual/09-troubleshooting.md) | [Game loop](technical/02-game-loop.md#errors) |
| Privacy policy, terms, analytics consent banner | [Settings](manual/08-settings.md#privacy) | [Accounts and data](technical/08-accounts-data.md#analytics) |

## Modes
| Feature | Player | Tech |
|---|---|---|
| RACE: 2 laps against 10 AI rivals, countdown, positions, results | [Game modes](manual/03-game-modes.md#race) | [Game loop](technical/02-game-loop.md), [AI rivals](technical/05-ai-rivals.md) |
| TIME ATTACK: solo laps, endless, best lap saved | [Game modes](manual/03-game-modes.md#time-attack) | [Game loop](technical/02-game-loop.md) |
| FREE ROAM: online shared city for up to 24 signed-in players | [Free Roam](manual/06-free-roam.md) | [Online play](technical/07-online.md) |
| Main menu car showcase (turntable, drag to orbit, scroll to zoom) | [Getting started](manual/01-getting-started.md#the-main-menu) | [Game loop](technical/02-game-loop.md#cameras) |
| Pause screen car showcase | [Flying](manual/04-flying.md#pause) | [Game loop](technical/02-game-loop.md#cameras) |

## Flying
| Feature | Player | Tech |
|---|---|---|
| Arcade flight: thrust, brake/reverse, steer, climb and dive | [Flying](manual/04-flying.md) | [Car](technical/04-car.md) |
| Mouse aim (the car steers toward the reticle) | [Flying](manual/04-flying.md#steering) | [Car](technical/04-car.md#mouse-aim) |
| Boost (nitro) with a refilling tank and visual effects | [Flying](manual/04-flying.md#boost) | [Car](technical/04-car.md#boost) |
| Spin dodge: barrel roll, sideways hop, 1.5× speed surge | [Combat](manual/05-combat.md#spin-dodge) | [Car](technical/04-car.md#spin-dodge) |
| Cockpit view (I / VIEW): per-car dashboard with live speed, boost, blaster heat, lap and time | [Flying](manual/04-flying.md#cockpit-view) | [HUD and UI](technical/09-hud-ui.md#cockpit-view) |
| 3 cameras (near chase, far chase, first person) and RMB free look | [Flying](manual/04-flying.md#cameras) | [Game loop](technical/02-game-loop.md#cameras) |
| Reset (R), out-of-bounds and wrong-way warnings | [Flying](manual/04-flying.md#reset-and-warnings) | [World](technical/03-world.md#race-rules) |
| Collisions with buildings, props and other cars | [Flying](manual/04-flying.md) | [World](technical/03-world.md#collisions) |
| 3 cars: ROADSTER (free), VIPER, TITAN; each with its own stats (v0.2.17) | [Progress](manual/07-progress.md#shop) | [Car](technical/04-car.md#model-and-colors) |
| 11 car colors, also shown to other players online | [Settings](manual/08-settings.md#car-color) | [Car](technical/04-car.md#model-and-colors) |

## Combat
| Feature | Player | Tech |
|---|---|---|
| Pulse blaster (hold, overheats) | [Combat](manual/05-combat.md#pulse-blaster) | [Weapons](technical/06-weapons-damage.md#pulse-blaster) |
| Seeker missiles (3 charges) and lock-on | [Combat](manual/05-combat.md#seeker-missiles-and-lock-on) | [Weapons](technical/06-weapons-damage.md#seeker-missiles) |
| EMP shockwave (area damage, clears incoming shots) | [Combat](manual/05-combat.md#emp) | [Weapons](technical/06-weapons-damage.md#emp) |
| Shield (health), smoke when damaged, wrecks and respawn | [Combat](manual/05-combat.md#shield-and-wrecks) | [Weapons](technical/06-weapons-damage.md#damage-and-wrecks) |
| Takedown rewards and streaks (DOUBLE, TRIPLE, RAMPAGE) | [Combat](manual/05-combat.md#takedowns) | [Weapons](technical/06-weapons-damage.md#takedowns) |

## Race content
| Feature | Player | Tech |
|---|---|---|
| Nine roads on one map (5.64 – 10.40 km a lap, 8 – 19 gates), each longer and harder; road picker | [Game modes](manual/03-game-modes.md#the-roads) | [World](technical/03-world.md#roads) |
| Road unlocks: win a road in 1st place to open the next (saved on the account) | [Progress](manual/07-progress.md#unlocking-roads) | [World](technical/03-world.md#one-city-nine-roads) |
| 10 named AI rivals with liveries, skill levels and their own fights | [Game modes](manual/03-game-modes.md#race) | [AI rivals](technical/05-ai-rivals.md) |
| Coins along the track, coin race and podium bonus | [Progress](manual/07-progress.md#coins) | [World](technical/03-world.md#coins) |
| Procedural neon city, flying traffic, clouds, animated ad screens | – | [World](technical/03-world.md#city-and-scenery) |

## Online
| Feature | Player | Tech |
|---|---|---|
| Google sign-in and unique nickname | [Getting started](manual/01-getting-started.md#signing-in) | [Accounts](technical/08-accounts-data.md) |
| Shop: buy cars with wallet coins (server-checked purchase), equip, preview on a stage | [Progress](manual/07-progress.md#shop) | [Accounts](technical/08-accounts-data.md#shop-purchases), [HUD and UI](technical/09-hud-ui.md#shop) |
| Server-checked race saving (race tickets, Cloud Functions) | [Progress](manual/07-progress.md#wallet) | [Accounts](technical/08-accounts-data.md#saving-a-race) |
| Coin wallet and lifetime coins saved online | [Progress](manual/07-progress.md#wallet) | [Accounts](technical/08-accounts-data.md#saving-a-race) |
| World Ranking: COIN, TIME (one ranking per road), KILLS | [Progress](manual/07-progress.md#world-ranking) | [Accounts](technical/08-accounts-data.md#ranking-queries) |
| FREE ROAM PvP: hits, wrecks, kills per session, lifetime kills | [Free Roam](manual/06-free-roam.md#fighting-and-kills) | [Online play](technical/07-online.md#hits-and-kills) |
| FREE ROAM roster, kill feed, name tags with health bars | [Free Roam](manual/06-free-roam.md#the-free-roam-screen) | [HUD and UI](technical/09-hud-ui.md#free-roam-hud) |
| FREE ROAM chat | [Free Roam](manual/06-free-roam.md#chat) | [Online play](technical/07-online.md#chat) |
| KILLER STATUS: CRIMSON FLAME aura after 3 player kills, VOID CROWN after 10, R lock, `/pkclear` (first chat command) | [Free Roam](manual/06-free-roam.md#killer-status) | [Online play](technical/07-online.md#killer-status), [Car](technical/04-car.md#killer-auras-crimson-flame-void-crown) |
| FPS, ping and time-online display | [Free Roam](manual/06-free-roam.md#fps-ping-and-time-online) | [Online play](technical/07-online.md#ping) |
| Online player count on the menu | [Game modes](manual/03-game-modes.md#free-roam) | [Online play](technical/07-online.md#endpoints) |
| Outdated tabs reload themselves after a game update | [Troubleshooting](manual/09-troubleshooting.md) | [Online play](technical/07-online.md#protocol-versions) |

## Not in v0.2
Other players' shop cars in FREE ROAM, paints or trails in the shop, private lobbies, voice chat, chat moderation
tools, more tracks, server-checked kills. See [Roadmap](technical/11-known-issues-roadmap.md#roadmap).
