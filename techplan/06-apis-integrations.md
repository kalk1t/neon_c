# 9. APIs and integrations

[← Index](TECHNICAL_DOCUMENTATION.md)

## 9.1 Callable Cloud Functions

HTTPS callables (`firebase-functions/v2/https` `onCall`), region us-central1, invoked with `httpsCallable(fns, name, {timeout: 15000})` (`src/online/firebase.js:41`). All require Firebase Auth (`uidOf(req)` throws `unauthenticated` otherwise). Besides them (v0.2.4): `relayKills`, a plain HTTPS POST (`onRequest`) only the live relay may call, with `Authorization: Bearer <RELAY_SECRET>` (403 otherwise, 400 on any bad entry): `{ players: [{ uid, kills 0-60, kc 0-1e6, killer }] }` (≤ 50) → `{ saved, missing }`; and `cleanRaceTickets`, a daily schedule (04:00 UTC). `finishRace` returns `{ gain, coins, totalCoins, capped, newTime }` since v0.2.4. Request and response shapes are in [04-components.md §7.6](04-components.md#76-cloud-functions-functionsindexjs).

## 9.2 FREE ROAM WebSocket protocol (version 7)

Endpoint: `wss://roadstersrace.com/live` (same origin; `server/serve.mjs` passes it to the relay process). Text JSON frames only; binary frames close the connection. Max frame 4,096 bytes before joining, 600 after [confirmed: `server/live.js:49-50,141`].

**Client → server**

| `t` | Fields | Limits / validation |
|---|---|---|
| `hi` | `token`, `c` (−1..15), `car` (a `CAR_IDS` id; one the profile doesn't own shows as the ROADSTER), `v` (7); test logins also `x`, `kc` | Once; 15 s to complete; `v` must equal `LIVE_PROTOCOL` |
| `s` | `d`: 10 or 11 finite numbers | Position within ±20,000; clamps on the rest |
| `f` | `k` ∈ bolt / missile / spin / flip (flip since v0.2.22; emp dropped since v0.2.21), `o`, `d` (vec3), `tg` | ≤ 14 / s |
| `hit` | `v` (victim id), `k` ∈ bolt / missile, `d` | ≤ 14 / s; within 5 s of the sender's own shot |
| `die` | `by` (killer id) | Once per 2 s per victim; killer must be connected and not the sender |
| `chat` | `s` | Control and bidi characters stripped, whitespace collapsed, ≤ 120 chars; token bucket 5, +1 per 3 s; `/…` = command |
| `ping` | `c` (client clock) | |

All messages: ≤ 45 per second per client; the excess is dropped silently [confirmed: `server/live.js:146-148`].

**Server → client**

| `t` | Fields | To |
|---|---|---|
| `welcome` | `id, name, n, slot, x, kc, car` | joiner |
| `join` / `leave` | `id, name, c, car, k, x, kc` / `id` | everyone |
| `w` | `n`, `p: [[id, …state]]` | everyone, 15/s |
| `f`, `hit` | as relayed | others / victim |
| `kill` | `k, v, n, kc` | everyone |
| `killer` | `id, on, kc` | everyone |
| `chat` | `id, name, s` | everyone |
| `cmd` | `c, ok, msg` | sender |
| `pong` | `c` | sender |
| `err` | `code`: `old`, `auth`, `nickname`, `full`, `again` | then closed |

Full table with history: `docs/technical/07-online.md`.

**HTTP:** `GET /live/count` → the online player count (used by the menu every 10 s) [confirmed: `server/live.js:287`, `src/online/live.js:20`]. The exact response format was not re-read [inferred: a small JSON or text body].

## 9.3 External services

| Service | Used for | How | Failure behaviour |
|---|---|---|---|
| Firebase Auth (Google provider) | sign-in | popup, redirect fallback; `/__/*` proxied through Vite to `roadsters-race.firebaseapp.com` | sign-in error message; game playable signed out |
| Cloud Firestore | profiles, nicknames, ranking | Web SDK (client), Admin SDK (functions), REST v1 (relay, unauthenticated public read) | 15 s timeouts; races kept pending with TRY AGAIN |
| Cloud Functions | economy | callables | as above |
| Google JWKS (`securetoken@system.gserviceaccount.com`) | relay token check | `jose.createRemoteJWKSet` (cached by jose) | sign-in to FREE ROAM fails (`err auth`) |
| Google Analytics (Firebase Analytics) | usage stats | only after consent (`rr.analytics = yes`); `identify(uid)` sets the user id | silently skipped |
| Firebase App Check (reCAPTCHA Enterprise) | bot protection | **on since v0.2.2** (2026-11-02): tokens issued (TTL 1 h), relay forwards them; **not enforced yet** (R9) | — |
| Google Fonts | Orbitron, Rajdhani | `<link>` in `index.html` | fallback fonts |
| Cloudflare (DNS, TLS, Tunnel) | public hosting | `cloudflared` scheduled task, token-based tunnel | site down if the PC or tunnel is down |

The tunnel's routing (hostname → `localhost:8080`) is configured in the Cloudflare dashboard, not in the repo [unknown in detail].
