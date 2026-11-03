# M7 · Running the live game

**Who:** You + Claude · **Time:** ongoing · **Needs:** M6 · **Next:** [M8](M8-before-the-shop.md)

## Goal
Keep the site up, ship updates safely, and know when you're outgrowing the free setup.

## Updating the game
1. Claude changes the code, and you try it with `npm run dev` on localhost.
2. When happy: commit and push, then `npm run release` in the game folder (no admin). It builds the commit
   separately, tests it, switches the site to it and checks it's live. Downtime is about 5 s.
   `npm run release -- --rollback` undoes it. (Since 2026-11-02; details in `game/docs/technical/10-operations.md`.)
3. If the Cloud Functions or security rules changed too: `npm run deploy -- functions client rules` (name only the
   parts that changed) instead of step 2. It runs the tests and deploys in the safe order.
4. Players get the new version when they reload. Cached files are safe because every build has new file names.

## Where to see your players and traffic
| What | Where |
|---|---|
| **Who is online right now** | Google Analytics → Reports → **Realtime** (players who allowed analytics) |
| Visitors per day, countries, referring sites (everyone) | Cloudflare → Analytics & Logs → **Web Analytics** |
| Races started and finished, sign-ups, coins earned | Google Analytics → Reports → Engagement → **Events** |
| Each player's email, visits, device, first and last visit | Firebase → Firestore → **players** collection |
| Nicknames, coins, races, best lap | Firebase → Firestore → **users** collection |
| Number of sign-ups | Firebase → **Authentication → Users** |
| Messages from players | your Gmail (forwarded from help@roadstersrace.com) |

## Weekly check (5 min)
- [ ] Firebase console → **Usage and billing**. The free daily limits are 50,000 database reads, 20,000 writes and
      50,000 sign-ups a month. One player is about 60 reads per visit (the ranking costs the most).
- [ ] Cloudflare → Analytics: visitors per day
- [ ] Answer emails to help@roadstersrace.com. Account deletion requests: delete `users/<id>`, `players/<id>`,
      `nicknames/<name>` and the user in Authentication, within 30 days.
- [ ] Firestore → `users`: look for offensive nicknames. Delete the `users/<id>` and `nicknames/<name>` docs to
      reset a nickname.
- [ ] Windows Update hasn't left the PC waiting on a restart for days

## Warning signs and what to do
| Sign | Action |
|---|---|
| Reads near 50k/day | Claude caches the ranking for 60 s, or moves to the Blaze plan (cents per day) |
| Site slow at peak times | Move the static files to Firebase Hosting or Cloudflare Pages, keep the domain (needs set-up first: `firebase.json` has no `hosting` section since 2026-11-02, and the relay needs a home) |
| Someone at 200 coins every 2 min for hours | Probably a script. Reset their doc and do [M8](M8-before-the-shop.md) sooner |
| PC must be off (travel, repairs) | Same as above |

## Backups
- On the free plan: Claude can add `npm run backup`, which saves all users and nicknames to a dated JSON file on
  your PC. Run it weekly.
