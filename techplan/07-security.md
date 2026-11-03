# 10. Security

[← Index](TECHNICAL_DOCUMENTATION.md)

No secrets are reproduced in this documentation. `.env.local` was not opened. The Firebase web config values are public by design once shipped in the bundle; they're kept out of git anyway.

## 10.1 Threat model summary

The browser is untrusted. Everything a client sends (race results, kills, killer status, positions, hits) can be forged with devtools or a modified client. The server side (rules, functions, relay) decides how much damage a forged value can do.

| Asset | Protection | Residual risk |
|---|---|---|
| Coins / wallet | Only Cloud Functions write `coins`, `totalCoins` (rules have no client branch for them) [confirmed: `firestore.rules:91`]; `finishRace` caps 200 per race, needs a server-issued ticket at least 60 s old, at most one save per 60 s (`RACE_GAP_S`, 120 s before v0.2.1) [confirmed: `functions/index.js:48-83`] | **Medium.** The coin count is client-claimed: a scripted client can earn 200 coins about once a minute (~12,000/h) without playing. The TITAN (7,500) is reachable in under an hour of automation. Cosmetic only |
| Best times / ranking | Lower bounds (`MIN_LAP_S` 30, `MIN_RACE_S` 60), must fit inside server-measured elapsed time | **Medium.** Any time between 60 s and the real elapsed time can be claimed: a forged 60.0 s time tops every road ranking |
| Road unlocks | `roadsWon` set only by `finishRace` on `place === 1` | Low/medium: `place` is client-claimed |
| Purchases | Server price list, transaction, `already-exists`, `funds` checks, audit log in `purchases` | Low |
| Lifetime kills, KILLER STATUS | **Fixed in v0.2.4 (R7):** the browser can't write them (rules); the relay saves them through `relayKills` with a shared secret [confirmed: `firestore.rules`, `functions/index.js`] | Low: a victim's game still reports its own wreck, so two accounts can feed each other kills (one per victim every 2 s) |
| KILLER STATUS | Rules: count rises ≤ 10 per write, `isKiller` must follow the count (≥ 3) or be a full clear [confirmed: `firestore.rules:112-121`] | Low impact (cosmetic + self-imposed R lock); any client can make itself a Killer in one write, and reach the VOID CROWN (10, `VOID_KILLS`) in a second |
| Relay kill crediting | `die` comes from the **victim** naming the killer; once per victim per 2 s | Two colluding accounts (or two tabs on two accounts) can farm 30 kills/min for one of them [inferred from `server/live.js:171-185`] |
| Hits / damage | The shooter decides hits (protocol 4); relay only checks the shooter fired within 5 s and the rate | A modified client can hit anyone it fires near; the victim's client applies damage |
| Identity in FREE ROAM | Firebase ID token verified against Google JWKS with project issuer/audience; nickname from Firestore; one connection per uid [confirmed: `server/live.js:81-98,218-221`] | Low |
| Mock logins | Only with `LIVE_MOCK=1` in `vite dev`; `preview` forces `allowMock: false` [confirmed: `server/live.js:296-297`] | Low (production uses `preview`) |
| Private data (`players`) | Owner-only read; email must equal token email [confirmed: `firestore.rules:38-63`] | Low; stored PII: email, Google name, device, timezone, referrer |
| Relay availability (DoS) | 24-player cap, message size/rate limits, 15 s sign-in timer, 30 s idle drop, 15 s ping; since v0.2.7 at most 6 open and 20 new connections a minute per address (`CF-Connecting-IP`) and 64 still signing in (HTTP 429 before the WebSocket opens) | **Low/medium.** A spread-out attack from many addresses can still fill the 64 sign-in places for a while; Cloudflare-side protection [unknown]. The relay is its own process since v0.2.4 (R6): a relay crash no longer takes the site down |
| Functions abuse / cost | `maxInstances` 10; auth required; **App Check not enforced** (`functions/.env` absent) [confirmed in repo] | Low/medium cost exposure; bots with real Google accounts can call the functions |

## 10.2 Client-side findings

