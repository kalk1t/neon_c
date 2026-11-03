# 4. Architecture

[← Index](TECHNICAL_DOCUMENTATION.md)

## 4.1 System context

```mermaid
flowchart LR
  subgraph Browser["Player's browser"]
    UI["index.html + style.css<br/>(menus, HUD, shop, chat)"]
    Game["src/main.js game loop + render, world, race,<br/>aim, camera, roam modules (three.js, physics, AI)"]
    Acc["src/account.js<br/>account state"]
    BE["src/online/firebase.js<br/>(or mock.js with ?mock in dev)"]
    LR["src/online/live.js<br/>LiveRoom (WebSocket)"]
    UI --- Game
    Game --> Acc --> BE
    Game --> LR
  end

  subgraph PC["Owner's Windows PC"]
    VP["serve.mjs :8080<br/>serves dist/; relay.mjs as a child process"]
    Relay["server/live.js<br/>relay at /live, /live/count"]
    VP --- Relay
  end

  CF["Cloudflare<br/>DNS + Tunnel (cloudflared)"]

  subgraph FB["Firebase project roadsters-race"]
    Auth["Firebase Auth<br/>(Google)"]
    FS[("Cloud Firestore<br/>nicknames, players, users,<br/>purchases, raceTickets")]
    Fn["Cloud Functions us-central1<br/>startRace, finishRace, buyCar"]
    GA["Google Analytics<br/>(consent-gated)"]
  end

  GF["Google Fonts"]
  JWKS["Google JWKS<br/>securetoken"]

  Browser -- "HTTPS / WSS roadstersrace.com" --> CF --> VP
  BE -- "ID token, popup/redirect<br/>(/__/auth proxied via Vite)" --> Auth
  BE -- "reads + limited writes<br/>(firestore.rules)" --> FS
  BE -- "httpsCallable" --> Fn
  Fn -- "Admin SDK transactions" --> FS
  BE --> GA
  Relay -- "verify ID token" --> JWKS
  Relay -- "REST read users/{uid}<br/>(public profile)" --> FS
  UI --> GF
```

[confirmed: `vite.config.js`, `server/live.js:65,94,270-287`, `src/online/firebase.js:31-41`, `functions/index.js:32,48,86`, `index.html`]

## 4.2 Responsibilities and trust boundaries

| Component | Runs on | Trusted for | Not trusted for |
|---|---|---|---|
| Browser game (`src/`) | player's device | rendering, physics, AI, input, race simulation | **anything** that reaches the server; the server must check it |
| `firestore.rules` | Firebase | the only check on direct client writes (rename, kill award, killer status, profile create, private info) | — |
| Cloud Functions | Firebase (Google) | **coins** (race rewards, purchases), best times, roads won, ticket timing | the race result itself: it is client-reported and only plausibility-checked |
| Relay `server/live.js` | owner's PC, in the Vite process | identity (verifies the Google ID token), rate limits, kill crediting (kills follow own shots, 2 s per victim), killer counting | hit detection: the shooter decides hits (protocol 4) |
| Cloudflare Tunnel | Cloudflare + `cloudflared` on the PC | TLS, public hostname | no WAF or rate-limit config is in the repo [unknown] |

Key design decisions [inferred from comments and changelog]:

1. **Serverless economy, PC-hosted realtime.** The coin economy moved server-side in v0.2 (milestone M8), with Cloud Functions plus a `raceTickets` timer. Realtime FREE ROAM stays on the PC because it is cheap and needs no paid infrastructure (`server/live.js:47`: "keeps the upload from this PC reasonable").
2. **The relay is its own process (since v0.2.4, R6).** In production `server/serve.mjs` serves the static files and starts `server/relay.mjs` (the relay from `server/live.js`) as a child process on `127.0.0.1:8091`, passing `/live` and `/live/count` to it; a crashed relay is restarted without touching the website. On the dev server `livePlugin` still attaches the relay to Vite (`vite.config.js`).
3. **Relay without the Admin SDK.** The relay verifies tokens with JWKS and reads public profiles through the Firestore REST API (`server/live.js` `identify`). **It saves kills and KILLER STATUS itself** (since v0.2.4, R7) through the `relayKills` function, signed with a shared secret (`RELAY_SECRET`); the browser can no longer write them (see [Security](07-security.md)).
4. **One map.** SKYLINE CITY only; R1/R3 maps are archived outside the repo (`maps.js` has one entry) [confirmed: `src/maps.js:8`].

## 4.3 Client internal architecture

```mermaid
flowchart TB
  main["main.js<br/>state machine: menu / race / roam / shop / pause / results"]
  subgraph World
    maps["maps.js"] --> city["city.js"] & env["env.js"] & sky["skyprops.js"] & ads["ads.js"]
    roads["roads.js"] --> track["track.js<br/>spline, Corridor grid"] --> course["course.js<br/>gates, dots, chevrons"]
    props["props.js<br/>rings"] 
    physics["physics.js<br/>grid of OBB/Cyl/Torus colliders"]
  end
  subgraph Actors
    car["car.js<br/>Car, CAR_MODELS"] --> killer["killer.js"] & nitro["nitro.js"]
    ai["ai.js<br/>rivals, WEAPON, COIN"]
    coins["coins.js"]
  end
  subgraph UI
    hud["hud.js"]; chat["chat.js"]; shop["shop.js"]; cockpit["cockpit.js"]; sysreq["sysreq.js"]
  end
  subgraph Input
    input["input.js<br/>keyboard, mouse, gamepad"]; touch["touch.js"]
  end
  audio["audio.js"]
  subgraph Online
    account["account.js"] --> backend["online/index.js → firebase.js | mock.js"]
    live["online/live.js LiveRoom"] --- proto["online/protocol.js"]
    analytics["online/analytics.js"]
  end
  main --> World & Actors & UI & Input & audio & Online
```

[confirmed: imports in `src/main.js`; file roles taken from module headers]

**Game loop** [confirmed: `src/main.js` `frame`; rules in `race.js`, weapons in `aim.js`, cameras in `camera.js`, FREE ROAM in `roam.js`]: `requestAnimationFrame` → read input → step the player and rivals (`stepRacer`) or the FREE ROAM car → physics against the collider grid → weapons → HUD → render. Graphics quality is `GFX.high` or `GFX.low` (`src/render.js` `GFX`), with automatic switching to low if the frame rate stays under 40 fps after 3 s (`src/render.js` `measureGfx`). Details: `docs/technical/02-game-loop.md`.

## 4.4 Online state ownership

| State | Owner (source of truth) | Copies |
|---|---|---|
| Coins (`totalCoins`), best times, roads won, owned cars | Firestore `users/{uid}`, written **only** by Cloud Functions | `account.profile` (live `onSnapshot`) |
| Nickname | `users/{uid}.nickname` + `nicknames/{lower}` (claimed in a client transaction under rules) | profile |
| Lifetime kills, KILLER STATUS | `users/{uid}.kills` / `isKiller` / `killerKillCount`, written **only** by the relay through `relayKills` (v0.2.4) | profile; `account.pendingKills` shows kills not saved yet |
| Killer status | relay counts (`p.kc`), **client saves** `isKiller` / `killerKillCount` | profile |
| FREE ROAM positions, health, shots | client-authoritative, relayed at 15/s | remote avatars interpolated with 0.15 s delay |
| Race in progress | client only; server keeps only the ticket start time | — |
| Settings (gfx, sens, volume, …) | `localStorage` per device | — |
