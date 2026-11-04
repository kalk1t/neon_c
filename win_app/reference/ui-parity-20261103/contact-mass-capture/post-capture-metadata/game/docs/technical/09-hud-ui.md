# 9. HUD and UI

✅ **Done** · [Technical contents](README.md) · Previous: [Accounts and data](08-accounts-data.md) · Next: [Running the server](10-operations.md)

All screens are HTML/CSS over the WebGL canvas (`index.html`, `src/style.css`); `src/hud.js` updates the in-race HUD
every frame. Fonts: Orbitron (headings, numbers) and Rajdhani (text).

## Screens
| Element | Shown | Contents |
|---|---|---|
| `#loading` | start-up | Title, progress bar, status text |
| `#menu` | `menu` | Mode buttons, ranking, car color, best lap, last FREE ROAM tally, settings, controls, system requirements |
| `#account` | menu | Sign-in button or account panel (nickname, wallet, world rank, kills, kill rank, EDIT, SIGN OUT) |
| `#site-foot` | menu | Contact, Privacy, Terms, Analytics settings |
| `#roads` | RACE / TIME ATTACK clicked (or Enter on the menu) | Road picker (`openRoads` in main.js): nine roads with length, gates, difficulty bars, best lap or lock; BACK / Esc closes |
| `#ranking` | on demand | Tabs COIN · TIME · KILLS; under TIME a road row 1-9 (`#rank-roads`, `account.rankRoad`); top 10, your position |
| `#shop`, `#shop-dialog` | `shop` | The shop (below) and its confirm / success dialogs |
| `#cockpit` | cockpit view while racing | The cockpit dashboard (below), placed just before `#hud` so the HUD draws over it |
| `#nick-modal` | on demand | Nickname form with validation errors |
| `#consent` | first visit | Analytics consent: ALLOW / NO THANKS |
| `#sysreq-panel` | on demand | Minimum specs and the device check |
| `#hud` | countdown, race, finished | The in-race HUD (below) |
| `#pause` | paused | RESUME, RESTART, MAIN MENU, contact; the car showcase behind |
| `#results` | 2 s after finishing | Place, time, laps, standings, coin race, online save status |
| `#rotate-note` | phone held upright | *TURN YOUR PHONE* |
| `#fatal` | fatal errors | Title and message |
| `#touch-ui` | phones while playing | Touch buttons (created by `touch.js`) |

## In-race HUD
![Race HUD](../media/v0.1/race-hud.jpg)

| Element | Position | Shows |
|---|---|---|
| **POS** `#pos-panel` | top left | Position / racers (RACE only) |
| Coins `#coin-panel` | top left | Coins and coin-race place (RACE) |
| **LAP TIMES** `#laps-panel` | left | Completed laps, best lap |
| Lap panel | top center | LAP n/2, current lap time, CHECKPOINT n/8 |
| Minimap `#minimap` | top right | *SKYLINE LOOP*: the loop, rings, racers. FREE ROAM: *SKYLINE CITY*, a city map with the towers, the hub and the players |
| Speed `#bar .speed` | bottom left | km/h or mph (`#speed-unit`, Settings **SPEED**), **BOOST** bar (pulses when on, blinks when low), **SHIELD** bar (blinks under 35) |
| Attack panel | bottom right | **PULSE** heat bar, **SEEKER** ring + 3 pips (ring fills with the next charge), **EMP** ring, **SPIN** ring |
| Crosshair `#crosshair` | where the nose points | Where the car is heading (projected 90 m ahead). With the mouse captured only a small dot (`.nose`) |
| Aim `#aim` | near the center | The mouse aim ring (only while the mouse is captured; the camera follows it, v0.2.12) |
| Lock `#lock` | on the target | Bracket that closes and rotates in while locking; *LOCKING / LOCKED name distance* |
| Hit marker `#hitmark` | on the aim | Flashes on a hit, bigger on a wreck |
| Next target `#target` | on the next ring | Diamond with distance; off-screen it becomes the edge arrow `#edge-arrow` |
| Name tags `#tags` | over cars | Rivals: rank, name, coins, health bar; visible within 320 m, fading with distance. Other players: name, distance, health bar, at any distance |
| Messages `#msg`, `#submsg` | upper center | Big call-outs (3-2-1, TAKEDOWN, WRECKED) and smaller lines (CHECKPOINT, HIT BY…) |
| `#nitro-call` | lower center | NITRO call-out on ignition |
| `#flash` | full screen | Colored edge flash: hits red, boost/spin cyan |
| `#lockhint` | center | *CLICK TO CAPTURE MOUSE* while racing without the mouse captured |