| # | Finding | Evidence | Severity | Notes |
|---|---|---|---|---|
| C1 | ~~`window.NEON` debug handle is exposed in production~~ **Fixed in v0.2.4 (D9):** only dev builds define it (`import.meta.env.DEV`) | `src/main.js` | — | — |
| C2 | ~~`hud.sub()` sets `innerHTML` with remote names~~ **Fixed in v0.2.7:** `sub()` writes plain text (`textContent`); the kill feed, chat and roster already did. The results lists escape names (`esc`) | `src/hud.js` | — | — |
| C3 | ~~No Content-Security-Policy~~ **Fixed in v0.2.7, report-only first:** `server/serve.mjs` sends a CSP on every page (scripts only from the site and the Google services Firebase loads; no inline or eval'd scripts; `object-src 'none'`; `frame-ancestors 'self'`); violations go to `/csp-report`, logged as `[csp]` lines in `server.log`. A real build loaded through it had no violations (2026-11-03: reCAPTCHA, Analytics, Firebase, Google Fonts). Next: `CSP_ENFORCE=1` once the log stays clean for signed-in players | `server/serve.mjs` | Low | Enforce after a few days of clean reports |
| C4 | Race results, `place`, coins are client-claimed | `src/account.js:401` | Medium | By design; see 10.1 |
| C5 | The Firebase auth helper is proxied through the site (`/__` → `roadsters-race.firebaseapp.com`) | `vite.config.js` | Info | Required for first-party sign-in; trusts that host |

## 10.3 Operational security

- The PC runs the web server as the owner's user (S4U, limited rights) and the tunnel as SYSTEM [confirmed: `server/install-autostart.ps1:12`, `server/install-tunnel.ps1:15`].
- Since v0.2.4 (R5) production runs `server/serve.mjs`, a small Node server: only `dist/` (paths can't leave it; tested), the `/__` proxy to `roadsters-race.firebaseapp.com`, and `/live` passed to the relay process; unknown `Host` headers get 403.
- `neon_c/live/server.log` collects server and relay logs with nicknames (joins, wrecks, sign-in failures); since v0.2.4 **no chat text**, and the launcher rotates it at 5 MB keeping 3 old files (R12) [confirmed: `server/live.js`, `server/launcher.mjs`].
- `RELAY_SECRET` (v0.2.4) sits in `.env.local` (and each release's copy) on the PC and in Secret Manager. A leak lets someone write kill counts and KILLER STATUS, nothing else; change it with `functions:secrets:set` + `.env.local` + release.
- Each release folder holds a copy of `.env.local` (Firebase web config, public by design) [confirmed: `server/release.mjs`].
- The tunnel token is passed as a parameter to `install-tunnel.ps1` and stored in the scheduled task [inferred from `param([string]$Token)`]. Anyone with admin on the PC can read it.
- `.firebaserc`, `.env.local` and the logs are git-ignored [confirmed: `.gitignore`].

## 10.4 Recommendations

1. Turn on App Check (reCAPTCHA Enterprise key → `VITE_APPCHECK_SITE_KEY`, release, watch metrics, then `ENFORCE_APP_CHECK=true` in a committed `functions/.env` + Firestore enforcement). In progress: code ready 2026-11-02. Note: the relay's unauthenticated Firestore REST read of `users/{uid}` needs the player's App Check token once Firestore is enforced; the client now sends it (`hi.ac`) and the relay forwards it.
2. ~~Move kill and killer saving server-side~~ Done in v0.2.4 (R7): the relay saves through `relayKills` (shared secret); `isKillAward` and `isKillerUpdate` are gone from the rules.
3. Credit kills only when the named killer recently `hit` the victim (the relay already sees `hit` messages).
4. ~~Strip `window.NEON` from production builds~~ Done in v0.2.4 (D9).
5. Use `textContent` in `hud.sub` call sites that include names, or escape them.
6. ~~Add a CSP~~ Done in v0.2.7 (report-only, then enforce with `CSP_ENFORCE=1`).
7. ~~Rotate `server.log` and avoid logging chat text.~~ Done in v0.2.4 (R12).
