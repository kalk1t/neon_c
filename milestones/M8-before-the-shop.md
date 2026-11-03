# M8 · Secure the coins before the shop

**Who:** Claude (you upgrade the plan) · **Time:** 1–2 sessions · **Needs:** M7 · **Next:** building the shop

## Why
Right now the player's browser saves coins and the security rules limit it (at most 200 per race, one race every
2 minutes). That's fine for a ranking, but once coins **buy things**, someone with a script can still add about
6,000 coins an hour. The server must decide coin awards and purchases, not the browser.

## Steps

### 1. Blaze plan (You)
- [ ] Firebase console → **Upgrade → Blaze (pay as you go)**. The free amounts still apply, and a small game
      usually costs $0–1 a month.
- [ ] Google Cloud → Billing → **Budgets & alerts** → create a $5/month budget with email alerts.

### 2. Server-side race results (Claude)
- [ ] A Cloud Function `startRace` gives a signed race ticket (time, track, a random seed).
- [ ] A Cloud Function `finishRace` checks the ticket, a realistic race time for the lap count, a coin count no
      higher than the coins on the track, and one finish per ticket. Then it awards the coins.
- [ ] **App Check** (reCAPTCHA Enterprise) so only the real website can call the functions.

### 3. Lock the wallet (Claude)
- [ ] Change `firestore.rules` so players can **only rename**. `coins`, `totalCoins`, `races` and `bestLap` become
      server-only.
- [ ] Rules tests updated: a direct coin write from the browser now fails.

### 4. Shop foundations (Claude)
- [ ] A `shop/{itemId}` collection (price, type: car skin / trail / nickname colour) that only admins can edit.
- [ ] A Cloud Function `buyItem` that checks the price, subtracts it from `coins` (never `totalCoins`, so the ranking
      doesn't drop) and adds the item to `users/{uid}/inventory`, all in one transaction.
- [ ] A purchase log in `users/{uid}/purchases` for support questions.

## ✅ Done when
- Writing coins directly from the browser console fails, races still award coins through the function, and a test
  purchase subtracts coins correctly.