### HUD scaling
`style.css` shrinks the HUD with CSS `zoom`: **0.8** on screens ≤1180 px wide or ≤720 px tall, **0.55** on phones held
sideways (≤520 px tall). Because `zoom` also scales `left`/`top`, everything placed over the 3D view (crosshair, aim,
lock, tags, markers) divides its screen position by the zoom (`hud.z`). (This was the cause of the "left-shifted
crosshair" bug fixed in v0.1.)

## FREE ROAM HUD
![Free roam](../media/v0.1/free-roam.jpg)

With `body.roam`, CSS hides POS, coins, lap times, the lap panel, the target marker and the edge arrow, and shows:

| Element | Contents |
|---|---|
| `#live-panel` | Status (*ONLINE · N DRIVERS* with a green dot, or the connection message), `#live-stats` (FPS · MS · ONLINE), `#live-list` (roster: everyone in their car color, sorted by session kills, you marked) |
| `#feed` | Kill feed (top center), last 4 lines, each fades after 4.5 s |
| `#chat` | Chat log + input on the left, above the speed panel. Phones: above BRAKE; while typing it moves to the top, clear of the keyboard |

## Touch UI
Built by `touch.js` into `#touch-ui` (above the HUD). Buttons: BRAKE, BOOST, FIRE (hold), SEEKER, EMP, SPIN (tap),
II, ◎ TILT, CHAT (FREE ROAM only). The left half of the screen is a drag joystick (radius 60 px). The speed panel
moves to the bottom center, clear of the thumbs.

Tilt maths: the gravity direction from `deviceorientation` is converted into screen space for the current rotation,
giving a **wheel angle** (steer: full at 24°, dead zone 2.5°) and a **pitch** relative to the angle at calibration
(climb: full at 18°). Tilt readings older than 1.5 s are ignored. `enable()` must run inside a tap: it asks iOS for
motion permission, enters full screen and locks landscape where allowed.

A phone layout switches to desktop as soon as a real mouse moves or a key is pressed (touchscreen laptops).

## Shop
`src/shop.js` (`Shop` class), markup `#shop` in `index.html`, styles at the end of `style.css`. The car list is
`SHOP_CARS` in `src/online/index.js` (id, name, price, description); the server's prices are `SHOP_PRICES` in
`functions/rules.js`, and the two must match.

- **Card status** per car: `equipped`, `owned`, `sale` (affordable), `expensive` (shows *NEED n MORE*), `signin`,
  `closed` (backend without `buyCar`: *PURCHASES OPEN SOON*). The big action button follows the selected card.
- **Stage:** selecting a card sets `shop.previewId`; `main.js` loads and shows that car on the showcase camera
  (`#shop-stage` takes drag and wheel). Closing returns to the equipped car.
- **Buying:** confirm dialog (wallet before → after), `account.buyCar(id)` → the backend's `buyCar`, then a success
  dialog with **EQUIP NOW / LATER**. A second click while buying is ignored. Errors map to readable text (funds,
  owned, signed out, offline).
- **Equipped car:** `localStorage['neon.car']`, used only if the profile owns it (`profile.cars`), else ROADSTER.
- **Stats** (`#shop-stats`, `statRows` in `shop.js`, v0.2.17): bars for TOP SPEED, ACCELERATION (time to 90%), HANDLING
  and ARMOR against the best of the three cars, with the value (speed in the chosen unit), then the car's traits in
  gold (light / heavy, spin cooldown, seekers, EMP reach; the ROADSTER: *ALL-ROUNDER*).
- **NEW tag:** `SHOP_NEWS` (car id) marks the newest car.
- Keyboard: Esc closes (or closes the dialog), arrows move between cards. Phones: cards in a row along the bottom.

## Cockpit view
`src/cockpit.js` (`Cockpit` class, instance `dash` in `main.js`). Art in `public/cockpit/`: per car a
`base.webp` (1672 × 941 dashboard with a transparent windshield) and control layers, plus shared icons. Converted
from the packs in `Assets/01_Vehicle/carNN_interface` (PNG → WebP, cropped to their opaque content; ~1 MB in total).

| Car | Pack | Controls |
|---|---|---|
| ROADSTER | `car01` Flight Deck | Squared yoke (rotates up to 16°) |
| VIPER | `car02` Twin Controls | Left and right side grips (8°) |
| TITAN | `car03` Armored Sport | Round wheel (16°), dim reticle in the left dial |

- **Layout:** `LAYOUTS` gives every element a box `[x, y, w, h]` as fractions of the dashboard. `fit()` covers the
  screen (wide screens crop sky, narrow ones the sides) and sinks `DROP` (20%) of the dashboard below the bottom
  edge, so the windshield is larger. Font size = dashboard width / 100.
- **Read-outs** (`dash.update` every frame while playing; only changed values are written): speed (km/h or mph, `dash.setUnit`), BOOST %
  and bar, PULSE (READY / HEAT n% above 60% / OVERHEAT), lap (`LAP n/2`, `LAP n` in Time Attack, FREE ROAM) and the
  lap time (FREE ROAM: time since joining). The controls rotate with the eased steering input.
- **Lifecycle:** `dash.setCar(shop.equipped)` at every `startRace`; a car's art loads the first time its cockpit is
  shown. `body.interior` hides the bottom speed and lap panels. `#view-hint` (desktop) shows *I · COCKPIT VIEW* /
  *I · EXTERIOR VIEW*; phones get the **VIEW** touch button (taps `KeyI`).

## System requirements
`src/sysreq.js`, called once the renderer exists. Reads the GPU name through `WEBGL_debug_renderer_info` and
shortens ANGLE strings (*ANGLE (NVIDIA, NVIDIA GeForce RTX 3060 (0x…) Direct3D11…)* → *NVIDIA GeForce RTX 3060*).
Checks:

| Check | Passes when |
|---|---|
| WebGL 2 | always (the game wouldn't run otherwise) |
| GPU | the renderer string doesn't contain SwiftShader, llvmpipe, "software" or "Basic Render" (= no hardware acceleration) |
| CPU | `navigator.hardwareConcurrency` ≥ 2 (or unknown) |
| Memory | `navigator.deviceMemory` ≥ 4 GB (or unknown; only Chrome/Edge report it, capped at 8) |

All pass → *✔ YOUR DEVICE IS READY*, else *⚠ CHECK YOUR SETUP*. The minimum list is the `MIN` table in the file.

## Main menu layout
The menu card scrolls when it is taller than the window (`align-items: safe center`). On desktop screens up to 820 px
tall, spacing, the title and the buttons are tightened so it usually fits without scrolling. On phones a separate
compact layout applies.

## Sound
`src/audio.js` synthesizes everything with the Web Audio API (oscillators and filtered noise, no audio files): engine
hum (pitch with speed, louder on boost), countdown, checkpoint, lap, coin, boost on/off, blaster, overheat, missile,
EMP, deny, lock tick and lock tone, rival zaps, hit marker, impacts, damage, explosions. Distant events are quieter and
panned left/right from the camera (`volAt`, `panAt`). Audio starts on the first click (browser rule).
