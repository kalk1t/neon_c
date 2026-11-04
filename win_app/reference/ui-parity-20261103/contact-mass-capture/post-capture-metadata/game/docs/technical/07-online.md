# 7. Online play (FREE ROAM)

🔄 **Changing** · [Technical contents](README.md) · Previous: [Weapons and damage](06-weapons-damage.md) · Next: [Accounts and data](08-accounts-data.md)

Files: `server/live.js` (relay), `src/online/live.js` (client, `LiveRoom`), `src/online/protocol.js` (shared version),
FREE ROAM glue in `src/roam.js` (`startRoam`, `updateRoam`, `roamHit`, `roamKill`, `remoteFire`).

## Design in one paragraph
The server is a **relay**, not a game simulation. Each browser simulates its own car and sends its state about
15 times a second; the server forwards everyone's latest state to everyone 15 times a second and passes on events
(shots, hits, wrecks, chat). Each browser draws the other cars **150 ms in the past**, interpolating between
snapshots for smooth motion. **The shooter decides hits** (on the cars as drawn on their screen) and tells the victim;
the **victim applies** the damage (with its own spawn protection and spin dodge) and reports its own wreck, and the
**server credits the kill**. Nothing about positions is stored.

```mermaid
sequenceDiagram
  participant A as Shooter's game
  participant S as Live relay (server/live.js)
  participant B as Victim's game
  A->>S: f (bolt: origin, direction)
  S->>B: f (draw the bolt; looks only)
  Note over A: A's bolt meets B's car as drawn on A's screen
  A->>S: hit (v = B, k = bolt, direction)
  S->>B: hit (from A)
  Note over B: roamHit: protection? dodging? → damage, knock-back
  Note over B: health reaches 0 → wreck, respawn in 3 s
  B->>S: die (by = A)
  S->>A: kill (k = A, v = B, n = A's session kills, kc = A's killer count)
  S->>B: kill
  Note over A: takedown rewards, account.addKills(1)
```

## Endpoints
In production (since v0.2.4) `server/serve.mjs` passes both to the relay, which runs as **its own process**
(`server/relay.mjs`, `127.0.0.1:8091`): a relay crash restarts only the relay, never the website. On the dev server
`livePlugin` adds them to Vite itself.

