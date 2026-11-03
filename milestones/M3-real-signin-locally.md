# M3 · Real Google sign-in on your PC

**Who:** You + Claude · **Time:** about 15 min · **Needs:** M2 · **Next:** [M4](M4-domain-to-cloudflare.md)

## Goal
The real online features work on `localhost` before the public domain is involved.

## Steps

### 1. Config (Claude)
- [x] Claude creates `game/.env.local` with the 4 values from M2, and `game/.firebaserc` with your project ID.

### 2. Firebase command-line tool (You)
In a terminal in `C:\Users\User\Projects\neon_c\game`:
- [x] firebase-tools is already in the project (run it as `npx firebase …` from the `game` folder)
- [x] `npx firebase login` → a browser opens → choose your Google account → Allow
- [x] `npx firebase projects:list` shows `roadsters-race`

### 3. Upload the security rules (You or Claude)
- [x] `npx firebase deploy --only firestore` → ends with `Deploy complete!`
- [x] The Firebase console → Firestore → **Rules** tab shows the Roadsters Race rules.
- [x] The **Indexes** tab shows the index as *Enabled*. It can take a few minutes to build.
      → 30 Oct: logged in, rules deployed, and the game reads the real database ("NO PILOTS YET — BE THE FIRST").

### 4. Play it (You)
- [x] `npm run dev` → open http://localhost:5173 (the address must be **localhost**, not 127.0.0.1)
- [x] **SIGN IN WITH GOOGLE** → choose an account → the nickname box appears → save a nickname
- [x] Race → finish → the results show `SAVED +N · WALLET X · WORLD RANK #1`
- [x] Open **World Ranking** → you are listed

### 5. Verify in the console (You)
- [x] Firestore → Data: `users/<your-id>` has your nickname, coins, totalCoins and races = 1
- [x] `nicknames/<lowercase nickname>` exists
- [x] Sign out → sign back in: same nickname and wallet, and no nickname box this time

## ✅ Done when (completed 30 Oct: first pilot `kalk1t`, 64 coins; analytics works)
- A real race on localhost saves coins to Firestore and shows in the World Ranking.

## If something goes wrong
- **"ONLINE NOT CONFIGURED"** → `.env.local` is missing or wrong. Restart `npm run dev` after changing it.
- **auth/unauthorized-domain** → use `localhost`, not an IP address.
- **Missing or insufficient permissions** → the rules weren't uploaded; repeat step 3.
- **"The query requires an index"** → wait for the index to finish building (step 3), then reload.