| Endpoint | Use |
|---|---|
| `GET /live/count` | `{ "online": n }`: the menu polls it every 10 s for *N DRIVING NOW* |
| `WebSocket /live` | The FREE ROAM connection (other upgrades, e.g. Vite's hot reload, are left alone) |

Test logins (`mock:Name` tokens) are only accepted by the **dev** server and only with the environment variable
`LIVE_MOCK=1`. Production never accepts them.

## Joining
1. The client opens the socket and sends `hi` with a **Firebase ID token**, its car color, its shop car (`car`) and its
   `LIVE_PROTOCOL`. Everyone sees that car (since v0.2.4): the relay accepts a known car the profile owns (`cars`),
   otherwise the ROADSTER; other games load its model on first sight and show the ROADSTER until then.
2. The server rejects an outdated protocol at once (`err old`).
3. It verifies the token's signature against **Google's public keys** (`jose`), issuer and audience = the Firebase
   project, then reads the player's **nickname** from the public `users/{uid}` document (Firestore REST, 8 s timeout).
   No nickname → `err nickname`. A nickname or uid on the [ban list](#moderation) → `err banned` (no retry).
4. **One car per account:** an older connection of the same uid gets `err again` and is closed.
5. Full (**24** players) → `err full` (the client retries).
6. The player gets the lowest free **spawn slot**, a `welcome` (id, name, online count, slot), a `join` for everyone
   already there; everyone else gets a `join` for the newcomer.

Timeouts: the server drops a socket that hasn't signed in within **15 s**; the client gives up waiting for `welcome`
after **12 s** and reconnects.

## Messages
JSON text frames. `t` is the type.

**Client → server**
| t | Fields | Notes |
|---|---|---|
| `hi` | `token, c, v` (+ `x, kc` for test logins) | Sign in. `c` color −1…15, `v` protocol version. Test logins (`mock:Name`) send their saved KILLER STATUS; real ones have it read from the profile |
| `s` | `d: [x, y, z, yaw, pitch, roll, speed, thrust, nitro, time, hp]` | Own state, ~15/s while flying, 2/s when paused or wrecked. `time` = sender's clock (s), `hp` 0–100 |
| `f` | `k, o: [x,y,z], d: [x,y,z], tg` | Fired: `bolt`, `missile`, `emp` or `spin` (`d[0]` = side). `tg` = homing target id |
| `hit` | `v, k, d` | Our bolt/missile hit player `v` on our screen |
| `die` | `by` | We were wrecked by player `by` |
| `chat` | `s` | Chat text. Starting with `/`: a [chat command](#chat-commands), never shown to anyone |
| `ping` | `c` | Latency check (client clock), every 2 s |

**Server → client**
| t | Fields | Notes |
|---|---|---|
| `welcome` | `id, name, n, slot, x, kc` | Joined; `n` = players online; `x` / `kc` = your [KILLER STATUS](#killer-status) and killer kill count |
| `join` / `leave` | `id, name, c, k, x, kc` / `id` | `k` = their kills this session, `x` = they are a Killer, `kc` = their Killer kill count (which aura) |
| `w` | `n, p: [[id, ...d], ...]` | Everyone's latest state, 15 times a second |
| `f` | `id, k, o, d, tg` | Someone fired |
| `hit` | `id, k, d` | To the victim only |
| `kill` | `k, v, n, kc` | Player `k` wrecked `v`; `n` = k's session kills, `kc` = k's killer kill count. To everyone |
| `killer` | `id, on, kc` | Player `id` became a Killer (`on`) or cleared it with `/pkclear`. To everyone |
| `cmd` | `c, ok, msg` | The answer to chat command `/c`. To the sender only |
| `chat` | `id, name, s` | To everyone, including the sender (as a receipt) |
| `pong` | `c` | The ping's `c`, back to the sender |
| `err` | `code` | `auth`, `nickname`, `full`, `again`, `old`; then the socket closes |

## Server limits and validation
| Rule | Value |
|---|---|
| Max players | 24 (`MAX_PLAYERS`) |
| Connections per address | Since v0.2.7: at most 6 open (`PER_IP`) and 20 new a minute (`PER_IP_MINUTE`) per player address (Cloudflare `CF-Connecting-IP`, trusted only on connections from the PC itself), and 64 still signing in from everyone together (`MAX_PENDING`). Over a limit the upgrade gets HTTP 429 and `server.log` says *refused a connection: ...* (once a minute per address; the address isn't logged). The game's reconnect back-off (1-15 s) stays far below. Connections made on the PC itself (dev server, tests) aren't limited per address |
| Message size | 600 bytes once joined, 4 KB for `hi`; bigger or binary → disconnect |
| Message rate | 45 per second per client; excess dropped (connection kept) |
| Fire events | 14 per second; kind must be known; vectors finite and within ±20,000 |
| Hits | 14 per second; kind `bolt` or `missile`; only within **5 s** after one of the shooter's own shots |
| Wrecks (`die`) | the killer must be another connected player; one per victim per **2 s** (`DIE_MS`) |
| State | 10 or 11 finite numbers, position within ±20,000; values clamped (pitch/roll ±1.6, speed −50…400, hp 0–100) and rounded |
| Chat | see [Chat](#chat) |
| Idle | no message for **30 s** → dropped. WebSocket ping every 15 s keeps Cloudflare from closing quiet sockets and removes dead ones |

## Movement
- **Send:** `LiveRoom.send` every 66 ms (15/s) while flying, 500 ms otherwise.
- **Receive:** each `w` snapshot is stored per player (last 20) with the sender's timestamp. A per-player **clock
  offset** is tracked from the least-delayed packets (eases down fast, up slowly; resets on a jump over 2 s).
- **Draw (`sample`):** the state at *now − offset − 150 ms* on the sender's clock, linearly interpolated between the
  two snapshots around it (yaw by the shortest angle). A jump over **150 m** (a respawn) snaps instead of flying across
  the map. If the newest snapshot is older than the render time, it is held.
- **Remote cars** come from a pool (`takeAvatar`): rival car objects repainted in the player's chosen color, created on
  demand.
- A remote car is **hidden while its hp is 0** or for 1.2 s after a `kill` names it (its own hp-0 state may still be
  in flight); shots pass through it meanwhile.

## Hits and kills
| Step | Where | Detail |
|---|---|---|
| Fire | shooter | Local projectile + `f` event; the others draw the shot (looks only) |
| Hit test | shooter | Shot vs. the remote car as drawn → `live.hit(victim, kind, direction)`, hit marker and sound |
| Hit check | server | Passed on to the victim only if plausible ([below](#hit-checks)) |
| Damage | victim | `roamHit`: ignored if not in `race` state, wrecked, spawn-protected (3 s) or dodging (*DODGED*). Otherwise damage and knock-back ([values](06-weapons-damage.md#free-roam-roamhit-applied-by-the-victims-own-game)) |
| EMP | victim | Not a `hit` message: each game checks the distance to the EMP's origin itself |
| Wreck | victim | hp ≤ 0 → explode, `die(by)`, down for **3 s** (`ROAM_WRECK`, *RESPAWN IN 3 · 2 · 1*), respawn at its own slot with 3 s protection |
| Kill | server | Only if a hit or EMP of the killer reached the victim in the last 30 s; credits `killer.kills++` (per session) and `killer.kc++` ([KILLER STATUS](#killer-status)), broadcasts `kill` |
| Rewards | killer | Takedown message and streak, +boost, +seeker, `account.addKills(1)` (saved to the profile) |
| Feed | others | *killer ✖ victim* in the kill feed; the roster re-sorts by kills |

**Why shooter-decided:** the victim's real position is ~150 ms + latency ahead of where the shooter sees it. When the
victim decided hits, shots that visibly hit often missed. Now hits land where the car is drawn. The trade-off: with
high latency a victim can occasionally be hit just after dodging behind cover on their own screen.

### Hit checks
`server/hits.js` (S3, v0.2.10). The relay keeps each player's positions for 3 s (from their `s` messages) and each
shot (`f`) until it can no longer hit. Generous on purpose: a fair hit must never be dropped by lag.

| Check | Rule |
|---|---|
| Shot origin (`f`, all but `spin`) | within 20 m + 0.3 × speed of one of the shooter's positions in the last 1.5 s; otherwise not passed on (an EMP too) |
| A shot per hit | an unused shot of the same kind, no older than its life + 1 s (bolt 1.5 s, seeker 3.6 s); each shot hits once |
| Reach | the victim, at one of its positions in the last 1.5 s, within (shooter speed + 750 bolt / 330 seeker) × min(life, flight time + 0.6 s) + 40 m |
| Line of fire | within 80 m, or within ~53° of the shot's direction (cos ≥ 0.6). A seeker homing on this victim: any direction |
| EMP reach | victims within `EMP_RADIUS 95 × EMP_CAR_MAX 1.25` (the TITAN's EMP) + 40 m of the blast |
| Wreck (`die`) | the killer's hit was passed on to this victim, or their EMP went off within 95 + 40 m of them, in the last 30 s |

Anything that fails is ignored and counted; `[s3] Name: N implausible ignored (origin 1, noshot 3, range 1, aim 1,
wreck by X 1) uid …` goes to `server.log` at most once a minute per player. Fair players should never appear there: if
they do, the margins above need widening. The weapon values are the game's; `tests/constants.test.mjs` checks that.

Still trusted (S4, until the Windows app's server): each player's own hp, whether an EMP hit them, and dodges.

## Health regeneration
After 5 s without a hit, `player.hp += 8 × dt` up to 100 (`ROAM_REGEN`). The current hp is sent in every state
message, so others see it on the name-tag health bar.

## Spawn slots
The hub is at (0, 175, 70) above the start deck: 24 slots, 4 across (14 m apart), rows 17 m apart going back,
every other row 5 m higher. The server gives each player the lowest free slot number. Locally, `roamSpawn` skips a slot
if another car is within 9 m of it. Before `welcome` arrives the player waits on a random back-row slot, then moves to
the assigned one unless they have already flown more than 30 m away.

## World limits
FREE ROAM only: altitude **28–1,600 m** and **9 km** from the center. The car is clamped at the limits and its velocity
into the limit removed. The race course (gate, rings, path, signs) and its colliders are removed (`course.setRoam`).

## Chat
| Rule | Value | Where |
|---|---|---|
| Length | 120 characters (Unicode code points) | client `maxLength`, server cut |
| Cleaning | control characters, zero-width and text-direction marks removed; whitespace collapsed; empty → dropped | server `CHAT_STRIP` |
| Rate | burst of **5**, then refills 1 per **3 s** (token bucket); excess silently dropped | server |
| Client pace | min 1.2 s between sends, otherwise *SLOW DOWN* | `chat.js` |
| Filter | blocked words → stars, for chat and nicknames; a line of only blocked words is dropped. See [Moderation](#moderation) | server |
| Logging | chat text is **not** logged (since v0.2.4, R12); only `/report` writes the reported player's last 8 lines (as written) to `server.log` | server |
| Display | `textContent` only (never HTML); names in the sender's car color; your own lines white; fades after 12 s; last 30 kept | `chat.js`, `roam.js` |

Opening the chat focuses a text input. Keys typed into it never reach the game (`input.js` ignores events from
inputs, and the chat stops propagation). On phones, the input is focused from the **CHAT** button's touchend, so the
keyboard opens.

### Chat commands
A chat line starting with `/` is a command: the server runs it, answers the sender alone with `cmd { c, ok, msg }`
and never broadcasts the line. Names are case-insensitive. Commands share the chat rate limit.

| Command | Effect |
|---|---|
| `/pkclear` | Clears your [KILLER STATUS](#killer-status): `isKiller` false, count 0. Broadcasts `killer { on: false }` if you were one |
| `/report NAME` | Logs `[report] Reporter reported Name (uid …): "line" \| …` with the target's last 8 lines as written (memory only, gone when they leave). Matches the shown or registered nickname. One per 30 s |
| `/help` | Lists the commands |
| anything else | `ok: false`, *UNKNOWN COMMAND /NAME · /help LISTS THEM* |

`/mute NAME`, `/unmute NAME` and `/muted` never reach the server: the game handles them (`src/online/mutes.js`,
localStorage `neon.muted`, up to 200 names) and hides chat from muted names (`roam.js`).

Without a connection (the relay unreachable), `/pkclear` typed in the FREE ROAM chat answers *NOT CONNECTED ·
/pkclear WORKS ONCE YOU ARE ONLINE AGAIN*: only the relay saves the status since v0.2.4. (The `?mock` test backend
still clears it in the game, `clearKiller` in `main.js`.)

### Moderation
`server/moderation.js` (S6, v0.2.9), passed to `liveRelay({ mod })`.

| File | Content |
|---|---|
| `server/blocked-words.txt` | Starter word list (in git) |
| `<dir>/blocked-words.txt` | The owner's extra words, one per line, `#` comments |
| `<dir>/bans.txt` | Banned nicknames or uids, one per line, any case, `#` comments |

`<dir>` is `neon_c/live` for a release (`LIVE_MOD_DIR` overrides it) and the game folder for the dev server. The files
are re-read when their modification time changes (checked every 10 s); a ban then removes a connected player
(`err banned`). Matching: letters lower-cased, look-alikes mapped (`0→o 1→i 3→e 4→a 5→s 7→t 8→b @→a $→s !→i |→i`),
letters repeated 3+ times squeezed to one; words of 4+ letters match inside a token, shorter ones only a whole token. A matched token
becomes stars of the same length.

## KILLER STATUS
3 confirmed FREE ROAM player kills (`KILLER_KILLS` in `src/online/protocol.js`) make a player a **Killer** until they
type `/pkclear`.

| Piece | Where | Detail |
|---|---|---|
| Count | relay (`p.kc`) | Read from the profile on joining (`isKiller`, `killerKillCount` via the public Firestore REST read; test logins send `x`, `kc` in `hi`). Junk values start from 0; a saved count ≥ 3 always means a Killer. `+1` per confirmed `die` (the [same checks](#server-limits-and-validation) as kills: another connected player, one per victim per 2 s). AI rivals never count |
| Becoming one | relay | The kill that reaches 3 broadcasts `killer { id, on: true, kc }` (once) |
| Saving | the relay | Since v0.2.4 (R7): a few seconds after a change (a kill, `/pkclear`), the relay saves `kills` (+n), `killerKillCount` and `isKiller` through the `relayKills` function, signed with `RELAY_SECRET` ([kills](08-accounts-data.md#free-roam-kills)). The game only shows it: `LiveRoom` calls `on.killer` on `welcome`, own `kill` and `killer` messages → `account.setKiller(isKiller, kc)` |
| Persistence | profile | `account.isKiller` reads the profile, so the status survives mode changes, car switches, reloads and reconnects |
| Aura | `killer.js` | The CRIMSON FLAME (3+ kills) or the VOID CROWN (10+, `VOID_KILLS`) on the player's car (`syncKiller` every frame, from the saved count) and on remote cars (`join.x` / `join.kc`, `kill.kc`, `killer` messages); see [Car](04-car.md#killer-auras-crimson-flame-void-crown) |
| R | `race.js` | While a Killer, R doesn't respawn: *RESPAWN UNAVAILABLE WHILE KILLER*. Wreck respawns still work |

## Ping
The client sends `ping { c: performance.now() }` every 2 s; on `pong`, round-trip time = now − c, smoothed
(30% per sample). Shown as **MS** (green < 80, yellow < 160, red), together with **FPS** (frames counted over 0.25 s
windows) and **ONLINE** (time since the first `welcome` of this FREE ROAM session).

## Reconnecting
On a dropped socket the client clears the other players and retries after 1, 2, 4, 8, 15, 15… seconds. It doesn't
retry after `nickname`, `again` or `old`. On `auth` it fetches a fresh token once and retries; a second `auth`
failure stops. Session kills reset on every new `welcome` (the server counts per connection).

## Protocol versions
`LIVE_PROTOCOL` in `src/online/protocol.js` is imported by **both** the client and the server.

| Version | Added |
|---|---|
| 1 | FREE ROAM (positions, shots, wrecks) |
| 2 | Version check; FREE ROAM kills saved to accounts |
| 3 | Spin dodge (`f` kind `spin`) |
| 4 | Chat, shooter-decided `hit`, `ping`/`pong` |
| 5 | KILLER STATUS (`welcome.x/kc`, `join.x`, `kill.kc`, `killer`), chat commands (`cmd`) |
| 6 | `join.kc`: the Killer kill count, for the VOID CROWN rank |
| 7 | The relay saves kills and KILLER STATUS (v0.2.4); shop cars: `hi.car`, `welcome.car`, `join.car` |
| 8 | Moderation (v0.2.9): `err banned`, filtered names and chat, `/report`, `/help` |
| 9 | PULSE blaster rework (v0.2.14): bolts do 21 damage, 750 m/s for 1.5 s (no message changes; old games would apply 6) |
| 10 | Faster cars (v0.2.15): 135 m/s, 196 on nitro (no message changes; old games fly 95 / 138) |
| 11 | Shop cars differ (v0.2.17, `CAR_STATS`); the `s` message's hp is a **percentage** of the car's own health |

A tab with a different version gets `err old`, shows *GAME UPDATED · RELOADING…* and reloads, but **at most once a
minute** (`sessionStorage['neon.reloadedAt']`). If the reload doesn't fix it (the server is the outdated side, e.g.
new files served before a restart), it shows *SERVER UPDATING · TRY AGAIN SOON* instead of looping.

**Rule:** bump `LIVE_PROTOCOL` whenever messages or FREE ROAM rules change in a way old and new games would disagree
on. Then rebuild **and** restart the server together.
